#include <cstring>
#include <functional>
#include <list>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Shader.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Resource/Id.h"
#include "Spark/BitmapImage.h"
#include "Spark/BitmapResource.h"
#include "Spark/Font.h"
#include "Spark/Frame.h"
#include "Spark/Movie.h"
#include "Spark/MovieFactory.h"
#include "Spark/Optimizer.h"
#include "Spark/Packer.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SwfReader.h"
#include "Spark/Editor/EmptyMovieAsset.h"
#include "Spark/Editor/MovieAsset.h"
#include "Spark/Editor/Pipeline.h"
#include "Svg/IShapeVisitor.h"
#include "Svg/Parser.h"
#include "Svg/PathShape.h"
#include "Xml/Document.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const Guid c_idFlashShaderAssets(L"{14D6A2DB-796D-E54D-9D70-73DE4AE7C4E8}");

struct AtlasBitmap
{
	uint16_t id;
	Ref< const BitmapImage > bitmap;
	Packer::Rectangle packedRect;
};

struct AtlasBucket
{
	Ref< Packer > packer;
	std::list< AtlasBitmap > bitmaps;
};

class ShapeVisitor : public svg::IShapeVisitor
{
public:
	ShapeVisitor(
		const std::function< void(svg::Shape*) >& enter,
		const std::function< void(svg::Shape*) >& leave
	)
	:	m_enter(enter)
	,	m_leave(leave)
	{
	}

	virtual void enter(svg::Shape* shape) override final
	{
		m_enter(shape);
	}

	virtual void leave(svg::Shape* shape) override final
	{
		m_leave(shape);
	}

private:
	std::function< void(svg::Shape*) > m_enter;
	std::function< void(svg::Shape*) > m_leave;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.Pipeline", 0, Pipeline, editor::IPipeline)

Pipeline::Pipeline()
:	m_generateMips(false)
,	m_sharpenStrength(0.0f)
,	m_useTextureCompression(true)
,	m_textureSizeDenom(1)
,	m_textureAtlasSize(1024)
{
}

bool Pipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_generateMips = settings->getProperty< bool >(L"Pipeline.GenerateMips", false);
	m_sharpenStrength = settings->getProperty< bool >(L"Pipeline.SharpenStrength", false);
	m_useTextureCompression = settings->getProperty< bool >(L"Pipeline.UseTextureCompression", true);
	m_textureSizeDenom = settings->getProperty< int32_t >(L"Pipeline.TextureSizeDenom", 1);
	m_textureAtlasSize = settings->getProperty< int32_t >(L"Pipeline.TextureAtlasSize", 1024);
	return true;
}

void Pipeline::destroy()
{
}

TypeInfoSet Pipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< EmptyMovieAsset >();
	typeSet.insert< MovieAsset >();
	return typeSet;
}

bool Pipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const MovieAsset* movieAsset = dynamic_type_cast< const MovieAsset* >(sourceAsset))
		pipelineDepends->addDependency(traktor::Path(m_assetPath), movieAsset->getFileName().getOriginal());
	pipelineDepends->addDependency(c_idFlashShaderAssets, editor::PdfBuild | editor::PdfResource);	// Solid
	return true;
}

bool Pipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< Movie > movie;
	bool optimize = false;

	if (const MovieAsset* movieAsset = dynamic_type_cast< const MovieAsset* >(sourceAsset))
	{
		Ref< IStream > sourceStream = pipelineBuilder->openFile(traktor::Path(m_assetPath), movieAsset->getFileName().getOriginal());
		if (!sourceStream)
		{
			log::error << L"Failed to import Spark movie; unable to open file \"" << movieAsset->getFileName().getOriginal() << L"\"" << Endl;
			return false;
		}

		std::wstring extension = toLower(movieAsset->getFileName().getExtension());
		if (extension == L"swf")
		{
			Ref< SwfReader > swf = new SwfReader(sourceStream);
			movie = MovieFactory(movieAsset->m_includeAS).createMovie(swf);
			if (!movie)
			{
				log::error << L"Failed to import Spark movie; unable to parse SWF." << Endl;
				return false;
			}
		}
		else if (extension == L"svg")
		{
			xml::Document xd;
			if (!xd.loadFromStream(sourceStream))
			{
				log::error << L"Failed to import Spark movie; unable to read SVG." << Endl;
				return false;
			}

			Ref< svg::Shape > shape = svg::Parser().parse(&xd);
			if (!shape)
			{
				log::error << L"Failed to import Spark movie; unable to parse SVG." << Endl;
				return false;
			}

			ShapeVisitor visitor(
				[&](svg::Shape* svg) {

					if (const auto ps = dynamic_type_cast< svg::PathShape* >(svg))
					{
						spark::Path path;

						const auto& p = ps->getPath();
						for (const auto& sp : p.getSubPaths())
						{
							switch (sp.type)
							{
							case svg::SptLinear:
								path.moveTo(0, 0, Path::CmAbsolute);
								path.lineTo(0, 0, Path::CmAbsolute);
								break;

							case svg::SptQuadric:
								break;

							case svg::SptCubic:
								// Since Spark doesn't support cubic paths we need to approximate with quadratic.
								break;

							default:
								break;
							}
						}
					}
					// Ref< Shape > shape = new Shape();
					// shape->create(1, image->getWidth() * 20, image->getHeight() * 20);
				},
				[&](svg::Shape*) {
				}
			);
			shape->visit(&visitor);
		}
		else
		{
			Ref< drawing::Image > image = drawing::Image::load(sourceStream, movieAsset->getFileName().getExtension());
			if (image)
			{
				// Create a single frame and place shape.
				Ref< Frame > frame = new Frame();

				Frame::PlaceObject p;
				p.hasFlags = Frame::PfHasCharacterId;
				p.depth = 1;
				p.characterId = 1;
				frame->placeObject(p);

				// Create sprite and add frame.
				Ref< Sprite > sprite = new Sprite();
				sprite->addFrame(frame);

				// Create quad shape and fill with bitmap.
				Ref< Shape > shape = new Shape();
				shape->create(1, image->getWidth() * 20, image->getHeight() * 20);

				// Setup dictionary.
				movie = new Movie(Aabb2(Vector2(0.0f, 0.0f), Vector2(image->getWidth() * 20.0f, image->getHeight() * 20.0f)), sprite);
				movie->defineBitmap(1, new BitmapImage(image));
				movie->defineCharacter(1, shape);
			}
		}

		safeClose(sourceStream);

		optimize = movieAsset->m_staticMovie;
	}
	else if (const EmptyMovieAsset* emptyMovieAsset = dynamic_type_cast< const EmptyMovieAsset* >(sourceAsset))
	{
		const Color4ub& bc = emptyMovieAsset->getBackgroundColor();

		Ref< Sprite > sprite = new Sprite(0, emptyMovieAsset->getFrameRate());

		Ref< Frame > frame = new Frame();
		frame->changeBackgroundColor(Color4f(bc.r, bc.g, bc.b, bc.a) / Scalar(255.0f));
		sprite->addFrame(frame);

		movie = new Movie(
			Aabb2(
				Vector2(0.0f, 0.0f),
				Vector2(emptyMovieAsset->getStageWidth() * 20.0f, emptyMovieAsset->getStageHeight() * 20.0f)
			),
			sprite
		);
	}

	// Show some information about the Flash.
	log::info << L"Spark movie successfully loaded," << Endl;
	log::info << IncreaseIndent;
	log::info << movie->getFonts().size() << L" font(s)" << Endl;
	log::info << movie->getBitmaps().size() << L" bitmap(s)" << Endl;
	log::info << movie->getSounds().size() << L" sound(s)" << Endl;
	log::info << movie->getCharacters().size() << L" character(s)" << Endl;
	log::info << DecreaseIndent;

	// Merge all characters of first frame into a single sprite.
	if (optimize)
	{
		movie = Optimizer().merge(movie);
		if (!movie)
		{
			log::error << L"Failed to import Spark movie; failed to optimize static movie." << Endl;
			return false;
		}
	}

	// Generate triangles of every shape in movie.
	Optimizer().triangulate(movie, false);

	// Replace all bitmaps with resource references to textures.
	SmallMap< uint16_t, Ref< Bitmap > > bitmaps = movie->getBitmaps();

	// Create atlas buckets of small bitmaps.
	std::list< AtlasBucket > buckets;
	std::list< AtlasBitmap > standalone;
	Packer::Rectangle r;

	for (SmallMap< uint16_t, Ref< Bitmap > >::const_iterator i = bitmaps.begin(); i != bitmaps.end(); ++i)
	{
		const BitmapImage* bitmapData = dynamic_type_cast< const BitmapImage* >(i->second);
		if (!bitmapData)
		{
			log::warning << L"Skipped bitmap as it not a static bitmap (" << type_name(i->second) << L")" << Endl;
			continue;
		}

		bool foundBucket = false;

		for (std::list< AtlasBucket >::iterator j = buckets.begin(); j != buckets.end(); ++j)
		{
			if (j->packer->insert(bitmapData->getWidth() + 2, bitmapData->getHeight() + 2, r))
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect = r;
				ab.packedRect.x += 1;
				ab.packedRect.y += 1;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				j->bitmaps.push_back(ab);
				foundBucket = true;
				break;
			}
		}

		if (!foundBucket)
		{
			buckets.push_back(AtlasBucket());

			AtlasBucket& b = buckets.back();
			b.packer = new Packer(m_textureAtlasSize, m_textureAtlasSize);

			if (b.packer->insert(bitmapData->getWidth() + 2, bitmapData->getHeight() + 2, r))
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect = r;
				ab.packedRect.x += 1;
				ab.packedRect.y += 1;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				b.bitmaps.push_back(ab);
			}
			else
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect.x = 0;
				ab.packedRect.y = 0;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				standalone.push_back(ab);
			}
		}
	}

	log::info << L"Packed bitmaps into " << uint32_t(buckets.size()) << L" atlas(es)." << Endl;

	uint32_t count = 1;

	for (std::list< AtlasBucket >::const_iterator i = buckets.begin(); i != buckets.end(); ++i)
	{
		log::info << L"Atlas " << count << L", containing " << uint32_t(i->bitmaps.size()) << L" bitmaps." << Endl;

		if (i->bitmaps.size() > 1)
		{
			Ref< drawing::Image > atlasImage = new drawing::Image(
				drawing::PixelFormat::getA8B8G8R8(),
				m_textureAtlasSize,
				m_textureAtlasSize
			);

			atlasImage->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

			for (std::list< AtlasBitmap >::const_iterator j = i->bitmaps.begin(); j != i->bitmaps.end(); ++j)
			{
				Ref< drawing::Image > bitmapImage = new drawing::Image(
					drawing::PixelFormat::getA8B8G8R8(),
					j->bitmap->getWidth(),
					j->bitmap->getHeight()
				);

				std::memcpy(
					bitmapImage->getData(),
					j->bitmap->getBits(),
					j->bitmap->getWidth() * j->bitmap->getHeight() * 4
				);

				for (int32_t y = -1; y < j->packedRect.height + 1; ++y)
				{
					for (int32_t x = -1; x < j->packedRect.width + 1; ++x)
					{
						int32_t sx = x;
						int32_t sy = y;

						if (sx < 0)
							sx = j->packedRect.width - 1;
						else if (sx > j->packedRect.width - 1)
							sx = 0;

						if (sy < 0)
							sy = j->packedRect.height - 1;
						else if (sy > j->packedRect.height - 1)
							sy = 0;

						Color4f tmp;
						bitmapImage->getPixel(sx, sy, tmp);

						atlasImage->setPixel(j->packedRect.x + x, j->packedRect.y + y, tmp);
					}
				}
			}

#if defined(_DEBUG)
			atlasImage->save(L"SparkBitmapAtlas" + toString(count) + L".png");
#endif

			Guid bitmapOutputGuid = outputGuid.permutation(count++);

			Ref< render::TextureOutput > output = new render::TextureOutput();
			output->m_textureFormat = render::TfInvalid;
			output->m_generateNormalMap = false;
			output->m_scaleDepth = 0.0f;
			output->m_generateMips = m_generateMips;
			output->m_keepZeroAlpha = false;
			output->m_textureType = render::Tt2D;
			output->m_hasAlpha = false;
			output->m_ignoreAlpha = false;
			output->m_scaleImage = false;
			output->m_scaleWidth = 0;
			output->m_scaleHeight = 0;
			output->m_enableCompression = m_useTextureCompression;
			output->m_enableNormalMapCompression = false;
			output->m_inverseNormalMapY = false;
			output->m_linearGamma = true;
			output->m_generateSphereMap = false;
			output->m_preserveAlphaCoverage = false;
			output->m_alphaCoverageReference = 0.0f;
			output->m_sharpenRadius = m_sharpenStrength > 0.0f ? 5 : 0;
			output->m_sharpenStrength = m_sharpenStrength;
			output->m_systemTexture = true;

			if (m_textureSizeDenom > 1)
			{
				output->m_scaleImage = true;
				output->m_scaleWidth = atlasImage->getWidth() / m_textureSizeDenom;
				output->m_scaleHeight = atlasImage->getHeight() / m_textureSizeDenom;
			}

			std::wstring bitmapOutputPath = traktor::Path(outputPath).getPathOnly() + L"/Textures/" + bitmapOutputGuid.format();
			if (!pipelineBuilder->buildOutput(
				sourceInstance,
				output,
				bitmapOutputPath,
				bitmapOutputGuid,
				atlasImage
			))
				return false;

			for (std::list< AtlasBitmap >::const_iterator j = i->bitmaps.begin(); j != i->bitmaps.end(); ++j)
			{
				movie->defineBitmap(j->id, new BitmapResource(
					j->packedRect.x,
					j->packedRect.y,
					j->packedRect.width,
					j->packedRect.height,
					m_textureAtlasSize,
					m_textureAtlasSize,
					bitmapOutputGuid
				));
			}
		}
		else if (i->bitmaps.size() == 1)
		{
			AtlasBitmap ab = i->bitmaps.front();
			ab.packedRect.x = 0;
			ab.packedRect.y = 0;
			ab.packedRect.width = ab.bitmap->getWidth();
			ab.packedRect.height = ab.bitmap->getHeight();
			standalone.push_back(ab);
		}
	}

	log::info << uint32_t(standalone.size()) << L" bitmap(s) didn't fit in any atlas..." << Endl;

	for (std::list< AtlasBitmap >::const_iterator i = standalone.begin(); i != standalone.end(); ++i)
	{
		Ref< drawing::Image > bitmapImage = new drawing::Image(
			drawing::PixelFormat::getA8B8G8R8(),
			i->bitmap->getWidth(),
			i->bitmap->getHeight()
		);

		std::memcpy(
			bitmapImage->getData(),
			i->bitmap->getBits(),
			i->bitmap->getWidth() * i->bitmap->getHeight() * 4
		);

#if defined(_DEBUG)
		bitmapImage->save(L"SparkBitmap" + toString(count) + L".png");
#endif

		Guid bitmapOutputGuid = outputGuid.permutation(count++);

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfInvalid;
		output->m_generateNormalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = m_generateMips;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = m_useTextureCompression;
		output->m_enableNormalMapCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_linearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = m_sharpenStrength > 0.0f ? 5 : 0;
		output->m_sharpenStrength = m_sharpenStrength;
		output->m_systemTexture = true;

		if (m_textureSizeDenom > 1)
		{
			output->m_scaleImage = true;
			output->m_scaleWidth = bitmapImage->getWidth() / m_textureSizeDenom;
			output->m_scaleHeight = bitmapImage->getHeight() / m_textureSizeDenom;
		}

		std::wstring bitmapOutputPath = traktor::Path(outputPath).getPathOnly() + L"/Textures/" + bitmapOutputGuid.format();
		if (!pipelineBuilder->buildOutput(
			sourceInstance,
			output,
			bitmapOutputPath,
			bitmapOutputGuid,
			bitmapImage
		))
			return false;

		movie->defineBitmap(i->id, new BitmapResource(
			0,
			0,
			bitmapImage->getWidth(),
			bitmapImage->getHeight(),
			bitmapImage->getWidth(),
			bitmapImage->getHeight(),
			bitmapOutputGuid
		));
	}

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to import Spark movie; unable to create instance." << Endl;
		return false;
	}

	instance->setObject(movie);

	if (!instance->commit())
	{
		log::info << L"Failed to import Spark movie; unable to commit instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > Pipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

	}
}

#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "I18N/Text.h"
#include "Render/Shader/Nodes.h"
#include "Render/Editor/TextureAsset.h"
#include "Render/Editor/Shader/Facades/TextureNodeFacade.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/Node.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureNodeFacade", TextureNodeFacade, NodeFacade)

TextureNodeFacade::TextureNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = new ui::custom::DefaultNodeShape(graphControl);
}

Ref< Node > TextureNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Texture();
}

Ref< ui::custom::Node > TextureNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	Texture* texture = checked_type_cast< Texture* >(shaderNode);

	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
		i18n::Text(L"SHADERGRAPH_NODE_TEXTURE"),
		shaderNode->getInformation(),
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);

	editorNode->setColor(traktor::Color(255, 255, 200));

	Guid textureGuid = texture->getExternal();
	if (!textureGuid.isNull())
	{
		Ref< TextureAsset > textureAsset = editor->getSourceDatabase()->getObjectReadOnly< TextureAsset >(textureGuid);
		if (textureAsset)
		{
			std::wstring assetPath = editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
			Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());

			Ref< drawing::Image > textureImage = drawing::Image::load(fileName);
			if (textureImage)
			{
				drawing::ScaleFilter scale(
					64,
					64,
					drawing::ScaleFilter::MnAverage,
					drawing::ScaleFilter::MgNearest
				);
				textureImage = textureImage->applyFilter(&scale);

				// Create alpha preview.
				if (textureImage->getPixelFormat().getAlphaBits() > 0 && textureAsset->m_hasAlpha == true && textureAsset->m_ignoreAlpha == false)
				{
					for (int32_t y = 0; y < textureImage->getHeight(); ++y)
					{
						for (int32_t x = 0; x < textureImage->getWidth(); ++x)
						{
							drawing::Color alpha =
								((x >> 2) & 1) ^ ((y >> 2) & 1) ?
								drawing::Color(0.4f, 0.4f, 0.4f) :
								drawing::Color(0.6f, 0.6f, 0.6f);

							drawing::Color pixel;
							textureImage->getPixel(x, y, pixel);

							pixel = pixel * pixel.getAlpha() + alpha * (1.0f - pixel.getAlpha());
							pixel.setAlpha(1.0f);

							textureImage->setPixel(x, y, pixel);
						}
					}
				}
				else	// Create solid alpha channel.
				{
					textureImage->convert(drawing::PixelFormat::getR8G8B8A8());

					for (int32_t y = 0; y < textureImage->getHeight(); ++y)
					{
						for (int32_t x = 0; x < textureImage->getWidth(); ++x)
						{
							drawing::Color pixel;
							textureImage->getPixel(x, y, pixel);

							pixel.setAlpha(1.0f);
							textureImage->setPixel(x, y, pixel);
						}
					}
				}

				Ref< ui::Bitmap > nodeImage = new ui::Bitmap();
				if (nodeImage->create(textureImage))
					editorNode->setImage(nodeImage);
			}
		}
	}

	return editorNode;
}

void TextureNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	editor::TypeBrowseFilter filter(type_of< TextureAsset >());
	Ref< db::Instance > instance = editor->browseInstance(&filter);
	if (instance)
		checked_type_cast< Texture*, false >(shaderNode)->setExternal(instance->getGuid());
}

void TextureNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setColor(validationSucceeded ? traktor::Color(200, 255, 255) : traktor::Color(255, 255, 200));
}

	}
}

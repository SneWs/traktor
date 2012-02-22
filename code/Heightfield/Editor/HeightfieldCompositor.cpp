#include <algorithm>
#include <limits>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Float.h"
#include "Core/Math/Winding3.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Heightfield/Editor/Convert.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldLayer.h"
#include "Heightfield/Editor/IBrush.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

Ref< drawing::Image > readRawTerrain(IStream* stream)
{
	uint32_t fileSize = stream->available();

	const uint32_t heightByteSize = 2;

	uint32_t heights = fileSize / heightByteSize;
	uint32_t size = uint32_t(std::sqrt(float(heights)));

	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getR16(),
		size,
		size
	);

	stream->read(image->getData(), fileSize);
	stream->close();

	return image;
}

Ref< drawing::Image > readRawTerrain(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	return file ? readRawTerrain(file) : 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldCompositor", HeightfieldCompositor, Object)

Ref< HeightfieldCompositor > HeightfieldCompositor::createFromAsset(const HeightfieldAsset* asset, const std::wstring& assetPath)
{
	Ref< drawing::Image > baseImage;
	Ref< drawing::Image > offsetImage;
	Ref< drawing::Image > accumImage;
	Ref< drawing::Image > mergedImage;

	// Load base layer as image.
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());
	if ((baseImage = readRawTerrain(fileName)) == 0)
	{
		log::error << L"Unable to read heightfield source \"" << fileName.getPathName() << L"\"" << Endl;
		return 0;
	}

	// Flip base image.
	if (asset->getInvertX() || asset->getInvertZ())
	{
		drawing::MirrorFilter mirrorFilter(asset->getInvertX(), asset->getInvertZ());
		baseImage = baseImage->applyFilter(&mirrorFilter);
		T_ASSERT (baseImage);
	}

	uint32_t size = baseImage->getWidth();

	// Rescale base layer to fit 2^x.
	size = ((size / asset->getDetailSkip() + asset->getPatchDim() - 1) / asset->getPatchDim()) * asset->getPatchDim() * asset->getDetailSkip();
	if (size < asset->getPatchDim() || (size / asset->getDetailSkip()) % asset->getPatchDim() != 0)
	{
		log::error << L"Invalid patch dimension or detail skip value in heightfield asset" << Endl;
		return 0;
	}

	drawing::ScaleFilter scaleFilter(
		size,
		size,
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgLinear
	);
	baseImage = baseImage->applyFilter(&scaleFilter);
	T_ASSERT (baseImage);

	// Create offset image.
	offsetImage = new drawing::Image(drawing::PixelFormat::getR16(), size, size);
	offsetImage->clear(Color4f(0.5f, 0.5f, 0.5f, 0.5f));

	// Create accumulation image.
	accumImage = new drawing::Image(drawing::PixelFormat::getR16(), size, size);
	accumImage->clear(Color4f(0.5f, 0.5f, 0.5f, 0.5f));

	// Create merged image by cloning base image.
	mergedImage = baseImage->clone();

	// Create compositor instance.
	Ref< HeightfieldCompositor > compositor = new HeightfieldCompositor();
	compositor->m_size = size;
	compositor->m_worldExtent = asset->getWorldExtent();
	compositor->m_baseLayer = new HeightfieldLayer(baseImage);
	compositor->m_offsetLayer = new HeightfieldLayer(offsetImage);
	compositor->m_accumLayer = new HeightfieldLayer(accumImage);
	compositor->m_mergedLayer = new HeightfieldLayer(mergedImage);

	return compositor;
}

bool HeightfieldCompositor::readInstanceData(const db::Instance* assetInstance)
{
	Ref< IStream > offsetStream = assetInstance->readData(L"Offset");
	if (offsetStream)
	{
		Ref< drawing::Image > offsetImage = readRawTerrain(offsetStream);
		if (!offsetImage)
			return false;

		if (offsetImage->getWidth() != m_size)
		{
			drawing::ScaleFilter scaleOffsetFilter(
				m_size,
				m_size,
				drawing::ScaleFilter::MnAverage,
				drawing::ScaleFilter::MgLinear
			);
			offsetImage = offsetImage->applyFilter(&scaleOffsetFilter);
			T_ASSERT (offsetImage);
		}

		offsetStream->close();
		offsetStream = 0;

		m_offsetLayer->m_image = offsetImage;
		updateMergedLayer();
	}
	return true;
}

bool HeightfieldCompositor::writeInstanceData(db::Instance* assetInstance) const
{
	Ref< IStream > offsetStream = assetInstance->writeData(L"Offset");
	if (!offsetStream)
		return false;

	const drawing::Image* offsetImage = m_offsetLayer->getImage();
	const height_t* heights = static_cast< const height_t* >(offsetImage->getData());

	Writer(offsetStream).write(
		heights,
		m_size * m_size,
		sizeof(height_t)
	);
	offsetStream->close();

	return true;
}

float HeightfieldCompositor::getNearestHeight(float x, float z) const
{
	float gx = worldToGridX(x);
	float gz = worldToGridZ(z);

	int32_t igx = int32_t(gx);
	int32_t igz = int32_t(gz);

	Color4f c;
	m_mergedLayer->getImage()->getPixel(igx, igz, c);

	return (c.getRed() - 0.5f) * m_worldExtent.y();
}

float HeightfieldCompositor::getBilinearHeight(float x, float z) const
{
	float gx = worldToGridX(x);
	float gz = worldToGridZ(z);

	int32_t igx = int32_t(gx);
	int32_t igz = int32_t(gz);

	Color4f c0, c1, c2, c3;
	m_mergedLayer->getImage()->getPixel(igx, igz, c0);
	m_mergedLayer->getImage()->getPixel(igx + 1, igz, c1);
	m_mergedLayer->getImage()->getPixel(igx, igz + 1, c2);
	m_mergedLayer->getImage()->getPixel(igx + 1, igz + 1, c3);

	float fx = gx - igx;
	float fz = gz - igz;

	float h[] = 
	{
		(c0.getRed() - 0.5f) * m_worldExtent.y(),
		(c1.getRed() - 0.5f) * m_worldExtent.y(),
		(c2.getRed() - 0.5f) * m_worldExtent.y(),
		(c3.getRed() - 0.5f) * m_worldExtent.y()
	};

	float hn = lerp(h[0], h[1], fx);
	float hf = lerp(h[2], h[3], fx);

	return lerp(hn, hf, fz);
}

bool HeightfieldCompositor::queryRay(const Vector4& localRayOrigin, const Vector4& localRayDirection, Scalar& outDistance, Vector4* outPosition) const
{
	const uint32_t c_cellSize = 64;
	const uint32_t c_skip = 4;

	Scalar k;
	Scalar kIn, kOut;

	Aabb3 boundingBox(-m_worldExtent * Scalar(0.5f), m_worldExtent * Scalar(0.5f));
	if (!boundingBox.intersectRay(localRayOrigin, localRayDirection, kIn, kOut))
		return false;

	float dx = m_worldExtent.x() / (m_size - 1);
	float dz = m_worldExtent.z() / (m_size - 1);

	Winding3 w;
	w.points.resize(3);

	bool foundIntersection = false;

	outDistance = Scalar(std::numeric_limits< float >::max());

	for (uint32_t cz = 0; cz < m_size; cz += c_cellSize)
	{
		float cwz1 = cz * dz - m_worldExtent.z() * 0.5f;
		float cwz2 = (cz + c_cellSize) * dz - m_worldExtent.z() * 0.5f;

		for (uint32_t cx = 0; cx < m_size; cx += c_cellSize)
		{
			float cwx1 = cx * dx - m_worldExtent.x() * 0.5f;
			float cwx2 = (cx + c_cellSize) * dx - m_worldExtent.x() * 0.5f;

			float cwy[] =
			{
				getNearestHeight(cwx1, cwz1),
				getNearestHeight(cwx2, cwz1),
				getNearestHeight(cwx1, cwz2),
				getNearestHeight(cwx2, cwz2)
			};

			float cwy1 = *std::min_element(cwy, cwy + sizeof_array(cwy));
			float cwy2 = *std::max_element(cwy, cwy + sizeof_array(cwy));

			Aabb3 bb;
			bb.mn = Vector4(cwx1, cwy1, cwz1, 1.0f);
			bb.mx = Vector4(cwx2, cwy2, cwz2, 1.0f);

			if (bb.intersectRay(localRayOrigin, localRayDirection, kIn, kOut))
			{
				for (uint32_t iz = cz; iz <= cz + c_cellSize; iz += c_skip)
				{
					float wz1 = iz * dz - m_worldExtent.z() * 0.5f;
					float wz2 = (iz + c_skip) * dz - m_worldExtent.z() * 0.5f;

					for (uint32_t ix = cx; ix <= cx + c_cellSize; ix += c_skip)
					{
						float wx1 = ix * dx - m_worldExtent.x() * 0.5f;
						float wx2 = (ix + c_skip) * dx - m_worldExtent.x() * 0.5f;

						float wy[] =
						{
							getNearestHeight(wx1, wz1),
							getNearestHeight(wx2, wz1),
							getNearestHeight(wx1, wz2),
							getNearestHeight(wx2, wz2)
						};

						Vector4 wv[] =
						{
							Vector4(wx1, wy[0], wz1, 1.0f),
							Vector4(wx2, wy[1], wz1, 1.0f),
							Vector4(wx1, wy[2], wz2, 1.0f),
							Vector4(wx2, wy[3], wz2, 1.0f)
						};

						w.points[0] = wv[0];
						w.points[1] = wv[1];
						w.points[2] = wv[2];

						if (w.rayIntersection(localRayOrigin, localRayDirection, k))
						{
							if (k < outDistance)
							{
								outDistance = k;
								foundIntersection = true;
							}
						}

						w.points[0] = wv[1];
						w.points[1] = wv[3];
						w.points[2] = wv[2];

						if (w.rayIntersection(localRayOrigin, localRayDirection, k))
						{
							if (k < outDistance)
							{
								outDistance = k;
								foundIntersection = true;
							}
						}
					}
				}
			}
		}
	}

	if (foundIntersection && outPosition)
		*outPosition = localRayOrigin + localRayDirection * outDistance;

	return foundIntersection;
}

void HeightfieldCompositor::begin()
{
	m_dirtyDraw = Region();
}

void HeightfieldCompositor::strokeBrush(const Vector4& fromPosition, const Vector4& toPosition, const IBrush* brush, Region* outDirtyRegion)
{
	T_ASSERT (brush);

	int32_t x0 = worldToGridX(fromPosition.x());
	int32_t z0 = worldToGridZ(fromPosition.z());

	int32_t x1 = worldToGridX(toPosition.x());
	int32_t z1 = worldToGridZ(toPosition.z());

	float dx = x1 - x1;
	float dz = z1 - z1;

	Region dirtyStroke;

	float wx0 = gridToWorldX(x0);
	float wz0 = gridToWorldZ(z0);

	brush->apply(this, Vector4(wx0, 0.0f, wz0, 0.0f), dirtyStroke);

	if (std::abs(dx) > std::abs(dz) && dx != 0)
	{
		if (dx < 0.0f)
		{
			std::swap(x0, x1);
			std::swap(z0, z1);
		}

		float k = dz / dx;
		float z = z0 + k;

		for (int32_t x = x0 + 1; x < x1; ++x)
		{
			float wx = gridToWorldX(x);
			float wz = gridToWorldZ(z);

			Region dirtyBrush;
			brush->apply(this, Vector4(wx, 0.0f, wz, 0.0f), dirtyBrush);
			dirtyStroke.contain(dirtyBrush);

			z += k;
		}
	}
	else if (dz != 0.0f)
	{
		if (dz < 0.0f)
		{
			std::swap(x0, x1);
			std::swap(z0, z1);
		}

		float k = dx / dz;
		float x = x0 + k;

		for (int32_t z = z0 + 1; z < z1; ++z)
		{
			float wx = gridToWorldX(x);
			float wz = gridToWorldZ(z);

			Region dirtyBrush;
			brush->apply(this, Vector4(wx, 0.0f, wz, 0.0f), dirtyBrush);
			dirtyStroke.contain(dirtyBrush);

			x += k;
		}
	}

	if (!dirtyStroke.empty())
	{
		updateMergedLayer(dirtyStroke);
		m_dirtyDraw.contain(dirtyStroke);
	}

	if (outDirtyRegion)
		*outDirtyRegion = dirtyStroke;
}

void HeightfieldCompositor::end(Region* outDirtyRegion)
{
	if (!m_dirtyDraw.empty())
	{
		drawing::Image* offsetImage = m_offsetLayer->getImage();
		drawing::Image* accumImage = m_accumLayer->getImage();

		height_t* offsetHeights = static_cast< height_t* >(offsetImage->getData());
		height_t* accumHeights = static_cast< height_t* >(accumImage->getData());

		for (int32_t iz = m_dirtyDraw.minZ; iz < m_dirtyDraw.maxZ; ++iz)
		{
			for (int32_t ix = m_dirtyDraw.minX; ix < m_dirtyDraw.maxX; ++ix)
			{
				uint32_t o = ix + iz * m_size;

				height_t& offset = offsetHeights[o];
				height_t accum = accumHeights[o];

				offset += unpackSignedHeight(accum);
			}
		}

		accumImage->clear(Color4f(0.5f, 0.5f, 0.5f, 0.5f));

		updateMergedLayer(m_dirtyDraw);
	}

	if (outDirtyRegion)
		*outDirtyRegion = m_dirtyDraw;
}

void HeightfieldCompositor::copyHeights(height_t* h) const
{
	const height_t* s = static_cast< const height_t* >(m_mergedLayer->getImage()->getData());
	std::memcpy(h, s, m_size * m_size * sizeof(height_t));
}

float HeightfieldCompositor::worldToGridX(float x) const
{
	return m_size * (x + m_worldExtent.x() * 0.5f) / m_worldExtent.x() - 0.5f;
}

float HeightfieldCompositor::worldToGridZ(float z) const
{
	return m_size * (z + m_worldExtent.z() * 0.5f) / m_worldExtent.z() - 0.5f;
}

float HeightfieldCompositor::gridToWorldX(float x) const
{
	return (x + 0.5f) * m_worldExtent.x() / m_size - m_worldExtent.x() * 0.5f;
}

float HeightfieldCompositor::gridToWorldZ(float z) const
{
	return (z + 0.5f) * m_worldExtent.z() / m_size - m_worldExtent.z() * 0.5f;
}

void HeightfieldCompositor::updateMergedLayer()
{
	updateMergedLayer(Region(0, 0, m_size, m_size));
}

void HeightfieldCompositor::updateMergedLayer(const Region& r)
{
	const drawing::Image* baseImage = m_baseLayer->getImage();
	const drawing::Image* offsetImage = m_offsetLayer->getImage();
	const drawing::Image* accumImage = m_accumLayer->getImage();
	drawing::Image* mergedImage = m_mergedLayer->getImage();

	Region cr = r;
	
	cr.intersect(Region(0, 0, m_size, m_size));
	if (cr.empty())
		return;

	const height_t* baseHeights = static_cast< const height_t* >(baseImage->getData());
	const height_t* offsetHeights = static_cast< const height_t* >(offsetImage->getData());
	const height_t* accumHeights = static_cast< const height_t* >(accumImage->getData());
	height_t* mergedHeights = static_cast< height_t* >(mergedImage->getData());

	for (int32_t iz = cr.minZ; iz < cr.maxZ; ++iz)
	{
		for (int32_t ix = cr.minX; ix <= cr.maxX; ++ix)
		{
			uint32_t o = ix + iz * m_size;

			height_t offset = offsetHeights[o];
			height_t accum = accumHeights[o];

			offset += unpackSignedHeight(accum);

			height_t merged = baseHeights[o];
			merged += unpackSignedHeight(offset);

			mergedHeights[o] = merged;
		}
	}
}

	}
}

#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.MirrorFilter", MirrorFilter, ImageFilter)

MirrorFilter::MirrorFilter(bool flipHorizontal, bool flipVertical)
:	m_flipHorizontal(flipHorizontal)
,	m_flipVertical(flipVertical)
{
}

Image* MirrorFilter::apply(const Image* image)
{
	Ref< Image > final = gc_new< Image >(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	Color in;

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		int32_t dy = m_flipVertical ? (image->getHeight() - y - 1) : y;
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in);
			int32_t dx = m_flipHorizontal ? (image->getWidth() - x - 1) : x;
			final->setPixel(dx, dy, in);
		}
	}

	return final;
}
	
	}
}

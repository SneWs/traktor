#pragma once

#include "Core/Config.h"
#include "Core/Ref.h"
#include "Ui/Rect.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace ui
	{

/*! \brief System bitmap interface.
 * \ingroup UI
 */
class ISystemBitmap
{
public:
	virtual bool create(uint32_t width, uint32_t height) = 0;

	virtual void destroy() = 0;

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos) = 0;

	virtual Ref< drawing::Image > getImage() const = 0;

	virtual Size getSize() const = 0;
};

	}
}


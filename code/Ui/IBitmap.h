#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Ui/Size.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace ui
	{

class ISystemBitmap;

/*! Bitmap interface.
 * \ingroup UI
 */
class T_DLLCLASS IBitmap : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual Size getSize() const = 0;

	virtual Ref< drawing::Image > getImage() const = 0;

	virtual ISystemBitmap* getSystemBitmap() const = 0;
};

	}
}


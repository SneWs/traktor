#pragma once

#include "Flash/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS BitmapFilter : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	virtual Ref< BitmapFilter > clone() = 0;

protected:
	BitmapFilter(const char* const prototype);
};

	}
}


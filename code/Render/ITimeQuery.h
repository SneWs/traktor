#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief GPU time stamp recording query.
 * \ingroup Render
 */
class T_DLLCLASS ITimeQuery : public Object
{
	T_RTTI_CLASS;

public:
	virtual void begin() = 0;

	virtual int32_t stamp() = 0;

	virtual void end() = 0;

	virtual bool ready() const = 0;

	virtual uint64_t get(int32_t index) const = 0;
};

	}
}


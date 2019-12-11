#pragma once

#include <limits>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! Stream copy.
 * \ingroup Core
 *
 * Copy entire stream data from one stream
 * to another.
 */
class T_DLLCLASS StreamCopy : public Object
{
	T_RTTI_CLASS;

public:
	StreamCopy(IStream* target, IStream* source);

	bool execute(uint64_t copyBytes = std::numeric_limits< uint64_t >::max());

private:
	Ref< IStream > m_target;
	Ref< IStream > m_source;
};

}


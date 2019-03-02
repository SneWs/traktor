#pragma once

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

/*! \brief Stream compare.
 * \ingroup Core
 *
 * Compare feed of two streams; position of both streams
 * are preserved.
 */
class T_DLLCLASS StreamCompare : public Object
{
	T_RTTI_CLASS;

public:
	StreamCompare(IStream* first, IStream* second);

	bool execute();

private:
	Ref< IStream > m_first;
	Ref< IStream > m_second;
};

}


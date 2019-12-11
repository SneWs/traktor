#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Sound;

/*! Sound rendering interface.
 * \ingroup Spark
 */
class T_DLLCLASS ISoundRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual void play(const Sound* sound) = 0;
};

	}
}


#pragma once

#include "Physics/Joint.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! Fixed joint.
 * \ingroup Physics
 */
class T_DLLCLASS FixedJoint : public Joint
{
	T_RTTI_CLASS;
};

	}
}


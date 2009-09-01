#ifndef traktor_animation_IPoseController_H
#define traktor_animation_IPoseController_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class Skeleton;

/*! \brief Pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS IPoseController : public Object
{
	T_RTTI_CLASS(IPoseController)

public:
	struct Velocity
	{
		Vector4 linear;
		Vector4 angular;
	};

	virtual void evaluate(
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& boneTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	) = 0;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) = 0;
};

	}
}

#endif	// traktor_animation_IPoseController_H

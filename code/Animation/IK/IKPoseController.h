#ifndef traktor_animation_IKPoseController_H
#define traktor_animation_IKPoseController_H

#include "Animation/IPoseController.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class PhysicsManager;
class Body;

	}

	namespace animation
	{

/*! \brief IK pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS IKPoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	IKPoseController(physics::PhysicsManager* physicsManager, IPoseController* poseController, uint32_t solverIterations);

	virtual ~IKPoseController();

	void setIgnoreBody(physics::Body* ignoreBody);

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

	virtual bool evaluate(
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	);

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	);

	inline Ref< IPoseController > getNeutralPoseController() const { return m_poseController; }

private:
	Ref< physics::PhysicsManager > m_physicsManager;
	Ref< IPoseController > m_poseController;
	uint32_t m_solverIterations;
	Ref< physics::Body > m_ignoreBody;
};

	}
}

#endif	// traktor_animation_IKPoseController_H

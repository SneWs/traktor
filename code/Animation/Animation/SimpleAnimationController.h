#pragma once

#include <map>
#include "Animation/IPoseController.h"
#include "Animation/Animation/StateContext.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class Animation;
class StateContext;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS SimpleAnimationController : public IPoseController
{
	T_RTTI_CLASS;

public:
	SimpleAnimationController(const resource::Proxy< Animation >& animation, bool linearInterpolation);

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms
	) override final;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) override final;

private:
	resource::Proxy< Animation > m_animation;
	bool m_linearInterpolation;
	int32_t m_indexHint;
	float m_timeOffset;
	float m_lastTime;
	Pose m_evaluationPose;
};

	}
}


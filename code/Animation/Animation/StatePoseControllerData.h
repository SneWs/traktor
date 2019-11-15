#pragma once

#include "Core/Math/Range.h"
#include "Animation/IPoseControllerData.h"
#include "Resource/Id.h"

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

class StateGraph;

/*! Animation evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS StatePoseControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	StatePoseControllerData();

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	inline const resource::Id< StateGraph >& getStateGraph() const { return m_stateGraph; }

private:
	resource::Id< StateGraph > m_stateGraph;
	Range< float > m_randomTimeOffset;
};

	}
}


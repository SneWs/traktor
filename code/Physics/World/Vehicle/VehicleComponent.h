#pragma once

#include "Core/RefArray.h"
#include "World/IEntityComponent.h"

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

class Body;
struct CollisionInfo;
class PhysicsManager;
class VehicleComponentData;
class Wheel;

/*! Vehicle simulation component.
 * \ingroup Physics
 */
class T_DLLCLASS VehicleComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	VehicleComponent(
		PhysicsManager* physicsManager,
		const VehicleComponentData* data,
		Body* body,
		const RefArray< Wheel >& wheels,
		uint32_t traceInclude,
		uint32_t traceIgnore
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void setSteerAngle(float steerAngle);

	float getSteerAngle() const;

	float getSteerAngleFiltered() const;

	void setEngineThrottle(float engineThrottle);

	float getEngineThrottle() const;

	const RefArray< Wheel >& getWheels() const { return m_wheels; }

private:
	world::Entity* m_owner;
	Ref< PhysicsManager > m_physicsManager;
	Ref< const VehicleComponentData > m_data;
	Ref< Body > m_body;
	RefArray< Wheel > m_wheels;
	uint32_t m_traceInclude;
	uint32_t m_traceIgnore;
	Scalar m_totalMass;
	float m_steerAngle;
	float m_steerAngleTarget;
	float m_engineThrottle;
	bool m_airBorn;

	void updateSteering(float dT);

	void updateSuspension(float dT);

	void updateFriction(float dT);

	void updateEngine(float dT);

	void updateWheels(float dT);
};

	}
}


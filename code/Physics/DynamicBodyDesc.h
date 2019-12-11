#pragma once

#include "Physics/BodyDesc.h"

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

/*! Dynamic rigid body description.
 * \ingroup Physics
 */
class T_DLLCLASS DynamicBodyDesc : public BodyDesc
{
	T_RTTI_CLASS;

public:
	DynamicBodyDesc();

	explicit DynamicBodyDesc(ShapeDesc* shape);

	void setMass(float mass);

	float getMass() const;

	void setAutoDeactivate(bool autoDeactivate);

	bool getAutoDeactivate() const;

	void setActive(bool active);

	bool getActive() const;

	void setLinearDamping(float linearDamping);

	float getLinearDamping() const;

	void setAngularDamping(float angularDamping);

	float getAngularDamping() const;

	void setFriction(float friction);

	float getFriction() const;

	void setLinearThreshold(float linearThreshold);

	float getLinearThreshold() const;

	void setAngularThreshold(float angularThreshold);

	float getAngularThreshold() const;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_mass;
	bool m_autoDeactivate;
	bool m_active;
	float m_linearDamping;
	float m_angularDamping;
	float m_friction;
	float m_linearThreshold;
	float m_angularThreshold;
};

	}
}


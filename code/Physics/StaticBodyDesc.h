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

/*! Static rigid body description.
 * \ingroup Physics
 */
class T_DLLCLASS StaticBodyDesc : public BodyDesc
{
	T_RTTI_CLASS;

public:
	StaticBodyDesc() = default;

	explicit StaticBodyDesc(ShapeDesc* shape);

	void setFriction(float friction);

	float getFriction() const;

	void setRestitution(float restitution);

	float getRestitution() const;

	void setKinematic(bool kinematic);

	bool isKinematic() const;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_friction = 0.75f;
	float m_restitution = 0.0f;
	bool m_kinematic = false;
};

	}
}


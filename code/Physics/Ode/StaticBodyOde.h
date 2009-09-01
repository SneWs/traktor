#ifndef traktor_physics_StaticBodyOde_H
#define traktor_physics_StaticBodyOde_H

#include <ode/ode.h>
#include "Physics/StaticBody.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_ODE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

struct DestroyCallback;

/*!
 * \ingroup ODE
 */
class T_DLLCLASS StaticBodyOde : public StaticBody
{
	T_RTTI_CLASS(StaticBodyOde)

public:
	StaticBodyOde(DestroyCallback* callback, dGeomID geomId);

	virtual ~StaticBodyOde();

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

	virtual Transform getTransform() const;

	virtual void setActive(bool active);

	virtual bool isActive() const;

	virtual void setEnable(bool enable);

	virtual bool isEnable() const;

	const dGeomID getGeomId() const;

private:
	DestroyCallback* m_callback;
	dGeomID m_geomId;
};

	}
}

#endif	// traktor_physics_StaticBodyOde_H

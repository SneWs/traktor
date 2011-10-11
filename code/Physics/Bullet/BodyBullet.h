#ifndef traktor_physics_BodyBullet_H
#define traktor_physics_BodyBullet_H

#include <btBulletDynamicsCommon.h>
#include "Core/Misc/InvokeOnce.h"
#include "Physics/Bullet/Types.h"

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
template < typename Outer >
class BodyBullet : public Outer
{
public:
	BodyBullet(
		IWorldCallback* callback,
		btDynamicsWorld* dynamicsWorld,
		btRigidBody* body,
		btCollisionShape* shape,
		uint32_t collisionGroup,
		uint32_t collisionMask
	)
	:	m_callback(callback)
	,	m_dynamicsWorld(dynamicsWorld)
	,	m_body(body)
	,	m_shape(shape)
	,	m_collisionGroup(collisionGroup)
	,	m_collisionMask(collisionMask)
	,	m_enable(false)
	{
	}

	virtual ~BodyBullet()
	{
		destroy();
	}

	virtual void destroy()
	{
		invokeOnce< IWorldCallback, Body*, btRigidBody*, btCollisionShape* >(m_callback, &IWorldCallback::destroyBody, this, m_body, m_shape);

		m_dynamicsWorld = 0;
		m_body = 0;
		m_shape = 0;

		Outer::destroy();
	}

	virtual void setEnable(bool enable)
	{
		if (enable == m_enable)
			return;

		if (enable)
			m_callback->insertBody(m_body, (uint16_t)m_collisionGroup, (uint16_t)m_collisionMask);
		else
			m_callback->removeBody(m_body);

		m_enable = enable;
	}

	virtual bool isEnable() const
	{
		return m_enable;
	}

	btDynamicsWorld* getBtDynamicsWorld() const { return m_dynamicsWorld; }

	btRigidBody* getBtRigidBody() const { return m_body; }

	btCollisionShape* getBtCollisionShape() const { return m_shape; }

	uint32_t getCollisionGroup() const { return m_collisionGroup; }

	uint32_t getCollisionMask() const { return m_collisionMask; }

protected:
	IWorldCallback* m_callback;
	btDynamicsWorld* m_dynamicsWorld;
	btRigidBody* m_body;
	btCollisionShape* m_shape;
	uint32_t m_collisionGroup;
	uint32_t m_collisionMask;
	bool m_enable;
};

	}
}

#endif	// traktor_physics_BodyBullet_H

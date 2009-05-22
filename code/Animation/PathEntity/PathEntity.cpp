#include "Animation/PathEntity/PathEntity.h"
#include "World/WorldContext.h"
#include "World/Entity/EntityUpdate.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntity", PathEntity, world::SpatialEntity)

PathEntity::PathEntity(const Matrix44& transform, const Path& path, TimeMode timeMode, world::SpatialEntity* entity)
:	m_transform(transform)
,	m_path(path)
,	m_timeMode(timeMode)
,	m_entity(entity)
,	m_timeScale(1.0f)
,	m_timeDeltaSign(1.0f)
,	m_time(0.0f)
{
}

PathEntity::~PathEntity()
{
	destroy();
}

void PathEntity::destroy()
{
	if (m_entity)
	{
		m_entity->destroy();
		m_entity = 0;
	}
}

void PathEntity::setTransform(const Matrix44& transform)
{
	m_transform = transform;
}

bool PathEntity::getTransform(Matrix44& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb PathEntity::getBoundingBox() const
{
	return m_entity ? m_entity->getBoundingBox() : Aabb();
}

void PathEntity::update(const world::EntityUpdate* update)
{
	if (!m_entity)
		return;

	if (m_timeMode != TmManual)
	{
		m_time += update->getDeltaTime() * m_timeDeltaSign;

		switch (m_timeMode)
		{
		case TmOnce:
			if (m_time > m_path.getEndTime())
			{
				m_time = m_path.getEndTime();
				m_timeDeltaSign = 0.0f;
			}
			break;

		case TmLoop:
			if (m_time > m_path.getEndTime())
				m_time = 0.0f;
			break;

		case TmPingPong:
			if (m_time > m_path.getEndTime())
			{
				m_time = m_path.getEndTime();
				m_timeDeltaSign = -1.0f;
			}
			else if (m_time < 0.0f)
			{
				m_time = 0.0f;
				m_timeDeltaSign = 1.0f;
			}
			break;
		}
	}

	Path::Frame frame = m_path.evaluate(m_time);
	Matrix44 transform = frame.orientation.toMatrix44() * translate(frame.position);

	m_entity->setTransform(transform * m_transform);
	m_entity->update(update);
}

void PathEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView)
{
	if (m_entity)
		worldContext->build(worldRenderView, m_entity);
}

	}
}

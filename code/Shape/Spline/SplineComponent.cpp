#include "Shape/Spline/ControlPointComponent.h"
#include "Shape/Spline/LayerComponent.h"
#include "Shape/Spline/SplineComponent.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/GroupComponent.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineComponent", SplineComponent, world::IEntityComponent)

SplineComponent::SplineComponent()
:	m_owner(nullptr)
,	m_dirty(true)
{
}

void SplineComponent::destroy()
{
	m_owner = nullptr;
}

void SplineComponent::setOwner(world::Entity* owner)
{
	m_owner = dynamic_type_cast< world::ComponentEntity* >(owner);
}

void SplineComponent::setTransform(const Transform& transform)
{
}

void SplineComponent::update(const world::UpdateParams& update)
{
	auto groupComponent = m_owner->getComponent< world::GroupComponent >();
	if (!groupComponent)
		return;

	// Get control points and layers.
	RefArray< ControlPointComponent > controlPoints;
	RefArray< LayerComponent > layers;
	for (auto entity : groupComponent->getEntities())
	{
		auto cmpent = dynamic_type_cast< world::ComponentEntity* >(entity);
		if (!cmpent)
			continue;

		auto controlPoint = cmpent->getComponent< ControlPointComponent >();
		if (controlPoint)
			controlPoints.push_back(controlPoint);

		auto layer = cmpent->getComponent< LayerComponent >();
		if (layer)
			layers.push_back(layer);
	}

	// Check if any control point is dirty.
	bool controlPointsDirty = m_dirty;
	for (auto controlPoint : controlPoints)
	{
		if (controlPoint->checkDirty())
		{
			controlPointsDirty |= true;
			break;
		}
	}

	// Update transform path if any control point is dirty.
	if (controlPointsDirty)
	{
		m_path = TransformPath();
		for (uint32_t i = 0; i < controlPoints.size(); ++i)
		{
			Transform T = controlPoints[i]->getTransform();

			TransformPath::Key k;
			k.T = (float)i / (controlPoints.size() - 1);
			k.position = T.translation();
			k.orientation = T.rotation().toEulerAngles();
			m_path.insert(k);
		}

		for (auto layer : layers)
			layer->pathChanged(m_path);

		m_dirty = false;
	}
}

	}
}

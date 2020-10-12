#include "World/Entity.h"
#include "World/IEntityComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.Entity", Entity, Object)

Entity::Entity(const std::wstring& name, const Transform& transform)
:	m_name(name)
,	m_transform(transform)
{
}

Entity::Entity(const std::wstring& name, const Transform& transform, const RefArray< IEntityComponent >& components)
:	m_name(name)
,	m_transform(transform)
,	m_components(components)
{
	for (auto component : m_components)
	{
		m_updating = component;
		component->setOwner(this);
		component->setTransform(m_transform);
	}
	m_updating = nullptr;
}

Entity::~Entity()
{
}

void Entity::destroy()
{
	for (auto component : m_components)
		component->destroy();
	m_components.clear();
}

const std::wstring& Entity::getName() const
{
	return m_name;
}

void Entity::setTransform(const Transform& transform)
{
	m_transform = transform;
	for (auto component : m_components)
	{
		if (component != m_updating)
			component->setTransform(transform);
	}
}

Transform Entity::getTransform() const
{
	return m_transform;
}

Aabb3 Entity::getBoundingBox() const
{
	Aabb3 boundingBox;
	for (auto component : m_components)
		boundingBox.contain(component->getBoundingBox());
	return boundingBox;
}

void Entity::update(const UpdateParams& update)
{
	for (auto component : m_components)
	{
		m_updating = component;
		component->update(update);
	}
	m_updating = nullptr;
}

void Entity::setComponent(IEntityComponent* component)
{
	T_FATAL_ASSERT (component);

	component->setOwner(this);
	component->setTransform(m_transform);

	// Replace existing component of same type.
	for (auto comp : m_components)
	{
		if (is_type_of(type_of(comp), type_of(component)))
		{
			comp = component;
			return;
		}
	}

	// No such component, add last.
	m_components.push_back(component);
}

IEntityComponent* Entity::getComponent(const TypeInfo& componentType) const
{
	for (auto component : m_components)
	{
		if (is_type_of(componentType, type_of(component)))
			return component;
	}
	return nullptr;
}

	}
}

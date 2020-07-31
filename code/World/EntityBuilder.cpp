#include <limits>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "World/EntityBuilder.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEventData.h"
#include "World/IEntityFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilder", EntityBuilder, IEntityBuilder)

void EntityBuilder::addFactory(const IEntityFactory* entityFactory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_entityFactories.push_back(entityFactory);
	m_resolvedFactoryCache.clear();
}

void EntityBuilder::removeFactory(const IEntityFactory* entityFactory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	auto it = std::find(m_entityFactories.begin(), m_entityFactories.end(), entityFactory);
	if (it != m_entityFactories.end())
	{
		m_entityFactories.erase(it);
		m_resolvedFactoryCache.clear();
	}
}

const IEntityFactory* EntityBuilder::getFactory(const EntityData* entityData) const
{
	if (!entityData)
		return nullptr;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const TypeInfo& entityDataType = type_of(entityData);
	const IEntityFactory* entityFactory = nullptr;

	auto it = m_resolvedFactoryCache.find(&entityDataType);
	if (it != m_resolvedFactoryCache.end())
	{
		// This type of entity has already been created; reuse same factory.
		entityFactory = it->second;
	}
	else
	{
		// Need to find factory best suited to create entity from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator it = m_entityFactories.begin(); it != m_entityFactories.end() && minClassDifference > 0; ++it)
		{
			const TypeInfoSet& typeSet = (*it)->getEntityTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, entityDataType))
				{
					uint32_t classDifference = type_difference(**j, entityDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *it;
					}
				}
			}
		}
		m_resolvedFactoryCache.insert(std::make_pair(
			&entityDataType,
			entityFactory
		));
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for \"" << entityData->getName() << L"\" of " << type_name(entityData) << Endl;
		return nullptr;
	}

	return entityFactory;
}

const IEntityFactory* EntityBuilder::getFactory(const IEntityEventData* entityEventData) const
{
	if (!entityEventData)
		return nullptr;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const TypeInfo& entityEventDataType = type_of(entityEventData);
	const IEntityFactory* entityFactory = nullptr;

	auto it = m_resolvedFactoryCache.find(&entityEventDataType);
	if (it != m_resolvedFactoryCache.end())
	{
		// This type of entity has already been created; reuse same factory.
		entityFactory = it->second;
	}
	else
	{
		// Need to find factory best suited to create entity from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator it = m_entityFactories.begin(); it != m_entityFactories.end() && minClassDifference > 0; ++it)
		{
			const TypeInfoSet& typeSet = (*it)->getEntityEventTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, entityEventDataType))
				{
					uint32_t classDifference = type_difference(**j, entityEventDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *it;
					}
				}
			}
		}
		m_resolvedFactoryCache.insert(std::make_pair(
			&entityEventDataType,
			entityFactory
		));
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for event of " << type_name(entityEventData) << Endl;
		return nullptr;
	}

	return entityFactory;
}

const IEntityFactory* EntityBuilder::getFactory(const IEntityComponentData* entityComponentData) const
{
	if (!entityComponentData)
		return nullptr;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const TypeInfo& entityComponentDataType = type_of(entityComponentData);
	const IEntityFactory* entityFactory = nullptr;

	auto it = m_resolvedFactoryCache.find(&entityComponentDataType);
	if (it != m_resolvedFactoryCache.end())
	{
		// This type of entity has already been created; reuse same factory.
		entityFactory = it->second;
	}
	else
	{
		// Need to find factory best suited to create entity from it's data.
		uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
		for (RefArray< const IEntityFactory >::const_iterator it = m_entityFactories.begin(); it != m_entityFactories.end() && minClassDifference > 0; ++it)
		{
			const TypeInfoSet& typeSet = (*it)->getEntityComponentTypes();
			for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
			{
				if (is_type_of(**j, entityComponentDataType))
				{
					uint32_t classDifference = type_difference(**j, entityComponentDataType);
					if (classDifference < minClassDifference)
					{
						minClassDifference = classDifference;
						entityFactory = *it;
					}
				}
			}
		}
		m_resolvedFactoryCache.insert(std::make_pair(
			&entityComponentDataType,
			entityFactory
		));
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for component of " << type_name(entityComponentData) << Endl;
		return nullptr;
	}

	return entityFactory;
}

Ref< Entity > EntityBuilder::create(const EntityData* entityData) const
{
	const IEntityFactory* entityFactory = getFactory(entityData);
	return entityFactory ? entityFactory->createEntity(this, *entityData) : nullptr;
}

Ref< IEntityEvent > EntityBuilder::create(const IEntityEventData* entityEventData) const
{
	const IEntityFactory* entityFactory = getFactory(entityEventData);
	return entityFactory ? entityFactory->createEntityEvent(this, *entityEventData) : nullptr;
}

Ref< IEntityComponent > EntityBuilder::create(const IEntityComponentData* entityComponentData) const
{
	const IEntityFactory* entityFactory = getFactory(entityComponentData);
	return entityFactory ? entityFactory->createEntityComponent(this, *entityComponentData) : nullptr;
}

const IEntityBuilder* EntityBuilder::getCompositeEntityBuilder() const
{
	return this;
}

	}
}

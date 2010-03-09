#include <limits>
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/EntityAdapter.h"
#include "World/Entity/IEntityFactory.h"
#include "World/Entity/IEntityManager.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/Entity.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "Database/Database.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Misc/Save.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

Ref< const world::EntityData > resolveRealEntityData(db::Database* database, const world::EntityData* entityData)
{
	Ref< world::EntityData > realEntityData;
	if (const world::ExternalEntityData* externalEntityData = dynamic_type_cast< const world::ExternalEntityData* >(entityData))
	{
		realEntityData = database->getObjectReadOnly< world::EntityData >(externalEntityData->getGuid());
	}
	else if (const world::ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const world::ExternalSpatialEntityData* >(entityData))
	{
		Ref< world::SpatialEntityData > realSpatialEntityData = database->getObjectReadOnly< world::SpatialEntityData >(externalSpatialEntityData->getGuid());
		if (realSpatialEntityData)
		{
			realSpatialEntityData->setTransform(externalSpatialEntityData->getTransform());
			realEntityData = realSpatialEntityData;
		}
	}
	return realEntityData ? realEntityData : entityData;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapterBuilder", EntityAdapterBuilder, world::IEntityBuilder)

EntityAdapterBuilder::EntityAdapterBuilder(SceneEditorContext* context)
:	m_context(context)
{
}

void EntityAdapterBuilder::addFactory(world::IEntityFactory* entityFactory)
{
	m_entityFactories.push_back(entityFactory);
}

void EntityAdapterBuilder::removeFactory(world::IEntityFactory* entityFactory)
{
	T_BREAKPOINT;
}

void EntityAdapterBuilder::begin(world::IEntityManager* entityManager)
{
	m_entityManager = entityManager;

	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		Ref< EntityAdapter > entityAdapter = *i;

		// Destroy existing entity; will be re-created.
		Ref< world::Entity > entity = entityAdapter->getEntity();
		if (entity)
		{
			entity->destroy();
			entityAdapter->setEntity(0);
		}

		// Unlink adapter from parent.
		EntityAdapter* parent = entityAdapter->getParent();
		if (parent)
			parent->unlink(entityAdapter);

		// Insert into map from instance guid to adapters.
		Guid instanceGuid = entityAdapter->getInstance()->getGuid();
		m_cachedInstances[instanceGuid].push_back(entityAdapter);
	}

	T_ASSERT (!m_rootAdapter);
}

Ref< world::Entity > EntityAdapterBuilder::create(const std::wstring& name, const world::EntityData* entityData, const Object* instanceData)
{
	if (!entityData)
		return 0;

	// Resolve entity data; we cannot create external entities through external factory as we need to have both datas.
	Ref< const world::EntityData > realEntityData = resolveRealEntityData(m_context->getSourceDatabase(), entityData);
	T_ASSERT (realEntityData);

	// Save "outer" entity data in current adapter; need this information in order to determine entity editor etc..
	if (m_currentAdapter && !m_currentAdapter->getRealEntityData())
		m_currentAdapter->setRealEntityData(const_cast< world::EntityData* >(realEntityData.ptr()));

	// Find entity factory.
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	Ref< world::IEntityFactory > entityFactory;

	for (RefArray< world::IEntityFactory >::iterator i = m_entityFactories.begin(); i != m_entityFactories.end() && minClassDifference > 0; ++i)
	{
		const TypeInfoSet& typeSet = (*i)->getEntityTypes();
		for (TypeInfoSet::const_iterator j = typeSet.begin(); j != typeSet.end() && minClassDifference > 0; ++j)
		{
			if (is_type_of(**j, type_of(realEntityData)))
			{
				uint32_t classDifference = type_difference(**j, type_of(realEntityData));
				if (classDifference < minClassDifference)
				{
					minClassDifference = classDifference;
					entityFactory = *i;
				}
			}
		}
	}

	if (!entityFactory)
	{
		log::error << L"Unable to find entity factory for \"" << type_name(realEntityData) << L"\"" << Endl;
		return 0;
	}

	// Create entity from entity data through specialized factory.
	Ref< world::Entity > entity = entityFactory->createEntity(this, name, *realEntityData, instanceData);
	if (!entity)
	{
		log::error << L"Unable to create entity from \"" << type_name(realEntityData) << L"\"" << Endl;
		return 0;
	}

	// Add this entity to the manager.
	if (m_entityManager)
		m_entityManager->insertEntity(name, entity);

	return entity;
}

Ref< world::Entity > EntityAdapterBuilder::build(const world::EntityInstance* instance)
{
	if (!instance)
		return 0;

	Ref< EntityAdapter > entityAdapter;
	Ref< world::Entity > entity;

	RefArray< EntityAdapter >& cachedAdapters = m_cachedInstances[instance->getGuid()];
	if (!cachedAdapters.empty())
	{
		entityAdapter = cachedAdapters.front(); cachedAdapters.pop_front();
		entity = entityAdapter->getEntity();
	}
	else
	{
		entityAdapter = new EntityAdapter(const_cast< world::EntityInstance* >(instance));
	}

	if (!entity)
	{
		if (m_currentAdapter)
			m_currentAdapter->link(entityAdapter);
		else
		{
			T_ASSERT (!m_rootAdapter);
			m_rootAdapter = entityAdapter;
		}

		{
			T_ANONYMOUS_VAR(Save< Ref< EntityAdapter > >)(m_currentAdapter, entityAdapter);
			entity = create(instance->getName(), instance->getEntityData(), instance->getInstanceData());
		}

		entityAdapter->setEntity(entity);
	}

	m_builtInstances.insert(std::make_pair(instance, entity));
	return entity;
}

Ref< world::Entity > EntityAdapterBuilder::get(const world::EntityInstance* instance) const
{
	std::map< const world::EntityInstance*, Ref< world::Entity > >::const_iterator i = m_builtInstances.find(instance);
	return i != m_builtInstances.end() ? i->second : 0;
}

void EntityAdapterBuilder::end()
{
	T_ASSERT (m_currentAdapter == 0);
	m_cachedInstances.clear();
}

Ref< EntityAdapter > EntityAdapterBuilder::getRootAdapter() const
{
	return m_rootAdapter;
}

	}
}

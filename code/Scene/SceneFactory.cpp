#include "Scene/SceneFactory.h"
#include "Scene/SceneAsset.h"
#include "Scene/Scene.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/Entity.h"
#include "Database/Database.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneFactory", SceneFactory, resource::IResourceFactory)

SceneFactory::SceneFactory(db::Database* database, world::IEntityBuilder* entityBuilder)
:	m_database(database)
,	m_entityBuilder(entityBuilder)
{
}

const TypeSet SceneFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Scene >());
	return typeSet;
}

Object* SceneFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid, bool& outCacheable)
{
	Ref< SceneAsset > asset = m_database->getObjectReadOnly< SceneAsset >(guid);
	if (!asset)
		return 0;

	Ref< world::IEntityManager > entityManager = gc_new< world::EntityManager >();

	m_entityBuilder->begin(entityManager);

	Ref< world::Entity > rootEntity = m_entityBuilder->build(
		asset->getInstance()
	);
	
	m_entityBuilder->end();

	if (!rootEntity)
		return 0;

	return gc_new< Scene >(
		entityManager,
		rootEntity,
		asset->getWorldRenderSettings()
	);
}

	}
}

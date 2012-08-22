#include "Database/Database.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Scene/SceneResource.h"
#include "World/Entity/EntitySchema.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneFactory", SceneFactory, resource::IResourceFactory)

SceneFactory::SceneFactory(
	db::Database* database,
	render::IRenderSystem* renderSystem,
	world::IEntityBuilder* entityBuilder
)
:	m_database(database)
,	m_renderSystem(renderSystem)
,	m_entityBuilder(entityBuilder)
{
}

const TypeInfoSet SceneFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Scene >());
	return typeSet;
}

bool SceneFactory::isCacheable() const
{
	return false;
}

Ref< Object > SceneFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< SceneResource > sceneResource = m_database->getObjectReadOnly< SceneResource >(guid);
	if (!sceneResource)
		return 0;

	Ref< world::IEntitySchema > entitySchema = new world::EntitySchema();
	return sceneResource->createScene(
		resourceManager,
		m_renderSystem,
		m_entityBuilder,
		entitySchema
	);
}

	}
}

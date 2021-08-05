#include "Core/Misc/SafeDestroy.h"
#include "Scene/ISceneController.h"
#include "Scene/Scene.h"
#include "World/Entity.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.Scene", Scene, Object)

Scene::Scene(
	ISceneController* controller,
	world::Entity* rootEntity,
	world::WorldRenderSettings* worldRenderSettings
)
:	m_rootEntity(rootEntity)
,	m_controller(controller)
,	m_worldRenderSettings(worldRenderSettings)
{
}

Scene::Scene(ISceneController* controller, Scene* scene)
:	m_rootEntity(scene->m_rootEntity)
,	m_controller(controller)
,	m_worldRenderSettings(scene->m_worldRenderSettings)
{
}

Scene::~Scene()
{
	m_rootEntity = nullptr;
	m_controller = nullptr;
	m_worldRenderSettings = nullptr;
}

void Scene::destroy()
{
	safeDestroy(m_rootEntity);
	m_controller = nullptr;
	m_worldRenderSettings = nullptr;
}

void Scene::updateController(const world::UpdateParams& update)
{
	if (m_controller)
		m_controller->update(this, update.totalTime, update.deltaTime);
}

void Scene::updateEntity(const world::UpdateParams& update)
{
	if (m_rootEntity)
		m_rootEntity->update(update);
}

world::Entity* Scene::getRootEntity() const
{
	return m_rootEntity;
}

ISceneController* Scene::getController() const
{
	return m_controller;
}

world::WorldRenderSettings* Scene::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

	}
}

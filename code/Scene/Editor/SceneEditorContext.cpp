#include <limits>
#include <stack>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/DeepClone.h"
#include "Resource/IResourceManager.h"
#include "Scene/ISceneController.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Event.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/EntitySchema.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

// Find adapter by traverse entity adapter hierarchy.
template < typename Predicate >
EntityAdapter* findAdapter(EntityAdapter* entityAdapter, Predicate predicate)
{
	if (predicate(entityAdapter))
		return entityAdapter;

	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		EntityAdapter* foundEntityAdapter = findAdapter(*i, predicate);
		if (foundEntityAdapter)
			return foundEntityAdapter;
	}

	return 0;
}

// Find child adapter.
template < typename Predicate >
EntityAdapter* findChildAdapter(EntityAdapter* entityAdapter, Predicate predicate)
{
	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		if (predicate(*i))
			return *i;
	}
	return 0;
}

struct FindFromEntity
{
	const world::Entity* m_entity;

	FindFromEntity(const world::Entity* entity)
	:	m_entity(entity)
	{
	}

	bool operator () (const EntityAdapter* entityAdapter) const
	{
		return entityAdapter->getEntity() == m_entity;
	}
};

struct FindFromType
{
	const TypeInfo& m_entityDataType;

	FindFromType(const TypeInfo& entityDataType)
	:	m_entityDataType(entityDataType)
	{
	}

	bool operator () (const EntityAdapter* entityAdapter) const
	{
		return is_type_of(m_entityDataType, type_of(entityAdapter->getEntityData()));
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorContext", SceneEditorContext, ui::EventSubject)

SceneEditorContext::SceneEditorContext(
	editor::IEditor* editor,
	editor::IDocument* document,
	db::Database* resourceDb,
	db::Database* sourceDb,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	physics::PhysicsManager* physicsManager
)
:	m_editor(editor)
,	m_document(document)
,	m_resourceDb(resourceDb)
,	m_sourceDb(sourceDb)
,	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_physicsManager(physicsManager)
,	m_guideSize(2.0f)
,	m_pickEnable(true)
,	m_snapMode(SmNone)
,	m_snapSpacing(0.0f)
,	m_physicsEnable(false)
,	m_playing(false)
,	m_timeScale(1.0f)
,	m_time(0.0f)
{
	for (int i = 0; i < sizeof_array(m_cameras); ++i)
		m_cameras[i] = new Camera(
			lookAt(
				Vector4(-4.0f, 4.0f, -4.0f, 1.0f),
				Vector4(0.0f, 0.0f, 0.0f, 1.0f)
			)
		);
}

SceneEditorContext::~SceneEditorContext()
{
	destroy();
}

void SceneEditorContext::destroy()
{
	m_editor = 0;
	m_document = 0;
	m_resourceDb = 0;
	m_sourceDb = 0;
	m_resourceManager = 0;
	m_renderSystem = 0;
	for (int32_t i = 0; i < sizeof_array(m_debugTexture); ++i)
		m_debugTexture[i] = 0;
	m_physicsManager = 0;
	m_editorProfiles.clear();
	m_editorPlugins.clear();
	m_controllerEditor = 0;
	m_modifier = 0;
	for (int32_t i = 0; i < sizeof_array(m_cameras); ++i)
		m_cameras[i] = 0;
	m_sceneAsset = 0;
	m_scene = 0;
	m_rootEntityAdapter = 0;
	m_followEntityAdapter = 0;
	m_renderEntityStack.clear();
}

void SceneEditorContext::addEditorProfile(ISceneEditorProfile* editorProfile)
{
	m_editorProfiles.push_back(editorProfile);
}

void SceneEditorContext::addEditorPlugin(ISceneEditorPlugin* editorPlugin)
{
	m_editorPlugins.push_back(editorPlugin);
}

void SceneEditorContext::setControllerEditor(ISceneControllerEditor* controllerEditor)
{
	m_controllerEditor = controllerEditor;
}

void SceneEditorContext::setModifier(IModifier* modifier)
{
	if ((m_modifier = modifier) != 0)
		m_modifier->selectionChanged();
}

IModifier* SceneEditorContext::getModifier() const
{
	return m_modifier;
}

void SceneEditorContext::setGuideSize(float guideSize)
{
	m_guideSize = guideSize;
}

float SceneEditorContext::getGuideSize() const
{
	return m_guideSize;
}

void SceneEditorContext::setPickEnable(bool pickEnable)
{
	m_pickEnable = pickEnable;
}

bool SceneEditorContext::getPickEnable() const
{
	return m_pickEnable;
}

void SceneEditorContext::setSnapMode(SnapMode snapMode)
{
	m_snapMode = snapMode;
}

SceneEditorContext::SnapMode SceneEditorContext::getSnapMode() const
{
	return m_snapMode;
}

void SceneEditorContext::setSnapSpacing(float snapSpacing)
{
	m_snapSpacing = snapSpacing;
}

float SceneEditorContext::getSnapSpacing() const
{
	return m_snapSpacing;
}

void SceneEditorContext::setPhysicsEnable(bool physicsEnable)
{
	m_physicsEnable = physicsEnable;
}

bool SceneEditorContext::getPhysicsEnable() const
{
	return m_physicsEnable;
}

Camera* SceneEditorContext::getCamera(int index) const
{
	T_ASSERT (index >= 0)
	T_ASSERT (index < sizeof_array(m_cameras));
	return m_cameras[index];
}

void SceneEditorContext::setFollowEntityAdapter(EntityAdapter* followEntityAdapter)
{
	m_followEntityAdapter = followEntityAdapter;
}

void SceneEditorContext::setPlaying(bool playing)
{
	m_playing = playing;
}

bool SceneEditorContext::isPlaying() const
{
	return m_playing;
}

void SceneEditorContext::setTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float SceneEditorContext::getTimeScale() const
{
	return m_timeScale;
}

void SceneEditorContext::setTime(float time)
{
	m_time = time;
}

float SceneEditorContext::getTime() const
{
	return m_time;
}

void SceneEditorContext::drawGuide(render::PrimitiveRenderer* primitiveRenderer, EntityAdapter* entityAdapter)
{
	if (entityAdapter)
	{
		IEntityEditor* entityEditor = entityAdapter->getEntityEditor();
		if (entityEditor)
			entityEditor->drawGuide(primitiveRenderer);
	}
}

void SceneEditorContext::setSceneAsset(SceneAsset* sceneAsset)
{
	m_sceneAsset = sceneAsset;
}

void SceneEditorContext::buildEntities()
{
	if (m_sceneAsset)
	{
		// Create entity editor factories.
		RefArray< IEntityEditorFactory > entityEditorFactories;
		for (RefArray< ISceneEditorProfile >::iterator i = m_editorProfiles.begin(); i != m_editorProfiles.end(); ++i)
			(*i)->createEntityEditorFactories(this, entityEditorFactories);
		
		Ref< EntityAdapterBuilder > entityBuilder = new EntityAdapterBuilder(this, entityEditorFactories);

		// Create entity factories.
		for (RefArray< ISceneEditorProfile >::iterator i = m_editorProfiles.begin(); i != m_editorProfiles.end(); ++i)
		{
			RefArray< world::IEntityFactory > entityFactories;
			(*i)->createEntityFactories(this, entityFactories);

			for (RefArray< world::IEntityFactory >::iterator j = entityFactories.begin(); j != entityFactories.end(); ++j)
				entityBuilder->addFactory(*j);
		}

		// Create entity schema and build root instance.
		Ref< world::IEntitySchema > entitySchema = new world::EntitySchema();

		entityBuilder->begin(entitySchema);
		Ref< world::Entity > rootEntity = entityBuilder->create(m_sceneAsset->getEntityData());

		// Update scene controller also.
		Ref< ISceneController > controller;
		if (m_sceneAsset->getControllerData())
			controller = m_sceneAsset->getControllerData()->createController(entityBuilder, entitySchema);

		entityBuilder->end();
		log::debug << entityBuilder->getAdapterCount() << L" entity adapter(s) built" << Endl;

		// Save new root entity adapter.
		m_rootEntityAdapter = entityBuilder->getRootAdapter();

		// Bind post process settings.
		resource::Proxy< world::PostProcessSettings > postProcessSettings;
		m_resourceManager->bind(m_sceneAsset->getPostProcessSettings(), postProcessSettings);

		// Create our scene.
		m_scene = new Scene(
			controller,
			entitySchema,
			rootEntity,
			m_sceneAsset->getWorldRenderSettings(),
			postProcessSettings
		);
	}
	else
	{
		m_scene = 0;
		m_rootEntityAdapter = 0;
	}

	raisePostBuild();
}

void SceneEditorContext::selectEntity(EntityAdapter* entityAdapter, bool select)
{
	if (entityAdapter && entityAdapter->m_selected != select)
	{
		entityAdapter->m_selected = select;
		if (entityAdapter->m_entityEditor)
			entityAdapter->m_entityEditor->entitySelected(select);
	}
}

void SceneEditorContext::selectAllEntities(bool select)
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
		selectEntity(*i, select);
}

uint32_t SceneEditorContext::getEntities(RefArray< EntityAdapter >& outEntityAdapters, uint32_t flags) const
{
	typedef std::pair< RefArray< EntityAdapter >::const_iterator, RefArray< EntityAdapter >::const_iterator > range_t;

	outEntityAdapters.resize(0);

	if (!m_rootEntityAdapter)
		return 0;

	std::stack< range_t > stack;

	RefArray< EntityAdapter > rootEntityAdapters(1);
	rootEntityAdapters[0] = m_rootEntityAdapter;

	stack.push(std::make_pair(rootEntityAdapters.begin(), rootEntityAdapters.end()));
	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			EntityAdapter* entityAdapter = *r.first++;

			bool include = true;

			if (flags & GfSelectedOnly)
				include &= entityAdapter->isSelected();
			if (flags & GfNoSelected)
				include &= !entityAdapter->isSelected();

			if (flags & GfExternalOnly)
				include &= entityAdapter->isExternal();
			if (flags & GfNoExternal)
				include &= !entityAdapter->isExternal();

			if (flags & GfExternalChildOnly)
				include &= entityAdapter->isChildOfExternal();
			if (flags & GfNoExternalChild)
				include &= !entityAdapter->isChildOfExternal();

			if (include)
				outEntityAdapters.push_back(entityAdapter);

			if (flags & GfDescendants)
			{
				const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
				if (!children.empty())
					stack.push(std::make_pair(children.begin(), children.end()));
			}
		}
		else
			stack.pop();
	}

	return uint32_t(outEntityAdapters.size());
}

EntityAdapter* SceneEditorContext::findAdapterFromEntity(const world::Entity* entity) const
{
	return findAdapter(m_rootEntityAdapter, FindFromEntity(entity));
}

EntityAdapter* SceneEditorContext::findAdapterFromType(const TypeInfo& entityDataType) const
{
	return findAdapter(m_rootEntityAdapter, FindFromType(entityDataType));
}

EntityAdapter* SceneEditorContext::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, bool onlyPickable) const
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	EntityAdapter* minEntityAdapter = 0;
	Scalar minDistance(1e8f);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		// Must be unlocked, visible and no child of external.
		if ((*i)->isLocked() || !(*i)->isVisible() || (*i)->isChildOfExternal())
			continue;

		IEntityEditor* entityEditor = (*i)->getEntityEditor();
		if (!entityEditor)
			continue;

		// Do not trace non-pickable.
		if (onlyPickable && !entityEditor->isPickable())
			continue;

		// Trace bounding box to see if ray intersect.
		Scalar distance = minDistance;
		if (entityEditor->queryRay(worldRayOrigin, worldRayDirection, distance))
		{
			if (distance < minDistance)
			{
				minEntityAdapter = *i;
				minDistance = distance;
			}
		}
	}

	return minEntityAdapter;
}

void SceneEditorContext::cloneSelected()
{
	RefArray< EntityAdapter > selectedEntityAdapters;
	getEntities(selectedEntityAdapters, GfDescendants | GfSelectedOnly);
	if (selectedEntityAdapters.empty())
		return;

	for (RefArray< EntityAdapter >::iterator i = selectedEntityAdapters.begin(); i != selectedEntityAdapters.end(); ++i)
	{
		Ref< EntityAdapter > parentContainerGroup = (*i)->getParentContainerGroup();
		if (!parentContainerGroup)
			continue;

		Ref< world::EntityData > clonedEntityData = DeepClone((*i)->getEntityData()).create< world::EntityData >();
		T_ASSERT (clonedEntityData);

		Ref< EntityAdapter > clonedEntityAdapter = new EntityAdapter();
		clonedEntityAdapter->setEntityData(clonedEntityData);
		parentContainerGroup->addChild(clonedEntityAdapter);

		(*i)->m_selected = false;
		clonedEntityAdapter->m_selected = true;
	}

	buildEntities();
	raiseSelect(this);
}

EntityAdapter* SceneEditorContext::beginRenderEntity(const world::Entity* entity)
{
	EntityAdapter* currentRenderAdapter = m_rootEntityAdapter;

	if (!m_renderEntityStack.empty())
		currentRenderAdapter = m_renderEntityStack.back();

	if (currentRenderAdapter)
	{
		EntityAdapter* renderEntityAdapter = 0;

		if (currentRenderAdapter->getEntity() == entity)
			renderEntityAdapter = currentRenderAdapter;
		else
			renderEntityAdapter = findChildAdapter(currentRenderAdapter, FindFromEntity(entity));

		m_renderEntityStack.push_back(renderEntityAdapter);
		return renderEntityAdapter;
	}
	else
	{
		m_renderEntityStack.push_back(0);
		return 0;
	}
}

void SceneEditorContext::endRenderEntity()
{
	T_ASSERT (!m_renderEntityStack.empty());
	m_renderEntityStack.pop_back();
}

void SceneEditorContext::setDebugTexture(uint32_t index, render::ITexture* debugTexture)
{
	m_debugTexture[index] = debugTexture;
}

render::ITexture* SceneEditorContext::getDebugTexture(uint32_t index)
{
	return m_debugTexture[index];
}

ISceneEditorPlugin* SceneEditorContext::getEditorPluginOf(const TypeInfo& pluginType) const
{
	for (RefArray< ISceneEditorPlugin >::const_iterator i = m_editorPlugins.begin(); i != m_editorPlugins.end(); ++i)
	{
		if (&type_of(*i) == &pluginType)
			return *i;
	}
	return 0;
}

void SceneEditorContext::raisePreModify()
{
	raiseEvent(EiPreModify, 0);
}

void SceneEditorContext::raisePostModify()
{
	raiseEvent(EiPostModify, 0);
}

void SceneEditorContext::raisePostFrame(ui::Event* event)
{
	raiseEvent(EiPostFrame, event);
}

void SceneEditorContext::raisePostBuild()
{
	raiseEvent(EiPostBuild, 0);
}

void SceneEditorContext::raiseSelect(Object* item)
{
	// Notify modifier about selection change.
	if (m_modifier)
		m_modifier->selectionChanged();

	// Notify selection change event listeners.
	ui::Event event(this, item);
	raiseEvent(EiSelect, &event);
}

void SceneEditorContext::addPreModifyEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPreModify, eventHandler);
}

void SceneEditorContext::addPostModifyEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostModify, eventHandler);
}

void SceneEditorContext::addPostFrameEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostFrame, eventHandler);
}

void SceneEditorContext::addPostBuildEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiPostBuild, eventHandler);
}

void SceneEditorContext::addSelectEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiSelect, eventHandler);
}

	}
}

#include <limits>
#include <stack>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Timer/Timer.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Render/ITexture.h"
#include "Resource/IResourceManager.h"
#include "Scene/ISceneController.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityAdapterBuilder.h"
#include "Scene/Editor/IComponentEditorFactory.h"
#include "Scene/Editor/IEntityEditorFactory.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/LayerEntityEditor.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/Events/CameraMovedEvent.h"
#include "Scene/Editor/Events/FrameEvent.h"
#include "Scene/Editor/Events/MeasurementEvent.h"
#include "Scene/Editor/Events/ModifierChangedEvent.h"
#include "Scene/Editor/Events/PostBuildEvent.h"
#include "Scene/Editor/Events/PostFrameEvent.h"
#include "Scene/Editor/Events/PostModifyEvent.h"
#include "Scene/Editor/Events/PreModifyEvent.h"
#include "Scene/Editor/Events/RedrawEvent.h"
#include "Ui/Events/SelectionChangeEvent.h"
#include "World/Entity.h"
#include "World/EntityBuilder.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/EntitySchema.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/GroupComponent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorContext", SceneEditorContext, ui::EventSubject)

SceneEditorContext::SceneEditorContext(
	editor::IEditor* editor,
	editor::IDocument* document,
	db::Database* resourceDb,
	db::Database* sourceDb,
	world::EntityEventManager* eventManager,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	physics::PhysicsManager* physicsManager,
	script::IScriptContext* scriptContext
)
:	m_editor(editor)
,	m_document(document)
,	m_resourceDb(resourceDb)
,	m_sourceDb(sourceDb)
,	m_eventManager(eventManager)
,	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_physicsManager(physicsManager)
,	m_scriptContext(scriptContext)
,	m_guideSize(1.0f)
,	m_pickEnable(true)
,	m_snapMode(SmNone)
,	m_snapSpacing(0.0f)
,	m_physicsEnable(false)
,	m_playing(false)
,	m_timeScale(1.0f)
,	m_time(0.0f)
,	m_buildCount(0)
,	m_entityCount(0)
{
	for (int i = 0; i < sizeof_array(m_cameras); ++i)
		m_cameras[i] = new Camera();
}

SceneEditorContext::~SceneEditorContext()
{
	destroy();
}

void SceneEditorContext::destroy()
{
	safeDestroy(m_scene);
	safeDestroy(m_resourceManager);

	m_editor = nullptr;
	m_document = nullptr;
	m_resourceDb = nullptr;
	m_sourceDb = nullptr;
	m_eventManager = nullptr;
	m_renderSystem = nullptr;
	m_physicsManager = nullptr;
	m_editorProfiles.clear();
	m_editorPlugins.clear();
	m_entityEditorFactories.clear();
	m_componentEditorFactories.clear();
	m_controllerEditor = nullptr;

	if (m_modifier != nullptr)
	{
		m_modifier->deactivate();
		m_modifier = nullptr;
	}

	for (int32_t i = 0; i < sizeof_array(m_cameras); ++i)
		m_cameras[i] = nullptr;

	m_sceneAsset = nullptr;
	m_scene = nullptr;
	m_layerEntityAdapters.clear();
	m_entityAdapterMap.clear();

	removeAllEventHandlers();
}

void SceneEditorContext::addEditorProfile(ISceneEditorProfile* editorProfile)
{
	m_editorProfiles.push_back(editorProfile);
}

void SceneEditorContext::addEditorPlugin(ISceneEditorPlugin* editorPlugin)
{
	m_editorPlugins.push_back(editorPlugin);
}

void SceneEditorContext::createFactories()
{
	m_entityEditorFactories.resize(0);
	m_componentEditorFactories.resize(0);

	for (auto editorProfile : m_editorProfiles)
	{
		editorProfile->createEntityEditorFactories(this, m_entityEditorFactories);
		editorProfile->createComponentEditorFactories(this, m_componentEditorFactories);
	}
}

void SceneEditorContext::setControllerEditor(ISceneControllerEditor* controllerEditor)
{
	m_controllerEditor = controllerEditor;
}

void SceneEditorContext::setModifier(IModifier* modifier)
{
	if (m_modifier != nullptr)
		m_modifier->deactivate();

	if ((m_modifier = modifier) != nullptr)
	{
		if (m_modifier->activate())
			m_modifier->selectionChanged();
		else
			m_modifier = nullptr;
	}

	raiseModifierChanged();
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
	if (m_snapMode != snapMode)
	{
		m_snapMode = snapMode;
		if (m_modifier)
			m_modifier->selectionChanged();
	}
}

SceneEditorContext::SnapMode SceneEditorContext::getSnapMode() const
{
	return m_snapMode;
}

void SceneEditorContext::setSnapSpacing(float snapSpacing)
{
	if (m_snapSpacing != snapSpacing)
	{
		m_snapSpacing = snapSpacing;
		if (m_modifier)
			m_modifier->selectionChanged();
	}
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

void SceneEditorContext::resetPhysics()
{
	for (auto body : m_physicsManager->getBodies())
		body->reset();
}

Camera* SceneEditorContext::getCamera(int index) const
{
	T_ASSERT(index >= 0)
	T_ASSERT(index < sizeof_array(m_cameras));
	return m_cameras[index];
}

void SceneEditorContext::moveToEntityAdapter(EntityAdapter* entityAdapter)
{
	if (!entityAdapter)
		return;

	Aabb3 boundingBox = entityAdapter->getBoundingBox();
	if (boundingBox.empty())
		return;

	Scalar distance = boundingBox.getExtent().get(majorAxis3(boundingBox.getExtent())) * Scalar(3.0f);

	Transform T = entityAdapter->getTransform();

	for (uint32_t i = 0; i < sizeof_array(m_cameras); ++i)
	{
		T_ASSERT(m_cameras[i]);

		Vector4 P = T.translation() - m_cameras[i]->getOrientation() * Vector4(0.0f, 0.0f, distance, 0.0f);
		m_cameras[i]->place(P);
	}
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

void SceneEditorContext::setDrawGuide(const std::wstring& guideId, bool shouldDraw)
{
	m_drawGuide[guideId] = shouldDraw;
}

bool SceneEditorContext::shouldDrawGuide(const std::wstring& guideId) const
{
	auto it = m_drawGuide.find(guideId);
	return it != m_drawGuide.end() ? it->second : true;
}

void SceneEditorContext::setSceneAsset(SceneAsset* sceneAsset)
{
	m_sceneAsset = sceneAsset;
}

const IEntityEditorFactory* SceneEditorContext::findEntityEditorFactory(const TypeInfo& entityDataType) const
{
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	const IEntityEditorFactory* foundEntityEditorFactory = nullptr;

	for (auto entityEditorFactory : m_entityEditorFactories)
	{
		for (const auto& entityEditorDataType : entityEditorFactory->getEntityDataTypes())
		{
			if (is_type_of(*entityEditorDataType, entityDataType))
			{
				uint32_t classDifference = type_difference(*entityEditorDataType, entityDataType);
				if (classDifference < minClassDifference)
				{
					foundEntityEditorFactory = entityEditorFactory;
					minClassDifference = classDifference;
				}
			}
		}
	}

	return foundEntityEditorFactory;
}

const IComponentEditorFactory* SceneEditorContext::findComponentEditorFactory(const TypeInfo& componentDataType) const
{
	uint32_t minClassDifference = std::numeric_limits< uint32_t >::max();
	const IComponentEditorFactory* foundComponentEditorFactory = nullptr;

	for (auto componentEditorFactory : m_componentEditorFactories)
	{
		for (const auto& componentEditorDataType : componentEditorFactory->getComponentDataTypes())
		{
			if (is_type_of(*componentEditorDataType, componentDataType))
			{
				uint32_t classDifference = type_difference(*componentEditorDataType, componentDataType);
				if (classDifference < minClassDifference)
				{
					foundComponentEditorFactory = componentEditorFactory;
					minClassDifference = classDifference;
				}
			}
		}
	}

	return foundComponentEditorFactory;
}

void SceneEditorContext::buildEntities()
{
	// Reset physics before creating entities in case
	// a new body is created with an initial velocity etc.
	resetPhysics();

	if (m_sceneAsset)
	{
		Ref< world::EntitySchema > entitySchema = new world::EntitySchema();
		Ref< world::EntityBuilder > entityBuilder = new world::EntityBuilder();
		Ref< world::EntityBuilderWithSchema > entityBuilderSchema = new world::EntityBuilderWithSchema(entityBuilder, entitySchema);

		// Create entity factories.
		RefArray< const world::IEntityFactory > entityFactories;
		for (auto editorProfile : m_editorProfiles)
			editorProfile->createEntityFactories(this, entityFactories);

		// Create root group entity as scene instances doesn't have a concept of layers.
		Ref< world::Entity > rootEntity = new world::Entity();
		Ref< world::GroupComponent > rootGroup = new world::GroupComponent();
		rootEntity->setComponent(rootGroup);

		// Create entities from scene layers.
		RefArray< world::LayerEntityData > layers = m_sceneAsset->getLayers();
		RefArray< world::LayerEntityData >::iterator i = std::remove(layers.begin(), layers.end(), (world::LayerEntityData*)nullptr);
		layers.erase(i, layers.end());

		m_layerEntityAdapters.resize(layers.size());
		for (uint32_t i = 0; i < layers.size(); ++i)
		{
			world::LayerEntityData* layerEntityData = layers[i];
			T_ASSERT(layerEntityData);

			Ref< EntityAdapterBuilder > entityAdapterBuilder = new EntityAdapterBuilder(this, entityBuilderSchema, m_layerEntityAdapters[i]);
			for (auto entityFactory : entityFactories)
				entityAdapterBuilder->addFactory(entityFactory);

			Ref< world::Entity > entity = entityAdapterBuilder->create(layerEntityData);
			T_FATAL_ASSERT(entity != nullptr);

			m_layerEntityAdapters[i] = entityAdapterBuilder->getRootAdapter();
			T_FATAL_ASSERT(m_layerEntityAdapters[i] != nullptr);
			T_FATAL_ASSERT(m_layerEntityAdapters[i]->getEntityData() == layerEntityData);
			T_FATAL_ASSERT(m_layerEntityAdapters[i]->getEntity() == entity);
			T_FATAL_ASSERT(m_layerEntityAdapters[i]->getParent() == nullptr);

			rootGroup->addEntity(entity);

			log::debug << L"Layer " << i << L", cache hit " << entityAdapterBuilder->getCacheHit() << L", miss " << entityAdapterBuilder->getCacheMiss() << Endl;
		}

		// Update scene controller also.
		Ref< ISceneController > controller;
		if (m_sceneAsset->getControllerData())
		{
			RefArray< EntityAdapter > entityAdapters;
			getEntities(entityAdapters);

			SmallMap< const world::EntityData*, Ref< world::Entity > > entityProducts;
			for (auto entityAdapter : entityAdapters)
				entityProducts.insert(std::make_pair(
					entityAdapter->getEntityData(),
					entityAdapter->getEntity()
				));

			controller = m_sceneAsset->getControllerData()->createController(entityProducts, true);
		}

		// Bind post process parameters.
		SmallMap< render::handle_t, resource::Proxy< render::ITexture > > postProcessParams;
		for (const auto param : m_sceneAsset->getImageProcessParams())
		{
			if (!m_resourceManager->bind(param.second, postProcessParams[render::getParameterHandle(param.first)]))
				log::error << L"Unable to bind post processing parameter \"" << param.first << L"\"" << Endl;
		}

		// Create our scene.
		m_scene = new Scene(
			controller,
			entitySchema,
			rootEntity,
			m_sceneAsset->getWorldRenderSettings(),
			postProcessParams
		);
	}
	else
		m_scene = nullptr;

	// Create map from entity to adapter.
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	m_entityAdapterMap.clear();
	for (auto entityAdapter : entityAdapters)
	{
		const auto it = m_entityAdapterMap.find(entityAdapter->getEntity());
		T_FATAL_ASSERT(it == m_entityAdapterMap.end());
		m_entityAdapterMap.insert(entityAdapter->getEntity(), entityAdapter);
	}

	m_entityCount = entityAdapters.size();
	raisePostBuild();
	++m_buildCount;
}

void SceneEditorContext::buildController()
{
	T_ASSERT(m_scene);

	Ref< ISceneController > controller;
	if (m_sceneAsset->getControllerData())
	{
		RefArray< EntityAdapter > entityAdapters;
		getEntities(entityAdapters);

		SmallMap< const world::EntityData*, Ref< world::Entity > > entityProducts;
		for (auto entityAdapter : entityAdapters)
			entityProducts.insert(std::make_pair(
				entityAdapter->getEntityData(),
				entityAdapter->getEntity()
			));

		controller = m_sceneAsset->getControllerData()->createController(entityProducts, true);
	}

	// Create our scene.
	m_scene = new Scene(
		controller,
		m_scene
	);
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

	for (auto entityAdapter : entityAdapters)
		selectEntity(entityAdapter, select);
}

uint32_t SceneEditorContext::getEntities(RefArray< EntityAdapter >& outEntityAdapters, uint32_t flags) const
{
	typedef std::pair< RefArray< EntityAdapter >::const_iterator, RefArray< EntityAdapter >::const_iterator > range_t;

	outEntityAdapters.resize(0);

	if (m_layerEntityAdapters.empty())
		return 0;

	outEntityAdapters.reserve(4096);

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_layerEntityAdapters.begin(), m_layerEntityAdapters.end()));
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

uint32_t SceneEditorContext::findAdaptersOfType(const TypeInfo& entityType, RefArray< EntityAdapter >& outEntityAdapters, uint32_t flags) const
{
	typedef std::pair< RefArray< EntityAdapter >::const_iterator, RefArray< EntityAdapter >::const_iterator > range_t;

	outEntityAdapters.resize(0);

	if (m_layerEntityAdapters.empty())
		return 0;

	outEntityAdapters.reserve(4096);

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_layerEntityAdapters.begin(), m_layerEntityAdapters.end()));
	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			EntityAdapter* entityAdapter = *r.first++;

			if (
				is_type_of(entityType, type_of(entityAdapter->getEntity())) ||
				entityAdapter->getComponent(entityType) != 0
			)
			{
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
			}

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
	SmallMap< const world::Entity*, EntityAdapter* >::const_iterator i = m_entityAdapterMap.find(entity);
	return i != m_entityAdapterMap.end() ? i->second : nullptr;
}

EntityAdapter* SceneEditorContext::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, bool onlyPickable) const
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	EntityAdapter* minEntityAdapter = nullptr;
	Scalar minDistance(1e8f);

	for (auto entityAdapter : entityAdapters)
	{
		// Must be public, unlocked, visible and no child of external.
		if (
			entityAdapter->isPrivate() ||
			entityAdapter->isLocked() ||
			!entityAdapter->isVisible() ||
			entityAdapter->isChildOfExternal()
		)
			continue;

		IEntityEditor* entityEditor = entityAdapter->getEntityEditor();
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
				minEntityAdapter = entityAdapter;
				minDistance = distance;
			}
		}
	}

	return minEntityAdapter;
}

uint32_t SceneEditorContext::queryFrustum(const Frustum& worldFrustum, RefArray< EntityAdapter >& outEntityAdapters, bool onlyPickable) const
{
	RefArray< EntityAdapter > entityAdapters;
	getEntities(entityAdapters);

	for (auto entityAdapter : entityAdapters)
	{
		// Must be public, unlocked, visible and no child of external.
		if (
			entityAdapter->isPrivate() ||
			entityAdapter->isLocked() ||
			!entityAdapter->isVisible() ||
			entityAdapter->isChildOfExternal()
		)
			continue;

		IEntityEditor* entityEditor = entityAdapter->getEntityEditor();
		if (!entityEditor)
			continue;

		// Do not trace non-pickable.
		if (onlyPickable && !entityEditor->isPickable())
			continue;

		// Query if entity inside frustum.
		if (entityEditor->queryFrustum(worldFrustum))
			outEntityAdapters.push_back(entityAdapter);
	}

	return (uint32_t)outEntityAdapters.size();
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
		T_ASSERT(clonedEntityData);

		Ref< EntityAdapter > clonedEntityAdapter = new EntityAdapter(this);
		clonedEntityAdapter->prepare(clonedEntityData, 0, 0);
		parentContainerGroup->addChild(clonedEntityAdapter);

		(*i)->m_selected = false;
		clonedEntityAdapter->m_selected = true;
	}

	buildEntities();
	raiseSelect();
}

ISceneEditorPlugin* SceneEditorContext::getEditorPluginOf(const TypeInfo& pluginType) const
{
	for (auto editorPlugin : m_editorPlugins)
	{
		if (&type_of(editorPlugin) == &pluginType)
			return editorPlugin;
	}
	return nullptr;
}

void SceneEditorContext::raisePreModify()
{
	PreModifyEvent preModifyEvent(this);
	raiseEvent(&preModifyEvent);
}

void SceneEditorContext::raisePostModify()
{
	PostModifyEvent postModifyEvent(this);
	raiseEvent(&postModifyEvent);
}

void SceneEditorContext::raisePostFrame(ui::Event* event)
{
	PostFrameEvent postFrameEvent(this);
	raiseEvent(&postFrameEvent);
}

void SceneEditorContext::raisePostBuild()
{
	PostBuildEvent postBuildEvent(this);
	raiseEvent(&postBuildEvent);
}

void SceneEditorContext::raiseSelect()
{
	// Notify modifier about selection change.
	if (m_modifier)
		m_modifier->selectionChanged();

	// Notify selection change event listeners.
	ui::SelectionChangeEvent selectionChangeEvent(this);
	raiseEvent(&selectionChangeEvent);
}

void SceneEditorContext::raiseCameraMoved()
{
	CameraMovedEvent cameraMovedEvent(this);
	raiseEvent(&cameraMovedEvent);
}

void SceneEditorContext::raiseModifierChanged()
{
	ModifierChangedEvent modifierChangedEvent(this);
	raiseEvent(&modifierChangedEvent);
}

void SceneEditorContext::raiseRedraw()
{
	RedrawEvent redrawEvent(this);
	raiseEvent(&redrawEvent);
}

void SceneEditorContext::raiseMeasurement(int32_t pass, const std::wstring& name, double start, double duration)
{
	MeasurementEvent measurementEvent(this, pass, name, start, duration);
	raiseEvent(&measurementEvent);
}

	}
}

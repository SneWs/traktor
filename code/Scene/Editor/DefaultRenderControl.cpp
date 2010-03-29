#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/Settings.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Scene/Editor/DebugRenderControl.h"
#include "Scene/Editor/DefaultRenderControl.h"
#include "Scene/Editor/OrthogonalRenderControl.h"
#include "Scene/Editor/PerspectiveRenderControl.h"
#include "Scene/Editor/ReferencesRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/PopupMenu.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"

// Resources
#include "Resources/SceneEdit.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultRenderControl", DefaultRenderControl, ISceneRenderControl)

bool DefaultRenderControl::create(ui::Widget* parent, SceneEditorContext* context, int32_t index)
{
	m_context = context;
	T_ASSERT (m_context);

	m_index = index;

	Ref< Settings > settings = context->getEditor()->getSettings();
	T_ASSERT (settings);

	int32_t viewType = settings->getProperty< PropertyInteger >(L"SceneEditor.View" + toString(index), 0);
	bool postProcessEnable = settings->getProperty< PropertyBoolean >(L"Scene.Editor.PostProcessEnable" + toString(index), true);

	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsClientBorder, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	if (!m_toolBar->create(m_container))
		return false;

	m_toolBar->addImage(ui::Bitmap::load(c_ResourceSceneEdit, sizeof(c_ResourceSceneEdit), L"png"), 17);

	m_toolView = new ui::custom::ToolBarDropDown(ui::Command(L"Scene.Editor.View"), 100, i18n::Text(L"SCENE_EDITOR_VIEW_MODE"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_PERSPECTIVE"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_FRONT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BACK"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_TOP"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BOTTOM"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_LEFT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_RIGHT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_REFERENCES"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_DEBUG"));
	m_toolView->select(viewType);

	m_toolToggleGrid = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_GRID"),
		ui::Command(1, L"Scene.Editor.ToggleGrid"),
		16,
		ui::custom::ToolBarButton::BsDefaultToggled
	);

	m_toolToggleGuide = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_GUIDE"),
		ui::Command(1, L"Scene.Editor.ToggleGuide"),
		5,
		ui::custom::ToolBarButton::BsDefaultToggled
	);

	m_toolTogglePostProcess = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_POSTPROCESS"),
		ui::Command(1, L"Scene.Editor.TogglePostProcess"),
		6,
		postProcessEnable ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle
	);

	m_toolBar->addItem(m_toolView);
	m_toolBar->addItem(m_toolToggleGrid);
	m_toolBar->addItem(m_toolToggleGuide);
	m_toolBar->addItem(m_toolTogglePostProcess);
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &DefaultRenderControl::eventToolClick));

	createRenderControl(viewType);

	return true;
}

void DefaultRenderControl::destroy()
{
	m_toolView = 0;

	if (m_renderControl)
	{
		m_renderControl->destroy();
		m_renderControl = 0;
	}

	if (m_container)
	{
		m_container->destroy();
		m_container = 0;
	}
}

void DefaultRenderControl::updateWorldRenderer()
{
	if (m_renderControl)
		m_renderControl->updateWorldRenderer();
}

bool DefaultRenderControl::handleCommand(const ui::Command& command)
{
	if (m_renderControl)
		return m_renderControl->handleCommand(command);
	else
		return false;
}

void DefaultRenderControl::update()
{
	if (m_renderControl)
		m_renderControl->update();
}

void DefaultRenderControl::createRenderControl(int32_t type)
{
	if (m_renderControl)
	{
		m_renderControl->destroy();
		m_renderControl = 0;
	}

	switch (type)
	{
	case 0:
		{
			Ref< PerspectiveRenderControl > renderControl = new PerspectiveRenderControl();
			if (renderControl->create(m_container, m_context, m_index))
				m_renderControl = renderControl;
		}
		break;

	case 1:	// Front
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveZ))
				m_renderControl = renderControl;
		}
		break;

	case 2:	// Back
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeZ))
				m_renderControl = renderControl;
		}
		break;

	case 3:	// Top
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveY))
				m_renderControl = renderControl;
		}
		break;

	case 4:	// Bottom
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeY))
				m_renderControl = renderControl;
		}
		break;

	case 5:	// Left
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveX))
				m_renderControl = renderControl;
		}
		break;

	case 6:	// Right
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeX))
				m_renderControl = renderControl;
		}
		break;

	case 7:	// References
		{
			Ref< ReferencesRenderControl > renderControl = new ReferencesRenderControl();
			if (renderControl->create(m_container, m_context))
				m_renderControl = renderControl;
		}
		break;

	case 8:	// Debug
		{
			Ref< DebugRenderControl > renderControl = new DebugRenderControl();
			if (renderControl->create(m_container, m_context))
				m_renderControl = renderControl;
		}
		break;
	}

	T_ASSERT (m_renderControl);

	m_container->update();
	m_renderControl->updateWorldRenderer();

	if (m_toolToggleGrid->isToggled())
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGrid"));
	else
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGrid"));

	if (m_toolToggleGuide->isToggled())
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGuide"));
	else
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGuide"));

	if (m_toolTogglePostProcess->isToggled())
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnablePostProcess"));
	else
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisablePostProcess"));

	Ref< Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	settings->setProperty< PropertyInteger >(L"SceneEditor.View" + toString(m_index), type);
}

void DefaultRenderControl::eventToolClick(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent*, false >(event);
	if (cmdEvent->getCommand() == L"Scene.Editor.View")
	{
		int32_t selected = m_toolView->getSelected();
		T_ASSERT (selected >= 0);
		createRenderControl(selected);
	}
	else if (cmdEvent->getCommand() == L"Scene.Editor.ToggleGrid")
	{
		if (m_toolToggleGrid->isToggled())
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGrid"));
		else
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGrid"));
	}
	else if (cmdEvent->getCommand() == L"Scene.Editor.ToggleGuide")
	{
		if (m_toolToggleGuide->isToggled())
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGuide"));
		else
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGuide"));
	}
	else if (cmdEvent->getCommand() == L"Scene.Editor.TogglePostProcess")
	{
		Ref< Settings > settings = m_context->getEditor()->getSettings();
		T_ASSERT (settings);

		if (m_toolTogglePostProcess->isToggled())
		{
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnablePostProcess"));
			settings->setProperty< PropertyBoolean >(L"SceneEditor.PostProcessEnable" + toString(m_index), true);
		}
		else
		{
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisablePostProcess"));
			settings->setProperty< PropertyBoolean >(L"SceneEditor.PostProcessEnable" + toString(m_index), false);
		}
	}
}

	}
}

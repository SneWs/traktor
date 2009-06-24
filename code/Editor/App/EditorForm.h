#ifndef traktor_editor_EditorForm_H
#define traktor_editor_EditorForm_H

#include "Core/Heap/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Guid.h"
#include "Editor/IEditor.h"
#include "Ui/Form.h"
#include "Ui/Command.h"

namespace traktor
{

class CommandLine;
class Thread;

	namespace ui
	{

class ShortcutTable;
class Dock;
class DockPane;
class MenuBar;
class PopupMenu;
class MenuItem;
class Tab;
class Event;

		namespace custom
		{

class ToolBar;
class StatusBar;
class ProgressBar;

		}
	}

	namespace db
	{

class Database;

	}

	namespace editor
	{

class DatabaseView;
class PropertiesView;
class HeapView;
class LogView;
class Settings;
class Project;
class IEditorPageFactory;
class IEditorPage;
class IObjectEditorFactory;
class IObjectEditor;
class IEditorTool;
class PipelineHash;

/*! \brief Main editor form.
 *
 * This is the surrounding form containing editor pages and the
 * database view.
 */
class EditorForm
:	public ui::Form
,	public IEditor
{
	T_RTTI_CLASS(EditorForm)

public:
	bool create(const CommandLine& cmdLine);

	void destroy();

	/*! \name IEditor implementation */
	//@{

	virtual Settings* getSettings();

	virtual IProject* getProject();

	virtual render::RenderSystem* getRenderSystem();

	virtual void setPropertyObject(Object* properties);

	virtual Object* getPropertyObject();

	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south);

	virtual void destroyAdditionalPanel(ui::Widget* widget);

	virtual void showAdditionalPanel(ui::Widget* widget);

	virtual void hideAdditionalPanel(ui::Widget* widget);

	virtual const Type* browseType(const Type* base);

	virtual db::Instance* browseInstance(const IBrowseFilter* filter);

	virtual bool isEditable(const Type& type) const;

	virtual bool openEditor(db::Instance* instance);

	virtual IEditorPage* getActiveEditorPage();

	virtual void setActiveEditorPage(IEditorPage* editorPage);

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild);

	virtual void buildAsset(const Guid& assetGuid, bool rebuild);

	virtual void buildAssets(bool rebuild);

	//@}

private:
	RefArray< IEditorPageFactory > m_editorPageFactories;
	RefArray< IObjectEditorFactory > m_objectEditorFactories;
	RefArray< IEditorTool > m_editorTools;
	std::list< ui::Command > m_shortcutCommands;
	Ref< ui::ShortcutTable > m_shortcutTable;
	Ref< ui::Dock > m_dock;
	Ref< ui::DockPane > m_paneAdditionalEast;
	Ref< ui::DockPane > m_paneAdditionalSouth;
	RefArray< ui::Widget > m_otherPanels;
	Ref< ui::MenuBar > m_menuBar;
	Ref< ui::MenuItem > m_menuItemOtherPanels;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::StatusBar > m_statusBar;
	Ref< ui::custom::ProgressBar > m_buildProgress;
	Ref< ui::Tab > m_tab;
	Ref< ui::PopupMenu > m_menuTab;
	Ref< ui::MenuItem > m_menuTools;
	Ref< DatabaseView > m_dataBaseView;
	Ref< PropertiesView > m_propertiesView;
	Ref< HeapView > m_heapView;
	Ref< LogView > m_logView;
	Ref< render::RenderSystem > m_renderSystem;
	Ref< Settings > m_settings;
	Ref< Project > m_project;
	Ref< IEditorPage > m_activeEditorPage;
	Thread* m_threadBuild;

	void buildAssetsThread(std::vector< Guid > assetGuids, bool rebuild);

	void updateTitle();

	void updateOtherPanels();

	void newProject();

	void openProject();

	bool closeProject();

	void updateProjectViews();

	void saveCurrentDocument();

	void saveAllDocuments();

	void closeCurrentEditor();

	void closeAllEditors();

	void closeAllOtherEditors();

	void activatePreviousEditor();

	void activateNextEditor();

	Settings* loadSettings(const std::wstring& settingsFile);

	void saveSettings(const std::wstring& settingsFile);

	void loadDictionary();

	PipelineHash* loadPipelineHash();

	void savePipelineHash(PipelineHash* pipelineHash);

	void checkModified();

	bool currentModified();

	bool anyModified();

	bool handleCommand(const ui::Command& command);

	/*! \name Event handlers. */
	//@{

	void eventShortcut(ui::Event* event);

	void eventMenuClick(ui::Event* event);

	void eventToolClicked(ui::Event* event);

	void eventTabButtonDown(ui::Event* event);

	void eventTabSelChange(ui::Event* event);

	void eventTabClose(ui::Event* event);

	void eventClose(ui::Event* event);

	void eventTimer(ui::Event* event);

	//@}
};

	}
}

#endif	// traktor_editor_EditorForm_H

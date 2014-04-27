#ifndef traktor_script_ScriptEditor_H
#define traktor_script_ScriptEditor_H

#include <list>
#include "Editor/IObjectEditor.h"
#include "Script/IScriptManager.h"
#include "Script/Editor/IScriptDebuggerSessions.h"
#include "Script/Editor/IScriptOutline.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Bitmap;
class Event;
class ListBox;
class Tab;

		namespace custom
		{

class GridRow;
class GridView;
class Splitter;
class SyntaxRichEdit;
class StatusBar;
class ToolBar;

		}
	}

	namespace script
	{

class IScriptContext;
class IScriptManager;
class Preprocessor;
class Script;

class T_DLLCLASS ScriptEditor
:	public editor::IObjectEditor
,	public IErrorCallback
,	public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	ScriptEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual ui::Size getPreferredSize() const;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< Script > m_script;
	Ref< IScriptDebuggerSessions > m_scriptDebuggerSessions;
	Ref< IScriptManager > m_scriptManager;
	Ref< IScriptOutline > m_scriptOutline;
	Ref< Preprocessor > m_preprocessor;
	Ref< ui::Bitmap > m_bitmapFunction;
	Ref< ui::Bitmap > m_bitmapFunctionReference;
	Ref< ui::custom::Splitter > m_splitter;
	Ref< ui::custom::GridView > m_outlineGrid;
	Ref< ui::ListBox > m_dependencyList;
	Ref< ui::custom::SyntaxRichEdit > m_edit;
	Ref< ui::custom::StatusBar > m_compileStatus;
	Ref< ui::Tab > m_tabSessions;
	std::wstring m_findNeedle;
	int32_t m_compileCountDown;


	/*! \name IErrorCallback */
	/*! \{ */

	virtual void syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message);

	virtual void otherError(const std::wstring& message);

	/*! \} */


	/*! \name IScriptDebuggerSessions::IListener */
	/*! \{ */

	virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler);

	virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler);

	virtual void notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual void notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber);

	/*! \} */


	void updateDependencyList();

	void buildOutlineGrid(ui::custom::GridView* grid, ui::custom::GridRow* parent, const IScriptOutline::Node* on);

	void eventOutlineDoubleClick(ui::Event* event);

	void eventDependencyToolClick(ui::Event* event);

	void eventDependencyListDoubleClick(ui::Event* event);

	void eventDebuggerToolClick(ui::Event* event);

	void eventToolBarEditClick(ui::Event* event);

	void eventScriptChange(ui::Event* event);

	void eventScriptDoubleClick(ui::Event* event);

	void eventTimer(ui::Event* event);

	void eventBreakPoint(ui::Event* event);
};

	}
}

#endif	// traktor_script_ScriptEditor_H

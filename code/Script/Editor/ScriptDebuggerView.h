#ifndef traktor_script_ScriptDebuggerView_H
#define traktor_script_ScriptDebuggerView_H

#include "Script/IScriptDebugger.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Command;

		namespace custom
		{

class GridRow;
class GridRowStateChangeEvent;
class GridView;
class ToolBar;
class ToolBarButtonClickEvent;

		}
	}

	namespace script
	{

class Variable;

class ScriptDebuggerView
:	public ui::Container
,	public IScriptDebugger::IListener
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerView(editor::IEditor* editor, IScriptDebugger* scriptDebugger);

	virtual ~ScriptDebuggerView();

	bool create(ui::Widget* parent);

	void destroy();

	bool handleCommand(const ui::Command& command);

private:
	editor::IEditor* m_editor;
	Ref< IScriptDebugger > m_scriptDebugger;
	Ref< ui::custom::ToolBar > m_debuggerTools;
	Ref< ui::custom::GridView > m_callStackGrid;
	Ref< ui::custom::GridView > m_localsGrid;
	RefArray< StackFrame > m_stackFrames;
	int32_t m_activeFrame;

	Ref< ui::custom::GridRow > createVariableRow(const Variable* local);

	void updateLocals(int32_t depth);

	/*! \name IScriptDebugger::IListener */
	/*! \{ */

	virtual void debugeeStateChange(IScriptDebugger* scriptDebugger);

	/*! \} */

	void eventDebuggerToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventCallStackGridDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventLocalsGridStateChange(ui::custom::GridRowStateChangeEvent* event);
};

	}
}

#endif	// traktor_script_ScriptDebuggerView_H

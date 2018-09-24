/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
class GridRow;
class GridRowStateChangeEvent;
class GridView;
class Menu;
class ToolBar;
class ToolBarButtonClickEvent;

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

	virtual void destroy() T_OVERRIDE T_FINAL;

	bool handleCommand(const ui::Command& command);

private:
	editor::IEditor* m_editor;
	Ref< IScriptDebugger > m_scriptDebugger;
	Ref< ui::ToolBar > m_debuggerTools;
	Ref< ui::GridView > m_callStackGrid;
	Ref< ui::GridView > m_localsGrid;
	Ref< ui::Menu > m_localsPopup;
	RefArray< StackFrame > m_stackFrames;
	int32_t m_activeFrame;

	Ref< ui::GridRow > createVariableRow(const Variable* local);

	void updateLocals(int32_t depth);

	/*! \name IScriptDebugger::IListener */
	/*! \{ */

	virtual void debugeeStateChange(IScriptDebugger* scriptDebugger) T_OVERRIDE T_FINAL;

	/*! \} */

	void eventDebuggerToolClick(ui::ToolBarButtonClickEvent* event);

	void eventCallStackGridDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventLocalsGridStateChange(ui::GridRowStateChangeEvent* event);

	void eventLocalsGridButtonDown(ui::MouseButtonDownEvent* event);
};

	}
}

#endif	// traktor_script_ScriptDebuggerView_H

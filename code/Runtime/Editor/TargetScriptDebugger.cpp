/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Runtime/Editor/TargetScriptDebugger.h"
#include "Runtime/Target/ScriptDebuggerBreadcrumbs.h"
#include "Runtime/Target/ScriptDebuggerBreakpoint.h"
#include "Runtime/Target/ScriptDebuggerControl.h"
#include "Runtime/Target/ScriptDebuggerLocals.h"
#include "Runtime/Target/ScriptDebuggerStateChange.h"
#include "Runtime/Target/ScriptDebuggerStackFrame.h"
#include "Runtime/Target/ScriptDebuggerStatus.h"

namespace traktor::runtime
{
	namespace
	{

const int32_t c_timeout = 30000;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetScriptDebugger", TargetScriptDebugger, script::IScriptDebugger)

TargetScriptDebugger::TargetScriptDebugger(net::BidirectionalObjectTransport* transport)
:	m_transport(transport)
,	m_running(true)
{
}

void TargetScriptDebugger::update()
{
	Ref< ScriptDebuggerStateChange > stateChange;
	while (m_transport->recv< ScriptDebuggerStateChange >(0, stateChange) == net::BidirectionalObjectTransport::Result::Success)
	{
		m_running = stateChange->isRunning();
		for (auto listener : m_listeners)
			listener->debugeeStateChange(this);
	}
}

bool TargetScriptDebugger::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	const ScriptDebuggerBreakpoint bp(true, scriptId, lineNumber);
	if (!m_transport->send(&bp))
	{
		log::error << L"Target script debugger error; Unable to send while setting breakpoint." << Endl;
		return false;
	}
	return true;
}

bool TargetScriptDebugger::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	const ScriptDebuggerBreakpoint bp(false, scriptId, lineNumber);
	if (!m_transport->send(&bp))
	{
		log::error << L"Target script debugger error; Unable to send while setting breakpoint." << Endl;
		return false;
	}
	return true;
}

bool TargetScriptDebugger::captureStackFrame(uint32_t depth, Ref< script::StackFrame >& outStackFrame)
{
	const ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureStack, depth);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while requesting stack frame." << Endl;
		return false;
	}

	Ref< ScriptDebuggerStackFrame > sf;
	if (m_transport->recv< ScriptDebuggerStackFrame >(c_timeout, sf) != net::BidirectionalObjectTransport::Result::Success)
	{
		log::error << L"Target script debugger error; No response while requesting stack frame." << Endl;
		return false;
	}

	outStackFrame = sf->getFrame();
	return outStackFrame != nullptr;
}

bool TargetScriptDebugger::captureLocals(uint32_t depth, RefArray< script::Variable >& outLocals)
{
	const ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureLocals, depth);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while requesting locals." << Endl;
		return false;
	}

	Ref< ScriptDebuggerLocals > l;
	if (m_transport->recv< ScriptDebuggerLocals >(c_timeout, l) != net::BidirectionalObjectTransport::Result::Success)
	{
		log::error << L"Target script debugger error; No response while requesting locals." << Endl;
		return false;
	}

	outLocals = l->getLocals();
	return true;
}

bool TargetScriptDebugger::captureObject(uint32_t object, RefArray< script::Variable >& outMembers)
{
	const ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureObject, object);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while requesting object capture." << Endl;
		return false;
	}

	Ref< ScriptDebuggerLocals > l;
	if (m_transport->recv< ScriptDebuggerLocals >(c_timeout, l) != net::BidirectionalObjectTransport::Result::Success)
	{
		log::error << L"Target script debugger error; No response while requesting object capture." << Endl;
		return false;
	}

	outMembers = l->getLocals();
	return true;
}

bool TargetScriptDebugger::captureBreadcrumbs(AlignedVector< uint32_t >& outBreadcrumbs)
{
	const ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcCaptureBreadcrumbs);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while requesting breadcrumbs capture." << Endl;
		return false;
	}

	Ref< ScriptDebuggerBreadcrumbs> bc;
	if (m_transport->recv< ScriptDebuggerBreadcrumbs >(c_timeout, bc) != net::BidirectionalObjectTransport::Result::Success)
	{
		log::error << L"Target script debugger error; No response while requesting breadcrumbs capture." << Endl;
		return false;
	}

	outBreadcrumbs = bc->getBreadcrumbs();
	return true;
}

bool TargetScriptDebugger::isRunning() const
{
	return m_running;
}

bool TargetScriptDebugger::actionBreak()
{
	const ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcBreak);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while break target." << Endl;
		return false;
	}
	return true;
}

bool TargetScriptDebugger::actionContinue()
{
	const ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcContinue);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while continue target." << Endl;
		return false;
	}
	return true;
}

bool TargetScriptDebugger::actionStepInto()
{
	const ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepInto);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while step-into target." << Endl;
		return false;
	}
	return true;
}

bool TargetScriptDebugger::actionStepOver()
{
	const ScriptDebuggerControl ctrl(ScriptDebuggerControl::AcStepOver);
	if (!m_transport->send(&ctrl))
	{
		log::error << L"Target script debugger error; Unable to send while step-over target." << Endl;
		return false;
	}
	return true;
}

void TargetScriptDebugger::addListener(IListener* listener)
{
	T_ASSERT(listener);
	m_listeners.push_back(listener);
	listener->debugeeStateChange(this);
}

void TargetScriptDebugger::removeListener(IListener* listener)
{
	T_ASSERT(listener);
	m_listeners.remove(listener);
}

}

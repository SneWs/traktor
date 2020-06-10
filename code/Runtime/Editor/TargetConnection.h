#pragma once

#include "Runtime/Target/TargetPerformance.h"
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Profiler.h"

namespace traktor
{

class ILogTarget;

	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace script
	{

class IScriptDebuggerSessions;

	}

	namespace runtime
	{

class TargetScriptDebugger;
class TargetScriptProfiler;

/*! \brief
 * \ingroup Runtime
 */
class TargetConnection : public Object
{
	T_RTTI_CLASS;

public:
	struct IProfilerEventsCallback
	{
		virtual void receivedProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary) = 0;

		virtual void receivedProfilerEvents(double, const AlignedVector< Profiler::Event >& events) = 0;
	};

	TargetConnection(
		const std::wstring& name,
		net::BidirectionalObjectTransport* transport,
		ILogTarget* targetLog,
		script::IScriptDebuggerSessions* targetDebuggerSessions
	);

	virtual ~TargetConnection();

	void destroy();

	void shutdown();

	bool update();

	const std::wstring& getName() const { return m_name; }

	net::BidirectionalObjectTransport* getTransport() const { return m_transport; }

	const TargetPerformance& getPerformance() const { return m_performance; }

	void setProfilerEventsCallback(IProfilerEventsCallback* profilerEventsCallback);

private:
	std::wstring m_name;
	Ref< net::BidirectionalObjectTransport > m_transport;
	Ref< ILogTarget > m_targetLog;
	Ref<script::IScriptDebuggerSessions> m_targetDebuggerSessions;
	Ref< TargetScriptDebugger > m_targetDebugger;
	Ref< TargetScriptProfiler > m_targetProfiler;
	TargetPerformance m_performance;
	IProfilerEventsCallback* m_profilerEventsCallback;
	Semaphore m_lock;
};

	}
}


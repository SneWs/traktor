#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Core/Timer/Profiler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! Target profiler events from running target.
 * \ingroup Runtime
 */
class T_DLLCLASS TargetProfilerEvents : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetProfilerEvents() = default;

	explicit TargetProfilerEvents(double currentTime, const Profiler::eventQueue_t& events);

	double getCurrentTime() const { return m_currentTime; }

	const Profiler::eventQueue_t& getEvents() const { return m_events; }

	virtual void serialize(ISerializer& s) override final;

private:
	double m_currentTime = 0.0;
	Profiler::eventQueue_t m_events;
};

	}
}


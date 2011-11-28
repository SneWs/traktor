#include "Core/Log/Log.h"
#include "Online/Impl/Tasks/TaskEnumStatistics.h"
#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskEnumStatistics", TaskEnumStatistics, ITask)

TaskEnumStatistics::TaskEnumStatistics(
	IStatisticsProvider* provider,
	Object* sinkObject,
	sink_method_t sinkMethod
)
:	m_provider(provider)
,	m_sinkObject(sinkObject)
,	m_sinkMethod(sinkMethod)
{
}

void TaskEnumStatistics::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	log::debug << L"Online; Begin enumerating statistics" << Endl;
	std::map< std::wstring, float > statistics;
	m_provider->enumerate(statistics);
	(m_sinkObject->*m_sinkMethod)(statistics);
	log::debug << L"Online; Finished enumerating statistics" << Endl;
}

	}
}

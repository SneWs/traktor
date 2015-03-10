#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/CompactSerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Net/Replication/INetworkTopology.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/ReplicatorProxy.h"
#include "Net/Replication/ReplicatorTypes.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const double c_resendTimeThreshold = 0.5;
const int32_t c_resendCountThreshold = 8;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReplicatorProxy", ReplicatorProxy, Object)

net_handle_t ReplicatorProxy::getHandle() const
{
	return m_handle;
}

const std::wstring& ReplicatorProxy::getName() const
{
	return m_name;
}

Object* ReplicatorProxy::getUser() const
{
	return m_user;
}

uint8_t ReplicatorProxy::getStatus() const
{
	return m_status;
}

bool ReplicatorProxy::isLatencyReliable() const
{
	return m_roundTrips.full();
}

double ReplicatorProxy::getLatency() const
{
	return m_latency;
}

double ReplicatorProxy::getLatencySpread() const
{
	return m_latencyStandardDeviation;
}

double ReplicatorProxy::getReverseLatency() const
{
	return m_latencyReverse;
}

double ReplicatorProxy::getReverseLatencySpread() const
{
	return m_latencyReverseStandardDeviation;
}

void ReplicatorProxy::resetLatencies()
{
	m_roundTrips.clear();
	m_latency = 0.0;
	m_latencyStandardDeviation = 0.0;
	m_latencyReverse = 0.0;
	m_latencyReverseStandardDeviation = 0.0;
}

bool ReplicatorProxy::isConnected() const
{
	return m_handle != 0;
}

bool ReplicatorProxy::setPrimary()
{
	if (m_replicator)
		return m_replicator->m_topology->setPrimaryHandle(m_handle);
	else
		return false;
}

bool ReplicatorProxy::isPrimary() const
{
	if (m_replicator)
		return m_replicator->m_topology->getPrimaryHandle() == m_handle;
	else
		return false;
}

bool ReplicatorProxy::isRelayed() const
{
	if (m_replicator)
	{
		int32_t nodeCount = m_replicator->m_topology->getNodeCount();
		for (int32_t i = 0; i < nodeCount; ++i)
		{
			if (m_replicator->m_topology->getNodeHandle(i) == m_handle)
				return m_replicator->m_topology->isNodeRelayed(i);
		}
		return false;
	}
	else
		return false;
}

void ReplicatorProxy::setObject(Object* object)
{
	m_object = object;
}

Object* ReplicatorProxy::getObject() const
{
	return m_object;
}

void ReplicatorProxy::setOrigin(const Transform& origin)
{
	m_origin = origin;
}

const Transform& ReplicatorProxy::getOrigin() const
{
	return m_origin;
}

void ReplicatorProxy::setStateTemplate(const StateTemplate* stateTemplate)
{
	m_stateTemplate = stateTemplate;

	// Old states must be immediately discarded; we cannot keep
	// states produced from old template.
	resetStates();
}

const StateTemplate* ReplicatorProxy::getStateTemplate() const
{
	return m_stateTemplate;
}

Ref< const State > ReplicatorProxy::getState(double time, double limit) const
{
	if (m_stateTemplate)
	{
		double k = m_stateReceivedTime - m_stateTime0;
		double offset = std::max(k - limit, 0.0);

		return m_stateTemplate->extrapolate(
			m_stateN2,
			float(m_stateTimeN2 + offset),
			m_stateN1,
			float(m_stateTimeN1 + offset),
			m_state0,
			float(m_stateTime0 + offset),
			float(time)
		);
	}
	else
		return 0;
}

Ref< const State > ReplicatorProxy::getFilteredState(double time, double limit, const State* currentState, float filterCoeff) const
{
	if (m_stateTemplate)
	{
		double k = m_stateReceivedTime - m_stateTime0;
		double offset = std::max(k - limit, 0.0);

		Ref< const State > extrapolatedState = m_stateTemplate->extrapolate(
			m_stateN2,
			float(m_stateTimeN2 + offset),
			m_stateN1,
			float(m_stateTimeN1 + offset),
			m_state0,
			float(m_stateTime0 + offset),
			float(time)
		);
		if (!extrapolatedState)
			return 0;

		if (currentState)
			return m_stateTemplate->extrapolate(
				0, 0.0f,
				extrapolatedState, 0.0f,
				currentState, 1.0f,
				filterCoeff
			);
		else
			return extrapolatedState;
	}
	else
		return 0;
}

void ReplicatorProxy::resetStates()
{
	m_state0 = 0;
	m_stateTime0 = 0.0;
	m_stateN1 = 0;
	m_stateTimeN1 = 0.0;
	m_stateN2 = 0;
	m_stateTimeN2 = 0.0;
}

void ReplicatorProxy::setSendState(bool sendState)
{
	m_sendState = sendState;
}

void ReplicatorProxy::sendEvent(const ISerializable* eventObject)
{
	Event e;

	e.msg.id = RmiEvent;
	e.msg.time = time2net(m_replicator->m_time);
	e.msg.event.sequence = m_sequence++;

	MemoryStream ms(e.msg.event.data, RmiEvent_MaxEventSize(), false, true);
	CompactSerializer cs(&ms, &m_replicator->m_eventTypes[0], m_replicator->m_eventTypes.size());
	if (!cs.writeObject(eventObject))
	{
		log::error << m_replicator->getLogPrefix() << L"Unable to wrap event object of " << type_name(eventObject) << L"." << Endl;
		return;
	}
	cs.flush();

	e.size = ms.tell();
	e.time = -c_resendTimeThreshold;
	e.count = 0;

	m_unacknowledgedEvents.push_back(e);
}

bool ReplicatorProxy::updateEventQueue()
{
	for (std::list< Event >::iterator i = m_unacknowledgedEvents.begin(); i != m_unacknowledgedEvents.end(); )
	{
		if (m_replicator->m_time0 - i->time > c_resendTimeThreshold)
		{
			m_replicator->m_topology->send(m_handle, &i->msg, RmiEvent_NetSize(i->size));
			i->time = m_replicator->m_time0;

			if (++i->count >= c_resendCountThreshold)
			{
				i = m_unacknowledgedEvents.erase(i);
				continue;
			}
		}
		++i;
	}
	return !m_unacknowledgedEvents.empty();
}

bool ReplicatorProxy::receivedEventAcknowledge(const ReplicatorProxy* from, uint8_t sequence)
{
	for (std::list< Event >::iterator i = m_unacknowledgedEvents.begin(); i != m_unacknowledgedEvents.end(); ++i)
	{
		if (i->msg.event.sequence == sequence)
		{
#if defined(_DEBUG)
			m_acknowledgeHistory.push_back(sequence);
#endif
			m_unacknowledgedEvents.erase(i);
			return true;
		}
	}

#if defined(_DEBUG)
	if (m_acknowledgeHistory.find(sequence) < 0)
		log::info << m_replicator->getLogPrefix() << L"Received acknowledge of unsent event from " << from->m_handle << L", sequence " << int32_t(sequence) << Endl;
#endif

	return false;
}

bool ReplicatorProxy::acceptEvent(uint8_t sequence, const ISerializable* eventObject)
{
	for (uint32_t i = 0; i < m_lastEvents.size(); ++i)
	{
		if (m_lastEvents[i].first == sequence)
			return false;
	}
	m_lastEvents.push_back(std::make_pair(sequence, 0));
	return true;
}

void ReplicatorProxy::updateLatency(double roundTrip, double latencyReverse, double latencyReverseSpread)
{
	// Keep circular buffer of latest round trips.
	m_roundTrips.push_back(roundTrip);

	double denom = 1.0 / double(m_roundTrips.size());

	// Calculate average latency.
	double sum = 0;
	for (uint32_t i = 0; i < m_roundTrips.size(); ++i)
		sum += m_roundTrips[i];
	m_latency = (sum / 2.0) * denom;

	// Calculate standard deviation.
	double sumDiff = 0;
	for (uint32_t i = 0; i < m_roundTrips.size(); ++i)
	{
		double diff = m_roundTrips[i] / 2.0 - m_latency;
		sumDiff += diff * diff;
	}
	m_latencyStandardDeviation = std::sqrt(sumDiff * denom);

	// Save reverse metrics.
	m_latencyReverse = latencyReverse;
	m_latencyReverseStandardDeviation = latencyReverseSpread;
}

bool ReplicatorProxy::receivedState(double stateTime, const void* stateData, uint32_t stateDataSize)
{
	if (!m_stateTemplate)
	{
		log::info << m_replicator->getLogPrefix() << L"Received state from " << m_handle << L" but no state template registered; state ignored." << Endl;
		return false;
	}

	Ref< const State > state = m_stateTemplate->unpack(stateData, stateDataSize);
	if (!state)
	{
		log::info << m_replicator->getLogPrefix() << L"Failed to unpack state from " << m_handle << L"; state ignored." << Endl;
		return false;
	}

	m_stateReceivedTime = m_replicator->getTime();

	if (stateTime >= m_stateTime0)
	{
		m_stateN2 = m_stateN1;
		m_stateTimeN2 = m_stateTimeN1;
		m_stateN1 = m_state0;
		m_stateTimeN1 = m_stateTime0;
		m_state0 = state;
		m_stateTime0 = stateTime;
	}
	else if (stateTime >= m_stateTimeN1)
	{
		m_stateN2 = m_stateN1;
		m_stateTimeN2 = m_stateTimeN1;
		m_stateN1 = state;
		m_stateTimeN1 = stateTime;
	}
	else if (stateTime >= m_stateTimeN2)
	{
		m_stateN2 = state;
		m_stateTimeN2 = stateTime;
	}
	else
	{
		log::info << m_replicator->getLogPrefix() << L"Received old state (" << int32_t((m_stateTimeN2 - stateTime) * 1000.0) << L" ms) from " << m_handle << L"; state ignored." << Endl;
		return false;
	}

	return true;
}

void ReplicatorProxy::disconnect()
{
	m_replicator = 0;
	m_handle = 0;
	m_user = 0;
	m_status = 0;
	m_object = 0;
	m_distance = 0.0f;
	m_sendState = false;
	m_sequence = 0;
	m_timeUntilTxPing = 0.0;
	m_timeUntilTxState = 0.0;
	m_latency = 0.0;
	m_latencyStandardDeviation = 0.0;
	m_latencyReverse = 0.0;
	m_latencyReverseStandardDeviation = 0.0;
	m_unacknowledgedEvents.clear();
	m_lastEvents.clear();
}

ReplicatorProxy::ReplicatorProxy(Replicator* replicator, net_handle_t handle, const std::wstring& name, Object* user)
:	m_replicator(replicator)
,	m_handle(handle)
,	m_name(name)
,	m_user(user)
,	m_status(0)
,	m_origin(Transform::identity())
,	m_distance(0.0f)
,	m_sendState(false)
,	m_stateTimeN2(0.0)
,	m_stateTimeN1(0.0)
,	m_stateTime0(0.0)
,	m_stateReceivedTime(0.0)
,	m_sequence(0)
,	m_timeUntilTxPing(0.0)
,	m_timeUntilTxState(0.0)
,	m_latency(0.0)
,	m_latencyStandardDeviation(0.0)
,	m_latencyReverse(0.0)
,	m_latencyReverseStandardDeviation(0.0)
{
}

	}
}

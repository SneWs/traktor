#ifndef traktor_net_ReplicatorProxy_H
#define traktor_net_ReplicatorProxy_H

#include <list>
#include "Core/Object.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Transform.h"
#include "Net/Replication/NetworkTypes.h"
#include "Net/Replication/ReplicatorTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace net
	{

class INetworkTopology;
class State;
class StateTemplate;

class T_DLLCLASS ReplicatorProxy : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief
	 */
	net_handle_t getHandle() const;

	/*! \brief
	 */
	const std::wstring& getName() const;

	/*! \brief
	 */
	uint8_t getStatus() const;

	/*! \brief
	 */
	double getLatency() const;

	/*! \brief
	 */
	double getLatencyUp() const;

	/*! \brief
	 */
	double getLatencyDown() const;

	/*! \brief
	 */
	bool isConnected() const;

	/*! \brief
	 */
	bool setPrimary();

	/*! \brief
	 */
	bool isPrimary() const;

	/*! \brief
	 */
	bool isRelayed() const;

	/*! \brief
	 */
	void setObject(Object* object);

	/*! \brief
	 */
	Object* getObject() const;

	/*! \brief
	 */
	void setOrigin(const Transform& origin);

	/*! \brief
	 */
	const Transform& getOrigin() const;

	/*! \brief
	 */
	void setStateTemplate(const StateTemplate* stateTemplate);

	/*! \brief
	 */
	const StateTemplate* getStateTemplate() const;

	/*! \brief
	 */
	Ref< const State > getState(double time, double limit) const;

	/*! \brief
	 */
	void resetStates();

	/*! \brief
	 */
	void setSendState(bool sendState);

	/*! \brief Send high priority event to this ghost.
	 */
	void sendEvent(const ISerializable* eventObject);

private:
	friend class Replicator;

	struct Event
	{
		double time;
		uint32_t size;
		uint32_t count;
		RMessage msg;
	};

	Replicator* m_replicator;
	net_handle_t m_handle;

	/*! \group Proxy information. */
	//@{

	std::wstring m_name;
	uint8_t m_status;
	Ref< Object > m_object;
	Transform m_origin;
	float m_distance;
	bool m_sendState;

	//@}

	/*! \group Shadow states. */
	//@{
	 
	Ref< const StateTemplate > m_stateTemplate;
	Ref< const State > m_stateN2;
	double m_stateTimeN2;
	Ref< const State > m_stateN1;
	double m_stateTimeN1;
	Ref< const State > m_state0;
	double m_stateTime0;
	double m_stateReceivedTime;

	//@}

	/*! \group Event management. */
	//@{

	uint8_t m_sequence;
	std::list< Event > m_events;
	CircularVector< std::pair< uint8_t, uint32_t >, 128 > m_lastEvents;
	CircularVector< uint8_t, 16 > m_acknowledgeHistory;

	//@}

	/*! \group Time measurements. */
	//@{

	double m_timeUntilTxPing;
	double m_timeUntilTxState;
	CircularVector< double, 17 > m_roundTrips;
	double m_latencyMedian;
	double m_latencyReverse;

	// @}

	bool updateEventQueue();

	bool receivedEventAcknowledge(const ReplicatorProxy* from, uint8_t sequence);

	bool acceptEvent(uint8_t sequence, const ISerializable* eventObject);

	void updateLatency(double roundTrip, double reverseLatency);

	bool receivedState(double stateTime, const void* stateData, uint32_t stateDataSize);

	void disconnect();

	ReplicatorProxy(Replicator* replicator, net_handle_t handle, const std::wstring& name);
};

	}
}

#endif	// traktor_net_ReplicatorProxy_H

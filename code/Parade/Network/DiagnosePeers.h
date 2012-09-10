#ifndef traktor_parade_DiagnosePeers_H
#define traktor_parade_DiagnosePeers_H

#include "Core/Timer/Timer.h"
#include "Parade/Network/IReplicatorPeers.h"

namespace traktor
{
	namespace parade
	{

class DiagnosePeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	DiagnosePeers(IReplicatorPeers* peers);

	virtual ~DiagnosePeers();

	virtual void destroy();

	virtual void update();

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const;

	virtual std::wstring getPeerName(handle_t handle) const;

	virtual bool receiveAnyPending();

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool sendReady(handle_t handle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

	virtual bool isPrimary() const;

private:
	Ref< IReplicatorPeers > m_peers;
	Timer m_timer;
	double m_lastT;
	uint32_t m_sent;
	uint32_t m_received;
};

	}
}

#endif	// traktor_parade_DiagnosePeers_H

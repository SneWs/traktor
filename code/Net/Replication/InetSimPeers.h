#ifndef traktor_net_InetSimPeers_H
#define traktor_net_InetSimPeers_H

#include <map>
#include "Net/Replication/IReplicatorPeers.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS InetSimPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	InetSimPeers(IReplicatorPeers* peers);

	virtual ~InetSimPeers();

	virtual void destroy();

	virtual bool update();

	virtual void setStatus(uint8_t status);

	virtual void setConnectionState(uint64_t connectionState);

	virtual handle_t getHandle() const;

	virtual std::wstring getName() const;

	virtual handle_t getPrimaryPeerHandle() const;

	virtual bool setPrimaryPeerHandle(handle_t handle);

	virtual uint32_t getPeers(std::vector< PeerInfo >& outPeers) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

	void setPeerConnectionState(handle_t peer, bool sendEnable, bool receiveEnable);

private:
	Ref< IReplicatorPeers > m_peers;
	std::map< handle_t, uint32_t > m_state;
};

	}
}

#endif	// traktor_net_InetSimPeers_H

#pragma once

#include <string>
#include "Net/Platform.h"
#include "Net/SocketAddress.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct addrinfo;

namespace traktor
{
	namespace net
	{

/*! IPv6 socket address.
 * \ingroup Net
 */
class T_DLLCLASS SocketAddressIPv6 : public SocketAddress
{
	T_RTTI_CLASS;

public:
	SocketAddressIPv6() = default;

	SocketAddressIPv6(uint16_t port);

	SocketAddressIPv6(const std::wstring& host, uint16_t port);

	virtual ~SocketAddressIPv6();

	virtual bool valid() const override final;

	virtual std::wstring getHostName() const override final;

	const addrinfo* getAddrInfo(int socktype) const;

private:
	addrinfo* m_info = nullptr;
};

	}
}


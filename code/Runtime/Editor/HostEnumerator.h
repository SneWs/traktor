#pragma once

#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

class PropertyGroup;

	namespace net
	{

class DiscoveryManager;

	}

	namespace runtime
	{

/*! Enumerator of hosts able to communicate with editor.
 * \ingroup Runtime
 */
class HostEnumerator : public Object
{
	T_RTTI_CLASS;

public:
	HostEnumerator(const PropertyGroup* settings, net::DiscoveryManager* discoveryManager);

	int32_t count() const;

	const std::wstring& getHost(int32_t index) const;

	const std::wstring& getDescription(int32_t index) const;

	bool supportPlatform(int32_t index, const std::wstring& platform) const;

	bool isLocal(int32_t index) const;

	void update();

private:
	struct Host
	{
		std::wstring description;
		std::wstring host;
		std::vector< std::wstring > platforms;
		bool local;

		Host();

		bool operator < (const Host& h) const;
	};

	Ref< net::DiscoveryManager > m_discoveryManager;
	mutable Semaphore m_lock;
	std::vector< Host > m_manual;
	mutable std::vector< Host > m_hosts;
	mutable std::vector< Host > m_pending;
};

	}
}


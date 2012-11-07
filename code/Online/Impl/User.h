#ifndef traktor_online_User_H
#define traktor_online_User_H

#include "Online/IUser.h"

namespace traktor
{
	namespace online
	{

class IUserProvider;

class User : public IUser
{
	T_RTTI_CLASS;

public:
	virtual bool getName(std::wstring& outName) const;

	virtual int32_t getTag() const;

	virtual bool isFriend() const;

	virtual bool invite();

	virtual bool setPresenceValue(const std::wstring& key, const std::wstring& value);

	virtual bool getPresenceValue(const std::wstring& key, std::wstring& outValue) const;

	virtual bool sendP2PData(const void* data, size_t size);

private:
	friend class Lobby;
	friend class UserCache;

	Ref< IUserProvider > m_userProvider;
	uint64_t m_handle;
	int32_t m_tag;

	User(IUserProvider* userProvider, uint64_t handle, int32_t tag);
};

	}
}

#endif	// traktor_online_User_H

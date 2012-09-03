#ifndef traktor_online_Lobby_H
#define traktor_online_Lobby_H

#include "Online/ILobby.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class TaskQueue;
class UserCache;

class Lobby : public ILobby
{
	T_RTTI_CLASS;

public:
	virtual ~Lobby();

	virtual Ref< Result > setMetaValue(const std::wstring& key, const std::wstring& value);

	virtual bool getMetaValue(const std::wstring& key, std::wstring& outValue) const;

	virtual Ref< Result > setParticipantMetaValue(const std::wstring& key, const std::wstring& value);

	virtual bool getParticipantMetaValue(const IUser* user, const std::wstring& key, std::wstring& outValue) const;

	virtual Ref< Result > join();

	virtual bool leave();

	virtual RefArray< IUser > getParticipants();

	virtual uint32_t getParticipantCount() const;

	virtual int32_t getIndex() const;

	virtual bool isOwner() const;

private:
	friend class TaskCreateLobby;
	friend class TaskFindMatchingLobbies;

	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	Ref< TaskQueue > m_taskQueue;
	uint64_t m_handle;

	Lobby(IMatchMakingProvider* matchMakingProvider, UserCache* userCache, TaskQueue* taskQueue, uint64_t handle);
};

	}
}

#endif	// traktor_online_Lobby_H

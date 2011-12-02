#ifndef traktor_online_TaskCreateLobby_H
#define traktor_online_TaskCreateLobby_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class LobbyResult;
class UserCache;

class TaskCreateLobby : public ITask
{
	T_RTTI_CLASS;

public:
	TaskCreateLobby(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		uint32_t maxUsers,
		LobbyResult* result
	);

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	uint32_t m_maxUsers;
	Ref< LobbyResult > m_result;
};

	}
}

#endif	// traktor_online_TaskCreateLobby_H

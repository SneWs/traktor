#ifndef traktor_online_SessionLocal_H
#define traktor_online_SessionLocal_H

#include "Online/ISession.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_PSN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class UserPsn;

class T_DLLCLASS SessionPsn : public ISession
{
	T_RTTI_CLASS;

public:
	SessionPsn(UserPsn* user);

	virtual void destroy();

	virtual bool isConnected() const;

	virtual Ref< IUser > getUser();

	virtual bool rewardAchievement(const std::wstring& achievementId);

	virtual bool withdrawAchievement(const std::wstring& achievementId);

	virtual Ref< ILeaderboard > getLeaderboard(const std::wstring& id);

	virtual bool setStatValue(const std::wstring& statId, float value);

	virtual bool getStatValue(const std::wstring& statId, float& outValue);

	virtual Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment);

	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const;

private:
	Ref< UserPsn > m_user;
};

	}
}

#endif	// traktor_online_SessionLocal_H

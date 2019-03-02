#pragma once

#include <list>
#include <steam/steam_api.h>
#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	SteamAchievements(SteamSessionManager* sessionManager, const std::list< std::wstring >& achievementIds);

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) override final;

	virtual bool set(const std::wstring& achievementId, bool reward) override final;

private:
	SteamSessionManager* m_sessionManager;
	std::set< std::wstring > m_achievementIds;

	STEAM_CALLBACK(SteamAchievements, OnAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);
};

	}
}


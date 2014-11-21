#import <GameKit/GameKit.h>
#include "Core/Log/Log.h"
#include "Net/Url.h"
#include "Online/Gc/GcAchievements.h"
#include "Online/Gc/GcGameConfiguration.h"
#include "Online/Gc/GcLeaderboards.h"
#include "Online/Gc/GcMatchMaking.h"
#include "Online/Gc/GcSaveData.h"
#include "Online/Gc/GcSessionManager.h"
#include "Online/Gc/GcStatistics.h"
#include "Online/Gc/GcUser.h"
#include "Online/Gc/GcVideoSharingEveryplay.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.GcSessionManager", 0, GcSessionManager, ISessionManagerProvider)

bool GcSessionManager::create(const IGameConfiguration* configuration)
{
	const GcGameConfiguration* gcgc = checked_type_cast< const GcGameConfiguration*, false >(configuration);

	// Ensure presence of game center API.
	Class gcClass = (NSClassFromString(@"GKLocalPlayer"));
	if (!gcClass)
	{
		log::error << L"GameCenter API missing; unable to create GameCenter session manager" << Endl;
		return false;
	}
	
	// Ensure iOS 4.1 or later.
	NSString* requiredVersion = @"4.1";
	NSString* currentVersion = [[UIDevice currentDevice] systemVersion];
	BOOL versionSupported = ([currentVersion compare: requiredVersion options:NSNumericSearch] != NSOrderedAscending);
	if (!versionSupported)	
	{
		log::error << L"Old iOS version; at least 4.1 is required" << Endl;
		return false;
	}

	// Authenticate user.
	if ([GKLocalPlayer localPlayer].authenticated == NO)
	{
		GKLocalPlayer* localPlayer = [GKLocalPlayer localPlayer];
		localPlayer.authenticateHandler = ^(UIViewController *viewController, NSError *error)
		{
			if (viewController != nil)
			{
				UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;				
				[rootViewController presentViewController: viewController animated: YES completion: nil];
			}
			else
			{
				if ([GKLocalPlayer localPlayer].authenticated)
				{
					if (error == nil)
						NSLog(@"LOCAL PLAYER AUTHENTICATED");
					else
						NSLog(@"LOCAL PLAYER NOT AUTHENTICATED");
				}
			}
		};
	}
	
	// Create provider wrappers.
	m_achievements = new GcAchievements(gcgc->m_achievementIds);
	m_leaderboards = new GcLeaderboards(gcgc->m_leaderboardIds);
//	m_matchMaking = new GcMatchMaking();
	m_saveData = new GcSaveData();
//	m_statistics = new GcStatistics();
//	m_user = new GcUser();

	// Create video sharing.
	Ref< GcVideoSharingEveryplay > videoSharing = new GcVideoSharingEveryplay();
	if (videoSharing->create(*gcgc))
		m_videoSharing = videoSharing;

	return true;
}

void GcSessionManager::destroy()
{
	m_videoSharing = 0;
	m_user = 0;
	m_statistics = 0;
	m_saveData = 0;
	m_matchMaking = 0;
	m_leaderboards = 0;
	m_achievements = 0;
}

bool GcSessionManager::update()
{
	return true;
}

std::wstring GcSessionManager::getLanguageCode() const
{
	return L"en";
}

bool GcSessionManager::isConnected() const
{
	return true;
}

bool GcSessionManager::requireUserAttention() const
{
	return false;
}

bool GcSessionManager::haveDLC(const std::wstring& id) const
{
	return false;
}

bool GcSessionManager::buyDLC(const std::wstring& id) const
{
	return false;
}

bool GcSessionManager::navigateUrl(const net::Url& url) const
{
	if (url.getString() == "gamecenter://default")
	{
		GKGameCenterViewController* gameCenterController = [[GKGameCenterViewController alloc] init];
		if (gameCenterController != nil)
		{
			//gameCenterController.gameCenterDelegate = self;
			gameCenterController.viewState = GKGameCenterViewControllerStateDefault;

			UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;				
			[rootViewController presentViewController: gameCenterController animated: YES completion: nil];
		}		
	}
	if (url.getString() == "gamecenter://leaderboards")
	{
		GKGameCenterViewController* gameCenterController = [[GKGameCenterViewController alloc] init];
		if (gameCenterController != nil)
		{
			//gameCenterController.gameCenterDelegate = self;
			gameCenterController.viewState = GKGameCenterViewControllerStateLeaderboards;

			UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;				
			[rootViewController presentViewController: gameCenterController animated: YES completion: nil];
		}		
	}
	else if (url.getString() == "gamecenter://achievements")
	{
		GKGameCenterViewController* gameCenterController = [[GKGameCenterViewController alloc] init];
		if (gameCenterController != nil)
		{
			//gameCenterController.gameCenterDelegate = self;
			gameCenterController.viewState = GKGameCenterViewControllerStateAchievements;

			UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;				
			[rootViewController presentViewController: gameCenterController animated: YES completion: nil];
		}		
	}
	else if (url.getString() == "gamecenter://challenges")
	{
		GKGameCenterViewController* gameCenterController = [[GKGameCenterViewController alloc] init];
		if (gameCenterController != nil)
		{
			//gameCenterController.gameCenterDelegate = self;
			gameCenterController.viewState = GKGameCenterViewControllerStateChallenges;

			UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;				
			[rootViewController presentViewController: gameCenterController animated: YES completion: nil];
		}		
	}
	else
		return false;

	return true;
}

uint64_t GcSessionManager::getCurrentUserHandle() const
{
	return 0;
}

bool GcSessionManager::getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const
{
	return false;
}

bool GcSessionManager::findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const
{
	return false;
}

bool GcSessionManager::haveP2PData() const
{
	return false;
}

uint32_t GcSessionManager::receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const
{
	return 0;
}

uint32_t GcSessionManager::getCurrentGameCount() const
{
	return 0;
}

IAchievementsProvider* GcSessionManager::getAchievements() const
{
	return m_achievements;
}

ILeaderboardsProvider* GcSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

IMatchMakingProvider* GcSessionManager::getMatchMaking() const
{
	return m_matchMaking;
}

ISaveDataProvider* GcSessionManager::getSaveData() const
{
	return m_saveData;
}

IStatisticsProvider* GcSessionManager::getStatistics() const
{
	return m_statistics;
}

IUserProvider* GcSessionManager::getUser() const
{
	return m_user;
}

IVideoSharingProvider* GcSessionManager::getVideoSharing() const
{
	return m_videoSharing;
}

	}
}

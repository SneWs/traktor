#ifndef traktor_online_Types_H
#define traktor_online_Types_H

#include <string>

namespace traktor
{
	namespace online
	{

enum LobbyAccess
{
	LaPublic = 0,
	LaPrivate = 1,
	LaFriends = 2
};

struct SaveDataDesc
{
	std::wstring title;
	std::wstring description;
};

	}
}

#endif	// traktor_online_Types_H

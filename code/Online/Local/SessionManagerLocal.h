#ifndef traktor_online_SessionManagerLocal_H
#define traktor_online_SessionManagerLocal_H

#include "Online/ISessionManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_LOCAL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class UserLocal;

class T_DLLCLASS SessionManagerLocal : public ISessionManager
{
	T_RTTI_CLASS(SessionManagerLocal)

public:
	SessionManagerLocal();

	virtual bool getAvailableUsers(RefArray< IUser >& outUsers);

	virtual Ref< IUser > getCurrentUser();

	virtual Ref< ISession > createSession(IUser* user);

private:
	Ref< UserLocal > m_currentUser;
};

	}
}

#endif	// traktor_online_SessionManagerLocal_H

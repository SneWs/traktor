#ifndef traktor_db_DbmGetEvent_H
#define traktor_db_DbmGetEvent_H

#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Get event from bus.
 * \ingroup Database
 */
class T_DLLCLASS DbmGetEvent : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmGetEvent(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual void serialize(ISerializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_DbmGetEvent_H

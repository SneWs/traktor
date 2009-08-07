#ifndef traktor_db_DbmGetDataNames_H
#define traktor_db_DbmGetDataNames_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class DbmGetDataNames : public IMessage
{
	T_RTTI_CLASS(DbmGetDataNames)

public:
	DbmGetDataNames(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_DbmGetDataNames_H

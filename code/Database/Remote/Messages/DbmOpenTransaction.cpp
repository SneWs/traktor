#include "Database/Remote/Messages/DbmOpenTransaction.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmOpenTransaction", 0, DbmOpenTransaction, IMessage)

DbmOpenTransaction::DbmOpenTransaction(uint32_t handle)
:	m_handle(handle)
{
}

bool DbmOpenTransaction::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}

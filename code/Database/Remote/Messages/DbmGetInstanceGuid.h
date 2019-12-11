#pragma once

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

/*! Get instance's guid.
 * \ingroup Database
 */
class T_DLLCLASS DbmGetInstanceGuid : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmGetInstanceGuid(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_handle;
};

	}
}


#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class ConnectionString;
class IProviderBus;
class IProviderGroup;

/*! Provider database interface.
 * \ingroup Database
 */
class T_DLLCLASS IProviderDatabase : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create database using connection string.
	 *
	 * \param connectionString Connection string.
	 * \return True if database was created successfully.
	 */
	virtual bool create(const ConnectionString& connectionString) = 0;

	/*! Open existing database using connection string.
	 *
	 * \param connectionString Connection string.
	 * \return True if database was succefully opened.
	 */
	virtual bool open(const ConnectionString& connectionString) = 0;

	/*! Close connection to database. */
	virtual void close() = 0;

	/*! Get event bus in database.
	 *
	 * \return Event bus interface.
	 */
	virtual Ref< IProviderBus > getBus() = 0;

	/*! Get database's root group.
	 *
	 * \return Root group.
	 */
	virtual Ref< IProviderGroup > getRootGroup() = 0;
};

	}
}


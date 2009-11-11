#ifndef traktor_db_LocalDatabase_H
#define traktor_db_LocalDatabase_H

#include "Core/Heap/Ref.h"
#include "Database/Provider/IProviderDatabase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_LOCAL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

	namespace db
	{

class Context;
class LocalBus;
class LocalGroup;

/*! \brief Local database provider.
 * \ingroup Database
 */
class T_DLLCLASS LocalDatabase : public IProviderDatabase
{
	T_RTTI_CLASS(LocalDatabase)

public:
	bool create(const Path& manifestPath); 

	bool open(const Path& manifestPath);

	virtual void close();

	virtual Ref< IProviderBus > getBus();

	virtual Ref< IProviderGroup > getRootGroup();

private:
	Ref< Context > m_context;
	Ref< LocalBus > m_bus;
	Ref< LocalGroup > m_rootGroup;
};

	}
}

#endif	// traktor_db_LocalDatabase_H

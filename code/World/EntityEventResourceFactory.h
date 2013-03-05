#ifndef traktor_world_EntityEventResourceFactory_H
#define traktor_world_EntityEventResourceFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace world
	{

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS EntityEventResourceFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	EntityEventResourceFactory(db::Database* db);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const;

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_world_EntityEventResourceFactory_H

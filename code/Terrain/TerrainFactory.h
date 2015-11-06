#ifndef traktor_terrain_TerrainFactory_H
#define traktor_terrain_TerrainFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
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

	namespace terrain
	{

/*! \brief Terrain resource factory.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	TerrainFactory(db::Database* db);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes() const T_OVERRIDE T_FINAL;

	virtual bool isCacheable() const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_terrain_TerrainFactory_H

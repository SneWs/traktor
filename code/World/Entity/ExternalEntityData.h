#pragma once

#include "Core/Guid.h"
#include "Resource/Id.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! External entity data.
 * \ingroup World
 */
class T_DLLCLASS ExternalEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	ExternalEntityData();

	ExternalEntityData(const resource::Id< EntityData >& entityData);

	void setEntityData(const resource::Id< EntityData >& entityData);

	const resource::Id< EntityData >& getEntityData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	resource::Id< EntityData > m_entityData;
};

	}
}


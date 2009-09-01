#ifndef traktor_spray_EffectEntityFactory_H
#define traktor_spray_EffectEntityFactory_H

#include "Core/Heap/Ref.h"
#include "World/Entity/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

/*! \brief Effect entity factory.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS(EffectEntityFactory)

public:
	EffectEntityFactory(resource::IResourceManager* resourceManager);

	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData, const Object* instanceData) const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

	}
}

#endif	// traktor_spray_EffectEntityFactory_H

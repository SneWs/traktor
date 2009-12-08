#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/SoundFactory.h"
#include "Database/Database.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundFactory", SoundFactory, resource::IResourceFactory)

SoundFactory::SoundFactory(db::Database* db, SoundSystem* soundSystem)
:	m_db(db)
,	m_soundSystem(soundSystem)
{
}

const TypeInfoSet SoundFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Sound >());
	return typeSet;
}

bool SoundFactory::isCacheable() const
{
	return true;
}

Ref< Object > SoundFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< ISoundResource > resource = instance->getObject< ISoundResource >();
	if (!resource)
		return 0;

	return resource->createSound(resourceManager, instance);
}

	}
}

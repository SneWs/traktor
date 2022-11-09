#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Transform;

}

namespace traktor::world
{

class Entity;
class IEntityEventInstance;
class EntityEventManager;

/*! Entity event.
 * \ingroup World
 */
class T_DLLCLASS IEntityEvent : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< IEntityEventInstance > createInstance(EntityEventManager* eventManager, Entity* sender, const Transform& Toffset) const = 0;
};

}

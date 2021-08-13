#pragma once

#include "Core/Math/Aabb3.h"
#include "World/Renderable.h"
#include "World/WorldTypes.h"

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

class Entity;

/*! Entity component.
 * \ingroup World
 */
class T_DLLCLASS IEntityComponent : public Renderable
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual void setOwner(Entity* owner) = 0;

	virtual void setTransform(const Transform& transform) = 0;

	virtual Aabb3 getBoundingBox() const = 0;

	virtual void update(const UpdateParams& update) = 0;
};

	}
}


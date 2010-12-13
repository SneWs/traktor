#ifndef traktor_world_TransientEntityRenderer_H
#define traktor_world_TransientEntityRenderer_H

#include "World/Entity/IEntityRenderer.h"

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

/*! \brief Transient entity renderer.
 * \ingroup World
 */
class T_DLLCLASS TransientEntityRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const;

	virtual void render(
		WorldContext* worldContext,
		WorldRenderView* worldRenderView,
		Entity* entity
	);

	virtual void flush(
		WorldContext* worldContext,
		WorldRenderView* worldRenderView
	);
};

	}
}

#endif	// traktor_world_TransientEntityRenderer_H

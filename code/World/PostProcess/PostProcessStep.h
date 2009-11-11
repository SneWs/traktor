#ifndef traktor_world_PostProcessStep_H
#define traktor_world_PostProcessStep_H

#include "Core/Heap/Ref.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
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

	namespace render
	{

class IRenderSystem;
class IRenderView;
class ScreenRenderer;

	}

	namespace world
	{

class PostProcess;

/*! \brief Post processing step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStep : public Serializable
{
	T_RTTI_CLASS(PostProcessStep)

public:
	/*! \brief Step instance. */
	class T_DLLCLASS Instance : public Object
	{
	public:
		virtual void destroy() = 0;

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const Frustum& viewFrustum,
			const Matrix44& projection,
			float shadowMapBias,
			float deltaTime
		) = 0;
	};

	virtual Ref< Instance > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const = 0;
};

	}
}

#endif	// traktor_world_PostProcessStep_H

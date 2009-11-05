#ifndef traktor_world_PostProcessStepRepeat_H
#define traktor_world_PostProcessStepRepeat_H

#include "Core/Heap/Ref.h"
#include "World/PostProcess/PostProcessStep.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Repeat step any number of times.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepRepeat : public PostProcessStep
{
	T_RTTI_CLASS(PostProcessStepRepeat)

public:
	class InstanceRepeat : public Instance
	{
	public:
		InstanceRepeat(uint32_t count, Instance* instance);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const Frustum& viewFrustum,
			const Matrix44& projection,
			float shadowMapBias,
			float deltaTime
		);

	private:
		uint32_t m_count;
		Ref< Instance > m_instance;
	};

	virtual Instance* create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(Serializer& s);

	inline const Ref< PostProcessStep >& getStep() const { return m_step; }

private:
	uint32_t m_count;
	Ref< PostProcessStep > m_step;
};

	}
}

#endif	// traktor_world_PostProcessStepRepeat_H

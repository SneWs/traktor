#ifndef traktor_world_PostProcessStepSsao_H
#define traktor_world_PostProcessStepSsao_H

#include "World/PostProcess/PostProcessStep.h"
#include "Resource/Proxy.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;
class ISimpleTexture;

	}

	namespace world
	{

/*! \brief Post SSAO step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSsao : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;	/*!< Shader parameter name. */
		int32_t source;		/*!< Render target set source. */
		uint32_t index;		/*!< Render target index. */

		Source();

		bool serialize(ISerializer& s);
	};

	class InstanceSsao : public Instance
	{
	public:
		InstanceSsao(
			const PostProcessStepSsao* step,
			const Vector4 offsets[32],
			render::ISimpleTexture* randomNormals
		);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		Ref< const PostProcessStepSsao > m_step;
		Vector4 m_offsets[32];
		Ref< render::ISimpleTexture > m_randomNormals;
	};

	virtual Ref< Instance > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(ISerializer& s);

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	mutable resource::Proxy< render::Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_world_PostProcessStepSsao_H

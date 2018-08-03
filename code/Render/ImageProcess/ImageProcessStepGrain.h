/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ImageProcessStepGrain_H
#define traktor_render_ImageProcessStepGrain_H

#include <vector>
#include "Core/Math/Random.h"
#include "Render/Types.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Render/ImageProcess/ImageProcessStep.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ISimpleTexture;
class Shader;

/*! \brief
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepGrain : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;		/*!< Shader parameter name. */
		std::wstring source;	/*!< Render target set source. */

		Source();

		void serialize(ISerializer& s);
	};

	class InstanceGrain : public Instance
	{
	public:
		struct Source
		{
			handle_t param;
			handle_t source;
		};

		InstanceGrain(const ImageProcessStepGrain* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources);

		virtual void destroy() T_OVERRIDE T_FINAL;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) T_OVERRIDE T_FINAL;

	private:
		Ref< const ImageProcessStepGrain > m_step;
		resource::Proxy< Shader > m_shader;
		std::vector< Source > m_sources;
		float m_time;
		Random m_random;
		handle_t m_handleTime;
		handle_t m_handleDeltaTime;
		handle_t m_handleNoiseOffset;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< Shader >& getShader() const { return m_shader; }

	const std::vector< Source >& getSources() const { return m_sources; }

private:
	resource::Id< Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_render_ImageProcessStepGrain_H

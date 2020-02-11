#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Render/Image2/IImageStep.h"
#include "Resource/Proxy.h"

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

class Shader;

/*!
 * \ingroup Render
 */
class T_DLLCLASS DirectionalBlurImageStep : public IImageStep
{
	T_RTTI_CLASS;

public:
	virtual void setup(const ImageGraph* imageGraph, const ImageGraphContext& cx, RenderPass& pass) const override final;

    virtual void build(
		const ImageGraph* imageGraph,
		const ImageGraphContext& cx,
		const RenderGraph& renderGraph,
		RenderContext* renderContext
	) const override final;

private:
	friend class DirectionalBlurImageStepData;

	struct Source
	{
		handle_t textureId;
		handle_t parameter;
	};

	resource::Proxy< render::Shader > m_shader;
	AlignedVector< Source > m_sources;
	AlignedVector< Vector4 > m_gaussianOffsetWeights;
	Vector4 m_direction;
};

	}
}
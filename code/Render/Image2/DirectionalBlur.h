#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Image2/ImagePassOp.h"
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
class T_DLLCLASS DirectionalBlur : public ImagePassOp
{
	T_RTTI_CLASS;

public:
	virtual void setup(const ImageGraph* imageGraph, const ImageGraphContext& cx, RenderPass& pass) const override final;

    virtual void build(
		const ImageGraph* imageGraph,
		const ImageGraphContext& cx,
		const RenderGraph& renderGraph,
		const ProgramParameters* sharedParams,
		RenderContext* renderContext
	) const override final;

private:
	friend class DirectionalBlurData;

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
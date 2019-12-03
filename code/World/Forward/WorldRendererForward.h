#pragma once

#include "Core/Containers/AlignedVector.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

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

class ImageProcess;
class IRenderTargetSet;
class ISimpleTexture;
class RenderContext;
class StructBuffer;

	}

	namespace world
	{

class GroupEntity;
class IShadowProjection;
class WorldContext;
class WorldEntityRenderers;

/*! World renderer implementation.
 * \ingroup World
 *
 * Simple and naive implementation supporting
 * only a single shadowing directional light
 * and max 2 overlapping lights.
 *
 * Operation
 * 1. Render global shadow map if directional light 0 enabled.
 * 2. Render depth pass if enabled (either explicitly or shadows enabled).
 * 3. Render screen-space shadow mask.
 * 4. Render visuals.
 *
 * Techniques used
 * "Default" - Visual, final, color output.
 * "Depth"   - Depth pass; write view distance into depth buffer.
 * "Shadow"  - Shadow map pass; write distance from light into shadow map.
 */
class T_DLLCLASS WorldRendererForward : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	WorldRendererForward();

	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void attach(Entity* entity) override final;

	virtual void build(WorldRenderView& worldRenderView, int32_t frame) override final;

	virtual bool beginRender(render::IRenderView* renderView, int32_t frame, const Color4f& clearColor) override final;

	virtual void render(render::IRenderView* renderView, int32_t frame) override final;

	virtual void endRender(render::IRenderView* renderView, int32_t frame, float deltaTime) override final;

	virtual render::ImageProcess* getVisualImageProcess() override final;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const override final;

private:
	struct FrameShadow
	{
		Ref< WorldContext > shadow;
		Matrix44 shadowLightView;
		Matrix44 shadowLightProjection;
		Matrix44 viewToLightSpace;
	};

	struct Frame
	{
		FrameShadow slice[MaxSliceCount];
		FrameShadow atlas[16];
		Ref< WorldContext > depth;
		Ref< WorldContext > visual;
		Ref< render::StructBuffer > lightSBuffer;
		Matrix44 projection;
		Matrix44 view;
		Frustum viewFrustum;
		float time;
		int32_t lightCount;
		bool haveDepth;
		bool haveShadows;

		Frame()
		:	time(0.0f)
		,	lightCount(0)
		,	haveDepth(false)
		,	haveShadows(false)
		{
		}
	};

	WorldRenderSettings m_settings;
	Quality m_toneMapQuality;
	Quality m_shadowsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;
	Ref< render::IRenderTargetSet > m_visualTargetSet;
	Ref< render::IRenderTargetSet > m_intermediateTargetSet;
	Ref< render::IRenderTargetSet > m_gbufferTargetSet;
	Ref< render::RenderContext > m_globalContext;
	Ref< render::ImageProcess > m_ambientOcclusion;
	Ref< render::ImageProcess > m_antiAlias;
	Ref< render::ImageProcess > m_visualImageProcess;
	Ref< render::ImageProcess > m_gammaCorrectionImageProcess;
	Ref< render::ImageProcess > m_toneMapImageProcess;

	// Directional shadow map.
	Ref< IShadowProjection > m_shadowProjection;
	Ref< render::IRenderTargetSet > m_shadowCascadeTargetSet;

	// Point/Spot shadow map.
	Ref< render::IRenderTargetSet > m_shadowAtlasTargetSet;

	Ref< GroupEntity > m_rootEntity;
	AlignedVector< Frame > m_frames;
	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;

	void buildGBuffer(WorldRenderView& worldRenderView, int32_t frame);

	void buildLights(WorldRenderView& worldRenderView, int32_t frame);

	void buildVisual(WorldRenderView& worldRenderView, int32_t frame);
};

	}
}

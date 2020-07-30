#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"
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

class ITexture;
class RenderGraph;
class ScreenRenderer;

/*!
 * \ingroup Render
 */
struct ImageGraphParams
{
    Frustum viewFrustum;
    Matrix44 lastView;
    Matrix44 view;
    Matrix44 viewToLight;
    Matrix44 projection;
    Vector4 godRayDirection;
    Vector4 shadowMapUvTransform;
    int32_t sliceCount;
    int32_t sliceIndex;
    float sliceNearZ;
    float sliceFarZ;
    float shadowFarZ;
    float shadowMapBias;
    float deltaTime;
    float time;
    int32_t frame;

    ImageGraphParams()
    :	shadowMapUvTransform(0.0f, 0.0f, 1.0f, 1.0f)
    ,	sliceCount(0)
    ,	sliceIndex(0)
    ,	sliceNearZ(0.0f)
    ,	sliceFarZ(0.0f)
    ,	shadowFarZ(0.0f)
    ,	shadowMapBias(0.0f)
    ,	deltaTime(0.0f)
    ,   time(0.0f)
    ,   frame(0)
    {
    }
};

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraphContext
{
public:
    explicit ImageGraphContext(ScreenRenderer* screenRenderer);

    ScreenRenderer* getScreenRenderer() const { return m_screenRenderer; }

    void associateTexture(handle_t textureId, ITexture* texture);

	void associateTextureTargetSet(handle_t textureId, handle_t targetSetId, int32_t colorIndex);

    void associateTextureTargetSetDepth(handle_t textureId, handle_t targetSetId);

    std::pair< handle_t, bool > findTextureTargetSetId(handle_t textureId) const;

    ITexture* findTexture(const RenderGraph& renderGraph, handle_t textureId) const;

	void setFloatParameter(handle_t handle, float value);

    const SmallMap< handle_t, float >& getFloatParameters() const { return m_scalarParameters; }

	void setVectorParameter(handle_t handle, const Vector4& value);

    const SmallMap< handle_t, Vector4 >& getVectorParameters() const { return m_vectorParameters; }

	void setTextureParameter(handle_t handle, const resource::Proxy< ITexture >& value);

    const SmallMap< handle_t, resource::Proxy< ITexture > >& getTextureParameters() const { return m_textureParameters; }

    void setParams(const ImageGraphParams& params) { m_params = params; }

    const ImageGraphParams& getParams() const { return m_params; }

private:
	struct TextureTargetSet
	{
		handle_t targetSetId;
		int32_t colorIndex;
        ITexture* texture;
	};

    Ref< ScreenRenderer > m_screenRenderer;
	SmallMap< handle_t, TextureTargetSet > m_textureTargetSet;
	SmallMap< handle_t, float > m_scalarParameters;
	SmallMap< handle_t, Vector4 > m_vectorParameters;
	SmallMap< handle_t, resource::Proxy< ITexture > > m_textureParameters;
    ImageGraphParams m_params;
};

	}
}
/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderSystemCapture_H
#define traktor_render_RenderSystemCapture_H

#include "Render/IRenderSystem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_CAPTURE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Performance capture render system.
 * \ingroup RenderCapture
 *
 * This render system is only a wrapper around
 * a "real" render system implementation.
 * It will record statistics and frame captures for
 * debugging purposes only.
 */
class T_DLLCLASS RenderSystemCapture : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	virtual bool create(const RenderSystemDesc& desc) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool reset(const RenderSystemDesc& desc) T_OVERRIDE T_FINAL;

	virtual void getInformation(RenderSystemInformation& outInfo) const T_OVERRIDE T_FINAL;

	virtual uint32_t getDisplayModeCount() const T_OVERRIDE T_FINAL;

	virtual DisplayMode getDisplayMode(uint32_t index) const T_OVERRIDE T_FINAL;

	virtual DisplayMode getCurrentDisplayMode() const T_OVERRIDE T_FINAL;

	virtual float getDisplayAspectRatio() const T_OVERRIDE T_FINAL;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< VertexBuffer > createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic) T_OVERRIDE T_FINAL;

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic) T_OVERRIDE T_FINAL;

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) T_OVERRIDE T_FINAL;

	virtual Ref< ITimeQuery > createTimeQuery() const T_OVERRIDE T_FINAL;

	virtual void purge() T_OVERRIDE T_FINAL;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const T_OVERRIDE T_FINAL;

private:
	Ref< IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_render_RenderSystemCapture_H

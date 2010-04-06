#ifndef traktor_render_RenderSystemSw_H
#define traktor_render_RenderSystemSw_H

#if defined(_WIN32)
#	define _WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <tchar.h>
#	include "Graphics/IGraphicsSystem.h"
#endif
#include "Render/IRenderSystem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Processor;

/*! \brief Software render system.
 * \ingroup SW
 *
 * Software render system implementation.
 */
class T_DLLCLASS RenderSystemSw : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemSw();

	virtual bool create(const RenderSystemCreateDesc& desc);

	virtual void destroy();

	virtual uint32_t getDisplayModeCount() const;

	virtual DisplayMode getDisplayMode(uint32_t index) const;

	virtual DisplayMode getCurrentDisplayMode() const;

	virtual bool handleMessages();

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc);

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc);

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc);

	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource);

	virtual Ref< IProgramCompiler > createProgramCompiler() const;

private:
	Ref< Processor > m_processor;
#if defined(_WIN32)
	HWND m_hWnd;
	std::vector< graphics::DisplayMode > m_displayModes;

	static LRESULT WINAPI wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
};

	}
}

#endif	// traktor_render_RenderSystemSw_H

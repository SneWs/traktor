#include "Core/Log/Log.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderViewEmbeddedWin32.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewEmbeddedWin32", RenderViewEmbeddedWin32, RenderViewWin32)

RenderViewEmbeddedWin32::RenderViewEmbeddedWin32(
	RenderSystemWin32* renderSystem,
	ParameterCache* parameterCache,
	IDirect3DDevice9* d3dDevice,
	const D3DPRESENT_PARAMETERS& d3dPresent,
	D3DFORMAT d3dDepthStencilFormat
)
:	RenderViewWin32(renderSystem, parameterCache, d3dDevice)
,	m_d3dPresent(d3dPresent)
,	m_d3dDepthStencilFormat(d3dDepthStencilFormat)
{
}

RenderViewEmbeddedWin32::~RenderViewEmbeddedWin32()
{
	close();
}

bool RenderViewEmbeddedWin32::nextEvent(RenderEvent& outEvent)
{
	return false;
}

bool RenderViewEmbeddedWin32::reset(const RenderViewDefaultDesc& desc)
{
	return false;
}

void RenderViewEmbeddedWin32::resize(int32_t width, int32_t height)
{
	T_ASSERT (m_renderStateStack.empty());

	HRESULT hr;

	if (!width || !height)
		return;
	if (m_d3dPresent.BackBufferWidth == width && m_d3dPresent.BackBufferHeight == height)
		return;

	m_d3dPresent.BackBufferWidth = width;
	m_d3dPresent.BackBufferHeight = height;

	// Re-create backbuffer and swap chain explicity by calling this view's resetDevice method;
	// not necessary to perform a complete "reset-device" cycle.
	hr = resetDevice();
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"Failed to resize render view");
}

int RenderViewEmbeddedWin32::getWidth() const
{
	return m_d3dPresent.BackBufferWidth;
}

int RenderViewEmbeddedWin32::getHeight() const
{
	return m_d3dPresent.BackBufferHeight;
}

bool RenderViewEmbeddedWin32::isActive() const
{
	if (m_d3dDevice)
		return GetForegroundWindow() == m_d3dPresent.hDeviceWindow;
	else
		return false;
}

bool RenderViewEmbeddedWin32::isFullScreen() const
{
	return !m_d3dPresent.Windowed;
}

HRESULT RenderViewEmbeddedWin32::lostDevice()
{
	m_d3dDevice.release();
	m_d3dSwapChain.release();
	m_d3dBackBuffer.release();
	m_d3dDepthStencilSurface.release();

	for (uint32_t i = 0; i < sizeof_array(m_d3dSyncQueries); ++i)
		m_d3dSyncQueries[i].release();

	m_renderStateStack.clear();
	m_currentVertexBuffer = 0;
	m_currentIndexBuffer = 0;
	m_currentProgram = 0;

	return S_OK;
}

HRESULT RenderViewEmbeddedWin32::resetDevice()
{
	HRESULT hr;

	hr = m_d3dDevice->CreateAdditionalSwapChain(
		&m_d3dPresent,
		&m_d3dSwapChain.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create additional swap chain; hr = " << hr << Endl;
		return hr;
	}

	hr = m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to get back buffer; hr = " << hr << Endl;
		return hr;
	}

	hr = m_d3dDevice->CreateDepthStencilSurface(
		m_d3dPresent.BackBufferWidth,
		m_d3dPresent.BackBufferHeight,
		m_d3dDepthStencilFormat,
		m_d3dPresent.MultiSampleType,
		0,
		TRUE,
		&m_d3dDepthStencilSurface.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to get depth/stencil surface; hr = " << hr << Endl;
		return hr;
	}

	for (uint32_t i = 0; i < sizeof_array(m_d3dSyncQueries); ++i)
	{
		hr = m_d3dDevice->CreateQuery(
			D3DQUERYTYPE_EVENT,
			&m_d3dSyncQueries[i].getAssign()
		);
		if (FAILED(hr))
		{
			log::warning << L"Unable to create synchronization query; hr = " << hr << Endl;
			m_d3dSyncQueries[i].release();
		}
	}

	return S_OK;
}

	}
}

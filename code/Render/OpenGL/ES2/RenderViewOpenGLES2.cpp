#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/ES2/ExtensionsGLES2.h"
#include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderSystemOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderViewOpenGLES2.h"
#include "Render/OpenGL/ES2/StateCache.h"
#include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES2/Android/ContextOpenGLES2.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/ContextOpenGLES2.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES2/Emscripten/ContextOpenGLES2.h"
#elif defined(__PNACL__)
#	include "Render/OpenGL/ES2/PNaCl/ContextOpenGLES2.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES2/Win32/ContextOpenGLES2.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/OpenGL/ES2/Linux/ContextOpenGLES2.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

struct RenderEventTypePred
{
	RenderEventType m_type;

	RenderEventTypePred(RenderEventType type)
	:	m_type(type)
	{
	}

	bool operator () (const RenderEvent& evt) const
	{
		return evt.type == m_type;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewOpenGLES2", RenderViewOpenGLES2, IRenderView)

RenderViewOpenGLES2::RenderViewOpenGLES2(ContextOpenGLES2* context)
:	m_context(context)
,	m_stateCache(new StateCache())
,	m_width(0)
,	m_height(0)
,	m_cursorVisible(true)
{
#if defined(_WIN32)
	m_context->getWindow()->addListener(this);
#endif

	m_width = m_context->getWidth();
	m_height = m_context->getHeight();

	m_viewport = Viewport(
		0,
		0,
		m_width,
		m_height,
		0.0f,
		1.0f
	);
}

RenderViewOpenGLES2::~RenderViewOpenGLES2()
{
}

bool RenderViewOpenGLES2::nextEvent(RenderEvent& outEvent)
{
#if defined(_WIN32)

	bool going = true;
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret <= 0 || msg.message == WM_QUIT)
			going = false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (!m_eventQueue.empty())
	{
		outEvent = m_eventQueue.front();
		m_eventQueue.pop_front();
		return true;
	}
	else
		return false;

#elif defined(__LINUX__)

	return m_context ? m_context->getWindow()->update(outEvent) : false;

#elif defined(__IOS__) || defined(__ANDROID__)

	int32_t width = m_context->getWidth();
	int32_t height = m_context->getHeight();

	if (width != m_width || height != m_height)
	{
		log::info << L"Device screen size changed, new size " << width << L" * " << height << Endl;

		m_width = width;
		m_height = height;

		m_viewport = Viewport(
			0,
			0,
			m_width,
			m_height,
			0.0f,
			1.0f
		);

		// Post a resize event as we need all systems to re-create
		// resources if necessary.
		outEvent.type = ReResize;
		outEvent.resize.width = width;
		outEvent.resize.height = height;
		return true;
	}

#endif
	return false;
}

void RenderViewOpenGLES2::close()
{
#if defined(_WIN32)
	if (m_context->getWindow())
		m_context->getWindow()->removeListener(this);
#endif
	m_context = 0;
}

bool RenderViewOpenGLES2::reset(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32) || defined(__LINUX__)
	m_context->getWindow()->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL ES 2.0 Renderer");
	if (desc.fullscreen)
		m_context->getWindow()->setFullScreenStyle();
	else
		m_context->getWindow()->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

	m_context->reset(desc.displayMode.width, desc.displayMode.height);

	m_width = m_context->getWidth();
	m_height = m_context->getHeight();
#endif
	return true;
}

bool RenderViewOpenGLES2::reset(int32_t width, int32_t height)
{
	return m_context->reset(width, height);
}

int RenderViewOpenGLES2::getWidth() const
{
	return m_width;
}

int RenderViewOpenGLES2::getHeight() const
{
	return m_height;
}

bool RenderViewOpenGLES2::isActive() const
{
	return true;
}

bool RenderViewOpenGLES2::isMinimized() const
{
	return false;
}

bool RenderViewOpenGLES2::isFullScreen() const
{
#if defined(T_OPENGL_ES2_HAVE_EGL) || defined(__PNACL__)
	return false;
#else
	return true;
#endif
}

void RenderViewOpenGLES2::showCursor()
{
	m_cursorVisible = true;
}

void RenderViewOpenGLES2::hideCursor()
{
	m_cursorVisible = false;
}

bool RenderViewOpenGLES2::isCursorVisible() const
{
	return m_cursorVisible;
}

bool RenderViewOpenGLES2::setGamma(float gamma)
{
	return false;
}

void RenderViewOpenGLES2::setViewport(const Viewport& viewport)
{
	T_ANONYMOUS_VAR(ContextOpenGLES2::Scope)(m_context);

	if (m_renderTargetStack.empty())
		m_viewport = viewport;
	else
		m_renderTargetStack.top().viewport = viewport;

	if (m_renderTargetStack.empty())
	{
		T_OGL_SAFE(glViewport(
			viewport.left,
			viewport.top,
			viewport.width,
			viewport.height
		));
	}

	T_OGL_SAFE(glDepthRangef(
		viewport.nearZ,
		viewport.farZ
	));
}

SystemWindow RenderViewOpenGLES2::getSystemWindow()
{
#if defined(_WIN32)
	SystemWindow sw;
	sw.hWnd = *m_context->getWindow();
	return sw;
#elif defined(__LINUX__)
	SystemWindow sw;
	sw.display = m_context->getWindow()->getDisplay();
	sw.window = m_context->getWindow()->getWindow();
	return sw;
#else
	return SystemWindow();
#endif
}

bool RenderViewOpenGLES2::beginFrame()
{
	if (!m_context->enter())
		return false;

	m_stateCache->reset();
	m_drawCalls = 0;
	m_primitiveCount = 0;

	return true;
}

void RenderViewOpenGLES2::endFrame()
{
}

void RenderViewOpenGLES2::present()
{
	m_context->swapBuffers();
	m_context->leave();
	m_context->deleteResources();
}

bool RenderViewOpenGLES2::beginPass(const Clear* clear)
{
	m_context->bindPrimary();

	T_OGL_SAFE(glViewport(
		m_viewport.left,
		m_viewport.top,
		m_viewport.width,
		m_viewport.height
	));

	T_OGL_SAFE(glDepthRangef(
		m_viewport.nearZ,
		m_viewport.farZ
	));

	return true;
}

bool RenderViewOpenGLES2::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear)
{
	return beginPass(renderTargetSet, 0, clear);
}

bool RenderViewOpenGLES2::beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear)
{
	RenderTargetSetOpenGLES2* rts = checked_type_cast< RenderTargetSetOpenGLES2* >(renderTargetSet);

	if (!rts->bind(m_context->getPrimaryDepth(), renderTarget))
		return false;

	RenderTargetStack s;
	s.renderTargetSet = rts;
	s.renderTarget = renderTarget;
	s.viewport = Viewport(0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f);

	T_OGL_SAFE(glViewport(
		s.viewport.left,
		s.viewport.top,
		s.viewport.width,
		s.viewport.height
	));

	T_OGL_SAFE(glDepthRangef(
		s.viewport.nearZ,
		s.viewport.farZ
	));

	m_renderTargetStack.push(s);

	if (clear && clear->mask != 0)
	{
		const GLuint c_clearMask[] =
		{
			0,
			GL_COLOR_BUFFER_BIT,
			GL_DEPTH_BUFFER_BIT,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			GL_STENCIL_BUFFER_BIT,
			GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT
		};

		GLuint cm = c_clearMask[clear->mask];

		if (cm & GL_COLOR_BUFFER_BIT)
		{
			m_stateCache->setColorMask(RenderStateOpenGL::CmAll);
			float r = clear->colors[0].getRed();
			float g = clear->colors[0].getGreen();
			float b = clear->colors[0].getBlue();
			float a = clear->colors[0].getAlpha();
			T_OGL_SAFE(glClearColor(r, g, b, a));
		}

		if (cm & GL_DEPTH_BUFFER_BIT)
		{
			m_stateCache->setDepthMask(GL_TRUE);
			T_OGL_SAFE(glClearDepthf(clear->depth));
		}

		if (cm & GL_STENCIL_BUFFER_BIT)
			T_OGL_SAFE(glClearStencil(clear->stencil));

		T_OGL_SAFE(glClear(cm));
	}

	return true;
}

void RenderViewOpenGLES2::endPass()
{
	if (m_renderTargetStack.empty())
		return;

	m_renderTargetStack.top().renderTargetSet->setContentValid(true);
	m_renderTargetStack.pop();

	if (!m_renderTargetStack.empty())
	{
		RenderTargetStack& s = m_renderTargetStack.top();

		s.renderTargetSet->bind(m_context->getPrimaryDepth(), s.renderTarget);

		T_OGL_SAFE(glViewport(
			s.viewport.left,
			s.viewport.top,
			s.viewport.width,
			s.viewport.height
		));

		T_OGL_SAFE(glDepthRangef(
			s.viewport.nearZ,
			s.viewport.farZ
		));
	}
	else
	{
		m_context->bindPrimary();

		T_OGL_SAFE(glViewport(
			m_viewport.left,
			m_viewport.top,
			m_viewport.width,
			m_viewport.height
		));

		T_OGL_SAFE(glDepthRangef(
			m_viewport.nearZ,
			m_viewport.farZ
		));
	}
}

void RenderViewOpenGLES2::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	VertexBufferOpenGLES2* vertexBufferGL = checked_type_cast< VertexBufferOpenGLES2* >(vertexBuffer);
	IndexBufferOpenGLES2* indexBufferGL = checked_type_cast< IndexBufferOpenGLES2* >(indexBuffer);
	ProgramOpenGLES2* programGL = checked_type_cast< ProgramOpenGLES2 * >(program);

	vertexBufferGL->activate(m_stateCache);

	float targetSize[2];
	float postTransform[4];
	bool invertCull;

	if (m_renderTargetStack.empty())
	{
		targetSize[0] = float(getWidth());
		targetSize[1] = float(getHeight());

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = 1.0f;

		invertCull = false;
	}
	else
	{
		const RenderTargetSetOpenGLES2* rts = m_renderTargetStack.top().renderTargetSet;
		targetSize[0] = float(rts->getWidth());
		targetSize[1] = float(rts->getHeight());

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = -1.0f;

		invertCull = true;
	}

	if (!programGL->activate(m_stateCache, targetSize, postTransform, invertCull, 0))
		return;

	GLenum primitiveType = GL_POINTS;
	GLuint vertexCount = 0;

	switch (primitives.type)
	{
	case PtPoints:
		primitiveType = GL_POINTS;
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT_M (0, L"PtLineStrip unsupported");
		break;

	case PtLines:
		primitiveType = GL_LINES;
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		primitiveType = GL_TRIANGLE_STRIP;
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		primitiveType = GL_TRIANGLES;
		vertexCount = primitives.count * 3;
		break;

	default:
		T_ASSERT(0);
	}

	if (primitives.indexed)
	{
		T_ASSERT_M (indexBufferGL, L"No index buffer");

		GLenum indexType = 0;
		GLint offsetMultiplier = 0;

		switch (indexBufferGL->getIndexType())
		{
		case ItUInt16:
			indexType = GL_UNSIGNED_SHORT;
			offsetMultiplier = 2;
			break;

		case ItUInt32:
			indexType = GL_UNSIGNED_INT;
			offsetMultiplier = 4;
			break;

		default:
			return;
		}

		indexBufferGL->activate(m_stateCache);

		T_OGL_SAFE(glDrawElements(
			primitiveType,
			vertexCount,
			indexType,
			(const GLubyte*)(primitives.offset * offsetMultiplier)
		));
	}
	else
	{
		T_OGL_SAFE(glDrawArrays(
			primitiveType,
			primitives.offset,
			vertexCount
		));
	}

	m_drawCalls++;
	m_primitiveCount += primitives.count;
}

void RenderViewOpenGLES2::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	VertexBufferOpenGLES2* vertexBufferGL = checked_type_cast< VertexBufferOpenGLES2* >(vertexBuffer);
	IndexBufferOpenGLES2* indexBufferGL = checked_type_cast< IndexBufferOpenGLES2* >(indexBuffer);
	ProgramOpenGLES2* programGL = checked_type_cast< ProgramOpenGLES2 * >(program);

	vertexBufferGL->activate(m_stateCache);

	float targetSize[2];
	float postTransform[4];
	bool invertCull;

	if (m_renderTargetStack.empty())
	{
		targetSize[0] = float(getWidth());
		targetSize[1] = float(getHeight());

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = 1.0f;

		invertCull = false;
	}
	else
	{
		const RenderTargetSetOpenGLES2* rts = m_renderTargetStack.top().renderTargetSet;
		targetSize[0] = float(rts->getWidth());
		targetSize[1] = float(rts->getHeight());

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = -1.0f;

		invertCull = true;
	}

	GLenum primitiveType;
	GLuint vertexCount;

	switch (primitives.type)
	{
	case PtPoints:
		primitiveType = GL_POINTS;
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT_M (0, L"PtLineStrip unsupported");
		break;

	case PtLines:
		primitiveType = GL_LINES;
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		primitiveType = GL_TRIANGLE_STRIP;
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		primitiveType = GL_TRIANGLES;
		vertexCount = primitives.count * 3;
		break;

	default:
		T_ASSERT(0);
	}

#if !defined(_WIN32) && !defined(__ANDROID__) && GL_EXT_draw_instanced

	if (!programGL->activate(m_stateCache, targetSize, postTransform, invertCull, 0))
		return;

	if (primitives.indexed)
	{
		T_ASSERT_M (indexBufferGL, L"No index buffer");

		GLenum indexType = 0;
		GLint offsetMultiplier = 0;

		switch (indexBufferGL->getIndexType())
		{
		case ItUInt16:
			indexType = GL_UNSIGNED_SHORT;
			offsetMultiplier = 2;
			break;

		case ItUInt32:
			indexType = GL_UNSIGNED_INT;
			offsetMultiplier = 4;
			break;

		default:
			return;
		}

		indexBufferGL->activate(m_stateCache);

		T_OGL_SAFE(glDrawElementsInstancedEXT(
			primitiveType,
			vertexCount,
			indexType,
			(const GLubyte*)(primitives.offset * offsetMultiplier),
			instanceCount
		));
	}
	else
	{
		T_OGL_SAFE(glDrawArraysInstancedEXT(
			primitiveType,
			primitives.offset,
			vertexCount,
			instanceCount
		));
	}

#else
#	if defined(__ANDROID__)

	if (s_glDrawElementsInstancedEXT != 0 && s_glDrawArraysInstancedEXT != 0)
	{
		if (!programGL->activate(m_stateCache, targetSize, postTransform, invertCull, 0))
			return;

		if (primitives.indexed)
		{
			T_ASSERT_M (indexBufferGL, L"No index buffer");

			GLenum indexType = 0;
			GLint offsetMultiplier = 0;

			switch (indexBufferGL->getIndexType())
			{
			case ItUInt16:
				indexType = GL_UNSIGNED_SHORT;
				offsetMultiplier = 2;
				break;

			case ItUInt32:
				indexType = GL_UNSIGNED_INT;
				offsetMultiplier = 4;
				break;

			default:
				return;
			}

			indexBufferGL->activate(m_stateCache);

			T_OGL_SAFE(s_glDrawElementsInstancedEXT(
				primitiveType,
				vertexCount,
				indexType,
				(const GLubyte*)(primitives.offset * offsetMultiplier),
				instanceCount
			));
		}
		else
		{
			T_OGL_SAFE(s_glDrawArraysInstancedEXT(
				primitiveType,
				primitives.offset,
				vertexCount,
				instanceCount
			));
		}
	}
	else
	{

#	endif

	for (uint32_t i = 0; i < instanceCount; ++i)
	{
		if (!programGL->activate(m_stateCache, targetSize, postTransform, invertCull, i))
			return;

		if (primitives.indexed)
		{
			T_ASSERT_M (indexBufferGL, L"No index buffer");

			GLenum indexType = 0;
			GLint offsetMultiplier = 0;

			switch (indexBufferGL->getIndexType())
			{
			case ItUInt16:
				indexType = GL_UNSIGNED_SHORT;
				offsetMultiplier = 2;
				break;

			case ItUInt32:
				indexType = GL_UNSIGNED_INT;
				offsetMultiplier = 4;
				break;

			default:
				return;
			}

			indexBufferGL->activate(m_stateCache);

			T_OGL_SAFE(glDrawElements(
				primitiveType,
				vertexCount,
				indexType,
				(const GLubyte*)(primitives.offset * offsetMultiplier)
			));
		}
		else
		{
			T_OGL_SAFE(glDrawArrays(
				primitiveType,
				primitives.offset,
				vertexCount
			));
		}
	}

#	if defined(__ANDROID__)
	}
#	endif
#endif

	m_drawCalls++;
	m_primitiveCount += primitives.count * instanceCount;
}

void RenderViewOpenGLES2::compute(IProgram* program, const int32_t* workSize)
{
}

bool RenderViewOpenGLES2::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
	return false;
}

void RenderViewOpenGLES2::pushMarker(const char* const marker)
{
#if defined(__IOS__)
	glPushGroupMarkerEXT(0, marker);
#endif
}

void RenderViewOpenGLES2::popMarker()
{
#if defined(__IOS__)
	glPopGroupMarkerEXT();
#endif
}

void RenderViewOpenGLES2::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
}

bool RenderViewOpenGLES2::getBackBufferContent(void* buffer) const
{
	return false;
}

#if defined(_WIN32)
bool RenderViewOpenGLES2::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	if (message == WM_CLOSE)
	{
		RenderEvent evt;
		evt.type = ReClose;
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_SIZE)
	{
		// Remove all pending resize events.
		m_eventQueue.remove_if(RenderEventTypePred(ReResize));

		// Push new resize event if not matching current size.
		int32_t width = LOWORD(lParam);
		int32_t height = HIWORD(lParam);

		if (width <= 0 || height <= 0)
			return false;

		if (width != m_context->getWidth() || height != m_context->getHeight())
		{
			RenderEvent evt;
			evt.type = ReResize;
			evt.resize.width = width;
			evt.resize.height = height;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SIZING)
	{
		RECT* rcWindowSize = (RECT*)lParam;

		int32_t width = rcWindowSize->right - rcWindowSize->left;
		int32_t height = rcWindowSize->bottom - rcWindowSize->top;

		if (width < 320)
			width = 320;
		if (height < 200)
			height = 200;

		if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->right = rcWindowSize->left + width;
		if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
			rcWindowSize->left = rcWindowSize->right - width;

		if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->bottom = rcWindowSize->top + height;
		if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
			rcWindowSize->top = rcWindowSize->bottom - height;

		outResult = TRUE;
	}
	else if (message == WM_SYSKEYDOWN)
	{
		if (wParam == VK_RETURN && (lParam & (1 << 29)) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		if (wParam == VK_RETURN && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SETCURSOR)
	{
		if (m_cursorVisible)
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		else
			SetCursor(NULL);
	}
	else
		return false;

	return true;
}
#endif

	}
}

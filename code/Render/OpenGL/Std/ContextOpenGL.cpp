#include <cstring>
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#elif defined(__LINUX__)
#	include "Render/OpenGL/Std/Linux/Window.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

typedef RefArray< ContextOpenGL > context_stack_t;

#if !defined(__LINUX__) && !defined(__APPLE__)
void APIENTRY debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
{
#	if !defined(_DEBUG)
	if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
		return;
#	endif

	const wchar_t* s = L"unknown";

	if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
		s = L"low";
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
		s = L"medium";
	else if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		s = L"high";

	if (message)
		log::info << L"OpenGL (" << s << L"): " << mbstows(message) << Endl;
	else
		log::info << L"OpenGL (" << s << L"): <empty>" << Endl;
}

void APIENTRY debugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
{
	if (message)
		log::info << L"OpenGL: " << mbstows(message) << Endl;
	else
		log::info << L"OpenGL: <empty>" << Endl;
}
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGL", ContextOpenGL, IContext)

ThreadLocal ContextOpenGL::ms_contextStack;

#if defined(_WIN32)

ContextOpenGL::ContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
:	m_hWnd(hWnd)
,	m_hDC(hDC)
,	m_hRC(hRC)
,	m_width(0)
,	m_height(0)
,	m_permitDepth(true)
,	m_currentRenderStateList(~0UL)

#elif defined(__APPLE__)

ContextOpenGL::ContextOpenGL(void* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_permitDepth(true)
,	m_currentRenderStateList(~0UL)

#elif defined(__LINUX__)

ContextOpenGL::ContextOpenGL(Window* window, GLXContext context)
:	m_window(window)
,	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_permitDepth(true)
,	m_currentRenderStateList(~0UL)

#endif
{
	update();
}

ContextOpenGL::~ContextOpenGL()
{
#if defined(_WIN32)
	T_ASSERT (!m_hRC);
#elif defined(__APPLE__)
	T_ASSERT (!m_context);
#elif defined(__LINUX__)
	T_ASSERT (!m_context);
#endif
}

void ContextOpenGL::update()
{
#if defined(_WIN32)
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	m_width = int32_t(rc.right - rc.left);
	m_height = int32_t(rc.bottom - rc.top);
#elif defined(__APPLE__)
	cglwUpdate(m_context);
	cglwGetSize(m_context, m_width, m_height);
#elif defined(__LINUX__)
	m_width = m_window->getWidth();
	m_height = m_window->getHeight();
#endif
}

void ContextOpenGL::swapBuffers(bool waitVBlank)
{
#if defined(_WIN32)
	SwapBuffers(m_hDC);
#elif defined(__APPLE__)
	cglwSwapBuffers(m_context, waitVBlank);
#elif defined(__LINUX__)
	glXSwapBuffers(
		m_window->getDisplay(),
		 m_window->getWindow()
	);
#endif
}

void ContextOpenGL::destroy()
{
#if defined(_WIN32)

	if (m_hRC)
	{
		wglMakeCurrent(0, 0);
		wglDeleteContext(m_hRC);

		ReleaseDC(m_hWnd, m_hDC);

		m_hWnd = 0;
		m_hDC = 0;
		m_hRC = 0;
	}

#elif defined(__APPLE__)

	if (m_context)
		m_context = 0;

#elif defined(__LINUX__)

	if (m_context)
	{
		glXDestroyContext(m_window->getDisplay(), m_context);
		m_context = 0;
	}

#endif
}

int32_t ContextOpenGL::getWidth() const
{
	return m_width;
}

int32_t ContextOpenGL::getHeight() const
{
	return m_height;
}

bool ContextOpenGL::enter()
{
	if (!m_lock.wait())
		return false;

#if defined(_WIN32)
	if (!wglMakeCurrent(m_hDC, m_hRC))
		return false;
#elif defined(__APPLE__)
	if (!cglwMakeCurrent(m_context))
		return false;
#elif defined(__LINUX__)
	if (!glXMakeCurrent(
		m_window->getDisplay(),
		m_window->getWindow(),
		m_context
	))
		return false;
#endif

	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	if (!stack)
	{
		stack = new context_stack_t();
		ms_contextStack.set(stack);
	}

#if !defined(__LINUX__) && !defined(__APPLE__)
	if (glDebugMessageCallbackARB)
	{
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
		glDebugMessageCallbackARB(&debugCallbackARB, 0);
#	if defined(_WIN32)
#		if defined(_DEBUG)
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#		else
		glEnable(GL_DEBUG_OUTPUT);
#		endif
#	endif
	}
	if (glDebugMessageCallbackAMD)
	{
		glDebugMessageEnableAMD(0, 0, 0, NULL, GL_TRUE);
		glDebugMessageCallbackAMD(&debugCallbackAMD, 0);
	}
#endif

	// Flush GL error stack.
	while (glGetError() != GL_NO_ERROR)
		;

	stack->push_back(this);
	return true;
}

void ContextOpenGL::leave()
{
	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	bool result = true;

	T_ASSERT (stack);
	T_ASSERT (!stack->empty());
	T_ASSERT (stack->back() == this);

	stack->pop_back();

#if defined(_WIN32)

	if (!stack->empty())
	{
		wglMakeCurrent(
			stack->back()->m_hDC,
			stack->back()->m_hRC
		);
	}
	else
		wglMakeCurrent(m_hDC, NULL);

#elif defined(__APPLE__)

	if (!stack->empty())
		cglwMakeCurrent(stack->back()->m_context);
	else
		cglwMakeCurrent(0);

#elif defined(__LINUX__)

	if (!stack->empty())
	{
		result = (glXMakeContextCurrent(
			stack->back()->m_window->getDisplay(),
			stack->back()->m_window->getWindow(),
			stack->back()->m_window->getWindow(),
			stack->back()->m_context
		) == True);
	}
	else
	{
		result = (glXMakeContextCurrent(
			m_window->getDisplay(),
			None,
			None,
			NULL
		) == True);
	}

#endif

	m_lock.release();
}

GLuint ContextOpenGL::createShaderObject(const char* shader, GLenum shaderType)
{
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;

	Adler32 adler;
	adler.begin();
	adler.feed(shader, std::strlen(shader));
	adler.end();

	uint32_t hash = adler.get();

	std::map< uint32_t, GLuint >::const_iterator i = m_shaderObjects.find(hash);
	if (i != m_shaderObjects.end())
		return i->second;

	GLuint shaderObject = glCreateShader(shaderType);
	T_OGL_SAFE(glShaderSource(shaderObject, 1, &shader, NULL));
	T_OGL_SAFE(glCompileShader(shaderObject));

	T_OGL_SAFE(glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status));
	if (status != 1)
	{
		T_OGL_SAFE(glGetShaderInfoLog(shaderObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL shader compile failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			log::error << Endl;
			FormatMultipleLines(log::error, mbstows(shader));
			return 0;
		}
	}

	m_shaderObjects.insert(std::make_pair(hash, shaderObject));
	return shaderObject;
}

uint32_t ContextOpenGL::createRenderStateObject(const RenderStateOpenGL& renderState)
{
	Adler32 adler;
	adler.feed(renderState.cullFaceEnable);
	adler.feed(renderState.cullFace);
	adler.feed(renderState.blendEnable);
	adler.feed(renderState.blendEquation);
	adler.feed(renderState.blendFuncSrc);
	adler.feed(renderState.blendFuncDest);
	adler.feed(renderState.depthTestEnable);
	adler.feed(renderState.colorMask);
	adler.feed(renderState.depthMask);
	adler.feed(renderState.depthFunc);
	adler.feed(renderState.alphaTestEnable);
	adler.feed(renderState.alphaFunc);
	adler.feed(renderState.alphaRef);
	adler.feed(renderState.stencilTestEnable);
	adler.feed(renderState.stencilOpFail);
	adler.feed(renderState.stencilOpZFail);
	adler.feed(renderState.stencilOpZPass);

	std::map< uint32_t, uint32_t >::iterator i = m_renderStateListCache.find(adler.get());
	if (i != m_renderStateListCache.end())
		return i->second;

	uint32_t list = m_renderStateList.size() + 1;

	m_renderStateList.push_back(renderState);
	if (m_renderStateList.back().cullFace == GL_FRONT)
		m_renderStateList.back().cullFace = GL_BACK;
	else
		m_renderStateList.back().cullFace = GL_FRONT;

	m_renderStateList.push_back(renderState);
	if (m_renderStateList.back().cullFace == GL_FRONT)
		m_renderStateList.back().cullFace = GL_BACK;
	else
		m_renderStateList.back().cullFace = GL_FRONT;

	RenderStateOpenGL& rs = m_renderStateList.back();

	rs.depthTestEnable = GL_FALSE;
	rs.depthMask = GL_FALSE;
	rs.stencilTestEnable = GL_FALSE;

	m_renderStateListCache.insert(std::make_pair(adler.get(), list));
	return list;
}

uint32_t ContextOpenGL::createSamplerStateObject(const SamplerStateOpenGL& samplerState)
{
	Adler32 adler;
	adler.feed(samplerState.minFilter);
	adler.feed(samplerState.magFilter);
	adler.feed(samplerState.wrapS);
	adler.feed(samplerState.wrapT);
	adler.feed(samplerState.wrapR);

	std::map< uint32_t, uint32_t >::iterator i = m_samplerStateListCache.find(adler.get());
	if (i != m_samplerStateListCache.end())
		return i->second;

	uint32_t list = m_samplerStateList.size() + 1;
	m_samplerStateList.push_back(samplerState);
	m_samplerStateListCache.insert(std::make_pair(adler.get(), list));

	return list;
}

void ContextOpenGL::bindRenderStateObject(uint32_t renderStateObject)
{
	renderStateObject += (m_permitDepth ? 0 : 1);
	if (renderStateObject == m_currentRenderStateList)
		return;

	T_ASSERT (renderStateObject > 0);
	T_ASSERT (renderStateObject <= m_renderStateList.size());
	const RenderStateOpenGL& rs = m_renderStateList[renderStateObject - 1];

	if (rs.cullFaceEnable)
		{ T_OGL_SAFE(glEnable(GL_CULL_FACE)); }
	else
		{ T_OGL_SAFE(glDisable(GL_CULL_FACE)); }

	T_OGL_SAFE(glCullFace(rs.cullFace));

	if (rs.blendEnable)
		{ T_OGL_SAFE(glEnable(GL_BLEND)); }
	else
		{ T_OGL_SAFE(glDisable(GL_BLEND)); }

	T_OGL_SAFE(glBlendFunc(rs.blendFuncSrc, rs.blendFuncDest));
	T_OGL_SAFE(glBlendEquation(rs.blendEquation));

	if (rs.depthTestEnable)
		{ T_OGL_SAFE(glEnable(GL_DEPTH_TEST)); }
	else
		{ T_OGL_SAFE(glDisable(GL_DEPTH_TEST)); }

	T_OGL_SAFE(glDepthFunc(rs.depthFunc));
	T_OGL_SAFE(glDepthMask(rs.depthMask));

	T_OGL_SAFE(glColorMask(
		(rs.colorMask & RenderStateOpenGL::CmRed) ? GL_TRUE : GL_FALSE,
		(rs.colorMask & RenderStateOpenGL::CmGreen) ? GL_TRUE : GL_FALSE,
		(rs.colorMask & RenderStateOpenGL::CmBlue) ? GL_TRUE : GL_FALSE,
		(rs.colorMask & RenderStateOpenGL::CmAlpha) ? GL_TRUE : GL_FALSE
	));

	if (rs.stencilTestEnable)
		{ T_OGL_SAFE(glEnable(GL_STENCIL_TEST)); }
	else
		{ T_OGL_SAFE(glDisable(GL_STENCIL_TEST)); }

	T_OGL_SAFE(glStencilMask(~0UL));
	T_OGL_SAFE(glStencilOp(rs.stencilOpFail, rs.stencilOpZFail, rs.stencilOpZPass));

	m_currentRenderStateList = renderStateObject;
}

void ContextOpenGL::bindSamplerStateObject(GLenum textureTarget, uint32_t samplerStateObject, bool haveMips, GLfloat maxAnisotropy)
{
	T_ASSERT (samplerStateObject > 0);
	T_ASSERT (samplerStateObject <= m_samplerStateList.size());
	const SamplerStateOpenGL& ss = m_samplerStateList[samplerStateObject - 1];

	T_OGL_SAFE(glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, ss.wrapS));
	T_OGL_SAFE(glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, ss.wrapT));

	if (textureTarget != GL_TEXTURE_2D)
		T_OGL_SAFE(glTexParameteri(textureTarget, GL_TEXTURE_WRAP_R, ss.wrapR));

	T_OGL_SAFE(glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, ss.magFilter));

	if (haveMips)
	{
		T_OGL_SAFE(glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, ss.minFilter));
		if (maxAnisotropy > 0.0f)
			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy));
	}
	else
		{ T_OGL_SAFE(glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR)); }
}

void ContextOpenGL::setPermitDepth(bool permitDepth)
{
	m_permitDepth = permitDepth;
}

void ContextOpenGL::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextOpenGL::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (!m_deleteResources.empty())
	{
		for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
			(*i)->deleteResource();
		m_deleteResources.resize(0);
	}
}

	}
}

#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/Extensions.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

typedef RefArray< ContextOpenGL > context_stack_t;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGL", ContextOpenGL, IContext)

ThreadLocal ContextOpenGL::ms_contextStack;

#if defined(_WIN32)

ContextOpenGL::ContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
:	m_hWnd(hWnd)
,	m_hDC(hDC)
,	m_hRC(hRC)
,	m_currentStateList(0)
,	m_width(0)
,	m_height(0)

#elif defined(__APPLE__)

ContextOpenGL::ContextOpenGL(void* context)
:	m_context(context)
,	m_currentStateList(0)
,	m_width(0)
,	m_height(0)

#else	// LINUX

ContextOpenGL::ContextOpenGL(Display* display, Window window, GLXContext context)
:	m_display(display)
,	m_window(window)
,	m_context(context)
,	m_currentStateList(0)
,	m_width(0)
,	m_height(0)

#endif
{
	std::memset(&m_renderState, 0, sizeof(m_renderState));
	update();
}

ContextOpenGL::~ContextOpenGL()
{
#if defined(_WIN32)
	T_ASSERT (!m_hRC);
#elif defined(__APPLE__)
	T_ASSERT (!m_context);
#else	// LINUX
	T_ASSERT (!m_context);
#endif
}

void ContextOpenGL::share(ContextOpenGL* context)
{
#if defined(_WIN32)
	wglShareLists(context->m_hRC, m_hRC);
	wglShareLists(m_hRC, context->m_hRC);
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
#endif
}

void ContextOpenGL::swapBuffers(bool waitVBlank)
{
#if defined(_WIN32)
	SwapBuffers(m_hDC);
#elif defined(__APPLE__)
	cglwSwapBuffers(m_context, waitVBlank);
#else	// LINUX
	glXSwapBuffers(m_display, m_window);
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

#else	// LINUX

	if (m_context)
	{
		glXDestroyContext(m_display, m_context);
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

#else	// LINUX

	if (m_window)
		glXMakeCurrent(
			m_display,
			m_window,
			m_context
		);

#endif

	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	if (!stack)
	{
		stack = new context_stack_t();
		ms_contextStack.set(stack);
	}

	stack->push_back(this);

	return true;
}

void ContextOpenGL::leave()
{
	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());

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

#endif

	m_lock.release();
}

void ContextOpenGL::enable(GLenum state)
{
	if (!m_enableStates[state])
	{
		T_OGL_SAFE(glEnable(state));
		m_enableStates[state] = true;
	}
}

void ContextOpenGL::disable(GLenum state)
{
	if (m_enableStates[state])
	{
		T_OGL_SAFE(glDisable(state));
		m_enableStates[state] = false;
	}
}

void ContextOpenGL::setRenderState(const RenderState& renderState, bool invertCull)
{
	if (renderState.cullFaceEnable)
	{
		if (!m_renderState.cullFaceEnable)
		{
			T_OGL_SAFE(glEnable(GL_CULL_FACE));
			m_renderState.cullFaceEnable = true;
		}
		if (renderState.cullFace != m_renderState.cullFace)
		{
			GLuint cullFace = renderState.cullFace;
			if (invertCull)
			{
				if (cullFace == GL_FRONT)
					cullFace = GL_BACK;
				else
					cullFace = GL_FRONT;
			}
			T_OGL_SAFE(glCullFace(cullFace));
			m_renderState.cullFace = cullFace;
		}
	}
	else
	{
		if (m_renderState.cullFaceEnable)
		{
			T_OGL_SAFE(glDisable(GL_CULL_FACE));
			m_renderState.cullFaceEnable = false;
		}
	}

	if (renderState.blendEnable)
	{
		if (!m_renderState.blendEnable)
		{
			T_OGL_SAFE(glEnable(GL_BLEND));
			m_renderState.blendEnable = true;
		}
		if (renderState.blendFuncSrc != m_renderState.blendFuncSrc || renderState.blendFuncDest != m_renderState.blendFuncDest)
		{
			T_OGL_SAFE(glBlendFunc(renderState.blendFuncSrc, renderState.blendFuncDest));
			m_renderState.blendFuncSrc = renderState.blendFuncSrc;
			m_renderState.blendFuncDest = renderState.blendFuncDest;
		}
		if (renderState.blendEquation != m_renderState.blendEquation)
		{
			T_OGL_SAFE(glBlendEquationEXT(renderState.blendEquation));
			m_renderState.blendEquation = renderState.blendEquation;
		}
	}
	else
	{
		if (m_renderState.blendEnable)
		{
			T_OGL_SAFE(glDisable(GL_BLEND));
			m_renderState.blendEnable = false;
		}
	}

	if (renderState.alphaTestEnable)
	{
		if (!m_renderState.alphaTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_ALPHA_TEST));
			m_renderState.alphaTestEnable = true;
		}
		if (renderState.alphaFunc != m_renderState.alphaFunc || renderState.alphaRef != m_renderState.alphaRef)
		{
			T_OGL_SAFE(glAlphaFunc(renderState.alphaFunc, renderState.alphaRef));
			m_renderState.alphaFunc = renderState.alphaFunc;
			m_renderState.alphaRef = renderState.alphaRef;
		}
	}
	else
	{
		if (m_renderState.alphaTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_ALPHA_TEST));
			m_renderState.alphaTestEnable = false;
		}
	}

	if (renderState.depthTestEnable)
	{
		if (!m_renderState.depthTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
			m_renderState.depthTestEnable = true;
		}
		if (renderState.depthFunc != m_renderState.depthFunc)
		{
			T_OGL_SAFE(glDepthFunc(renderState.depthFunc));
			m_renderState.depthFunc = renderState.depthFunc;
		}
	}
	else
	{
		if (m_renderState.depthTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_DEPTH_TEST));
			m_renderState.depthTestEnable = false;
		}
	}

	if (renderState.colorMask != m_renderState.colorMask)
	{
		T_OGL_SAFE(glColorMask(
			(renderState.colorMask & RenderState::CmRed) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmGreen) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmBlue) ? GL_TRUE : GL_FALSE,
			(renderState.colorMask & RenderState::CmAlpha) ? GL_TRUE : GL_FALSE
		));
		m_renderState.colorMask = renderState.colorMask;
	}

	if (renderState.depthMask != m_renderState.depthMask)
	{
		T_OGL_SAFE(glDepthMask(renderState.depthMask));
		m_renderState.depthMask = renderState.depthMask;
	}

	if (renderState.stencilTestEnable)
	{
		if (!m_renderState.stencilTestEnable)
		{
			T_OGL_SAFE(glEnable(GL_STENCIL_TEST));
			m_renderState.stencilTestEnable = true;
		}
		if (renderState.stencilFunc != m_renderState.stencilFunc || renderState.stencilRef != m_renderState.stencilRef)
		{
			T_OGL_SAFE(glStencilFunc(renderState.stencilFunc, renderState.stencilRef, ~0UL));
			m_renderState.stencilFunc = renderState.stencilFunc;
			m_renderState.stencilRef = renderState.stencilRef;
		}
		if (renderState.stencilOpFail != m_renderState.stencilOpFail || renderState.stencilOpZFail != m_renderState.stencilOpZFail || renderState.stencilOpZPass != m_renderState.stencilOpZPass)
		{
			T_OGL_SAFE(glStencilOp(
				renderState.stencilOpFail,
				renderState.stencilOpZFail,
				renderState.stencilOpZPass
			));
			m_renderState.stencilOpFail = renderState.stencilOpFail;
			m_renderState.stencilOpZFail = renderState.stencilOpZFail;
			m_renderState.stencilOpZPass = renderState.stencilOpZPass;
		}
	}
	else
	{
		if (m_renderState.stencilTestEnable)
		{
			T_OGL_SAFE(glDisable(GL_STENCIL_TEST));
			m_renderState.stencilTestEnable = false;
		}
	}
}

GLhandleARB ContextOpenGL::createShaderObject(const char* shader, GLenum shaderType)
{
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;
	
	Adler32 adler;
	adler.begin();
	adler.feed(shader, strlen(shader));
	adler.end();
	
	uint32_t hash = adler.get();
	
	std::map< uint32_t, GLhandleARB >::const_iterator i = m_shaderObjects.find(hash);
	if (i != m_shaderObjects.end())
		return i->second;

	GLhandleARB shaderObject = glCreateShaderObjectARB(shaderType);
	T_OGL_SAFE(glShaderSourceARB(shaderObject, 1, &shader, NULL));
	T_OGL_SAFE(glCompileShaderARB(shaderObject));

	T_OGL_SAFE(glGetObjectParameterivARB(shaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &status));
	if (status != 1)
	{
		T_OGL_SAFE(glGetInfoLogARB(shaderObject, sizeof(errorBuf), &errorBufLen, errorBuf));
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

void ContextOpenGL::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextOpenGL::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
		(*i)->deleteResource();
	m_deleteResources.resize(0);
}

	}
}

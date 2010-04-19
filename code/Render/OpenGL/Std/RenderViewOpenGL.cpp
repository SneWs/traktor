#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/VertexBufferOpenGL.h"
#include "Render/OpenGL/IndexBufferOpenGL.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderViewOpenGL.h"
#include "Render/OpenGL/Std/RenderSystemOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewOpenGL", RenderViewOpenGL, IRenderView)

#if defined(_WIN32)

RenderViewOpenGL::RenderViewOpenGL(
	const RenderViewDesc desc,
	ContextOpenGL* context,
	ContextOpenGL* globalContext,
	HWND hWnd
)
:	m_context(context)
,	m_globalContext(globalContext)
,	m_currentDirty(true)

#elif defined(__APPLE__)

RenderViewOpenGL::RenderViewOpenGL(
	const RenderViewDesc desc,
	ContextOpenGL* context,
	ContextOpenGL* globalContext
)
:	m_context(context)
,	m_globalContext(globalContext)
,	m_currentDirty(true)

#else

RenderViewOpenGL::RenderViewOpenGL(
	const RenderViewDesc desc,
	ContextOpenGL* context,
	ContextOpenGL* globalContext
)
:	m_context(context)
,	m_globalContext(globalContext)
,	m_currentDirty(true)

#endif
{
	T_CONTEXT_SCOPE(m_globalContext)

	Viewport viewport;
	viewport.left = 0;
	viewport.top = 0;
	viewport.width = m_context->getWidth();
	viewport.height = m_context->getHeight();
	viewport.nearZ = 0.0f;
	viewport.farZ = 1.0f;
	setViewport(viewport);

	m_primaryTargetDesc.count = 1;
	m_primaryTargetDesc.width = m_context->getWidth();
	m_primaryTargetDesc.height = m_context->getHeight();
	m_primaryTargetDesc.multiSample = desc.multiSample;
	m_primaryTargetDesc.depthStencil = bool(desc.depthBits > 0 || desc.stencilBits > 0);
	m_primaryTargetDesc.targets[0].format = TfR8G8B8A8;

	if (m_primaryTargetDesc.width > 0 && m_primaryTargetDesc.height > 0)
	{
		m_primaryTarget = new RenderTargetSetOpenGL(m_context);
		m_primaryTarget->create(m_primaryTargetDesc);
	}
}

RenderViewOpenGL::~RenderViewOpenGL()
{
}

void RenderViewOpenGL::close()
{
	safeDestroy(m_primaryTarget);
	safeDestroy(m_context);
}

bool RenderViewOpenGL::reset(const RenderViewDefaultDesc& desc)
{
	return false;
}

void RenderViewOpenGL::resize(int32_t width, int32_t height)
{
	safeDestroy(m_primaryTarget);

	m_context->update();

	{
		T_CONTEXT_SCOPE(m_globalContext)

		m_primaryTargetDesc.width = m_context->getWidth();
		m_primaryTargetDesc.height = m_context->getHeight();

		m_primaryTarget = new RenderTargetSetOpenGL(m_context);
		m_primaryTarget->create(m_primaryTargetDesc);
	}
}

int RenderViewOpenGL::getWidth() const
{
	return m_context->getWidth();
}

int RenderViewOpenGL::getHeight() const
{
	return m_context->getHeight();
}

bool RenderViewOpenGL::isFullScreen() const
{
	return false;
}

void RenderViewOpenGL::setViewport(const Viewport& viewport)
{
	T_CONTEXT_SCOPE(m_context)

	T_OGL_SAFE(glViewport(
		viewport.left,
		viewport.top,
		viewport.width,
		viewport.height
	));

	T_OGL_SAFE(glDepthRange(
		viewport.nearZ,
		viewport.farZ
	));
}

Viewport RenderViewOpenGL::getViewport()
{
	T_CONTEXT_SCOPE(m_context)

	GLint ext[4];
	T_OGL_SAFE(glGetIntegerv(GL_VIEWPORT, ext));

	GLfloat range[2];
	T_OGL_SAFE(glGetFloatv(GL_DEPTH_RANGE, range));

	Viewport viewport;
	viewport.left = ext[0];
	viewport.top = ext[1];
	viewport.width = ext[2];
	viewport.height = ext[3];
	viewport.nearZ = range[0];
	viewport.farZ = range[1];

	return viewport;
}

bool RenderViewOpenGL::begin()
{
	if (!m_primaryTarget)
		return false;

	m_context->enter();

	T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
	T_OGL_SAFE(glDepthFunc(GL_LEQUAL));

	return begin(m_primaryTarget, 0, false);
}

bool RenderViewOpenGL::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
{
	T_OGL_SAFE(glPushAttrib(GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT));

	RenderTargetSetOpenGL* rts = checked_type_cast< RenderTargetSetOpenGL* >(renderTargetSet);
	RenderTargetOpenGL* rt = checked_type_cast< RenderTargetOpenGL* >(rts->getColorTexture(renderTarget));
	
	rt->bind(keepDepthStencil);
	rt->enter(keepDepthStencil);

	m_renderTargetStack.push(rt);

	rts->setContentValid(true);
	m_currentDirty = true;

	return true;
}

void RenderViewOpenGL::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
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

	if (clearMask & CfColor)
	{
		float r = color[0];
		float g = color[1];
		float b = color[2];
		float a = color[3];
		T_OGL_SAFE(glClearColor(r, g, b, a));
		T_OGL_SAFE(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	}

	if (clearMask & CfDepth)
	{
		T_OGL_SAFE(glClearDepth(depth));
		T_OGL_SAFE(glDepthMask(GL_TRUE));
	}

	if (clearMask & CfStencil)
		T_OGL_SAFE(glClearStencil(stencil));

	T_OGL_SAFE(glClear(c_clearMask[clearMask]));
}

void RenderViewOpenGL::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	m_currentVertexBuffer = checked_type_cast< VertexBufferOpenGL* >(vertexBuffer);
	m_currentDirty = true;
}

void RenderViewOpenGL::setIndexBuffer(IndexBuffer* indexBuffer)
{
	m_currentIndexBuffer = checked_type_cast< IndexBufferOpenGL* >(indexBuffer);
	m_currentDirty = true;
}

void RenderViewOpenGL::setProgram(IProgram* program)
{
	m_currentProgram = checked_type_cast< ProgramOpenGL * >(program);
	m_currentDirty = true;
}

void RenderViewOpenGL::draw(const Primitives& primitives)
{
	if (m_currentDirty)
	{
		if (!m_currentProgram || !m_currentVertexBuffer)
			return;

		float targetSize[] = { m_context->getWidth(), m_context->getHeight() };
		if (!m_currentProgram->activate(targetSize))
			return;

		m_currentVertexBuffer->activate(
			m_currentProgram->getAttributeLocs()
		);

		m_currentDirty = false;
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
		T_ASSERT (0);
	}

	if (primitives.indexed)
	{
		T_ASSERT_M (m_currentIndexBuffer, L"No index buffer");

		GLenum indexType;
		GLint offsetMultiplier;

		switch (m_currentIndexBuffer->getIndexType())
		{
		case ItUInt16:
			indexType = GL_UNSIGNED_SHORT;
			offsetMultiplier = 2;
			break;

		case ItUInt32:
			indexType = GL_UNSIGNED_INT;
			offsetMultiplier = 4;
			break;
		}

		m_currentIndexBuffer->bind();

		const GLubyte* indices = static_cast< const GLubyte* >(m_currentIndexBuffer->getIndexData()) + primitives.offset * offsetMultiplier;

		T_OGL_SAFE(glDrawElements(
			primitiveType,
			vertexCount,
			indexType,
			indices
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

void RenderViewOpenGL::end()
{
	T_ASSERT (!m_renderTargetStack.empty());

	RenderTargetOpenGL* rt = m_renderTargetStack.top();
	m_renderTargetStack.pop();

	if (m_renderTargetStack.empty())
	{
		rt->blit();

		// Unbind primary target.
		T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
	}
	else
	{
		rt->resolve();

		// Rebind parent target.
		RenderTargetOpenGL* rt = m_renderTargetStack.top();
		rt->bind(false);
	}

	T_OGL_SAFE(glPopAttrib());
}

void RenderViewOpenGL::present()
{
	m_context->swapBuffers();
	m_context->leave();

	// Finished using view's context; restore global context and delete resources.
	m_globalContext->enter();
	m_globalContext->deleteResources();
	m_globalContext->leave();
}

void RenderViewOpenGL::pushMarker(const char* const marker)
{
}

void RenderViewOpenGL::popMarker()
{
}

	}
}

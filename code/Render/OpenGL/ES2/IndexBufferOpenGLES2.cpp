#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public IContext::IDeleteCallback
{
	GLuint m_bufferName;

	DeleteBufferCallback(GLuint bufferName)
	:	m_bufferName(bufferName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffers(1, &m_bufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferOpenGLES2", IndexBufferOpenGLES2, IndexBufferOpenGL)

IndexBufferOpenGLES2::IndexBufferOpenGLES2(IContext* context, IndexType indexType, uint32_t bufferSize, bool dynamic)
:	IndexBufferOpenGL(indexType, bufferSize)
,	m_context(context)
,	m_dynamic(dynamic)
{
	T_OGL_SAFE(glGenBuffers(1, &m_name));
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
	T_OGL_SAFE(glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, 0, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
}

IndexBufferOpenGLES2::~IndexBufferOpenGLES2()
{
	destroy();
}

void IndexBufferOpenGLES2::destroy()
{
	if (m_name)
	{
		if (m_context)
			m_context->deleteResource(new DeleteBufferCallback(m_name));
		m_name = 0;
	}
}

void* IndexBufferOpenGLES2::lock()
{
	if (m_buffer.ptr())
		return 0;

	int32_t bufferSize = getBufferSize();
	m_buffer.reset((uint8_t*)Alloc::acquireAlign(bufferSize, 16, "IB"));

	return m_buffer.ptr();
}

void IndexBufferOpenGLES2::unlock()
{
	if (!m_buffer.ptr())
		return;

	T_ANONYMOUS_VAR(IContext::Scope)(m_context);

	int32_t bufferSize = getBufferSize();
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
	T_OGL_SAFE(glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, m_buffer.ptr(), m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
	m_buffer.release();
}

void IndexBufferOpenGLES2::bind()
{
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
}

const GLvoid* IndexBufferOpenGLES2::getIndexData() const
{
	return 0;
}

	}
}

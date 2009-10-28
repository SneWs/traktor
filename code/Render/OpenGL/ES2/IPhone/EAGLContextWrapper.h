#ifndef traktor_render_EAGLContextWrapper_H
#define traktor_render_EAGLContextWrapper_H

#include "Render/OpenGL/Platform.h"

namespace traktor
{
	namespace render
	{

class EAGLContextWrapper
{
public:
	EAGLContextWrapper();
	
	bool create(void* nativeHandle, bool wantDepthBuffer);
	
	void destroy();
	
	void setCurrent();
	
	void swapBuffers();
	
	void resize(GLint width, GLint height);
	
	GLint getWidth() const { return m_width; }
	
	GLint getHeight() const { return m_height; }
	
	bool landscape() const { return m_landscape; }
	
private:
	void* m_layer;
	void* m_context;
	GLuint m_frameBuffer;
	GLuint m_renderBuffer;
	GLuint m_depthRenderBuffer;
	bool m_wantDepthBuffer;
	GLint m_width;
	GLint m_height;
	bool m_landscape;
	
	void createFrameBuffer();
	
	void destroyFrameBuffer();
};

	}
}

#endif	// traktor_render_EAGLContextWrapper_H

#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"
#include "Render/OpenGL/Std/UtilitiesOpenGL.h"
#include "Render/OpenGL/Std/VolumeTextureOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public ResourceContextOpenGL::IDeleteCallback
{
	GLuint m_textureName;

	DeleteTextureCallback(GLuint textureName)
	:	m_textureName(textureName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteTextures(1, &m_textureName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureOpenGL", VolumeTextureOpenGL, IVolumeTexture)

VolumeTextureOpenGL::VolumeTextureOpenGL(ResourceContextOpenGL* resourceContext)
:	m_resourceContext(resourceContext)
,	m_textureName(0)
,	m_width(0)
,	m_height(0)
,	m_depth(0)
,	m_pixelSize(0)
,	m_components(0)
,	m_format(0)
,	m_type(0)
{
}

VolumeTextureOpenGL::~VolumeTextureOpenGL()
{
	destroy();
}

bool VolumeTextureOpenGL::create(const VolumeTextureCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;
	m_depth = desc.depth;

	if (desc.sRGB)
	{
		if (!convertTextureFormat_sRGB(desc.format, m_pixelSize, m_components, m_format, m_type))
			return false;
	}
	else
	{
		if (!convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type))
			return false;
	}

	T_OGL_SAFE(glGenTextures(1, &m_textureName));

	if (desc.immutable)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		T_OGL_SAFE(glBindTexture(GL_TEXTURE_3D, m_textureName));

		T_OGL_SAFE(glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		T_OGL_SAFE(glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

		if (desc.format >= TfDXT1 && desc.format <= TfDXT5)
		{
			uint32_t mipPitch = getTextureMipPitch(desc.format, m_width, m_height);
			T_OGL_SAFE(glCompressedTexImage3D(
				GL_TEXTURE_3D,
				0,
				m_components,
				m_width,
				m_height,
				m_depth,
				0,
				mipPitch,
				desc.initialData[0].data
			));
		}
		else
		{
			T_OGL_SAFE(glTexImage3D(
				GL_TEXTURE_3D,
				0,
				m_components,
				m_width,
				m_height,
				m_depth,
				0,
				m_format,
				m_type,
				desc.initialData[0].data
			));
		}
	}

	return true;
}

void VolumeTextureOpenGL::destroy()
{
	if (m_textureName)
	{
		m_resourceContext->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

ITexture* VolumeTextureOpenGL::resolve()
{
	return this;
}

int32_t VolumeTextureOpenGL::getMips() const
{
	return 1;
}

int32_t VolumeTextureOpenGL::getWidth() const
{
	return m_width;
}

int32_t VolumeTextureOpenGL::getHeight() const
{
	return m_height;
}

int32_t VolumeTextureOpenGL::getDepth() const
{
	return m_depth;
}

void VolumeTextureOpenGL::bindTexture(GLuint textureUnit) const
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + textureUnit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_3D, m_textureName));
}

void VolumeTextureOpenGL::bindImage(GLuint imageUnit) const
{
}

void VolumeTextureOpenGL::bindSize(GLint locationSize) const
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(m_depth), GLfloat(1.0f)));
}

bool VolumeTextureOpenGL::haveMips() const
{
	return false;
}

	}
}

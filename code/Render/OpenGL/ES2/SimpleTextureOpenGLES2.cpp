#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public IContext::IDeleteCallback
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

bool convertTextureFormat(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType)
{
	switch (textureFormat)
	{
#if defined(__APPLE__) && defined(GL_RED_EXT)
	case TfR8:
		outPixelSize = 1;
		outComponents = GL_RED_EXT;
		outFormat = GL_RED_EXT;
		outType = GL_UNSIGNED_BYTE;
		break;
#endif

	case TfR8G8B8A8:
		outPixelSize = 4;
		outComponents = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR32G32B32A32F:
		outPixelSize = 16;
		outComponents = 4;
		outFormat = GL_RGBA;
		outType = GL_FLOAT;
		break;
		
#if defined(__APPLE__) && defined(GL_RED_EXT)
	case TfR16F:
		outPixelSize = 2;
		outComponents = GL_RED_EXT;
		outFormat = GL_RED_EXT;
		outType = GL_HALF_FLOAT_OES;
		break;
#endif
	
#if defined(__APPLE__) && defined(GL_RED_EXT)
	case TfR32F:
		outPixelSize = 4;
		outComponents = GL_RED_EXT;
		outFormat = GL_RED_EXT;
		outType = GL_FLOAT;
		break;
#endif

#if defined(GL_IMG_texture_compression_pvrtc)
	case TfPVRTC1:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC2:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC3:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC4:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;
#endif

	default:
		return false;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureOpenGLES2", SimpleTextureOpenGLES2, ISimpleTexture)

SimpleTextureOpenGLES2::SimpleTextureOpenGLES2(IContext* context)
:	m_context(context)
,	m_textureName(0)
,	m_width(0)
,	m_height(0)
,	m_pixelSize(0)
,	m_mipCount(0)
{
	std::memset(&m_shadowState, 0, sizeof(m_shadowState));
}

SimpleTextureOpenGLES2::~SimpleTextureOpenGLES2()
{
	destroy();
}

bool SimpleTextureOpenGLES2::create(const SimpleTextureCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;

	if (!convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type))
	{
		log::error << L"Unable to convert texture format" << Endl;
		return false;
	}

	T_OGL_SAFE(glGenTextures(1, &m_textureName));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));
	T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

	// Set default parameters as its might help driver.
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	// Allocate data buffer.
	uint32_t texturePitch = getTextureMipPitch(desc.format, desc.width, desc.height);
	if (desc.immutable)
	{
		for (int i = 0; i < desc.mipCount; ++i)
		{
			uint32_t width = std::max(m_width >> i, 1);
			uint32_t height = std::max(m_height >> i, 1);

			if (
				(desc.format >= TfDXT1 && desc.format <= TfDXT5) ||
				(desc.format >= TfPVRTC1 && desc.format <= TfPVRTC4)
			)
			{
				uint32_t mipSize = getTextureMipPitch(desc.format, width, height);
				T_OGL_SAFE(glCompressedTexImage2D(
					GL_TEXTURE_2D,
					i,
					m_components,
					width,
					height,
					0,
					mipSize,
					desc.initialData[i].data
				));
			}
			else
			{
				T_OGL_SAFE(glTexImage2D(
					GL_TEXTURE_2D,
					i,
					m_components,
					width,
					height,
					0,
					m_format,
					m_type,
					desc.initialData[i].data
				));
			}
		}
		
		T_OGL_SAFE(glFlush());
	}
	else
		m_data.resize(texturePitch);

	m_mipCount = desc.mipCount;
	return true;
}

void SimpleTextureOpenGLES2::destroy()
{
	if (m_textureName)
	{
		if (m_context)
			m_context->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

ITexture* SimpleTextureOpenGLES2::resolve()
{
	return this;
}

int SimpleTextureOpenGLES2::getWidth() const
{
	return m_width;
}

int SimpleTextureOpenGLES2::getHeight() const
{
	return m_height;
}

bool SimpleTextureOpenGLES2::lock(int level, Lock& lock)
{
	if (m_data.empty() || level >= m_mipCount)
		return false;

	lock.pitch = std::max(m_width >> level, 1) * m_pixelSize;
	lock.bits = &m_data[0];
	return true;
}

void SimpleTextureOpenGLES2::unlock(int level)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_context);
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));
	T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	T_OGL_SAFE(glTexImage2D(
		GL_TEXTURE_2D,
		level,
		m_components,
		std::max(m_width >> level, 1),
		std::max(m_height >> level, 1),
		0,
		m_format,
		m_type,
		&m_data[0]
	));
	T_OGL_SAFE(glFlush());
}

void SimpleTextureOpenGLES2::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));

	GLenum minFilter = GL_NEAREST;
	if (m_mipCount > 1)
		minFilter = samplerState.minFilter;

	if (m_shadowState.minFilter != minFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
		m_shadowState.minFilter = minFilter;
	}

	if (m_shadowState.magFilter != samplerState.magFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		m_shadowState.magFilter = samplerState.magFilter;
	}

	if (m_shadowState.wrapS != samplerState.wrapS)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, samplerState.wrapS));
		m_shadowState.wrapS = samplerState.wrapS;
	}

	if (m_shadowState.wrapT != samplerState.wrapT)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, samplerState.wrapT));
		m_shadowState.wrapT = samplerState.wrapT;
	}

	T_OGL_SAFE(glUniform1i(locationTexture, unit));
}

void SimpleTextureOpenGLES2::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(0), GLfloat(0)));
}

	}
}

#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Render/IVolumeTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ES2/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES2;

/*!
 * \ingroup OGL
 */
class VolumeTextureOpenGLES2
:	public IVolumeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	VolumeTextureOpenGLES2(ContextOpenGLES2* resourceContext);

	virtual ~VolumeTextureOpenGLES2();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getDepth() const override final;

	// ITextureBinding

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) override final;

	virtual void bindSize(GLint locationSize) override final;

private:
	Ref< ContextOpenGLES2 > m_resourceContext;
	GLuint m_textureName;
	int32_t m_width;
	int32_t m_height;
	int32_t m_depth;
	int32_t m_pixelSize;
	uint32_t m_mipCount;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	AutoArrayPtr< uint8_t > m_data;
	SamplerStateOpenGL m_shadowState;
};

	}
}

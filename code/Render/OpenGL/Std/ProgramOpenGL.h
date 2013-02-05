#ifndef traktor_render_ProgramOpenGL_H
#define traktor_render_ProgramOpenGL_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/IProgram.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGL;
class GlslProgram;
class ProgramResource;

/*!
 * \ingroup OGL
 */
class ProgramOpenGL : public IProgram
{
	T_RTTI_CLASS;

public:
	virtual ~ProgramOpenGL();

	static Ref< ProgramResource > compile(const GlslProgram& glslProgram, int optimize, bool validate);

	static Ref< ProgramOpenGL > create(ContextOpenGL* resourceContext, const ProgramResource* resource);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	bool activate(ContextOpenGL* renderContext, float targetSize[2]);

	const GLint* getAttributeLocs() const;

private:
	struct Uniform
	{
		GLint location;
		GLenum type;
		uint32_t offset;
		uint32_t length;
		bool dirty;
	};

	struct Sampler
	{
		GLint location;
		uint32_t texture;
		uint32_t stage;
		GLuint object[2];	// 0 - no mips, 1 - including mips
	};

	struct TextureSize
	{
		GLint location;
		uint32_t texture;
	};

	Ref< ContextOpenGL > m_resourceContext;
	GLuint m_program;
	RenderStateOpenGL m_renderState;
	GLuint m_renderStateList;
	GLint m_locationTargetSize;
	GLint m_attributeLocs[T_OGL_MAX_USAGE_INDEX];			//!< Vertex attribute locations.
	SmallMap< handle_t, uint32_t > m_parameterMap;			//!< Parameter to data map.
	std::vector< Uniform > m_uniforms;						//!< Scalar uniforms.
	std::vector< Sampler > m_samplers;						//!< Samplers.
	std::vector< TextureSize > m_textureSize;
	AlignedVector< float > m_uniformData;					//!< Scalar uniform data.
	RefArray< ITexture > m_textures;
	float m_targetSize[2];
	bool m_textureDirty;
	static ProgramOpenGL* ms_activeProgram;
	
	ProgramOpenGL(ContextOpenGL* resourceContext, GLuint program, const ProgramResource* resource);
};

	}
}

#endif	// traktor_render_ProgramOpenGL_H

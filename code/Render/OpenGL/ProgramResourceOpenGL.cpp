#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberSamplerState : public MemberComplex
{
public:
	MemberSamplerState(const std::wstring& name, SamplerState& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}
	
	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< GLenum >(L"minFilter", m_ref.minFilter);
		s >> Member< GLenum >(L"magFilter", m_ref.magFilter);
		s >> Member< GLenum >(L"wrapS", m_ref.wrapS);
		s >> Member< GLenum >(L"wrapT", m_ref.wrapT);
		return true;
	}
	
private:
	SamplerState& m_ref;
};

class MemberRenderState : public MemberComplex
{
public:
	MemberRenderState(const std::wstring& name, RenderState& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}
	
	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< GLboolean >(L"cullFaceEnable", m_ref.cullFaceEnable);
		s >> Member< GLenum >(L"cullFace", m_ref.cullFace);
		s >> Member< GLboolean >(L"blendEnable", m_ref.blendEnable);
		s >> Member< GLenum >(L"blendEquation", m_ref.blendEquation);
		s >> Member< GLenum >(L"blendFuncSrc", m_ref.blendFuncSrc);
		s >> Member< GLenum >(L"blendFuncDest", m_ref.blendFuncDest);
		s >> Member< GLboolean >(L"depthTestEnable", m_ref.depthTestEnable);
		s >> Member< uint32_t >(L"colorMask", m_ref.colorMask);
		s >> Member< GLboolean >(L"alphaTestEnable", m_ref.alphaTestEnable);
		s >> Member< GLenum >(L"alphaFunc", m_ref.alphaFunc);
		s >> Member< GLclampf >(L"alphaRef", m_ref.alphaRef);
		s >> Member< GLenum >(L"stencilFunc", m_ref.stencilFunc);
		s >> Member< GLint >(L"stencilRef", m_ref.stencilRef);
		s >> Member< GLenum >(L"stencilOpFail", m_ref.stencilOpFail);
		s >> Member< GLenum >(L"stencilOpZFail", m_ref.stencilOpZFail);
		s >> Member< GLenum >(L"stencilOpZPass", m_ref.stencilOpZPass);
		s >> MemberStaticArray< SamplerState, 16, MemberSamplerState >(L"samplerStates", m_ref.samplerStates);
		return true;
	}
	
private:
	RenderState& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceOpenGL", 2, ProgramResourceOpenGL, ProgramResource)

ProgramResourceOpenGL::ProgramResourceOpenGL()
{
}

ProgramResourceOpenGL::ProgramResourceOpenGL(
	const std::wstring& vertexShader,
	const std::wstring& fragmentShader,
	const std::map< std::wstring, int32_t >& samplerTextures,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
,	m_samplerTextures(samplerTextures)
,	m_renderState(renderState)
{
}

bool ProgramResourceOpenGL::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 2);

	s >> Member< std::wstring >(L"vertexShader", m_vertexShader);
	s >> Member< std::wstring >(L"fragmentShader", m_fragmentShader);
	s >> MemberStlMap< std::wstring, int32_t >(L"samplerTextures", m_samplerTextures);
	s >> MemberRenderState(L"renderState", m_renderState);

	return true;
}

	}
}

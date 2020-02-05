#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/Editor/Image2/IgaTarget.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

constexpr static MemberEnum< TextureFormat >::Key c_TextureFormat_Keys[] =
{
	{ L"TfInvalid", TfInvalid },
	{ L"TfR8", TfR8 },
	{ L"TfR8G8B8A8", TfR8G8B8A8 },
	{ L"TfR5G6B5", TfR5G6B5 },
	{ L"TfR5G5B5A1", TfR5G5B5A1 },
	{ L"TfR4G4B4A4", TfR4G4B4A4 },
	{ L"TfR16G16B16A16F", TfR16G16B16A16F },
	{ L"TfR32G32B32A32F", TfR32G32B32A32F },
	{ L"TfR16G16F", TfR16G16F },
	{ L"TfR32G32F", TfR32G32F },
	{ L"TfR16F", TfR16F },
	{ L"TfR32F", TfR32F },
	{ L"TfR11G11B10F", TfR11G11B10F },
	{ L"TfDXT1", TfDXT1 },
	{ L"TfDXT2", TfDXT2 },
	{ L"TfDXT3", TfDXT3 },
	{ L"TfDXT4", TfDXT4 },
	{ L"TfDXT5", TfDXT5 },
	{ L"TfPVRTC1", TfPVRTC1 },
	{ L"TfPVRTC2", TfPVRTC2 },
	{ L"TfPVRTC3", TfPVRTC3 },
	{ L"TfPVRTC4", TfPVRTC4 },
	{ L"TfETC1", TfETC1 },
	{ 0 }
};

class MemberTextureFormat : public MemberEnum< TextureFormat >
{
public:
	MemberTextureFormat(const wchar_t* const name, TextureFormat& en)
	:   MemberEnum< TextureFormat >(name, en, c_TextureFormat_Keys)
	{
	}
};

class MemberRenderGraphTargetSetDesc : public MemberComplex
{
public:
	typedef RenderGraphTargetSetDesc value_type;

	MemberRenderGraphTargetSetDesc(const wchar_t* const name, value_type& ref)
	:   MemberComplex(name, true)
	,   m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< int32_t >(L"count", m_ref.count);
		s >> Member< int32_t >(L"width", m_ref.width);
		s >> Member< int32_t >(L"height", m_ref.height);
		s >> Member< int32_t >(L"screenWidthDenom", m_ref.screenWidthDenom);
		s >> Member< int32_t >(L"screenHeightDenom", m_ref.screenHeightDenom);
		s >> Member< int32_t >(L"maxWidth", m_ref.maxWidth);
		s >> Member< int32_t >(L"maxHeight", m_ref.maxHeight);
		s >> Member< bool >(L"createDepthStencil", m_ref.createDepthStencil);
		s >> Member< bool >(L"usingDepthStencilAsTexture", m_ref.usingDepthStencilAsTexture);
		s >> Member< bool >(L"generateMips", m_ref.generateMips);
		s >> MemberStaticArray< TextureFormat, RenderGraphTargetSetDesc::MaxColorTargets, MemberTextureFormat >(L"colorFormats", m_ref.colorFormats);
	}

private:
	value_type& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.IgaTarget", 0, IgaTarget, ISerializable)

IgaTarget::IgaTarget()
{
	m_position[0] =
	m_position[1] = 0;
}

IgaTarget::IgaTarget(const std::wstring& name)
:   m_name(name)
{
	m_position[0] =
	m_position[1] = 0;
}

void IgaTarget::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& IgaTarget::getName() const
{
	return m_name;
}

void IgaTarget::setPosition(int32_t x, int32_t y)
{
	m_position[0] = x;
	m_position[1] = y;
}

const int32_t* IgaTarget::getPosition() const
{
	return m_position;
}

void IgaTarget::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberStaticArray< int32_t, 2 >(L"position", m_position, AttributePrivate());
	s >> MemberRenderGraphTargetSetDesc(L"targetSetDesc", m_targetSetDesc);
}

	}
}
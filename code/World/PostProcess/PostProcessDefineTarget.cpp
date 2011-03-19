#include "World/PostProcess/PostProcessDefineTarget.h"
#include "World/PostProcess/PostProcess.h"
#include "Render/IRenderSystem.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessDefineTarget", 1, PostProcessDefineTarget, PostProcessDefine)

PostProcessDefineTarget::PostProcessDefineTarget()
:	m_width(0)
,	m_height(0)
,	m_screenDenom(0)
,	m_format(render::TfInvalid)
,	m_depthStencil(false)
,	m_preferTiled(false)
,	m_multiSample(0)
{
}

bool PostProcessDefineTarget::define(PostProcess* postProcess, render::IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight)
{
	render::RenderTargetSetCreateDesc desc;

	desc.count = 1;
	desc.width = m_width + (m_screenDenom ? screenWidth / m_screenDenom : 0);
	desc.height = m_height + (m_screenDenom ? screenHeight / m_screenDenom : 0);
	desc.multiSample = m_multiSample;
	desc.createDepthStencil = m_depthStencil;
	desc.usingPrimaryDepthStencil = false;
	desc.preferTiled = m_preferTiled;
	desc.targets[0].format = m_format;

	Ref< render::RenderTargetSet > renderTargetSet = renderSystem->createRenderTargetSet(desc);
	if (!renderTargetSet)
	{
		log::error << L"Unable to create render target (" << desc.width << L"*" << desc.height << L", msaa " << desc.multiSample << L")" << Endl;
		return false;
	}

	postProcess->getTargetRef(render::getParameterHandle(m_id)) = renderTargetSet;

	return true;
}

bool PostProcessDefineTarget::serialize(ISerializer& s)
{
	const MemberEnum< render::TextureFormat >::Key kFormats[] =
	{
		{ L"TfR8", render::TfR8 },
		{ L"TfR8G8B8A8", render::TfR8G8B8A8 },
		{ L"TfR16G16B16A16F", render::TfR16G16B16A16F },
		{ L"TfR32G32B32A32F", render::TfR32G32B32A32F },
		{ L"TfR16F", render::TfR16F },
		{ L"TfR32F", render::TfR32F },
		{ 0, 0 }
	};

	s >> Member< std::wstring >(L"id", m_id);
	s >> Member< uint32_t >(L"width", m_width);
	s >> Member< uint32_t >(L"height", m_height);
	s >> Member< uint32_t >(L"screenDenom", m_screenDenom);
	s >> MemberEnum< render::TextureFormat >(L"format", m_format, kFormats);
	s >> Member< bool >(L"depthStencil", m_depthStencil);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"preferTiled", m_preferTiled);
	s >> Member< int32_t >(L"multiSample", m_multiSample);

	return true;
}

	}
}

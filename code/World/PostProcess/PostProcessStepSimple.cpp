#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/RenderTargetSet.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcessStepSimple.h"
#include "World/PostProcess/PostProcess.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSimple", 0, PostProcessStepSimple, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSimple::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	if (!resourceManager->bind(m_shader))
		return false;

	std::vector< InstanceSimple::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = render::getParameterHandle(m_sources[i].param);
		sources[i].paramSize = render::getParameterHandle(m_sources[i].param + L"_Size");
		sources[i].source = render::getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	return new InstanceSimple(this, sources);
}

bool PostProcessStepSimple::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	return true;
}

PostProcessStepSimple::Source::Source()
:	index(0)
{
}

bool PostProcessStepSimple::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
	return true;
}

// Instance

PostProcessStepSimple::InstanceSimple::InstanceSimple(const PostProcessStepSimple* step, const std::vector< Source >& sources)
:	m_step(step)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = render::getParameterHandle(L"Time");
	m_handleDeltaTime = render::getParameterHandle(L"DeltaTime");
	m_handleDepthRange = render::getParameterHandle(L"DepthRange");
}

void PostProcessStepSimple::InstanceSimple::destroy()
{
}

void PostProcessStepSimple::InstanceSimple::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	resource::Proxy< render::Shader > shader = m_step->m_shader;
	if (!shader.validate())
		return;

	postProcess->prepareShader(shader);

	shader->setFloatParameter(m_handleTime, m_time);
	shader->setFloatParameter(m_handleDeltaTime, params.deltaTime);
	shader->setFloatParameter(m_handleDepthRange, params.depthRange);

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		Ref< render::RenderTargetSet > source = postProcess->getTargetRef(i->source);
		if (source)
		{
			shader->setTextureParameter(i->param, source->getColorTexture(i->index));
			shader->setVectorParameter(i->paramSize, Vector4(
				float(source->getWidth()),
				float(source->getHeight()),
				0.0f,
				0.0f
			));
		}
	}

	screenRenderer->draw(renderView, shader);

	m_time += params.deltaTime;
}

	}
}

#include "Amalgam/IEnvironment.h"
#include "Amalgam/IStateManager.h"
#include "Amalgam/IUpdateControl.h"
#include "Amalgam/IUpdateInfo.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Parade/Layer.h"
#include "Parade/Stage.h"
#include "Parade/StageLoader.h"
#include "Parade/StageState.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptContext.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const resource::Id< render::Shader > c_shaderFade(Guid(L"{DC104971-11AE-5743-9AB1-53B830F74391}"));

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Stage", Stage, Object)

Stage::Stage(
	amalgam::IEnvironment* environment,
	const resource::Proxy< script::IScriptContext >& scriptContext,
	const std::map< std::wstring, Guid >& transitions,
	const Object* params
)
:	m_environment(environment)
,	m_scriptContext(scriptContext)
,	m_transitions(transitions)
,	m_params(params)
,	m_initialized(false)
,	m_running(true)
,	m_fade(1.0f)
{
	m_screenRenderer = new render::ScreenRenderer();
	m_screenRenderer->create(m_environment->getRender()->getRenderSystem());

	m_environment->getResource()->getResourceManager()->bind(
		c_shaderFade,
		m_shaderFade
	);
}

Stage::~Stage()
{
	destroy();
}

void Stage::destroy()
{
	if (m_scriptContext)
	{
		m_scriptContext->destroy();
		m_scriptContext.clear();
	}

	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->destroy();

	m_layers.clear();

	safeDestroy(m_screenRenderer);
}

void Stage::addLayer(Layer* layer)
{
	m_layers.push_back(layer);
}

void Stage::removeLayer(Layer* layer)
{
	m_layers.remove(layer);
}

void Stage::removeAllLayers()
{
	m_layers.resize(0);
}

Layer* Stage::findLayer(const std::wstring& name) const
{
	for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		if ((*i)->getName() == name)
			return *i;
	}
	return 0;
}

void Stage::terminate()
{
	m_running = false;
}

script::Any Stage::invokeScript(const std::wstring& fn, uint32_t argc, const script::Any* argv)
{
	if (validateScriptContext())
		return m_scriptContext->executeFunction(fn, argc, argv);
	else
		return script::Any();
}

Ref< Stage > Stage::loadStage(const std::wstring& name, const Object* params)
{
	std::map< std::wstring, Guid >::const_iterator i = m_transitions.find(name);
	if (i == m_transitions.end())
	{
		log::error << L"No transition \"" << name << L"\" found" << Endl;
		return 0;
	}

	Ref< StageLoader > stageLoader = StageLoader::create(m_environment, i->second, params);
	if (stageLoader->failed())
	{
		log::error << L"Stage loader failed" << Endl;
		return 0;
	}

	return stageLoader->get();
}

Ref< StageLoader > Stage::loadStageAsync(const std::wstring& name, const Object* params)
{
	std::map< std::wstring, Guid >::const_iterator i = m_transitions.find(name);
	if (i == m_transitions.end())
	{
		log::error << L"No transition \"" << name << L"\" found" << Endl;
		return 0;
	}

	return StageLoader::createAsync(m_environment, i->second, params);
}

bool Stage::gotoStage(Stage* stage)
{
	m_pendingStage = stage;
	return true;
}

bool Stage::update(amalgam::IStateManager* stateManager, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (!m_running)
		return false;

	if (!m_pendingStage)
	{
		for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			(*i)->prepare();

		if (validateScriptContext())
		{
			script::Any argv[] =
			{
				script::Any(&control),
				script::Any(const_cast< amalgam::IUpdateInfo* >(&info))
			};
			m_scriptContext->executeFunction(L"update", sizeof_array(argv), argv);
		}

		for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			(*i)->update(control, info);

		m_fade = max(0.0f, m_fade - info.getSimulationDeltaTime());
	}
	else
	{
		m_fade += info.getSimulationDeltaTime();
		if (m_fade > 1.0f)
		{
			stateManager->enter(new StageState(m_environment, m_pendingStage));
			m_pendingStage = 0;
		}
	}

	return true;
}

bool Stage::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->build(info, frame);
	return true;
}

void Stage::render(render::EyeType eye, uint32_t frame)
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->render(eye, frame);

	if (m_fade > FUZZY_EPSILON)
	{
		m_shaderFade->setVectorParameter(L"Color", Vector4(0.0f, 0.0f, 0.0f, m_fade));
		m_screenRenderer->draw(
			m_environment->getRender()->getRenderView(),
			m_shaderFade
		);
	}
}

void Stage::leave()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->leave();
}

void Stage::reconfigured()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->reconfigured();
}

void Stage::flushScript()
{
	m_initialized = false;
}

bool Stage::validateScriptContext()
{
	if (!m_scriptContext)
		return false;

	if (m_scriptContext.changed())
	{
		m_initialized = false;
		m_scriptContext.consume();
	}

	if (!m_initialized)
	{
		// Expose commonly used globals.
		m_scriptContext->setGlobal(L"stage", script::Any(this));
		m_scriptContext->setGlobal(L"environment", script::Any(m_environment));

		for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			if (!(*i)->getName().empty())
				m_scriptContext->setGlobal((*i)->getName(), script::Any(*i));
		}

		// Call script init; do this everytime we re-validate script.
		script::Any argv[] =
		{
			script::Any(const_cast< Object* >(m_params.c_ptr()))
		};
		m_scriptContext->executeMethod(this, L"initialize", sizeof_array(argv), argv);
		m_initialized = true;
	}

	return true;
}

	}
}

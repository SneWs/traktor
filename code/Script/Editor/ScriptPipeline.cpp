#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Script/IScriptManager.h"
#include "Script/IScriptResource.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptPipeline.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

struct ResolvedScript
{
	Guid guid;
	std::wstring name;
	Ref< const Script > script;

	bool operator == (const Guid& rh) const { return guid == rh; }
};

bool resolveScript(editor::IPipelineBuilder* pipelineBuilder, const Guid& scriptGuid, std::list< ResolvedScript >& outScripts)
{
	Ref< db::Instance > scriptInstance = pipelineBuilder->getSourceDatabase()->getInstance(scriptGuid);
	if (!scriptInstance)
		return false;

	Ref< const Script > script = scriptInstance->getObject< Script >();
	if (!script)
		return false;

	const std::vector< Guid >& dependencies = script->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (std::find(outScripts.begin(), outScripts.end(), *i) != outScripts.end())
			continue;

		if (!resolveScript(pipelineBuilder, *i, outScripts))
			return false;
	}

	ResolvedScript rs;
	rs.guid = scriptGuid;
	rs.name = scriptInstance->getName();
	rs.script = script;
	outScripts.push_back(rs);

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptPipeline", 5, ScriptPipeline, editor::DefaultPipeline)

bool ScriptPipeline::create(const editor::IPipelineSettings* settings)
{
	std::wstring scriptManagerTypeName = settings->getProperty< PropertyString >(L"Editor.ScriptManagerType");

	// Create script manager instance.
	const TypeInfo* scriptManagerType = TypeInfo::find(scriptManagerTypeName);
	if (!scriptManagerType)
	{
		log::error << L"Script pipeline failed; no such type \"" << scriptManagerTypeName << L"\"" << Endl;
		return false;
	}

	m_scriptManager = dynamic_type_cast< IScriptManager* >(scriptManagerType->createInstance());
	T_ASSERT (m_scriptManager);

	return editor::DefaultPipeline::create(settings);
}

TypeInfoSet ScriptPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Script >());
	return typeSet;
}

bool ScriptPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const Script > sourceScript = checked_type_cast< const Script* >(sourceAsset);

	const std::vector< Guid >& dependencies = sourceScript->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		pipelineDepends->addDependency(*i, editor::PdfUse);

	return true;
}

bool ScriptPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	// Create ordered list of dependent scripts.
	std::list< ResolvedScript > scripts;
	if (!resolveScript(pipelineBuilder, outputGuid, scripts))
	{
		log::error << L"Script pipeline failed; unable to resolve script dependencies" << Endl;
		return false;
	}

	// Concate all scripts into a single script; generate a map with line numbers to corresponding source.
	script::source_map_t sm;
	StringOutputStream ss;
	int32_t line = 0;

	for (std::list< ResolvedScript >::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
	{
		sm.push_back(std::make_pair(line, i->name));
		StringSplit< std::wstring > split(i->script->getText(), L"\r\n");
		for (StringSplit< std::wstring >::const_iterator j = split.begin(); j != split.end(); ++j)
		{
			ss << *j << Endl;
			++line;
		}
	}

	// Compile script; save binary blobs if possible.
	Ref< IScriptResource > resource = m_scriptManager->compile(ss.str(), &sm, 0);
	if (!resource)
	{
		log::error << L"Script pipeline failed; unable to compile script" << Endl;
		return false;
	}

	return DefaultPipeline::buildOutput(
		pipelineBuilder,
		resource,
		sourceAssetHash,
		buildParams,
		outputPath,
		outputGuid,
		reason
	);
}

	}
}

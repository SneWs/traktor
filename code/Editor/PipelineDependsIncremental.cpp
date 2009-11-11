#include <limits>
#include "Editor/PipelineDependsIncremental.h"
#include "Editor/PipelineSettings.h"
#include "Editor/PipelineDependency.h"
#include "Editor/IPipeline.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Misc/Save.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsIncremental", PipelineDependsIncremental, IPipelineDepends)

PipelineDependsIncremental::PipelineDependsIncremental(
	Settings* settings,
	db::Database* sourceDatabase,
	uint32_t recursionDepth
)
:	m_sourceDatabase(sourceDatabase)
,	m_maxRecursionDepth(recursionDepth)
,	m_currentRecursionDepth(0)
{
	std::vector< const Type* > pipelineTypes;
	type_of< IPipeline >().findAllOf(pipelineTypes);

	for (std::vector< const Type* >::iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
	{
		Ref< IPipeline > pipeline = dynamic_type_cast< IPipeline* >((*i)->newInstance());
		if (pipeline)
		{
			PipelineSettings pipelineSettings(settings);
			if (pipeline->create(&pipelineSettings))
				m_pipelines.push_back(std::make_pair(
					pipeline,
					pipelineSettings.getHash()
				));
			else
				log::error << L"Failed to create pipeline \"" << type_name(pipeline) << L"\"" << Endl;
		}
	}
}

PipelineDependsIncremental::~PipelineDependsIncremental()
{
	for (std::vector< std::pair< Ref< IPipeline >, uint32_t > >::iterator i = m_pipelines.begin(); i != m_pipelines.end(); ++i)
		i->first->destroy();
}

void PipelineDependsIncremental::addDependency(const Serializable* sourceAsset)
{
	if (!sourceAsset)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	if (findPipeline(sourceAsset->getType(), pipeline, pipelineHash))
	{
		Ref< const Object > dummyBuildParams;
		pipeline->buildDependencies(this, 0, sourceAsset, dummyBuildParams);
		T_ASSERT_M (!dummyBuildParams, L"Build parameters not used with non-producing dependencies");
	}
	else
		log::error << L"Unable to add dependency to source asset (" << type_name(sourceAsset) << L"); no pipeline found" << Endl;
}

void PipelineDependsIncremental::addDependency(const Serializable* sourceAsset, const std::wstring& name, const std::wstring& outputPath, const Guid& outputGuid, bool build)
{
	if (!sourceAsset)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (PipelineDependency* dependency = findDependency(outputGuid))
	{
		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->children.push_back(dependency);
		return;
	}

	addUniqueDependency(
		0,
		sourceAsset,
		name,
		outputPath,
		outputGuid,
		build
	);
}

void PipelineDependsIncremental::addDependency(db::Instance* sourceAssetInstance, bool build)
{
	if (!sourceAssetInstance)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (PipelineDependency* dependency = findDependency(sourceAssetInstance->getGuid()))
	{
		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->children.push_back(dependency);
		return;
	}

	// Checkout source asset instance.
	Ref< Serializable > sourceAsset = sourceAssetInstance->getObject();
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to checkout instance" << Endl;
		return;
	}

	addUniqueDependency(
		sourceAssetInstance,
		sourceAsset,
		sourceAssetInstance->getName(),
		sourceAssetInstance->getPath(),
		sourceAssetInstance->getGuid(),
		build
	);
}

void PipelineDependsIncremental::addDependency(const Guid& sourceAssetGuid, bool build)
{
	if (sourceAssetGuid.isNull() || !sourceAssetGuid.isValid())
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (PipelineDependency* dependency = findDependency(sourceAssetGuid))
	{
		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->children.push_back(dependency);
		return;
	}

	// Get source asset instance from database.
	Ref< db::Instance > sourceAssetInstance = m_sourceDatabase->getInstance(sourceAssetGuid);
	if (!sourceAssetInstance)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; no such instance" << Endl;
		return;
	}

	// Checkout source asset instance.
	Ref< Serializable > sourceAsset = sourceAssetInstance->getObject();
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to checkout instance" << Endl;
		return;
	}

	addUniqueDependency(
		sourceAssetInstance,
		sourceAsset,
		sourceAssetInstance->getName(),
		sourceAssetInstance->getPath(),
		sourceAssetInstance->getGuid(),
		build
	);
}

void PipelineDependsIncremental::addDependency(
	const Path& fileName
)
{
	T_ASSERT (m_currentDependency);
	m_currentDependency->files.insert(fileName);
}

void PipelineDependsIncremental::getDependencies(RefArray< PipelineDependency >& outDependencies) const
{
	outDependencies = m_dependencies;
}

Ref< db::Database > PipelineDependsIncremental::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< const Serializable > PipelineDependsIncremental::getObjectReadOnly(const Guid& instanceGuid)
{
	Ref< Serializable > object;

	std::map< Guid, Ref< Serializable > >::iterator i = m_readCache.find(instanceGuid);
	if (i != m_readCache.end())
		object = i->second;
	else
	{
		object = m_sourceDatabase->getObjectReadOnly(instanceGuid);
		m_readCache[instanceGuid] = object;
	}

	return object;
}

bool PipelineDependsIncremental::findPipeline(const Type& sourceType, Ref< IPipeline >& outPipeline, uint32_t& outPipelineHash) const
{
	uint32_t best = std::numeric_limits< uint32_t >::max();
	for (std::vector< std::pair< Ref< IPipeline >, uint32_t > >::const_iterator i = m_pipelines.begin(); i != m_pipelines.end(); ++i)
	{
		TypeSet typeSet = i->first->getAssetTypes();
		for (TypeSet::iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			uint32_t distance = 0;

			// Calculate distance in type hierarchy.
			const Type* type = &sourceType;
			while (type)
			{
				if (type == *j)
					break;

				++distance;
				type = type->getSuper();
			}

			// Keep closest matching type.
			if (type && distance < best)
			{
				outPipeline = i->first;
				outPipelineHash = i->second;
				if ((best = distance) == 0)
					break;
			}
		}
	}
	return bool(outPipeline != 0);
}

Ref< PipelineDependency > PipelineDependsIncremental::findDependency(const Guid& guid) const
{
	for (RefArray< PipelineDependency >::const_iterator i = m_dependencies.begin(); i != m_dependencies.end(); ++i)
	{
		if ((*i)->outputGuid == guid)
			return *i;
	}
	return 0;
}

void PipelineDependsIncremental::addUniqueDependency(
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	const std::wstring& name,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	bool build
)
{
	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	// Find appropriate pipeline.
	if (!findPipeline(sourceAsset->getType(), pipeline, pipelineHash))
	{
		log::error << L"Unable to add dependency to \"" << name << L"\"; no pipeline found" << Endl;
		return;
	}

	// Register dependency, add to "parent" dependency as well.
	Ref< PipelineDependency > dependency = gc_new< PipelineDependency >();
	dependency->name = name;
	dependency->pipeline = pipeline;
	dependency->pipelineHash = pipelineHash;
	dependency->sourceAsset = sourceAsset;
	dependency->outputPath = outputPath;
	dependency->outputGuid = outputGuid;
	dependency->build = build;
	dependency->reason = IPipeline::BrNone;
	dependency->parent = m_currentDependency;
	if (m_currentDependency)
		m_currentDependency->children.push_back(dependency);

	uint32_t dependencyIndex = uint32_t(m_dependencies.size());
	m_dependencies.push_back(dependency);

	bool result = true;

	if (m_currentRecursionDepth < m_maxRecursionDepth)
	{
		Save< uint32_t > saveDepth(m_currentRecursionDepth, m_currentRecursionDepth + 1);
		Save< Ref< PipelineDependency > > saveDependency(m_currentDependency, dependency);

		result = pipeline->buildDependencies(
			this,
			sourceInstance,
			sourceAsset,
			dependency->buildParams
		);
	}

	if (!result)
	{
		// Pipeline build dependencies failed; remove dependency from array.
		T_ASSERT (dependencyIndex < m_dependencies.size());
		m_dependencies.erase(m_dependencies.begin() + dependencyIndex);
	}
}

	}
}

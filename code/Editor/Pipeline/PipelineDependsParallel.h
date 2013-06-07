#ifndef traktor_editor_PipelineDependsParallel_H
#define traktor_editor_PipelineDependsParallel_H

#include <map>
#include "Core/Thread/ReaderWriterLock.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Editor/IPipelineDepends.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
	#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class JobQueue;

	namespace editor
	{

class IPipelineDb;
class IPipelineDependencySet;
class PipelineFactory;

/*! \brief Parallel pipeline dependency walker.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineDependsParallel : public IPipelineDepends
{
	T_RTTI_CLASS;

public:
	PipelineDependsParallel(
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		IPipelineDependencySet* dependencySet,
		IPipelineDb* pipelineDb
	);

	virtual ~PipelineDependsParallel();

	virtual void addDependency(
		const ISerializable* sourceAsset
	);

	virtual void addDependency(
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	);

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		uint32_t flags
	);

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		uint32_t flags
	);

	virtual void addDependency(
		const Path& basePath,
		const std::wstring& fileName
	);

	virtual void addDependency(
		const TypeInfo& sourceAssetType
	);

	virtual bool waitUntilFinished();

	virtual Ref< db::Database > getSourceDatabase() const;

	virtual Ref< db::Database > getOutputDatabase() const;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid);

private:
	Ref< JobQueue > m_jobQueue;
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IPipelineDependencySet > m_dependencySet;
	Ref< IPipelineDb > m_pipelineDb;
	ThreadLocal m_currentDependency;
	ReaderWriterLock m_readCacheLock;
	Semaphore m_dependencySetLock;
	std::map< Guid, Ref< ISerializable > > m_readCache;

	Ref< PipelineDependency > findOrCreateDependency(
		const Guid& guid,
		PipelineDependency* parentDependency,
		uint32_t flags,
		bool& outExists
	);

	void addUniqueDependency(
		PipelineDependency* parentDependency,
		PipelineDependency* currentDependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	);

	void updateDependencyHashes(
		PipelineDependency* dependency,
		const db::Instance* sourceInstance
	) const;

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< const ISerializable > sourceAsset);

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< const ISerializable > sourceAsset, std::wstring outputPath, Guid outputGuid, uint32_t flags);

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< db::Instance > sourceAssetInstance, uint32_t flags);

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Guid sourceAssetGuid, uint32_t flags);
};

	}
}

#endif	// traktor_editor_PipelineDependsParallel_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_PipelineBuilder_H
#define traktor_editor_PipelineBuilder_H

#include <list>
#include <map>
#include "Core/Io/Path.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/ReaderWriterLock.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/PipelineTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace db
	{

class Group;

	}

	namespace editor
	{

class IPipelineCache;
class IPipelineDb;
class IPipelineInstanceCache;
class PipelineFactory;

/*! \brief Pipeline manager.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineBuilder : public IPipelineBuilder
{
	T_RTTI_CLASS;

public:
	PipelineBuilder(
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		IPipelineCache* cache,
		IPipelineDb* db,
		IPipelineInstanceCache* instanceCache,
		IListener* listener,
		bool threadedBuildEnable
	);

	virtual bool build(const IPipelineDependencySet* dependencySet, bool rebuild) T_OVERRIDE T_FINAL;

	virtual Ref< ISerializable > buildOutput(const ISerializable* sourceAsset) T_OVERRIDE T_FINAL;

	virtual bool buildOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams) T_OVERRIDE T_FINAL;

	virtual Ref< ISerializable > getBuildProduct(const ISerializable* sourceAsset) T_OVERRIDE T_FINAL;

	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid) T_OVERRIDE T_FINAL;

	virtual Ref< db::Database > getOutputDatabase() const T_OVERRIDE T_FINAL;

	virtual Ref< db::Database > getSourceDatabase() const T_OVERRIDE T_FINAL;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > openFile(const Path& basePath, const std::wstring& fileName) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > createTemporaryFile(const std::wstring& fileName) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > openTemporaryFile(const std::wstring& fileName) T_OVERRIDE T_FINAL;

private:
	struct BuiltCacheEntry
	{
		const ISerializable* sourceAsset;
		Ref< ISerializable > product;
	};

	typedef std::list< BuiltCacheEntry > built_cache_list_t;

	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IPipelineCache > m_cache;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineInstanceCache > m_instanceCache;
	IListener* m_listener;
	bool m_threadedBuildEnable;
	
	Semaphore m_createOutputLock;
	ReaderWriterLock m_readCacheLock;
	Semaphore m_builtCacheLock;
	Semaphore m_workSetLock;

	std::vector< uint32_t > m_reasons;

	std::list< std::pair< uint32_t, Ref< const Object > > > m_workSet;
	std::map< Guid, Ref< ISerializable > > m_readCache;
	std::map< uint32_t, built_cache_list_t > m_builtCache;
	ThreadLocal m_buildInstances;

	int32_t m_progress;
	int32_t m_progressEnd;
	int32_t m_succeeded;
	int32_t m_succeededBuilt;
	int32_t m_failed;
	int32_t m_cacheHit;
	int32_t m_cacheMiss;
	int32_t m_cacheVoid;

	/*! \brief Perform build. */
	BuildResult performBuild(const IPipelineDependencySet* dependencySet, const PipelineDependency* dependency, const Object* buildParams, uint32_t reason);

	/*! \brief Isolate instance in cache. */
	bool putInstancesInCache(const Guid& guid, const PipelineDependencyHash& hash, const RefArray< db::Instance >& instances);

	/*! \brief Get isolated instance from cache. */
	bool getInstancesFromCache(const Guid& guid, const PipelineDependencyHash& hash);

	/*! \brief Build thread method. */
	void buildThread(const IPipelineDependencySet* dependencySet, Thread* controlThread, int32_t cpuCore);
};

	}
}

#endif	// traktor_editor_PipelineBuilder_H

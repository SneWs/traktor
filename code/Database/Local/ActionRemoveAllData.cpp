#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Database/Local/ActionRemoveAllData.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionRemoveAllData", ActionRemoveAllData, Action)

ActionRemoveAllData::ActionRemoveAllData(const Path& instancePath)
:	m_instancePath(instancePath)
{
}

bool ActionRemoveAllData::execute(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
	{
		log::error << L"Action remove failed; unable to read meta object" << Endl;
		return false;
	}

	for (const auto& blob : instanceMeta->getBlobs())
	{
		Path instanceDataPath = getInstanceDataPath(m_instancePath, blob.name);
		if (fileStore->remove(instanceDataPath))
			m_renamedFiles.push_back(instanceDataPath.getPathName());
		else
		{
			log::error << L"Action remove failed; unable to remove \"" << instanceDataPath.getPathName() << L"\"" << Endl;
			return false;
		}
	}

	return true;
}

bool ActionRemoveAllData::undo(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	for (const auto& renamedFile : m_renamedFiles)
	{
		if (!fileStore->rollback(renamedFile))
			return false;
	}
	m_renamedFiles.clear();
	return true;
}

void ActionRemoveAllData::clean(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	for (const auto& renamedFile : m_renamedFiles)
		fileStore->clean(renamedFile);
}

bool ActionRemoveAllData::redundant(const Action* action) const
{
	if (const ActionRemoveAllData* actionRemoveAllData = dynamic_type_cast< const ActionRemoveAllData* >(action))
		return m_instancePath == actionRemoveAllData->m_instancePath;
	else
		return false;
}

	}
}

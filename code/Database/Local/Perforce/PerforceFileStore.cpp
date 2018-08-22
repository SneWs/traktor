/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <time.h>
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Database/ConnectionString.h"
#include "Database/Types.h"
#include "Database/Local/Perforce/PerforceChangeList.h"
#include "Database/Local/Perforce/PerforceClient.h"
#include "Database/Local/Perforce/PerforceFileStore.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.PerforceFileStore", 0, PerforceFileStore, IFileStore)

bool PerforceFileStore::create(const ConnectionString& connectionString)
{
	time_t rawtime;
	struct tm* timeinfo;
	wchar_t buffer[1024];

	PerforceClientDesc desc;
	desc.m_host = OS::getInstance().getComputerName();
	desc.m_port = connectionString.get(L"p4port");
	desc.m_user = connectionString.get(L"p4user");
	desc.m_password= connectionString.get(L"p4pwd");
	desc.m_client = connectionString.get(L"p4client");
	desc.m_securityLevel = PerforceClientDesc::SlLow;

	// Create change list description.
	std::wstring changeListDescription = connectionString.get(L"p4changelist");
	if (changeListDescription.empty())
		changeListDescription = L"** CREATED %x **";

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	wcsftime(buffer, sizeof_array(buffer), changeListDescription.c_str(), timeinfo);
	changeListDescription = buffer;

	m_p4client = new PerforceClient(desc);

	RefArray< PerforceChangeList > changeLists;
	if (m_p4client->getChangeLists(changeLists))
	{
		for (auto changeList : changeLists)
		{
			if (compareIgnoreCase< std::wstring >(changeList->getDescription(), changeListDescription) == 0)
			{
				m_p4changeList = changeList;
				break;
			}
		}
	}

	if (!m_p4changeList)
	{
		m_p4changeList = m_p4client->createChangeList(changeListDescription);
		if (!m_p4changeList)
			return false;
	}

	return true;
}

void PerforceFileStore::destroy()
{
	m_p4changeList = 0;
	m_p4client = 0;
}

uint32_t PerforceFileStore::flags(const Path& filePath)
{
	uint32_t flags = IfNormal;

	PerforceAction action;
	m_p4client->isOpened(filePath.getPathName(), action);
	if (action == AtNotOpened)
		flags |= IfReadOnly;
	else
		flags |= IfModified;

	return flags;
}

bool PerforceFileStore::add(const Path& filePath)
{
	return m_p4client->addFile(m_p4changeList, filePath.getPathName());
}

bool PerforceFileStore::remove(const Path& filePath)
{
	std::wstring localFile = filePath.getPathName();

	PerforceAction pa;
	if (m_p4client->isOpened(localFile, pa))
	{
		if (pa == AtAdd)
		{
			// File has been added; revert in change list then remove local.
			if (!m_p4client->revertFile(m_p4changeList, localFile))
				return false;

			return FileSystem::getInstance().remove(localFile);
		}
		else if (pa == AtEdit)
		{
			// File has been opened for edit; revert in change list then open for delete.
			if (!m_p4client->revertFile(m_p4changeList, localFile))
				return false;
			
			return m_p4client->openForDelete(m_p4changeList, localFile);
		}
		else if (pa == AtDelete)
		{
			// File has already been opened for delete.
			return true;
		}

		// File not opened but exist in perforce; open for delete.
		return m_p4client->openForDelete(m_p4changeList, localFile);
	}
	else
	{
		// File not in perforce; delete local file.
		return FileSystem::getInstance().remove(localFile);
	}
}

bool PerforceFileStore::edit(const Path& filePath)
{
	std::wstring localFile = filePath.getPathName();

	PerforceAction pa;
	if (m_p4client->isOpened(localFile, pa))
	{
		if (pa == AtAdd || pa == AtEdit)
			return true;

		if (m_p4client->openForEdit(m_p4changeList, filePath.getPathName()))
			return true;
	}
	else
	{
		// File doesn't exist in P4; add local file.
		if (m_p4client->addFile(m_p4changeList, filePath.getPathName()))
			return true;
	}

	return false;
}

bool PerforceFileStore::rollback(const Path& filePath)
{
	return m_p4client->revertFile(m_p4changeList, filePath.getPathName());
}

bool PerforceFileStore::clean(const Path& filePath)
{
	return true;
}

	}
}

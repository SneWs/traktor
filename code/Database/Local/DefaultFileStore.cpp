/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Database/ConnectionString.h"
#include "Database/Types.h"
#include "Database/Local/DefaultFileStore.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.DefaultFileStore", DefaultFileStore, IFileStore)

bool DefaultFileStore::create(const ConnectionString& connectionString)
{
	m_editReadOnly = connectionString.get(L"readOnly") == L"edit";
	return true;
}

void DefaultFileStore::destroy()
{
}

uint32_t DefaultFileStore::flags(const Path& filePath)
{
	uint32_t flags = IfNormal;
	Ref< File > file = FileSystem::getInstance().get(filePath);
	if (file)
	{
		if (file->isReadOnly())
			flags |= IfReadOnly;
	}
	return flags;
}

bool DefaultFileStore::add(const Path& filePath)
{
	if (FileSystem::getInstance().exist(filePath))
		return true;
	else
	{
		// File doesn't exist.
		return false;
	}
}

bool DefaultFileStore::remove(const Path& filePath)
{
	if (FileSystem::getInstance().exist(filePath))
	{
		const Path filePathAlt = filePath.getPathName() + L"~";
		return FileSystem::getInstance().move(
			filePathAlt,
			filePath,
			true
		);
	}
	else
	{
		// File doesn't exist.
		return true;
	}
}

bool DefaultFileStore::edit(const Path& filePath)
{
	Ref< File > file = FileSystem::getInstance().get(filePath);
	if (!file)
	{
		// File doesn't exist; no need to take snapshot.
		return true;
	}

	if (file->isReadOnly())
	{
		if (m_editReadOnly)
		{
			// Read-only file; try remove flag and open for edit.
			if (!FileSystem::getInstance().modify(filePath, file->getFlags() & ~File::FfReadOnly))
				return false;
		}
		else
		{
			// Read-only file; not allowed to edit those.
			return false;
		}
	}

	const Path filePathAlt = filePath.getPathName() + L"~";
	return FileSystem::getInstance().copy(filePathAlt, filePath, true);
}

bool DefaultFileStore::rollback(const Path& filePath)
{
	const Path filePathAlt = filePath.getPathName() + L"~";
	return FileSystem::getInstance().move(filePath, filePathAlt, true);
}

bool DefaultFileStore::clean(const Path& filePath)
{
	const Path filePathAlt = filePath.getPathName() + L"~";
	return FileSystem::getInstance().remove(filePathAlt);
}

}

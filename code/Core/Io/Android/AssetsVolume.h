/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Io/IVolume.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct ANativeActivity;

namespace traktor
{

class FileSystem;

class T_DLLCLASS AssetsVolume : public IVolume
{
	T_RTTI_CLASS;

public:
	explicit AssetsVolume(ANativeActivity* activity);

	virtual std::wstring getDescription() const override final;

	virtual Ref< File > get(const Path& path) override final;

	virtual int find(const Path& mask, RefArray< File >& out) override final;

	virtual bool modify(const Path& fileName, uint32_t flags) override final;

	virtual bool modify(const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime) override final;

	virtual Ref< IStream > open(const Path& filename, uint32_t mode) override final;

	virtual Ref< IMappedFile > map(const Path& fileName) override final;

	virtual bool exist(const Path& filename) override final;

	virtual bool remove(const Path& filename) override final;

	virtual bool move(const Path& fileName, const std::wstring& newName, bool overwrite) override final;

	virtual bool copy(const Path& fileName, const std::wstring& newName, bool overwrite) override final;

	virtual bool makeDirectory(const Path& directory) override final;

	virtual bool removeDirectory(const Path& directory) override final;

	virtual bool renameDirectory(const Path& directory, const std::wstring& newName) override final;

	virtual bool setCurrentDirectory(const Path& directory) override final;

	virtual Path getCurrentDirectory() const override final;

private:
	ANativeActivity* m_activity;
};

}


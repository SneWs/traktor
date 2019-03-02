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

namespace traktor
{

class FileSystem;

class T_DLLCLASS NativeVolume : public IVolume
{
	T_RTTI_CLASS;

public:
	NativeVolume(const Path& currentDirectory);

	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< File > get(const Path& path) T_OVERRIDE T_FINAL;

	virtual int find(const Path& mask, RefArray< File >& out) T_OVERRIDE T_FINAL;

	virtual bool modify(const Path& fileName, uint32_t flags) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > open(const Path& filename, uint32_t mode) T_OVERRIDE T_FINAL;

	virtual bool exist(const Path& filename) T_OVERRIDE T_FINAL;

	virtual bool remove(const Path& filename) T_OVERRIDE T_FINAL;

	virtual bool move(const Path& fileName, const std::wstring& newName, bool overwrite) T_OVERRIDE T_FINAL;

	virtual bool copy(const Path& fileName, const std::wstring& newName, bool overwrite) T_OVERRIDE T_FINAL;

	virtual bool makeDirectory(const Path& directory) T_OVERRIDE T_FINAL;

	virtual bool removeDirectory(const Path& directory) T_OVERRIDE T_FINAL;

	virtual bool renameDirectory(const Path& directory, const std::wstring& newName) T_OVERRIDE T_FINAL;

	virtual bool setCurrentDirectory(const Path& directory) T_OVERRIDE T_FINAL;

	virtual Path getCurrentDirectory() const T_OVERRIDE T_FINAL;

	static void mountVolumes(FileSystem& fileSystem);

private:
	Path m_currentDirectory;

	std::wstring getSystemPath(const Path& path) const;
};

}


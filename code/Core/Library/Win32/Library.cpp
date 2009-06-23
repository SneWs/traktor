#include "Core/Platform.h"
#include "Core/Library/Library.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/String.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

Library::~Library()
{
}

bool Library::open(const Path& libraryName)
{
#if !defined(_DEBUG)
	const wchar_t suffix[] = L".dll";
#else
	const wchar_t suffix[] = L"_d.dll";
#endif
	tstring path;

	path = wstots(libraryName.getPathName() + suffix);
	m_handle = (void*)LoadLibrary(path.c_str());

	if (!m_handle)
	{
		path = wstots(libraryName.getPathName());
		m_handle = (void*)LoadLibrary(path.c_str());
	}
	
	return bool(m_handle != NULL);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths)
{
	TCHAR currentPath[32767];

	if (!GetEnvironmentVariable(_T("PATH"), currentPath, sizeof_array(currentPath)))
		return false;

	StringOutputStream newPathStream;

	std::vector< Path >::const_iterator i = searchPaths.begin();
	if (i != searchPaths.end())
	{
		newPathStream << i->getPathName();
		for (++i; i != searchPaths.end(); ++i)
			newPathStream << L";" << i->getPathName();
	}

	std::wstring newPath = newPathStream.str();

	if (!SetEnvironmentVariable(_T("PATH"), newPath.c_str()))
		return false;

	bool result = open(libraryName);

	SetEnvironmentVariable(_T("PATH"), currentPath);

	return result;
}

void Library::close()
{
	FreeLibrary((HMODULE)m_handle);
}

void* Library::find(const std::wstring& symbol)
{
#if !defined(WINCE)
	return (void*)GetProcAddress((HMODULE)m_handle, wstombs(symbol).c_str());
#else
	return (void*)GetProcAddress((HMODULE)m_handle, symbol.c_str());
#endif
}

}

#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace script
	{

class IErrorCallback;
class IScriptBlob;

/*! Script compiler.
 * \ingroup Script
 */
class T_DLLCLASS IScriptCompiler : public Object
{
	T_RTTI_CLASS;

public:
	/*! Compile script into "consumable" blob.
	 *
	 * \param fileName Script file name.
	 * \param script Script
	 * \param map Optional source map; used for debugging.
	 * \param errorCallback Optional callback which is called for each error found during compilation.
	 * \return Script blob; null if failed to compile.
	 */
	virtual Ref< IScriptBlob > compile(const std::wstring& fileName, const std::wstring& script, IErrorCallback* errorCallback) const = 0;
};

	}
}


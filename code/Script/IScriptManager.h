/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_IScriptManager_H
#define traktor_script_IScriptManager_H

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

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

class IScriptBlob;
class IScriptContext;
class IScriptDebugger;
class IScriptProfiler;

/*! \brief Script error callback.
* \ingroup Script
*/
class IErrorCallback
{
public:
	virtual ~IErrorCallback() {}

	virtual void syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message) = 0;

	virtual void otherError(const std::wstring& message) = 0;
};

/*! \brief
 * \ingroup Script
 */
struct ScriptStatistics
{
	uint32_t memoryUsage;
};

/*! \brief Script manager.
 * \ingroup Script
 *
 * A script manager is the major communicator
 * with the desired scripting environment.
 * It's purpose is to handle class registration
 * and context creations.
 * Each context should have at least the classes
 * registered up until the time of creation accessible.
 */
class T_DLLCLASS IScriptManager
:	public Object
,	public IRuntimeClassRegistrar
{
	T_RTTI_CLASS;

public:
	/*! \brief Destroy script manager. */
	virtual void destroy() = 0;

	/*! \brief Compile script into "consumable" blob.
	 *
	 * \param fileName Script file name.
	 * \param script Script
	 * \param map Optional source map; used for debugging.
	 * \param errorCallback Optional callback which is called for each error found during compilation.
	 * \return Script blob; null if failed to compile.
	 */
	virtual Ref< IScriptBlob > compile(const std::wstring& fileName, const std::wstring& script, IErrorCallback* errorCallback) const = 0;

	/*! \brief Create script context.
	 *
	 * \param strict Strict global variable declaration required.
	 * \return Script context instance.
	 */
	virtual Ref< IScriptContext > createContext(bool strict) = 0;

	/*! \brief Create debugger.
	 *
	 * \return Debugger instance.
	 */
	virtual Ref< IScriptDebugger > createDebugger() = 0;

	/*! \brief Create profiler.
	 *
	 * \return Profiler instance.
	 */
	virtual Ref< IScriptProfiler > createProfiler() = 0;

	/*! \brief Collect garbage.
	 *
	 * This is exposed in order to make some scripting languages which
	 * rely on garbage collection behave more sane in real-time applications
	 * such as games where we cannot afford both the memory required nor the
	 * time spent in normal garbage collection cycles.
	 * Thus we must be able to single step garbage collection at a
	 * high frequency in order to keep the heap as small as possible.
	 *
	 * If this isn't called the normal garbage collection frequency
	 * should be used as dictated by the scripting language.
	 */
	virtual void collectGarbage(bool full) = 0;

	/*! \brief
	 */
	virtual void getStatistics(ScriptStatistics& outStatistics) const = 0;
};

	}
}

#endif	// traktor_script_IScriptManager_H

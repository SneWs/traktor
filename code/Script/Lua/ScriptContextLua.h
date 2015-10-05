#ifndef traktor_script_ScriptContextLua_H
#define traktor_script_ScriptContextLua_H

#include "Script/IScriptContext.h"

struct lua_State;

namespace traktor
{
	namespace script
	{

class IScriptClass;
class ScriptDelegateLua;
class ScriptManagerLua;
class ScriptObjectLua;

/*! \brief LUA scripting context.
 * \ingroup LUA Script
 */
class ScriptContextLua : public IScriptContext
{
	T_RTTI_CLASS;

public:
	virtual ~ScriptContextLua();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool load(const IScriptBlob* scriptBlob) T_OVERRIDE T_FINAL;

	virtual void setGlobal(const std::string& globalName, const Any& globalValue) T_OVERRIDE T_FINAL;

	virtual Any getGlobal(const std::string& globalName) T_OVERRIDE T_FINAL;

	virtual Ref< const IRuntimeClass > findClass(const std::string& className) T_OVERRIDE T_FINAL;

	virtual bool haveFunction(const std::string& functionName) const T_OVERRIDE T_FINAL;

	virtual Any executeFunction(const std::string& functionName, uint32_t argc, const Any* argv) T_OVERRIDE T_FINAL;

	Any executeDelegate(ScriptDelegateLua* delegate, uint32_t argc, const Any* argv);

	Any executeMethod(ScriptObjectLua* self, int32_t methodRef, uint32_t argc, const Any* argv);

private:
	friend class ScriptDebuggerLua;
	friend class ScriptManagerLua;
	friend class ScriptProfilerLua;

	ScriptManagerLua* m_scriptManager;
	lua_State* m_luaState;
	int32_t m_environmentRef;
	const Object* m_lastSelf;

	ScriptContextLua(ScriptManagerLua* scriptManager, lua_State* luaState, int32_t environmentRef);

	static int32_t runtimeError(lua_State* luaState);
};

	}
}

#endif	// traktor_script_ScriptContextLua_H

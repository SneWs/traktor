/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Script/Lua/ScriptClassLua.h"
#include "Script/Lua/ScriptObjectLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

const int32_t c_tableKey_instance = -2;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptObjectLua", ScriptObjectLua, IRuntimeObject)

ScriptObjectLua::ScriptObjectLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState, int32_t tableRef)
:	m_scriptManager(scriptManager)
,	m_scriptContext(scriptContext)
,	m_luaState(luaState)
,	m_tableRef(tableRef)
{
}

ScriptObjectLua::~ScriptObjectLua()
{
	T_ANONYMOUS_VAR(UnwindStack)(m_luaState);

	// Explicit drop reference to object, don't wait for GC to collect.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_tableRef);	
	lua_rawgeti(m_luaState, -1, c_tableKey_instance);
	ITypedObject* object = reinterpret_cast< ITypedObject* >(lua_touserdata(m_luaState, -1));
	T_SAFE_RELEASE(object);

	// Ensure table reference is nil.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_tableRef);	// +1
	lua_pushnil(m_luaState);
	lua_rawseti(m_luaState, -2, c_tableKey_instance);

	// Unpin table.
	luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
}

Ref< const IRuntimeClass > ScriptObjectLua::getRuntimeClass() const
{
	T_ANONYMOUS_VAR(UnwindStack)(m_luaState);
	push();
	lua_getmetatable(m_luaState, -1);
	return ScriptClassLua::createFromStack(m_scriptManager, m_scriptContext, m_luaState);
}

void ScriptObjectLua::push() const
{
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
	T_ASSERT (lua_istable(m_luaState, -1));
}

	}
}

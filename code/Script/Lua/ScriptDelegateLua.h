#pragma once

#include "Core/Class/IRuntimeDelegate.h"

struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptContextLua;

class ScriptDelegateLua : public IRuntimeDelegate
{
	T_RTTI_CLASS;

public:
	ScriptDelegateLua(ScriptContextLua* context, lua_State*& luaState);

	virtual ~ScriptDelegateLua();

	void push();

	virtual Any call(int32_t argc, const Any* argv) override final;

private:
	ScriptContextLua* m_context;
	lua_State*& m_luaState;
	int32_t m_functionRef;
};

	}
}


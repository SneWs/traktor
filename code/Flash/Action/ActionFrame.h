#ifndef traktor_flash_ActionFrame_H
#define traktor_flash_ActionFrame_H

#include "Core/Object.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValueArray.h"
#include "Flash/Action/ActionValueStack.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;
class ActionObject;
class ActionDictionary;
class ActionFunction;

/*! \brief ActionScript execution stack frame.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFrame : public Object
{
	T_RTTI_CLASS;

public:
	ActionFrame(
		ActionContext* context,
		ActionObject* self,
		const uint8_t* code,
		uint16_t codeSize,
		uint16_t localRegisters,
		ActionDictionary* dictionary,
		ActionFunction* callee
	);

	void setRegister(uint16_t index, const ActionValue& value);

	ActionValue getRegister(uint16_t index) const;

	bool hasVariable(const std::string& variableName) const;

	void setVariable(const std::string& variableName, const ActionValue& variableValue);

	bool getVariable(const std::string& variableName, ActionValue& outVariableValue) const;

	void setDictionary(ActionDictionary* dictionary);

	inline ActionContext* getContext() const { return m_context; }

	inline ActionObject* getSelf() const { return m_self; }

	inline const uint8_t* getCode() const { return m_code; }

	inline uint16_t getCodeSize() const { return m_codeSize; }

	inline const std::map< std::string, ActionValue >& getVariables() const { return m_localVariables; }

	inline ActionDictionary* getDictionary() const { return m_dictionary; }

	inline ActionFunction* getCallee() const { return m_callee; }

	inline ActionValueStack& getStack() { return m_stack; }

private:
	ActionContext* m_context;
	ActionObject* m_self;
	const uint8_t* m_code;
	uint16_t m_codeSize;
	ActionValueArray m_localRegisters;
	std::map< std::string, ActionValue > m_localVariables;
	Ref< ActionDictionary > m_dictionary;
	ActionFunction* m_callee;
	ActionValueStack m_stack;
};

	}
}

#endif	// traktor_flash_ActionFrame_H

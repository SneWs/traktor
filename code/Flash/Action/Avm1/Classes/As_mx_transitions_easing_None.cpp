#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_None.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_None", As_mx_transitions_easing_None, ActionClass)

As_mx_transitions_easing_None::As_mx_transitions_easing_None()
:	ActionClass("mx.transitions.easing.None")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("easeIn", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_None::None_easeIn)));
	prototype->setMember("easeInOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_None::None_easeInOut)));
	prototype->setMember("easeOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_None::None_easeOut)));
	prototype->setMember("easeNone", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_None::None_easeNone)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue As_mx_transitions_easing_None::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void As_mx_transitions_easing_None::None_easeIn(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_None::None_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_None::None_easeOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_None::None_easeNone(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

	}
}

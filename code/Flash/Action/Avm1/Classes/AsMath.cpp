#include <cmath>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsMath.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMath", AsMath, ActionClass)

AsMath::AsMath()
:	ActionClass("Math")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("e", ActionValue(avm_number_t(2.7182818284590452354)));
	prototype->setMember("ln2", ActionValue(avm_number_t(0.69314718055994530942)));
	prototype->setMember("log2e", ActionValue(avm_number_t(1.4426950408889634074)));
	prototype->setMember("ln10", ActionValue(avm_number_t(2.30258509299404568402)));
	prototype->setMember("log10e", ActionValue(avm_number_t(0.43429448190325182765)));
	prototype->setMember("pi", ActionValue(avm_number_t(3.14159265358979323846)));
	prototype->setMember("sqrt1_2", ActionValue(avm_number_t(0.7071067811865475244)));
	prototype->setMember("sqrt2", ActionValue(avm_number_t(1.4142135623730950488)));
	prototype->setMember("abs", ActionValue(createNativeFunction(this, &AsMath::Math_abs)));
	prototype->setMember("acos", ActionValue(createNativeFunction(this, &AsMath::Math_acos)));
	prototype->setMember("asin", ActionValue(createNativeFunction(this, &AsMath::Math_asin)));
	prototype->setMember("atan", ActionValue(createNativeFunction(this, &AsMath::Math_atan)));
	prototype->setMember("atan2", ActionValue(createNativeFunction(this, &AsMath::Math_atan2)));
	prototype->setMember("ceil", ActionValue(createNativeFunction(this, &AsMath::Math_ceil)));
	prototype->setMember("cos", ActionValue(createNativeFunction(this, &AsMath::Math_cos)));
	prototype->setMember("exp", ActionValue(createNativeFunction(this, &AsMath::Math_exp)));
	prototype->setMember("floor", ActionValue(createNativeFunction(this, &AsMath::Math_floor)));
	prototype->setMember("log", ActionValue(createNativeFunction(this, &AsMath::Math_log)));
	prototype->setMember("max", ActionValue(createNativeFunction(this, &AsMath::Math_max)));
	prototype->setMember("min", ActionValue(createNativeFunction(this, &AsMath::Math_min)));
	prototype->setMember("pow", ActionValue(createNativeFunction(this, &AsMath::Math_pow)));
	prototype->setMember("random", ActionValue(createNativeFunction(this, &AsMath::Math_random)));
	prototype->setMember("round", ActionValue(createNativeFunction(this, &AsMath::Math_round)));
	prototype->setMember("sin", ActionValue(createNativeFunction(this, &AsMath::Math_sin)));
	prototype->setMember("sqrt", ActionValue(createNativeFunction(this, &AsMath::Math_sqrt)));
	prototype->setMember("tan", ActionValue(createNativeFunction(this, &AsMath::Math_tan)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsMath::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void AsMath::Math_abs(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::fabs(n));
}

void AsMath::Math_acos(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::acos(n));
}

void AsMath::Math_asin(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::asin(n));
}

void AsMath::Math_atan(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::atan(n));
}

void AsMath::Math_atan2(CallArgs& ca)
{
	avm_number_t x = ca.args[0].getNumberSafe();
	avm_number_t y = ca.args[1].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::atan2(x, y));
}

void AsMath::Math_ceil(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::ceil(n));
}

void AsMath::Math_cos(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::cos(n));
}

void AsMath::Math_exp(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::exp(n));
}

void AsMath::Math_floor(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::floor(n));
}

void AsMath::Math_log(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::log(n));
}

void AsMath::Math_max(CallArgs& ca)
{
	avm_number_t n1 = ca.args[0].getNumberSafe();
	for (size_t i = 1; i < ca.args.size(); ++i)
	{
		avm_number_t n = ca.args[i].getNumberSafe();
		if (n > n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_min(CallArgs& ca)
{
	avm_number_t n1 = ca.args[0].getNumberSafe();
	for (size_t i = 1; i < ca.args.size(); ++i)
	{
		avm_number_t n = ca.args[i].getNumberSafe();
		if (n < n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_pow(CallArgs& ca)
{
	avm_number_t b = ca.args[0].getNumberSafe();
	avm_number_t e = ca.args[1].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::pow(b, e));
}

void AsMath::Math_random(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(m_random.nextDouble()));
}

void AsMath::Math_round(CallArgs& ca)
{
	avm_number_t x = ca.args[0].getNumberSafe();
	avm_number_t f = (avm_number_t)std::abs(x);

	if (f - int(f) >= 0.5)
		f += 1.0;

	if (x < 0)
		f = -f;

	ca.ret = ActionValue(f);
}

void AsMath::Math_sin(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::sin(n));
}

void AsMath::Math_sqrt(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::sqrt(n));
}

void AsMath::Math_tan(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::tan(n));
}

	}
}

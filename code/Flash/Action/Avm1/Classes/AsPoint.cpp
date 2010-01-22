#include <cmath>
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionPoint.h"
#include "Flash/Action/Avm1/Classes/AsPoint.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsPoint", AsPoint, ActionClass)

Ref< AsPoint > AsPoint::getInstance()
{
	static Ref< AsPoint > instance = 0;
	if (!instance)
	{
		instance = new AsPoint();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsPoint::AsPoint()
:	ActionClass(L"Point")
{
}

void AsPoint::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"add", createNativeFunctionValue(this, &AsPoint::Point_add));
	prototype->setMember(L"clone", createNativeFunctionValue(this, &AsPoint::Point_clone));
	prototype->setMember(L"distance", createNativeFunctionValue(this, &AsPoint::Point_distance));
	prototype->setMember(L"equals", createNativeFunctionValue(this, &AsPoint::Point_equals));
	prototype->setMember(L"interpolate", createNativeFunctionValue(this, &AsPoint::Point_interpolate));
	prototype->setMember(L"normalize", createNativeFunctionValue(this, &AsPoint::Point_normalize));
	prototype->setMember(L"offset", createNativeFunctionValue(this, &AsPoint::Point_offset));
	prototype->setMember(L"polar", createNativeFunctionValue(this, &AsPoint::Point_polar));
	prototype->setMember(L"subtract", createNativeFunctionValue(this, &AsPoint::Point_subtract));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsPoint::Point_toString));

	prototype->addProperty(L"length", createNativeFunction(this, &AsPoint::Point_get_length), createNativeFunction(this, &AsPoint::Point_set_length));
	prototype->addProperty(L"x", createNativeFunction(this, &AsPoint::Point_get_x), createNativeFunction(this, &AsPoint::Point_set_x));
	prototype->addProperty(L"y", createNativeFunction(this, &AsPoint::Point_get_y), createNativeFunction(this, &AsPoint::Point_set_y));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsPoint::construct(ActionContext* context, const args_t& args)
{
	Ref< ActionPoint > pt = new ActionPoint();

	if (args.size() >= 2)
	{
		pt->x = args[0].getNumberSafe();
		pt->y = args[1].getNumberSafe();
	}

	return ActionValue::fromObject(pt);
}

void AsPoint::Point_add(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	Ref< ActionPoint > pr = ca.args[0].getObject< ActionPoint >();
	pt->x += pr->x;
	pt->y += pr->y;
}

void AsPoint::Point_clone(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	ca.ret = ActionValue::fromObject(new ActionPoint(pt->x, pt->y));
}

void AsPoint::Point_distance(CallArgs& ca)
{
	Ref< ActionPoint > pt1 = ca.args[0].getObject< ActionPoint >();
	Ref< ActionPoint > pt2 = ca.args[1].getObject< ActionPoint >();
	avm_number_t dx = pt2->x - pt1->x;
	avm_number_t dy = pt2->y - pt1->y;
	ca.ret = ActionValue(sqrtf(dx * dx + dy * dy));
}

void AsPoint::Point_equals(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	Ref< ActionPoint > pr = ca.args[0].getObject< ActionPoint >();
	ca.ret = ActionValue(bool(pt->x == pr->x && pt->y == pr->y));
}

void AsPoint::Point_interpolate(CallArgs& ca)
{
	Ref< ActionPoint > pt1 = ca.args[0].getObject< ActionPoint >();
	Ref< ActionPoint > pt2 = ca.args[1].getObject< ActionPoint >();
	avm_number_t f = ca.args[2].getNumberSafe();
	ca.ret = ActionValue::fromObject(new ActionPoint(
		pt1->x * (1.0f - f) + pt2->x * f,
		pt1->y * (1.0f - f) + pt2->y * f
	));
}

void AsPoint::Point_normalize(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	avm_number_t scale = ca.args[0].getNumberSafe();
	avm_number_t ln = sqrtf(pt->x * pt->x + pt->y * pt->y);
	pt->x *= scale / ln;
	pt->y *= scale / ln;
}

void AsPoint::Point_offset(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	pt->x += ca.args[0].getNumberSafe();
	pt->y += ca.args[1].getNumberSafe();
}

void AsPoint::Point_polar(CallArgs& ca)
{
	avm_number_t length = ca.args[0].getNumberSafe();
	avm_number_t angle = ca.args[1].getNumberSafe();
	ca.ret = ActionValue::fromObject(new ActionPoint(
		cosf(angle) * length,
		sinf(angle) * length
	));
}

void AsPoint::Point_subtract(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	Ref< ActionPoint > pr = ca.args[0].getObject< ActionPoint >();
	pt->x -= pr->x;
	pt->y -= pr->y;
}

void AsPoint::Point_toString(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	StringOutputStream ss; ss << pt->x << L", " << pt->y;
	ca.ret = ActionValue(ss.str());
}

void AsPoint::Point_get_length(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	ca.ret = ActionValue(sqrtf(pt->x * pt->x + pt->y * pt->y));
}

void AsPoint::Point_set_length(CallArgs& ca)
{
}

void AsPoint::Point_get_x(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	ca.ret = ActionValue(pt->x);
}

void AsPoint::Point_set_x(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	pt->x = ca.args[0].getNumberSafe();
}

void AsPoint::Point_get_y(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	ca.ret = ActionValue(pt->y);
}

void AsPoint::Point_set_y(CallArgs& ca)
{
	Ref< ActionPoint > pt = checked_type_cast< ActionPoint* >(ca.self);
	pt->y = ca.args[0].getNumberSafe();
}

	}
}

#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsTextField_StyleSheet.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextField_StyleSheet", AsTextField_StyleSheet, ActionClass)

Ref< AsTextField_StyleSheet > AsTextField_StyleSheet::getInstance()
{
	static Ref< AsTextField_StyleSheet > instance = 0;
	if (!instance)
	{
		instance = new AsTextField_StyleSheet();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsTextField_StyleSheet::AsTextField_StyleSheet()
:	ActionClass(L"TextField.StyleSheet")
{
}

void AsTextField_StyleSheet::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"clear", ActionValue(createNativeFunction(this, &AsTextField_StyleSheet::TextField_StyleSheet_clear)));
	prototype->setMember(L"getStyle", ActionValue(createNativeFunction(this, &AsTextField_StyleSheet::TextField_StyleSheet_getStyle)));
	prototype->setMember(L"getStyleNames", ActionValue(createNativeFunction(this, &AsTextField_StyleSheet::TextField_StyleSheet_getStyleNames)));
	prototype->setMember(L"load", ActionValue(createNativeFunction(this, &AsTextField_StyleSheet::TextField_StyleSheet_load)));
	prototype->setMember(L"parseCSS", ActionValue(createNativeFunction(this, &AsTextField_StyleSheet::TextField_StyleSheet_parseCSS)));
	prototype->setMember(L"setStyle", ActionValue(createNativeFunction(this, &AsTextField_StyleSheet::TextField_StyleSheet_setStyle)));
	prototype->setMember(L"transform", ActionValue(createNativeFunction(this, &AsTextField_StyleSheet::TextField_StyleSheet_transform)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsTextField_StyleSheet::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue(new ActionObject(this));
}

void AsTextField_StyleSheet::TextField_StyleSheet_clear(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_getStyle(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_getStyleNames(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_load(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_parseCSS(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_setStyle(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_transform(CallArgs& ca)
{
}

	}
}

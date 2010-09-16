#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSystem.h"
#include "Flash/Action/Avm1/Classes/AsSecurity.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSystem", AsSystem, ActionClass)

AsSystem::AsSystem()
:	ActionClass(L"System")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"security", ActionValue(new AsSecurity()));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsSystem::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

	}
}

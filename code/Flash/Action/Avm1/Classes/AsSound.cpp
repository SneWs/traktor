#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSound.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSound", AsSound, ActionClass)

AsSound::AsSound()
:	ActionClass(L"Sound")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"attachSound", ActionValue(createNativeFunction(this, &AsSound::Sound_attachSound)));
	prototype->setMember(L"getBytesLoaded", ActionValue(createNativeFunction(this, &AsSound::Sound_getBytesLoaded)));
	prototype->setMember(L"getBytesTotal", ActionValue(createNativeFunction(this, &AsSound::Sound_getBytesTotal)));
	prototype->setMember(L"getPan", ActionValue(createNativeFunction(this, &AsSound::Sound_getPan)));
	prototype->setMember(L"getTransform", ActionValue(createNativeFunction(this, &AsSound::Sound_getTransform)));
	prototype->setMember(L"getVolume", ActionValue(createNativeFunction(this, &AsSound::Sound_getVolume)));
	prototype->setMember(L"loadSound", ActionValue(createNativeFunction(this, &AsSound::Sound_loadSound)));
	prototype->setMember(L"setPan", ActionValue(createNativeFunction(this, &AsSound::Sound_setPan)));
	prototype->setMember(L"setTransform", ActionValue(createNativeFunction(this, &AsSound::Sound_setTransform)));
	prototype->setMember(L"setVolume", ActionValue(createNativeFunction(this, &AsSound::Sound_setVolume)));
	prototype->setMember(L"start", ActionValue(createNativeFunction(this, &AsSound::Sound_start)));
	prototype->setMember(L"stop", ActionValue(createNativeFunction(this, &AsSound::Sound_stop)));

	prototype->addProperty(L"checkPolicyFile", createNativeFunction(this, &AsSound::Sound_get_checkPolicyFile), createNativeFunction(this, &AsSound::Sound_set_checkPolicyFile));
	prototype->addProperty(L"duration", createNativeFunction(this, &AsSound::Sound_get_duration), 0);
	prototype->addProperty(L"id3", createNativeFunction(this, &AsSound::Sound_get_id3), 0);
	prototype->addProperty(L"position", createNativeFunction(this, &AsSound::Sound_get_position), 0);

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsSound::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void AsSound::Sound_attachSound(CallArgs& ca)
{
	log::warning << L"Sound.attachSound not implemented" << Endl;
}

void AsSound::Sound_getBytesLoaded(CallArgs& ca)
{
	log::warning << L"Sound.getBytesLoaded not implemented" << Endl;
	ca.ret = ActionValue(avm_number_t(0));
}

void AsSound::Sound_getBytesTotal(CallArgs& ca)
{
	log::warning << L"Sound.getBytesTotal not implemented" << Endl;
	ca.ret = ActionValue(avm_number_t(0));
}

void AsSound::Sound_getPan(CallArgs& ca)
{
	log::warning << L"Sound.getPan not implemented" << Endl;
	ca.ret = ActionValue(avm_number_t(0));
}

void AsSound::Sound_getTransform(CallArgs& ca)
{
	log::warning << L"Sound.getTransform not implemented" << Endl;
}

void AsSound::Sound_getVolume(CallArgs& ca)
{
	log::warning << L"Sound.getVolume not implemented" << Endl;
	ca.ret = ActionValue(avm_number_t(100));
}

void AsSound::Sound_loadSound(CallArgs& ca)
{
	log::warning << L"Sound.loadSound not implemented" << Endl;
}

void AsSound::Sound_setPan(CallArgs& ca)
{
	log::warning << L"Sound.setPan not implemented" << Endl;
}

void AsSound::Sound_setTransform(CallArgs& ca)
{
	log::warning << L"Sound.setTransform not implemented" << Endl;
}

void AsSound::Sound_setVolume(CallArgs& ca)
{
	log::warning << L"Sound.setVolume not implemented" << Endl;
}

void AsSound::Sound_start(CallArgs& ca)
{
	log::warning << L"Sound.start not implemented" << Endl;
}

void AsSound::Sound_stop(CallArgs& ca)
{
	log::warning << L"Sound.stop not implemented" << Endl;
}

void AsSound::Sound_get_checkPolicyFile(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsSound::Sound_set_checkPolicyFile(CallArgs& ca)
{
}

void AsSound::Sound_get_duration(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(0));
}

void AsSound::Sound_get_id3(CallArgs& ca)
{
	ca.ret = ActionValue("Not implemented");
}

void AsSound::Sound_get_position(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(0));
}

	}
}

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueArray.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSprite", FlashSprite, FlashCharacter)

FlashSprite::FlashSprite(uint16_t id, uint16_t frameRate)
:	FlashCharacter(id)
,	m_frameRate(frameRate)
{
}

uint16_t FlashSprite::getFrameRate() const
{
	return m_frameRate;
}

void FlashSprite::addFrame(FlashFrame* frame)
{
	m_frames.push_back(frame);
}

uint32_t FlashSprite::getFrameCount() const
{
	return uint32_t(m_frames.size());
}

FlashFrame* FlashSprite::getFrame(uint32_t frameId) const
{
	return m_frames[frameId];
}

int FlashSprite::findFrame(const std::string& frameLabel) const
{
	for (RefArray< FlashFrame >::const_iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		if (compareIgnoreCase< std::string >((*i)->getLabel(), frameLabel) == 0)
			return int(std::distance(m_frames.begin(), i));
	}
	return -1;
}

void FlashSprite::addInitActionScript(const IActionVMImage* initActionScript)
{
	m_initActionScripts.push_back(initActionScript);
}

const RefArray< const IActionVMImage >& FlashSprite::getInitActionScripts() const
{
	return m_initActionScripts;
}

Ref< FlashCharacterInstance > FlashSprite::createInstance(ActionContext* context, FlashCharacterInstance* parent, const std::string& name, const ActionObject* initObject) const
{
	Ref< FlashSpriteInstance > spriteInstance = new FlashSpriteInstance(context, parent, this);

	if (!name.empty())
		spriteInstance->setName(name);

	spriteInstance->eventInit();
	spriteInstance->updateDisplayList();

	if (initObject)
	{
		ActionObject* spriteInstanceAO = spriteInstance->getAsObject(context);
		T_ASSERT (spriteInstanceAO);

		const ActionObject::member_map_t& members = initObject->getLocalMembers();
		for (ActionObject::member_map_t::const_iterator i = members.begin(); i != members.end(); ++i)
		{
			// \fixme Same as enum2; need a visible member enumerator.
			if (i->first != ActionContext::Id__proto__ && i->first != ActionContext::IdPrototype)
				spriteInstanceAO->setMember(i->first, i->second);
		}
	}

	std::string spriteClassName;
	if (context->getMovie()->getExportName(getId(), spriteClassName))
	{
		ActionValue spriteClassValue;
		if (context->getGlobal()->getMember(spriteClassName, spriteClassValue))
		{
			ActionObject* spriteInstanceAO = spriteInstance->getAsObject(context);
			T_ASSERT (spriteInstanceAO);

			ActionValue prototype;
			spriteClassValue.getObjectAlways(context)->getMember(ActionContext::IdPrototype, prototype);
			spriteInstanceAO->setMember(ActionContext::Id__proto__, prototype);
			spriteInstanceAO->setMember(ActionContext::Id__ctor__, spriteClassValue);

			ActionFunction* classConstructor = spriteClassValue.getObject< ActionFunction >();
			if (classConstructor)
				classConstructor->call(spriteInstanceAO);
		}
	}

	return spriteInstance;
}

	}
}

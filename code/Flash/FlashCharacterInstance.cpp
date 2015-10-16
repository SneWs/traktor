#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacterInstance", FlashCharacterInstance, ActionObjectRelay)

int32_t FlashCharacterInstance::ms_instanceCount = 0;

FlashCharacterInstance::FlashCharacterInstance(ActionContext* context, const char* const prototype, FlashCharacterInstance* parent)
:	ActionObjectRelay(prototype)
,	m_context(context)
,	m_parent(parent)
,	m_visible(true)
,	m_enabled(true)
,	m_filter(0)
,	m_blendMode(0)
{
	Atomic::increment(ms_instanceCount);

	m_cxform.red[0] =
	m_cxform.green[0] =
	m_cxform.blue[0] =
	m_cxform.alpha[0] = 1.0f;
	m_cxform.red[1] =
	m_cxform.green[1] =
	m_cxform.blue[1] =
	m_cxform.alpha[1] = 0.0f;

	m_transform = Matrix33::identity();
}

FlashCharacterInstance::~FlashCharacterInstance()
{
	destroy();
	Atomic::decrement(ms_instanceCount);
}

int32_t FlashCharacterInstance::getInstanceCount()
{
	return ms_instanceCount;
}

void FlashCharacterInstance::destroy()
{
	if (m_context)
	{
		if (m_context->getFocus() == this)
			m_context->setFocus(0);
	}

	m_context = 0;
	m_parent = 0;
	m_eventScripts.clear();

	ActionObjectRelay::dereference();
}

ActionContext* FlashCharacterInstance::getContext() const
{
	return m_context;
}

FlashCharacterInstance* FlashCharacterInstance::getParent() const
{
	return m_parent;
}

void FlashCharacterInstance::setName(const std::string& name)
{
	m_name = name;
}

const std::string& FlashCharacterInstance::getName() const
{
	return m_name;
}

std::string FlashCharacterInstance::getTarget() const
{
	return m_parent ? (m_parent->getTarget() + "/" + getName()) : "";
}

void FlashCharacterInstance::setColorTransform(const SwfCxTransform& cxform)
{
	m_cxform = cxform;
}

const SwfCxTransform& FlashCharacterInstance::getColorTransform() const
{
	return m_cxform;
}

void FlashCharacterInstance::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& FlashCharacterInstance::getTransform() const
{
	return m_transform;
}

Matrix33 FlashCharacterInstance::getFullTransform() const
{
	if (m_parent)
		return m_parent->getFullTransform() * m_transform;
	else
		return m_transform;
}

void FlashCharacterInstance::setFilter(uint8_t filter)
{
	m_filter = filter;
}

uint8_t FlashCharacterInstance::getFilter() const
{
	return m_filter;
}

void FlashCharacterInstance::setFilterColor(const SwfColor& filterColor)
{
	m_filterColor = filterColor;
}

const SwfColor& FlashCharacterInstance::getFilterColor() const
{
	return m_filterColor;
}

void FlashCharacterInstance::setBlendMode(uint8_t blendMode)
{
	m_blendMode = blendMode;
}

uint8_t FlashCharacterInstance::getBlendMode() const
{
	return m_blendMode;
}

void FlashCharacterInstance::setVisible(bool visible)
{
	m_visible = visible;
}

bool FlashCharacterInstance::isVisible() const
{
	return m_visible;
}

void FlashCharacterInstance::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

bool FlashCharacterInstance::isEnabled() const
{
	return m_enabled;
}

void FlashCharacterInstance::setEvents(const SmallMap< uint32_t, Ref< const IActionVMImage > >& eventScripts)
{
	m_eventScripts = eventScripts;
}

const SmallMap< uint32_t, Ref< const IActionVMImage > >& FlashCharacterInstance::getEvents() const
{
	return m_eventScripts;
}

void FlashCharacterInstance::preDispatchEvents()
{
}

void FlashCharacterInstance::postDispatchEvents()
{
}

void FlashCharacterInstance::eventInit()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtInitialize);
	if (i != m_eventScripts.end())
	{
		ActionObject* self = getAsObject(m_context);
		Ref< ActionObject > super = self->getSuper();

		ActionFrame callFrame(
			m_context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(m_context->getGlobal()));

		i->second->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventConstruct()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtConstruct);
	if (i != m_eventScripts.end())
	{
		ActionObject* self = getAsObject(m_context);
		Ref< ActionObject > super = self->getSuper();

		ActionFrame callFrame(
			m_context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(m_context->getGlobal()));

		i->second->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventLoad()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtLoad);
	if (i != m_eventScripts.end())
	{
		ActionObject* self = getAsObject(m_context);
		Ref< ActionObject > super = self->getSuper();

		ActionFrame callFrame(
			m_context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(m_context->getGlobal()));

		i->second->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventFrame()
{
	SmallMap< uint32_t, Ref< const IActionVMImage > >::iterator i = m_eventScripts.find(EvtEnterFrame);
	if (i != m_eventScripts.end())
	{
		ActionObject* self = getAsObject(m_context);
		Ref< ActionObject > super = self->getSuper();

		ActionFrame callFrame(
			m_context,
			self,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(m_context->getGlobal()));

		i->second->execute(&callFrame);
	}
}

void FlashCharacterInstance::eventKey(wchar_t unicode)
{
}

void FlashCharacterInstance::eventKeyDown(int keyCode)
{
}

void FlashCharacterInstance::eventKeyUp(int keyCode)
{
}

void FlashCharacterInstance::eventMouseDown(int x, int y, int button)
{
}

void FlashCharacterInstance::eventMouseUp(int x, int y, int button)
{
}

void FlashCharacterInstance::eventMouseMove0(int x, int y, int button)
{
}

void FlashCharacterInstance::eventMouseMove1(int x, int y, int button)
{
}

void FlashCharacterInstance::eventSetFocus()
{
	executeScriptEvent(ActionContext::IdOnSetFocus, ActionValue());
}

void FlashCharacterInstance::eventKillFocus()
{
	executeScriptEvent(ActionContext::IdOnKillFocus, ActionValue());
}

bool FlashCharacterInstance::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	if (m_parent && memberName == ActionContext::IdParent)
	{
		outMemberValue = ActionValue(m_parent->getAsObject(context));
		return true;
	}
	else
		return false;
}

void FlashCharacterInstance::setParent(FlashCharacterInstance* parent)
{
	m_parent = parent;
}

bool FlashCharacterInstance::haveScriptEvent(uint32_t eventName)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	ActionValue memberValue;
	if (!self->getMember(eventName, memberValue))
		return false;

	Ref< ActionFunction > eventFunction = memberValue.getObject< ActionFunction >();
	if (!eventFunction)
		return false;

	return true;
}

bool FlashCharacterInstance::executeScriptEvent(uint32_t eventName, const ActionValue& arg)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	ActionValue memberValue;
	if (!self->getMember(eventName, memberValue))
		return false;

	Ref< ActionFunction > eventFunction = memberValue.getObject< ActionFunction >();
	if (!eventFunction)
		return false;

	ActionValueArray argv(m_context->getPool(), 1);
	argv[0] = arg;

	eventFunction->call(self, argv);
	return true;
}

void FlashCharacterInstance::trace(visitor_t visitor) const
{
	visitor(m_context);
	ActionObjectRelay::trace(visitor);
}

void FlashCharacterInstance::dereference()
{
	m_context = 0;
	m_parent = 0;
	ActionObjectRelay::dereference();
}

	}
}

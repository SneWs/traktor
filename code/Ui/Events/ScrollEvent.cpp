#include "Ui/Events/ScrollEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ScrollEvent", ScrollEvent, Event)

ScrollEvent::ScrollEvent(EventSubject* sender, int32_t position)
:	Event(sender)
,	m_position(position)
{
}

int32_t ScrollEvent::getPosition() const
{
	return m_position;
}

	}
}

#include "Ui/ToolTipEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolTipEvent", ToolTipEvent, Event)

ToolTipEvent::ToolTipEvent(EventSubject* sender, const Point& position)
:	Event(sender)
,	m_position(position)
{
}

void ToolTipEvent::setPosition(const Point& position)
{
	m_position = position;
}

const Point& ToolTipEvent::getPosition() const
{
	return m_position;
}

void ToolTipEvent::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& ToolTipEvent::getText() const
{
	return m_text;
}

	}
}

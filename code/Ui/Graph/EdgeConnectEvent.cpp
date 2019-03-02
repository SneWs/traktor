#include "Ui/Graph/EdgeConnectEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EdgeConnectEvent", EdgeConnectEvent, Event)

EdgeConnectEvent::EdgeConnectEvent(EventSubject* sender, Edge* edge)
:	Event(sender)
,	m_edge(edge)
{
}

Edge* EdgeConnectEvent::getEdge() const
{
	return m_edge;
}

	}
}

#include "Ui/Custom/Graph/EdgeDisconnectEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.EdgeDisconnectEvent", EdgeDisconnectEvent, Event)

EdgeDisconnectEvent::EdgeDisconnectEvent(EventSubject* sender, Edge* edge)
:	Event(sender)
,	m_edge(edge)
{
}

Edge* EdgeDisconnectEvent::getEdge() const
{
	return m_edge;
}

		}
	}
}

#include "Ui/Graph/NodeMovedEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NodeMovedEvent", NodeMovedEvent, Event)

NodeMovedEvent::NodeMovedEvent(EventSubject* sender, Node* node)
:	Event(sender)
,	m_node(node)
{
}

Node* NodeMovedEvent::getNode() const
{
	return m_node;
}

	}
}

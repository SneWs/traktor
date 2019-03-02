#include "Ui/Events/TabSelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TabSelectionChangeEvent", TabSelectionChangeEvent, SelectionChangeEvent)

TabSelectionChangeEvent::TabSelectionChangeEvent(EventSubject* sender, TabPage* tabPage)
:	SelectionChangeEvent(sender)
,	m_tabPage(tabPage)
{
}

TabPage* TabSelectionChangeEvent::getTabPage() const
{
	return m_tabPage;
}

	}
}

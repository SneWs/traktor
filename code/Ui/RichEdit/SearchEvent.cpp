#include "Ui/RichEdit/SearchEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SearchEvent", SearchEvent, ui::Event)

SearchEvent::SearchEvent(
	ui::EventSubject* sender,
	bool preview
)
:	ui::Event(sender)
,	m_preview(preview)
{
}

	}
}

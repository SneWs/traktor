#include "Script/Editor/SearchEvent.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.SearchEvent", SearchEvent, ui::Event)

SearchEvent::SearchEvent(
	ui::EventSubject* sender,
	const std::wstring& search,
	bool caseSensitive,
	bool wholeWord,
	bool wildCard,
	bool preview
)
:	ui::Event(sender)
,	m_search(search)
,	m_caseSensitive(caseSensitive)
,	m_wholeWord(wholeWord)
,	m_wildCard(wildCard)
,	m_preview(preview)
{
}

	}
}

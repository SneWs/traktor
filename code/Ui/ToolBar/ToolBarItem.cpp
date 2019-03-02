#include "Ui/ToolBar/ToolBarItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarItem", ToolBarItem, Object)

ToolBarItem::ToolBarItem()
:	m_enable(true)
{
}

void ToolBarItem::setEnable(bool enable)
{
	m_enable = enable;
}

bool ToolBarItem::isEnable() const
{
	return m_enable;
}

	}
}

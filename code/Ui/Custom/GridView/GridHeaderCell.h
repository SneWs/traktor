#ifndef traktor_ui_custom_GridHeaderCell_H
#define traktor_ui_custom_GridHeaderCell_H

#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class GridColumn;

class T_DLLCLASS GridHeaderCell : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	void setColumns(const RefArray< GridColumn >& columns);

	virtual void mouseDown(AutoWidget* widget, const Point& position);

	virtual void mouseUp(AutoWidget* widget, const Point& position);

	virtual void mouseMove(AutoWidget* widget, const Point& position);

	virtual void paint(AutoWidget* widget, Canvas& canvas, const Rect& rect);

private:
	RefArray< GridColumn > m_columns;
};

		}
	}
}

#endif	// traktor_ui_custom_GridHeaderCell_H

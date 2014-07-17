#ifndef traktor_ui_custom_AutoWidgetCell_H
#define traktor_ui_custom_AutoWidgetCell_H

#include "Ui/Associative.h"
#include "Ui/EventSubject.h"
#include "Ui/Rect.h"

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

class Canvas;

		namespace custom
		{

class AutoWidget;

/*! \brief Auto widget cell.
 * \ingroup UIC
 */
class T_DLLCLASS AutoWidgetCell
:	public ui::EventSubject
,	public Associative
{
	T_RTTI_CLASS;

public:
	AutoWidgetCell();

	virtual void placeCells(AutoWidget* widget, const Rect& rect);

	virtual AutoWidgetCell* hitTest(const Point& position);

	virtual bool beginCapture();

	virtual void endCapture();

	virtual void mouseDown(const Point& position);

	virtual void mouseUp(const Point& position);

	virtual void mouseMove(const Point& position);

	virtual void paint(Canvas& canvas, const Rect& rect);

protected:
	AutoWidget* getWidget();

private:
	AutoWidget* m_widget;
};

		}
	}
}

#endif	// traktor_ui_custom_AutoWidgetCell_H

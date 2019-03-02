#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Layout;
class SizeEvent;

/*! \brief Layout container.
 * \ingroup UI
 */
class T_DLLCLASS Container : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style = WsNone, Layout* layout = 0);

	virtual void fit();

	virtual void update(const Rect* rc = 0, bool immediate = false) override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferedSize() const override;

	virtual Size getMaximumSize() const override;

	Ref< Layout > getLayout() const;

	void setLayout(Layout* layout);

private:
	Ref< Layout > m_layout;

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}


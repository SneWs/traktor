#pragma once

#include "Ui/Dialog.h"

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

class Button;
class ButtonClickEvent;

/*! \brief Configuration dialog.
 * \ingroup UI
 */
class T_DLLCLASS ConfigDialog : public Dialog
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsApplyButton = (WsUser << 1)
	};

	bool create(Widget* parent, const std::wstring& text, int width, int height, int style, Layout* layout);

	virtual void destroy() override;

	virtual void update(const Rect* rc = 0, bool immediate = false) override;

	virtual Rect getInnerRect() const override;

private:
	Ref< Button > m_ok;
	Ref< Button > m_cancel;
	Ref< Button > m_apply;

	void eventButtonClick(ButtonClickEvent* event);
};

	}
}


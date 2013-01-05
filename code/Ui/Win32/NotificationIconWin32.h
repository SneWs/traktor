#ifndef traktor_ui_NotificationIconWin32_H
#define traktor_ui_NotificationIconWin32_H

#include "Ui/Itf/INotificationIcon.h"
#include "Ui/Win32/Window.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

/*! \brief
 * \ingroup UIW32
 */
class NotificationIconWin32 : public INotificationIcon
{
public:
	NotificationIconWin32(EventSubject* owner);

	virtual bool create(const std::wstring& text, IBitmap* image);

	virtual void destroy();

	virtual void setImage(IBitmap* image);

private:
	EventSubject* m_owner;
	Window m_hWnd;
	NOTIFYICONDATA m_nid;

	LRESULT eventNotification(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventTaskbarCreated(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_NotificationIconWin32_H

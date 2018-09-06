#ifndef traktor_ui_EventLoopGtk_H
#define traktor_ui_EventLoopGtk_H

#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WIN32_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace Gtk
{

	class Main;

}

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS EventLoopGtk : public IEventLoop
{
public:
	EventLoopGtk();

	virtual ~EventLoopGtk();

	virtual bool process(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual int32_t execute(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual void exit(int32_t exitCode) T_OVERRIDE T_FINAL;

	virtual int32_t getExitCode() const T_OVERRIDE T_FINAL;

	virtual int32_t getAsyncKeyState() const T_OVERRIDE T_FINAL;

	virtual bool isKeyDown(VirtualKey vk) const T_OVERRIDE T_FINAL;

	virtual Size getDesktopSize() const T_OVERRIDE T_FINAL;

private:
	bool m_terminated;
	int32_t m_exitCode;
};

	}
}

#endif	// traktor_ui_EventLoopGtk_H


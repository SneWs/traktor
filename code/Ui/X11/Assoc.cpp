#include "Core/Assert.h"
#include "Ui/X11/Assoc.h"

namespace traktor
{
    namespace ui
    {

Assoc& Assoc::getInstance()
{
    static Assoc s_instance;
    return s_instance;
}

void Assoc::bind(Drawable window, int32_t eventType, const std::function< void(XEvent& xe) >& fn)
{
    m_bindings[window][eventType] = fn;
}

void Assoc::unbind(Drawable window, int32_t eventType)
{
    m_bindings[window].erase(eventType);
}

void Assoc::unbind(Drawable window)
{
    m_bindings[window].clear();
}

void Assoc::dispatch(XEvent& xe)
{
    switch (xe.type)
    {
    case KeyPress:
        dispatch(xe.xkey.window, KeyPress, xe);
        break;

    case KeyRelease:
        dispatch(xe.xkey.window, KeyRelease, xe);
        break;

    case MotionNotify:
        dispatch(xe.xmotion.window, MotionNotify, xe);
        break;

    case ButtonPress:
        dispatch(xe.xbutton.window, ButtonPress, xe);
        break;

    case ButtonRelease:
        dispatch(xe.xbutton.window, ButtonRelease, xe);
        break;

    case ConfigureRequest:
        dispatch(xe.xconfigurerequest.window, ConfigureRequest, xe);
        break;

    case ConfigureNotify:
        dispatch(xe.xconfigure.window, ConfigureNotify, xe);
        break;
        
    case Expose:
        dispatch(xe.xexpose.window, Expose, xe);
        break;

    case ClientMessage:
        dispatch(xe.xclient.window, ClientMessage, xe);
        break;

    default:
        break;
    }
}

void Assoc::dispatch(Drawable window, int32_t eventType, XEvent& xe)
{
    auto bsi = m_bindings.find(window);
    if (bsi == m_bindings.end())
        return;

    auto bi = bsi->second.find(eventType);
    if (bi == bsi->second.end())
        return;

    std::function< void(XEvent& xe) > fn = bi->second;
    fn(xe);
}

    }
}
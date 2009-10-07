#import "Ui/Cocoa/NSWindowDelegateProxy.h"

@implementation NSWindowDelegateProxy

- (void)setCallback: (traktor::ui::INSWindowEventsCallback*)eventsCallback
{
	m_eventsCallback = eventsCallback;
}

- (void)windowDidMove: (NSNotification*)notification
{
	m_eventsCallback->event_windowDidMove();
}

- (void)windowDidResize: (NSNotification*)notification
{
	m_eventsCallback->event_windowDidResize();
}

@end

#import <Cocoa/Cocoa.h>

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/Std/OsX/CGLCustomWindow.h"
#include "Render/OpenGL/Std/OsX/CGLWindow.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
struct WindowData
{
	CGLCustomWindow* window;
	DisplayMode displayMode;
	bool fullscreen;
	CFDictionaryRef originalMode;
	NSString* title;
};

int32_t getDictionaryLong(CFDictionaryRef dict, const void* key)
{
	CFNumberRef numberRef = (CFNumberRef)CFDictionaryGetValue(dict, key);
	if (!numberRef)
		return 0;
	
	long value = 0;
	CFNumberGetValue(numberRef, kCFNumberLongType, &value);
	
	return int32_t(value);
}

void setWindowSize(NSWindow* window, int32_t width, int32_t height)
{
	NSRect frame = [window frame];
	frame.size.width = width;
	frame.size.height = height;
	
	NSRect content = [window contentRectForFrameRect: frame];
	frame.size.width += frame.size.width - content.size.width;
	frame.size.height += frame.size.height - content.size.height;
	
	[window setFrame: frame display: YES];
}

void setWindowRect(NSWindow* window, int32_t x, int32_t y, int32_t width, int32_t height)
{
	NSRect frame = [window frame];
	frame.size.width = width;
	frame.size.height = height;
	
	NSRect content = [window contentRectForFrameRect: frame];
	frame.origin.x = x;
	frame.origin.y = y;
	frame.size.width += frame.size.width - content.size.width;
	frame.size.height += frame.size.height - content.size.height;
	
	[window setFrame: frame display: YES];
}

void getValidDisplayModes(std::vector< CFDictionaryRef >& outValidModes)
{
	CFArrayRef availModes = CGDisplayAvailableModes(kCGDirectMainDisplay);
	for (int32_t i = 0; i < CFArrayGetCount(availModes); ++i)
	{
		CFDictionaryRef mode = (CFDictionaryRef)CFArrayGetValueAtIndex(availModes, i);
		if (!mode)
			continue;
			
		int32_t bitsPerPixel = getDictionaryLong(mode, kCGDisplayBitsPerPixel);
		if (bitsPerPixel < 15)
			continue;
			
		if (!CFDictionaryContainsKey(mode, kCGDisplayModeIsSafeForHardware))
			continue;
		if (CFDictionaryContainsKey(mode, kCGDisplayModeIsInterlaced))
			continue;
		if (CFDictionaryContainsKey(mode, kCGDisplayModeIsTelevisionOutput))
			continue;
		if (CFDictionaryContainsKey(mode, kCGDisplayModeIsStretched))
			continue;
			
		outValidModes.push_back(mode);
	}
}

		}
	
uint32_t cglwGetDisplayModeCount()
{
	std::vector< CFDictionaryRef > displayModes;
	getValidDisplayModes(displayModes);
	return uint32_t(displayModes.size());
}

bool cglwGetDisplayMode(uint32_t index, DisplayMode& outDisplayMode)
{
	std::vector< CFDictionaryRef > displayModes;
	getValidDisplayModes(displayModes);
	if (index >= displayModes.size())
		return false;
	
	CFDictionaryRef mode = displayModes[index];
	if (!mode)
		return false;
		
	outDisplayMode.width = getDictionaryLong(mode, kCGDisplayWidth);
	outDisplayMode.height = getDictionaryLong(mode, kCGDisplayHeight);
	outDisplayMode.refreshRate = getDictionaryLong(mode, kCGDisplayRefreshRate);
	outDisplayMode.colorBits = getDictionaryLong(mode, kCGDisplayBitsPerPixel);
	
	return true;
}

bool cglwSetDisplayMode(const DisplayMode& displayMode)
{
	CFDictionaryRef bestMatch = 0;

	log::info << L"cglwSetDisplayMode" << Endl;
	log::info << L"\twidth " << displayMode.width << Endl;
	log::info << L"\theight " << displayMode.height << Endl;
	log::info << L"\trefreshRate " << displayMode.refreshRate << Endl;
	log::info << L"\tcolorBits " << displayMode.colorBits << Endl;

	std::vector< CFDictionaryRef > displayModes;
	getValidDisplayModes(displayModes);

	for (std::vector< CFDictionaryRef >::iterator i = displayModes.begin(); i != displayModes.end(); ++i)
	{
		if (
			getDictionaryLong(*i, kCGDisplayWidth) == displayMode.width &&
			getDictionaryLong(*i, kCGDisplayHeight) == displayMode.height &&
			getDictionaryLong(*i, kCGDisplayRefreshRate) == displayMode.refreshRate &&
			getDictionaryLong(*i, kCGDisplayBitsPerPixel) == displayMode.colorBits
		)
		{
			bestMatch = *i;
			break;
		}
	}
		
	if (!bestMatch)
	{
		log::error << L"Unable to set display mode; no such display mode supported" << Endl;
		return false;
	}

	CGDisplaySwitchToMode(kCGDirectMainDisplay, bestMatch);
	return true;
}

bool cglwGetCurrentDisplayMode(DisplayMode& outDisplayMode)
{
	CFDictionaryRef mode = CGDisplayCurrentMode(kCGDirectMainDisplay);
//	if (!mode)
//		return false;
	
	outDisplayMode.width = getDictionaryLong(mode, kCGDisplayWidth);
	outDisplayMode.height = getDictionaryLong(mode, kCGDisplayHeight);
	outDisplayMode.refreshRate = getDictionaryLong(mode, kCGDisplayRefreshRate);
	outDisplayMode.colorBits = getDictionaryLong(mode, kCGDisplayBitsPerPixel);

	return true;
}

void* cglwCreateWindow(const std::wstring& title, const DisplayMode& displayMode, bool fullscreen)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[NSApplication sharedApplication];
	[NSApp finishLaunching];

	std::string mbs = wstombs(title);

	WindowData* windowData = new WindowData();
	windowData->window = 0;
	windowData->displayMode = displayMode;
	windowData->fullscreen = fullscreen;
	windowData->originalMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
	windowData->title = [[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding];
	
	NSRect frame = NSMakeRect(0, 0, displayMode.width, displayMode.height);
	
	if (fullscreen)
	{
		CGDisplayCapture(kCGDirectMainDisplay);

		if (!cglwSetDisplayMode(displayMode))
			return 0;
	
		NSInteger windowLevel = CGShieldingWindowLevel();

		windowData->window = [[CGLCustomWindow alloc]
			initWithContentRect: frame
			styleMask:NSBorderlessWindowMask
			backing: NSBackingStoreBuffered
			defer: YES
			screen: [NSScreen mainScreen]
		];

		[windowData->window setLevel: windowLevel];		
		[windowData->window setHidesOnDeactivate: YES];
		[windowData->window setOpaque: YES];
	}
	else
	{
		windowData->window = [[CGLCustomWindow alloc]
			initWithContentRect: frame
			styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
			backing: NSBackingStoreBuffered
			defer: YES
		];
	}
	
	[windowData->window init];

	[windowData->window setBackgroundColor: [NSColor blackColor]];
	[windowData->window setAcceptsMouseMovedEvents: YES];
	[windowData->window setTitle: windowData->title];
	
	if (!fullscreen)
		[windowData->window center];
	
	[windowData->window makeKeyAndOrderFront: nil];
	[windowData->window makeMainWindow];
	
	cglwUpdateWindow(windowData);
	
	if (fullscreen)
		CGDisplayHideCursor(kCGDirectMainDisplay);
	
	[pool release];	
	
	return windowData;
}

void cglwDestroyWindow(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	
	if (windowData->fullscreen)
		CGDisplayRelease(kCGDirectMainDisplay);
	
	[windowData->window release];
	[windowData->title release];
	
	delete windowData;
}

bool cglwModifyWindow(void* windowHandle, const DisplayMode& displayMode)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	
	if (windowData->fullscreen)
	{
		if (!cglwSetDisplayMode(displayMode))
			return false;
	}
	
	setWindowSize(
		windowData->window,
		displayMode.width,
		displayMode.height
	);
	
	// Center window if display mode is shrinking.
	if (
		!windowData->fullscreen &&
		(displayMode.width < windowData->displayMode.width ||
		displayMode.height < windowData->displayMode.height)
	)
		[windowData->window center];

	windowData->displayMode = displayMode;
	
	cglwUpdateWindow(windowHandle);

	return true;
}

void cglwSetFullscreen(void* windowHandle, bool fullscreen)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	
	if (windowData->fullscreen == fullscreen)
		return;

	if (fullscreen)
	{
		CGDisplayCapture(kCGDirectMainDisplay);

		if (!cglwSetDisplayMode(windowData->displayMode))
			return;

		NSInteger windowLevel = CGShieldingWindowLevel();

		[windowData->window setStyleMask: NSBorderlessWindowMask];
		[windowData->window setLevel: windowLevel];
		[windowData->window setOpaque: YES];
		
		setWindowRect(
			windowData->window,
			0,
			0,
			windowData->displayMode.width,
			windowData->displayMode.height
		);
		
		CGDisplayHideCursor(kCGDirectMainDisplay);
	}
	else
	{
		CGDisplaySwitchToMode(kCGDirectMainDisplay, windowData->originalMode);
		CGDisplayRelease(kCGDirectMainDisplay);
	
		[windowData->window setStyleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask];
		[windowData->window setLevel: NSNormalWindowLevel];
		[windowData->window setTitle: windowData->title];
		[windowData->window setOpaque: NO];

		setWindowSize(
			windowData->window,
			windowData->displayMode.width,
			windowData->displayMode.height
		);

		[windowData->window center];
		
		CGDisplayShowCursor(kCGDirectMainDisplay);
	}
	
	cglwUpdateWindow(windowHandle);
	
	windowData->fullscreen = fullscreen;
}

bool cglwIsFullscreen(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	return windowData->fullscreen;
}

bool cglwIsActive(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	return [windowData->window isKeyWindow] == YES;
}

bool cglwUpdateWindow(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	// Handle system events.
	NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
	if (event != nil)
	{
		NSEventType eventType = [event type];		
		if (eventType == NSKeyDown)
		{
			uint32_t modifierFlags = [event modifierFlags];
			int32_t keyCode = [event keyCode];
			
			// Toggle fullscreen with Cmd+M or Cmd+Return key combinations.
			if (
				(modifierFlags & kCGEventFlagMaskCommand) != 0 &&
				(keyCode == 0x2e || keyCode == 0x24)
			)
			{
				if (cglwIsFullscreen(windowHandle))
					cglwSetFullscreen(windowHandle, false);
				else
					cglwSetFullscreen(windowHandle, true);
			}
		}
		else if (eventType != NSKeyUp)
		{
			[NSApp sendEvent: event];
			[NSApp updateWindows];
		}
	}
	
	[pool release];
	return [windowData->window closed] == NO;
}

void* cglwGetWindowView(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	return [windowData->window contentView];
}
	
	}
}

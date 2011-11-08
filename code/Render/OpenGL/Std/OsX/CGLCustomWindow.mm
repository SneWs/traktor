//
//  CGLCustomWindow.m
//  Traktor MacOSX
//
//  Created by Anders Pistol on 2010-08-04.
//  Copyright 2010 Apple Inc. All rights reserved.
//

#import "Render/OpenGL/Std/OsX/CGLCustomWindow.h"

@implementation CGLCustomWindow

- (id) init
{
	m_closed = NO;
	return self;
}

- (BOOL) canBecomeKeyWindow
{
	return YES;
}

- (BOOL) canBecomeMainWindow
{
	return YES;
}

- (void) close
{
	m_closed = YES;
}

- (BOOL) closedSinceLast
{
	BOOL closed = m_closed;
	m_closed = NO;
	return closed;
}

@end

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/OpenGL/Std/Platform.h"

namespace traktor
{
	namespace render
	{

std::wstring getOpenGLErrorString(GLenum glError)
{
	switch (glError)
	{
	case GL_NO_ERROR:
		return L"GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return L"GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return L"GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return L"GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return L"GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return L"GL_OUT_OF_MEMORY";
	}
	return L"Unknown GL error";
}

	}
}

#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace editor
	{

class T_DLLCLASS IThumbnailGenerator : public Object
{
	T_RTTI_CLASS;

public:
	enum AlphaMode
	{
		AmNoAlpha = 0,
		AmWithAlpha = 1,
		AmAlphaOnly = 2
	};

	virtual Ref< drawing::Image > get(const Path& fileName, int32_t width, int32_t height, AlphaMode alphaMode) = 0;
};

	}
}


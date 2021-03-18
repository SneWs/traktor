#pragma once

#include "Render/Editor/Glsl/GlslResource.h"
#include "Render/Editor/Glsl/GlslType.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{
	
class T_DLLCLASS GlslImage : public GlslResource
{
	T_RTTI_CLASS;

public:
	explicit GlslImage(const std::wstring& name, uint8_t stages);

	virtual int32_t getOrdinal() const override final;
};

	}
}
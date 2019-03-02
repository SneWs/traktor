#pragma once

#include "Render/Editor/Texture/ICompressor.h"

namespace traktor
{
	namespace render
	{

class EtcCompressor : public ICompressor
{
	T_RTTI_CLASS;

public:
	virtual bool compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const override final;
};

	}
}


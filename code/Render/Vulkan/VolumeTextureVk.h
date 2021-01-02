#pragma once

#include "Render/IVolumeTexture.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class Buffer;
class Context;
class Image;

struct VolumeTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class VolumeTextureVk : public IVolumeTexture
{
	T_RTTI_CLASS;

public:
	explicit VolumeTextureVk(Context* context);

	virtual ~VolumeTextureVk();

	bool create(const VolumeTextureCreateDesc& desc, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getDepth() const override final;

	Image& getImage() const { return *m_textureImage; }

private:
	Context* m_context = nullptr;
	Ref< Buffer > m_stagingBuffer;
	Ref< Image > m_textureImage;
	VolumeTextureCreateDesc m_desc;
};

	}
}

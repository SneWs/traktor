#pragma once

#include "Render/ISimpleTexture.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

/*!
 * \ingroup Vulkan
 */
class RenderTargetVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetVk(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator
	);

	virtual ~RenderTargetVk();

	bool createPrimary(int32_t width, int32_t height, VkFormat format, VkImage image, const wchar_t* const tag);

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	void prepareForPresentation(VkCommandBuffer cmdBuffer);

	VkFormat getVkFormat() const { return m_format; }

	VkImage getVkImage() const { return m_image; }

	VkImageView getVkImageView() const { return m_imageView; }

	VkImageLayout getVkImageLayout() const { return m_imageLayout; }

private:
	friend class RenderTargetSetVk;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	VkFormat m_format;
	VkImage m_image;
	VmaAllocation m_allocation;
	VkImageView m_imageView;
	VkImageLayout m_imageLayout;
	int32_t m_width;
	int32_t m_height;

	void prepareAsTarget(VkCommandBuffer cmdBuffer);

	void prepareAsTexture(VkCommandBuffer cmdBuffer);

	void prepareForReadBack(VkCommandBuffer cmdBuffer);
};

	}
}

#pragma once

#include "Core/Ref.h"
#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{

class Buffer;
class Context;

/*!
 * \ingroup Vulkan
 */
class VertexBufferStaticVk : public VertexBufferVk
{
	T_RTTI_CLASS;

public:
	VertexBufferStaticVk(
		Context* context,
		uint32_t bufferSize,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	virtual ~VertexBufferStaticVk();

	bool create();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

	virtual VkBuffer getVkBuffer() const override final;

private:
	Context* m_context = nullptr;
	Ref< Buffer > m_stageBuffer;
	Ref< Buffer > m_deviceBuffer;
};

	}
}


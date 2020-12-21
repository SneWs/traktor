#pragma once

#include "Core/RefArray.h"
#include "Render/StructBuffer.h"
#include "Render/Vulkan/Buffer.h"

namespace traktor
{
	namespace render
	{

class Context;

class StructBufferVk : public StructBuffer
{
	T_RTTI_CLASS;

public:
	explicit StructBufferVk(Context* context, uint32_t bufferSize);

	bool create(int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t structOffset, uint32_t structCount) override final;

	virtual void unlock() override final;

	bool isValid() const { return !m_buffers.empty(); }

	VkBuffer getVkBuffer() const { return *m_buffers[m_index]; }

private:
	Ref< Context > m_context;
	RefArray< Buffer > m_buffers;
	int32_t m_index;
};

	}
}

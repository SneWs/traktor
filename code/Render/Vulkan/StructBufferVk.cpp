#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/StructBufferVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVk", StructBufferVk, StructBuffer)

StructBufferVk::StructBufferVk(Context* context, uint32_t bufferSize, uint32_t& instances)
:	StructBuffer(bufferSize)
,	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

StructBufferVk::~StructBufferVk()
{
	destroy();
	Atomic::decrement((int32_t&)m_instances);
}

bool StructBufferVk::create(int32_t inFlightCount)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_context->getPhysicalDevice(), &deviceProperties);
	uint32_t storageBufferOffsetAlignment = (uint32_t)deviceProperties.limits.minStorageBufferOffsetAlignment;

	m_alignedBufferSize = alignUp(bufferSize, storageBufferOffsetAlignment);
	m_inFlightCount = inFlightCount;

	m_buffer = new Buffer(m_context);
	m_buffer->create(m_alignedBufferSize * inFlightCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true, true);

	return true;
}

void StructBufferVk::destroy()
{
	safeDestroy(m_buffer);
	m_context = nullptr;
}

void* StructBufferVk::lock()
{
	uint8_t* ptr = (uint8_t*)m_buffer->lock();
	if (!ptr)
		return nullptr;

	return ptr + m_index * m_alignedBufferSize;
}

void* StructBufferVk::lock(uint32_t structOffset, uint32_t structCount)
{
	T_FATAL_ERROR;
	return nullptr;
}

void StructBufferVk::unlock()
{
	m_buffer->unlock();
	m_offset = m_index * m_alignedBufferSize;
	m_index = (m_index + 1) % m_inFlightCount;
}

	}
}
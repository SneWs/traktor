#include "Render/Vulkan/IndexBufferVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferVk", IndexBufferVk, IndexBuffer)

IndexBufferVk::IndexBufferVk(
	IndexType indexType,
	uint32_t bufferSize,
	Buffer&& buffer
)
:	IndexBuffer(indexType, bufferSize)
,	m_buffer(std::move(buffer))
{
}

void IndexBufferVk::destroy()
{
	m_buffer.destroy();
}

void* IndexBufferVk::lock()
{
	return m_buffer.lock();
}

void IndexBufferVk::unlock()
{
	m_buffer.unlock();
}

	}
}

#include "Core/Platform.h"
#include "Core/Memory/TrackAllocator.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{

TrackAllocator::TrackAllocator(IAllocator* systemAllocator)
:	m_systemAllocator(systemAllocator)
{
}

TrackAllocator::~TrackAllocator()
{
	if (!m_aliveBlocks.empty())
	{
#if defined(_WIN32)
		OutputDebugString(L"Memory leak detected, following allocation(s) not freed:\n");
		for (std::list< Block >::const_iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
		{
			wchar_t tmp[1024];

			wsprintf(tmp, L"0x%p, %d byte(s), tag \"%s\"\n", i->top, i->size, i->tag);
			OutputDebugString(tmp);

			for (int j = 0; j < sizeof_array(i->at); ++j)
			{
				wsprintf(tmp, L"   %d: 0x%p\n", j, i->at[j]);
				OutputDebugString(tmp);
			}
		}
#endif
	}
#if defined(_WIN32)
	else
		OutputDebugString(L"No memory leaks! Good work!\n");
#endif
}

void* TrackAllocator::alloc(size_t size, size_t align, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	void* ptr = m_systemAllocator->alloc(size, align, tag);
	if (!ptr)
		return 0;

	Block block;
	block.tag = tag;
	block.top = ptr;
	block.size = size;
	block.at[0] =
	block.at[1] =
	block.at[2] =
	block.at[3] = 0;

#if defined(_WIN32)

	uint32_t at_0 = 0, at_1 = 0;
	uint32_t at_2 = 0, at_3 = 0;
	__asm
	{
		mov ecx, [ebp]
		mov eax, [ecx + 4]
		mov at_0, eax
	}
	block.at[0] = (void*)(at_0 - 6);
	block.at[1] = (void*)at_1;
	block.at[2] = (void*)at_2;
	block.at[3] = (void*)at_3;

#endif

	m_aliveBlocks.push_back(block);

	return ptr;
}

void TrackAllocator::free(void* ptr)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (std::list< Block >::const_iterator i = m_aliveBlocks.begin(); i != m_aliveBlocks.end(); ++i)
	{
		if (i->top == ptr)
		{
			m_aliveBlocks.erase(i);
			break;
		}
	}

	m_systemAllocator->free(ptr);
}

}

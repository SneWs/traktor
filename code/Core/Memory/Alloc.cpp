#include <cstdlib>
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"

namespace traktor
{

void* Alloc::acquire(size_t size)
{
	return std::malloc(size);
}

void Alloc::free(void* ptr)
{
	std::free(ptr);
}

void* Alloc::acquireAlign(size_t size, size_t align)
{
#if defined(_WIN32) && !defined(WINCE)
	return _aligned_malloc(size, align);
#elif defined(_PS3)
	return std::memalign(align, size);
#else
	uint8_t* uptr = (uint8_t*)acquire(size + sizeof(size_t) + align);
	if (!uptr)
		return 0;
	uint8_t* aptr = alignUp(uptr + sizeof(size_t), align);
	*(size_t*)(aptr - sizeof(size_t)) = (size_t)uptr;
	return aptr;
#endif
}

void Alloc::freeAlign(void* ptr)
{
#if defined(_WIN32) && !defined(WINCE)
	_aligned_free(ptr);
#elif defined(_PS3)
	std::free(ptr);
#else
	if (ptr)
	{
		uint8_t* aptr = (uint8_t*)ptr;
		uint8_t* uptr = (uint8_t*)(*(size_t*)(aptr - sizeof(size_t)));
		Alloc::free(uptr);
	}
#endif
}

}

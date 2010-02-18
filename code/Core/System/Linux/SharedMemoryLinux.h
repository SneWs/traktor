#ifndef traktor_SharedMemoryLinux_H
#define traktor_SharedMemoryLinux_H

#include "Core/System/ISharedMemory.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

class SharedMemoryLinux : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryLinux(uint32_t size);

	virtual Ref< IStream > read(bool exclusive);

	virtual Ref< IStream > write();

private:
	AutoArrayPtr< uint8_t > m_data;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryLinux_H

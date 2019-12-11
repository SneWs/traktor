#pragma once

#include "Core/Ref.h"
#include "Core/Io/OutputStreamBuffer.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IEncoding;
class IStream;

/*! File output stream buffer.
 * \ingroup Core
 */
class T_DLLCLASS FileOutputStreamBuffer : public OutputStreamBuffer
{
public:
	FileOutputStreamBuffer(IStream* stream, IEncoding* encoding);

	void close();

	virtual int32_t overflow(const wchar_t* buffer, int32_t count) override final;

private:
	Semaphore m_lock;
	Ref< IStream > m_stream;
	Ref< IEncoding > m_encoding;
	AutoArrayPtr< uint8_t > m_encoded;
	uint32_t m_encodedSize;
};

}


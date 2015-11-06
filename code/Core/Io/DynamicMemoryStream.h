#ifndef traktor_DynamicMemoryStream_H
#define traktor_DynamicMemoryStream_H

#include <vector>
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Dynamic memory stream wrapper.
 * \ingroup Core
 */
class T_DLLCLASS DynamicMemoryStream : public IStream
{
	T_RTTI_CLASS;

public:
	DynamicMemoryStream(std::vector< uint8_t >& buffer, bool readAllowed = true, bool writeAllowed = true, const char* const name = 0);

	DynamicMemoryStream(bool readAllowed = true, bool writeAllowed = true, const char* const name = 0);

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool canRead() const T_OVERRIDE T_FINAL;

	virtual bool canWrite() const T_OVERRIDE T_FINAL;

	virtual bool canSeek() const T_OVERRIDE T_FINAL;

	virtual int tell() const T_OVERRIDE T_FINAL;

	virtual int available() const T_OVERRIDE T_FINAL;

	virtual int seek(SeekOriginType origin, int offset) T_OVERRIDE T_FINAL;

	virtual int read(void* block, int nbytes) T_OVERRIDE T_FINAL;

	virtual int write(const void* block, int nbytes) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

	const std::vector< uint8_t >& getBuffer() const;

	std::vector< uint8_t >& getBuffer();

private:
	std::vector< uint8_t > m_internal;
	std::vector< uint8_t >* m_buffer;
	uint32_t m_readPosition;
	bool m_readAllowed;
	bool m_writeAllowed;
#if defined(_DEBUG)
	const char* const m_name;
#endif
};

}

#endif	// traktor_DynamicMemoryStream_H

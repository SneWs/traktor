#ifndef traktor_compress_InflateStreamZip_H
#define traktor_compress_InflateStreamZip_H

#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_COMPRESS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace compress
	{

class InflateZipImpl;

/*! \brief Zip inflate stream.
 * \ingroup Compress
 */
class T_DLLCLASS InflateStreamZip : public IStream
{
	T_RTTI_CLASS;

public:
	InflateStreamZip(IStream* stream, uint32_t internalBufferSize = 4096);

	virtual ~InflateStreamZip();

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

private:
	Ref< InflateZipImpl > m_impl;
};

	}
}

#endif	// traktor_compress_InflateStreamZip_H

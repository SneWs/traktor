#pragma once

#include "Core/Ref.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace editor
	{

class MemCachedProto;

class MemCachedGetStream : public IStream
{
	T_RTTI_CLASS;

public:
	enum { MaxBlockSize = 65536 };

	MemCachedGetStream(MemCachedProto* proto, const std::string& key);

	bool requestEndBlock();

	bool requestNextBlock();

	virtual void close() override final;

	virtual bool canRead() const override final;

	virtual bool canWrite() const override final;

	virtual bool canSeek() const override final;

	virtual int64_t tell() const override final;

	virtual int64_t available() const override final;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) override final;

	virtual int64_t read(void* block, int64_t nbytes) override final;

	virtual int64_t write(const void* block, int64_t nbytes) override final;

	virtual void flush() override final;

private:
	Ref< MemCachedProto > m_proto;
	std::string m_key;
	uint32_t m_index;
	int32_t m_inblock;
	uint8_t m_block[MaxBlockSize + 2];
};

	}
}


#pragma once

#include "Net/Http/IHttpRequestContent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace net
	{

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS HttpRequestContent : public IHttpRequestContent
{
	T_RTTI_CLASS;

public:
	HttpRequestContent();

	explicit HttpRequestContent(const std::wstring& content);

	explicit HttpRequestContent(IStream* stream);

	void set(const std::wstring& content);

	void set(IStream* stream);

	virtual std::wstring getContentType() const override final;

	virtual uint32_t getContentLength() const override final;

	virtual bool encodeIntoStream(IStream* stream) const override final;

private:
	std::wstring m_contentType;
	AlignedVector< uint8_t > m_content;
};

	}
}


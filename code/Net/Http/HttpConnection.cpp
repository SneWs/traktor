/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Net/Http/HttpConnection.h"
#include "Net/Http/HttpResponse.h"
#include "Net/Http/HttpChunkStream.h"
#if !defined(__PS3__)
#	include "Net/SocketAddressIPv6.h"
#else
#	include "Net/SocketAddressIPv4.h"
#endif
#include "Net/SocketStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/StreamStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpConnection", HttpConnection, UrlConnection)

UrlConnection::EstablishResult HttpConnection::establish(const Url& url, Url* outRedirectionUrl)
{
	if (url.getProtocol() != L"http")
		return ErInvalidUrl;
	
	// Lookup host address.
#if !defined(__PS3__)
	SocketAddressIPv6 addr(url.getHost(), url.getPort());
#else
	SocketAddressIPv4 addr(url.getHost(), url.getPort());
#endif
	if (!addr.valid())
		return ErInvalidUrl;

	// Create and connect socket to host.
	m_socket = new TcpSocket();
	if (!m_socket->connect(addr))
		return ErConnectFailed;

	// Build GET string.
	std::wstring resource = url.getPath();
	
	std::wstring query = url.getQuery();
	if (!query.empty())
		resource += L"?" + query;
	
	std::wstring ref = url.getRef();
	if (!ref.empty())
		resource += L"#" + ref;

	// Create request header.
	AlignedVector< uint8_t > header;
	DynamicMemoryStream dms(header, false, true);
	FileOutputStream fos(&dms, new Utf8Encoding());
	fos << L"GET " << resource << L" HTTP/1.1\r\n";
	if (url.getPort() == 80)
		fos << L"Host: " << url.getHost() << L"\r\n";
	else
		fos << L"Host: " << url.getHost() << L":" << url.getPort() << L"\r\n";
	fos << L"Connection: keep-alive\r\n";
	fos << L"Cache-Control: max-age=0\r\n";
	fos << L"User-Agent: traktor/1.0\r\n";
	fos << L"Accept: */*\r\n";
	fos << L"\r\n";

	Ref< IStream > stream = new SocketStream(m_socket);

	// Send request header.
	if (stream->write(header.c_ptr(), header.size()) != header.size())
	{
		log::error << L"Unable to send HTTP request header." << Endl;
		return ErFailed;
	}

	// Accept and parse response from server.
	HttpResponse response;
	if (!response.parse(stream))
	{
		log::error << L"Invalid HTTP response header" << Endl;
		return ErFailed;
	}
		
	// Ensure it's a positive response.
	if (response.getStatusCode() < 200 || response.getStatusCode() >= 300)
	{
		// Handle redirect response.
		if (response.getStatusCode() == 302 && outRedirectionUrl)
		{
			std::wstring location = response.get(L"Location");
			if (!location.empty())
			{
				*outRedirectionUrl = location;
				return ErRedirect;
			}
		}

		// Unhandled errornous response.
		log::error << L"HTTP error, " << response.getStatusCode() << L", " << response.getStatusMessage() << Endl;
		return ErFailed;
	}
	
	// Reset offset in stream to origin.
	stream->seek(IStream::SeekSet, 0);

	// Create chunked-transfer stream if such encoding is required.
	if (response.get(L"Transfer-Encoding") == L"chunked")
		stream = new HttpChunkStream(stream);

	// If response contains content length field we can cap stream.
	int contentLength = parseString< int >(response.get(L"Content-Length"));
	if (contentLength > 0)
		stream = new StreamStream(stream, contentLength);

	m_stream = stream;
	m_url = url;

	return ErSucceeded;
}

Url HttpConnection::getUrl() const
{
	return m_url;
}

Ref< IStream > HttpConnection::getStream()
{
	return m_stream;
}

	}
}

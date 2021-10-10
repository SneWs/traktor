#include "Avalanche/Dictionary.h"
#include "Avalanche/IBlob.h"
#include "Avalanche/Protocol.h"
#include "Avalanche/Server/Connection.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Thread/ThreadPool.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Connection", Connection, Object)

Connection::Connection(Dictionary* dictionary)
:	m_dictionary(dictionary)
,	m_finished(false)
{
}

Connection::~Connection()
{
	if (m_thread)
	{
		ThreadPool::getInstance().stop(m_thread);
		T_FATAL_ASSERT(m_finished);
	}
}

bool Connection::create(net::TcpSocket* clientSocket)
{
	m_clientSocket = clientSocket;
	m_clientStream = new net::SocketStream(clientSocket, true, true, 5000);

	std::wstring name = L"<unknown>";

	auto remoteAddress = dynamic_type_cast< const net::SocketAddressIPv4* >(clientSocket->getRemoteAddress());
	if (remoteAddress)
		name = remoteAddress->getHostName();

	clientSocket->setQuickAck(true);

	auto fn = [=]()
	{
		log::info << L"Connection with " << name << L" established, ready to process requests." << Endl;
		while (!m_thread->stopped())
		{
			if (!process())
				break;
		}
		log::info << L"Connection with " << name << L" terminated." << Endl;
		m_finished = true;
	};

	if (!ThreadPool::getInstance().spawn(fn, m_thread))
		return false;

	return true;
}

bool Connection::update()
{
	return !m_finished;
}

bool Connection::process()
{
	int32_t result = m_clientSocket->select(true, false, false, 500);
	if (result == 0)
		return true;
	else if (result < 0)
		return false;

	uint8_t cmd = 0;
	if (m_clientStream->read(&cmd, sizeof(uint8_t)) != sizeof(uint8_t))
		return false;

	switch (cmd)
	{
	case c_commandPing:
		{
			if (m_clientStream->write(&c_replyOk, sizeof(uint8_t)) != sizeof(uint8_t))
				return false;
		}
		break;

	case c_commandStat:
		{
			Key key = Key::read(m_clientStream);
			if (!key.valid())
			{
				log::warning << L"Failed to read key; terminating connection." << Endl;
				return false;
			}

			Ref< const IBlob > blob = m_dictionary->get(key);
			if (blob)
			{
				if (m_clientStream->write(&c_replyOk, sizeof(uint8_t)) != sizeof(uint8_t))
					return false;

				int64_t blobSize = blob->size();
				if (m_clientStream->write(&blobSize, sizeof(int64_t)) != sizeof(int64_t))
					return false;
			}
			else
			{
				if (m_clientStream->write(&c_replyFailure, sizeof(uint8_t)) != sizeof(uint8_t))
					return false;
			}
		}
		break;

	case c_commandGet:
		{
			Key key = Key::read(m_clientStream);
			if (!key.valid())
			{
				log::warning << L"Failed to read key; terminating connection." << Endl;
				return false;
			}

			Ref< const IBlob > blob = m_dictionary->get(key);
			if (blob)
			{
				auto readStream = blob->read();
				if (readStream)
				{
					if (m_clientStream->write(&c_replyOk, sizeof(uint8_t)) != sizeof(uint8_t))
						return false;

					int64_t blobSize = blob->size();
					if (m_clientStream->write(&blobSize, sizeof(int64_t)) != sizeof(int64_t))
						return false;

					if (!StreamCopy(m_clientStream, readStream).execute(blob->size()))
					{
						log::error << L"[GET " << key.format() << L"] Unable to send " << blob->size() << L" byte(s) to client; terminating connection." << Endl;
						return false;
					}
					else
						log::info << L"[GET " << key.format() << L"] Sent " << blob->size() << L" bytes successfully." << Endl;
				}
				else
				{
					log::error <<  L"[GET " << key.format() << L"] Unable to acquire read stream from blob." << Endl;
					if (m_clientStream->write(&c_replyFailure, sizeof(uint8_t)) != sizeof(uint8_t))
						return false;
				}
			}
			else
			{
				log::info << L"[GET " << key.format() << L"] No such blob." << Endl;
				if (m_clientStream->write(&c_replyFailure, sizeof(uint8_t)) != sizeof(uint8_t))
					return false;
			}
		}
		break;

	case c_commandPut:
		{
			Key key = Key::read(m_clientStream);
			if (!key.valid())
			{
				log::warning << L"Failed to read key; terminating connection." << Endl;
				return false;
			}

			if (m_dictionary->get(key) != nullptr)
			{
				log::error << L"[PUT " << key.format() << L"] Cannot replace already existing blob." << Endl;
				if (m_clientStream->write(&c_replyFailure, sizeof(uint8_t)) != sizeof(uint8_t))
					return false;

				return true;
			}

			Ref< IBlob > blob = m_dictionary->create();
			if (blob)
			{
				if (m_clientStream->write(&c_replyOk, sizeof(uint8_t)) != sizeof(uint8_t))
					return false;

				for (;;)
				{
					int32_t subcmd = m_clientSocket->recv();
					if (subcmd == c_subCommandPutAppend)
					{
						int64_t chunkSize;
						if (m_clientStream->read(&chunkSize, sizeof(int64_t)) != sizeof(int64_t))
							return false;

						auto appendStream = blob->append();
						if (appendStream)
						{
							if (StreamCopy(appendStream, m_clientStream).execute(chunkSize))
							{
								if (m_clientStream->write(&c_replyOk, sizeof(uint8_t)) != sizeof(uint8_t))
									return false;
							}
							else
							{
								log::error << L"[PUT " << key.format() << L"] Unable to receive " << chunkSize << L" byte(s) from client; terminating connection." << Endl;
								return false;
							}
						}
						else
						{
							log::error << L"[PUT " << key.format() << L"] Failed to append data to blob." << Endl;
							if (m_clientStream->write(&c_replyFailure, sizeof(uint8_t)) != sizeof(uint8_t))
								return false;
						}
					}
					else if (subcmd == c_subCommandPutCommit)
					{
						if (m_dictionary->put(key, blob))
						{
							log::info << L"[PUT " << key.format() << L"] Committed " << blob->size() << L" byte(s) to dictionary successfully." << Endl;
							if (m_clientStream->write(&c_replyOk, sizeof(uint8_t)) != sizeof(uint8_t))
								return false;
						}
						else
						{
							if (m_clientStream->write(&c_replyFailure, sizeof(uint8_t)) != sizeof(uint8_t))
								return false;
						}
						break;
					}
					else if (subcmd == c_subCommandPutDiscard)
					{
						log::info << L"[PUT " << key.format() << L"] Discarded" << Endl;
						if (m_clientStream->write(&c_replyOk, sizeof(uint8_t)) != sizeof(uint8_t))
							return false;
						break;
					}
					else
					{
						if (subcmd >= 0)
							log::error << L"[PUT " << key.format() << L"] Invalid sub-command from client; terminating connection." << Endl;
						return false;
					}
				}
			}
			else
			{
				log::error << L"[PUT " << key.format() << L"] Failed to create blob." << Endl;
				if (m_clientStream->write(&c_replyFailure, sizeof(uint8_t)) != sizeof(uint8_t))
					return false;
			}
		}
		break;

	case c_commandStats:
		{
			Dictionary::Stats stats;
			m_dictionary->getStats(stats);
			if (m_clientStream->write(&stats.blobCount, sizeof(uint32_t)) != sizeof(uint32_t))
				return false;
			if (m_clientStream->write(&stats.memoryUsage, sizeof(uint64_t)) != sizeof(uint64_t))
				return false;
		}
		break;

	default:
		log::error << L"Invalid command from client; terminating connection." << Endl;
		return false;
	}

	return true;
}

	}
}

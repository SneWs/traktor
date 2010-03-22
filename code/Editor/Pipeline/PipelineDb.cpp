#include "Core/Io/StringOutputStream.h"
#include "Editor/Pipeline/PipelineDb.h"
#include "Editor/Pipeline/PipelineDbReport.h"
#include "Sql/IResultSet.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

const int32_t c_version = 1;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDb", PipelineDb, IPipelineDb)

bool PipelineDb::open(const std::wstring& connectionString)
{
	Ref< sql::ConnectionSqlite3 > connection = new sql::ConnectionSqlite3();
	if (!connection->connect(connectionString))
		return false;

	bool haveTable = connection->tableExists(L"PipelineHash");

	// Ensure db latest version.
	if (haveTable)
	{
		haveTable = false;

		if (connection->tableExists(L"Version"))
		{
			Ref< sql::IResultSet > rs;
			rs = connection->executeQuery(L"select * from Version");
			if (rs && rs->next())
			{
				if (rs->getInt32(0) == c_version)
					haveTable = true;
			}
		}

		// Drop all tables if incorrect version.
		if (!haveTable)
		{
			connection->executeUpdate(L"drop table PipelineHash");
			connection->executeUpdate(L"drop table TimeStamps");
			connection->executeUpdate(L"drop table Version");
		}
	}

	// Create tables if they doesn't exist.
	if (!haveTable)
	{
		T_ASSERT_M (!connection->tableExists(L"TimeStamps"), L"TimeStamps table already exists");

		if (connection->executeUpdate(
			L"create table PipelineHash ("
			L"id integer primary key,"
			L"guid char(37) unique,"
			L"pipelineVersion integer,"
			L"dependencyHash integer"
			L")"
		) <= 0)
			return false;

		if (connection->executeUpdate(
			L"create table TimeStamps ("
			L"id integer primary key,"
			L"hashId integer,"
			L"path varchar(1024),"
			L"epoch integer"
			L")"
		) <= 0)
			return false;

		if (connection->executeUpdate(L"create table Version (major integer)") <= 0)
			return false;

		if (connection->executeUpdate(L"insert into Version (major) values (" + toString(c_version) + L")") <= 0)
			return false;
	}

	m_connection = connection;
	return true;
}

void PipelineDb::close()
{
	if (m_connection)
	{
		m_connection->disconnect();
		m_connection = 0;
	}
}

void PipelineDb::set(const Guid& guid, const Hash& hash)
{
	Ref< sql::IResultSet > rs;
	StringOutputStream ss;

	// Remove existing records.
	ss << L"select id from PipelineHash where guid='" << guid.format() << L"'";
	rs = m_connection->executeQuery(ss.str());
	if (rs && rs->next())
	{
		int32_t hashId = rs->getInt32(0);

		ss.reset();
		ss << L"delete from TimeStamps where hashId='" << hashId << L"'";
		m_connection->executeUpdate(ss.str());

		ss.reset();
		ss << L"delete from PipelineHash where id='" << hashId << L"'";
		m_connection->executeUpdate(ss.str());
	}

	// Insert hash.
	ss.reset();
	ss <<
		L"insert into PipelineHash (guid, pipelineVersion, dependencyHash) "
		L"values (" <<
		L"'" << guid.format() << L"'," <<
		hash.pipelineVersion << L"," <<
		hash.dependencyHash <<
		L")";
	m_connection->executeUpdate(ss.str());

	// Insert time stamps.
	if (!hash.timeStamps.empty())
	{
		int32_t hashId = m_connection->lastInsertId();
		for (std::map< Path, DateTime >::const_iterator i = hash.timeStamps.begin(); i != hash.timeStamps.end(); ++i)
		{
			ss.reset();
			ss <<
				L"insert into TimeStamps (hashId, path, epoch) "
				L"values (" <<
				hashId << L"," <<
				L"'" << i->first.getPathName() << L"'," <<
				i->second.getSecondsSinceEpoch() <<
				L")";
			m_connection->executeUpdate(ss.str());
		}
	}
}

bool PipelineDb::get(const Guid& guid, Hash& outHash) const
{
	Ref< sql::IResultSet > rs;
	StringOutputStream ss;

	// Get hash record.
	ss << L"select * from PipelineHash where guid='" << guid.format() << L"'";
	rs = m_connection->executeQuery(ss.str());
	if (!rs || !rs->next())
		return false;

	int32_t id = rs->getInt32(L"id");

	outHash.pipelineVersion = rs->getInt32(L"pipelineVersion");
	outHash.dependencyHash = rs->getInt32(L"dependencyHash");
	
	// Get time stamps.
	ss.reset();
	ss << L"select * from TimeStamps where hashId=" << id;
	rs = m_connection->executeQuery(ss.str());
	if (rs)
	{
		while (rs->next())
		{
			std::wstring path = rs->getString(L"path");
			int64_t epoch = rs->getInt64(L"epoch");
			outHash.timeStamps.insert(std::make_pair(
				path,
				DateTime(epoch)
			));
		}
	}

	return true;
}

Ref< IPipelineReport > PipelineDb::createReport(const std::wstring& name, const Guid& guid)
{
	return new PipelineDbReport(m_connection, L"Report_" + name, guid);
}

	}
}

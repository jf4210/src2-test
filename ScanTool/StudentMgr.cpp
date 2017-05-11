#include "stdafx.h"
#include "StudentMgr.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

CStudentMgr::CStudentMgr() :_session(NULL), _mem(NULL)
{
	Poco::Data::SQLite::Connector::registerConnector();
}


CStudentMgr::~CStudentMgr()
{
	if (_mem)
	{
		_mem->close();
		delete _mem;
		_mem = NULL;
	}
	if (_session)
	{
		_session->close();
		delete _session;
		_session = NULL;
	}
	Poco::Data::SQLite::Connector::unregisterConnector();
}

bool CStudentMgr::InitDB(std::string strPath)
{
	try
	{
		_strDbPath = strPath;
		_session = new Poco::Data::Session(Poco::Data::SQLite::Connector::KEY, strPath);
	}
	catch (Poco::Exception& e)
	{
		std::string strErr = "DB初始化时session创建失败(" + e.displayText() + ")";
		g_pLogger->information(strErr);
	}
	
	return _session != NULL ? true : false;
}

bool CStudentMgr::InitTable(std::string strTableName)
{
	if (_session && !_session->isConnected())
		return false;
#if 1
	try
	{
		std::string strSql1 = "DROP TABLE IF EXISTS " + strTableName;
		std::string strSql2 = Poco::format("CREATE TABLE IF NOT EXISTS %s (zkzh VARCHAR(30), name VARCHAR(60), classRoom VARCHAR(60), school VARCHAR(120), scaned INTEGER(3))", strTableName);
		*_session << strSql1, now;
		*_session << strSql2, now;
	}
	catch (Poco::Exception& e)
	{
		std::string strErr = "InitTable初始化失败(" + e.displayText() + ")";
		g_pLogger->information(strErr);
		return false;
	}
#else
	try
	{
		std::string strSql1 = "DROP TABLE IF EXISTS " + strTableName;
		std::string strSql2 = "CREATE TABLE IF NOT EXISTS student (zkzh VARCHAR(30), name VARCHAR(60), classRoom VARCHAR(60), school VARCHAR(120), scaned INTEGER(3))";
		*_session << strSql1, now;
		*_session << strSql2, now;
	}
	catch (Poco::Exception& e)
	{
		std::string strErr = "InitTable初始化失败(" + e.displayText() + ")";
		g_pLogger->information(strErr);
		return false;
	}
#endif
	return true;
}

bool CStudentMgr::InsertData(STUDENT_LIST& lData, std::string strTable)
{
	bool bResult = false;
	if (_session && !_session->isConnected())
		return false;

	try
	{
		Poco::Stopwatch sw;
		sw.start();

	#if 1
		_mem = new Poco::Data::Session(Poco::Data::SQLite::Connector::KEY, ":memory:");
		Poco::Data::SQLite::Utility::fileToMemory(*_mem, _strDbPath);

		std::string strSql = Poco::format("INSERT INTO %s VALUES(:ln, :fn, :cn, :sn, :status)", strTable);
		Poco::Data::Statement stmt((*_mem << strSql, use(lData)));
		stmt.execute();

		Poco::Data::SQLite::Utility::memoryToFile(_strDbPath, *_mem);
	#else
		std::string strSql = Poco::format("INSERT INTO %s VALUES(:ln, :fn, :cn, :sn, :status)", strTable);
		Poco::Data::Statement stmt((*_session << strSql, use(lData)));
		stmt.execute();
	#endif
		bResult = true;
		sw.stop();
		std::string strLog = Poco::format("插入报名库数据完成[%.6fs]", (double)sw.elapsed()/1000000);
		g_pLogger->information(strLog);
	}
	catch (Poco::Exception& e)
	{
		std::string strErr = "报名库InsertData失败(" + e.displayText() + ")";
		g_pLogger->information(strErr);
		bResult = false;
	}
	return bResult;
}

bool CStudentMgr::SearchStudent(std::string strKey, int nType, STUDENT_LIST& lResult)
{
	bool bResult = false;
	if (_session && !_session->isConnected())
		return false;

	try
	{
		Poco::Stopwatch sw;
		sw.start();
		std::string strTable = "student";
		std::string strSql;
		if (nType == 1)
			strSql = Poco::format("select * from %s where name like '%%%s%%';", strTable, CMyCodeConvert::Gb2312ToUtf8(strKey));
		else
			strSql = Poco::format("select * from %s where zkzh like '%%%s%%';", strTable, CMyCodeConvert::Gb2312ToUtf8(strKey));
		Poco::Data::Statement stmt((*_session << strSql, into(lResult)));
//		Poco::Data::Statement stmt((*_mem << strSql, into(lResult)));
		stmt.execute();
		sw.stop();
		bResult = true;
		std::string strLog = Poco::format("查询报名库数据完成[%.3fms]", (double)sw.elapsed()/1000);
		g_pLogger->information(strLog);
	}
	catch (Poco::Exception& e)
	{
		std::string strErr = "查询报名库失败(" + e.displayText() + ")";
		g_pLogger->information(strErr);
		bResult = false;
	}
	return bResult;
}

bool CStudentMgr::UpdateStudentStatus(std::string strTable, std::string strZkzh, int nStatus)
{
	bool bResult = false;
	if (_session && !_session->isConnected())
		return false;

	try
	{
		std::string strSql;
		strSql = Poco::format("update %s set scaned = %d where zkzh=%s;", strTable, nStatus, CMyCodeConvert::Gb2312ToUtf8(strZkzh));
		Poco::Data::Statement stmt((*_session << strSql, now));
		stmt.execute();
		bResult = true;
		std::string strLog = Poco::format("更新考生(%s)扫描状态%d", strZkzh, nStatus);
		g_pLogger->information(strLog);
	}
	catch (Poco::Exception& e)
	{
		std::string strErr = "更新考生" + strZkzh + "扫描状态失败(" + e.displayText() + ")";
		g_pLogger->information(strErr);
		bResult = false;
	}
	return bResult;
}

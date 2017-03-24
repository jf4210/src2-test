#include "stdafx.h"
#include "StudentMgr.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

CStudentMgr::CStudentMgr() :_session(NULL)
{
	Poco::Data::SQLite::Connector::registerConnector();
}


CStudentMgr::~CStudentMgr()
{
	if (!_session)
		_session->close();
	Poco::Data::SQLite::Connector::unregisterConnector();
}

bool CStudentMgr::InitDB(std::string strPath)
{
	try
	{
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
	if (!_session->isConnected())
		return false;

	try
	{
		std::string strSql1 = "DROP TABLE IF EXISTS " + strTableName;
		std::string strSql2 = "CREATE TABLE IF NOT EXISTS student (zkzh VARCHAR(30), name VARCHAR(60))";
		*_session << strSql1, now;
		*_session << strSql2, now;
	}
	catch (Poco::Exception& e)
	{
		std::string strErr = "InitTable初始化失败(" + e.displayText() + ")";
		g_pLogger->information(strErr);
		return false;
	}
	return true;
}

bool CStudentMgr::InsertData(STUDENT_LIST& lData, std::string strTable)
{
	bool bResult = false;
	if (!_session->isConnected())
		return false;

	try
	{
		Poco::Stopwatch sw;
		sw.start();
		for (auto obj : lData)
		{
			std::string strSql = Poco::format("INSERT INTO %s VALUES(:ln, :fn)", strTable);
			std::string strZkzh = obj.strZkzh;
			std::string strName = CMyCodeConvert::Gb2312ToUtf8(obj.strName);
			*_session << strSql, use(strZkzh), use(strName), now;
		}
		sw.stop();
		std::string strLog = Poco::format("插入报名库数据完成[%uus]", sw.elapsed());
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
	if (!_session->isConnected())
		return false;

	try
	{
// 		Poco::Stopwatch sw;
// 		sw.start();
// 		for (auto obj : lData)
// 		{
// 			std::string strSql = Poco::format("INSERT INTO %s VALUES(:ln, :fn)", strTable);
// 			std::string strZkzh = obj.strZkzh;
// 			std::string strName = CMyCodeConvert::Gb2312ToUtf8(obj.strName);
// 			*_session << strSql, use(strZkzh), use(strName), now;
// 		}
// 		sw.stop();
// 		std::string strLog = Poco::format("插入报名库数据完成[%uus]", sw.elapsed());
// 		g_pLogger->information(strLog);
	}
	catch (Poco::Exception& e)
	{
		std::string strErr = "报名库InsertData失败(" + e.displayText() + ")";
		g_pLogger->information(strErr);
		bResult = false;
	}
	return bResult;
}

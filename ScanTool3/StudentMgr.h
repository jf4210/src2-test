#pragma once
#include "global.h"

class CStudentMgr
{
public:
	CStudentMgr();
	~CStudentMgr();
	
public:
	bool InitDB(std::string strPath);
	bool InitTable(std::string strTableName);
	bool InsertData(STUDENT_LIST& lData, std::string strTable);
	bool InsertMemData(STUDENT_LIST& lData, std::string strTable);
	bool SearchStudent(std::string strTable, std::string strKey, int nType, STUDENT_LIST& lResult);
	bool UpdateStudentStatus(std::string strTable, std::string strZkzh, int nStatus);
	bool GetKCFromZkzh(std::string strTable, std::string strZkzh, std::string& strKC);	//获取考场代码
private:
	std::string strData;
	std::string _strDbPath;
	Poco::Data::Session* _session;
	Poco::Data::Session* _mem;
	Poco::Data::Session* _mem2;
};


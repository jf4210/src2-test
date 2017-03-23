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

private:
	std::string strData;
	Poco::Data::Session* _session;
};


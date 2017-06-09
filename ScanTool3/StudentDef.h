#pragma once
#include <string>

#include "Poco/Data/Statement.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/SQLChannel.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/SQLite/Notifier.h"
#include "Poco/Data/SQLite/Connector.h"

#include "Poco/Nullable.h"
#include "Poco/Data/Transaction.h"
#include "Poco/Data/DataException.h"
#include "Poco/Data/SQLite/SQLiteException.h"
#include "Poco/Data/TypeHandler.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/SQLite/Notifier.h"
#include "Poco/Data/SQLite/Connector.h"


#define		NewBmkTest		//新报名库格式测试

#ifdef NewBmkTest
typedef struct _subjectScanStatus_
{
	int		nSubjectID;
	int		nScaned;
	_subjectScanStatus_()
	{
		nScaned = 0;
		nSubjectID = 0;
	}
}ST_SubjectScanStatus, *pST_SubjectScanStatus;
typedef std::list<ST_SubjectScanStatus> L_SUBJECTSCANSTATUS;

typedef struct _allStudentInfo4Exam_	//针对考试的学生列表信息，包含各个科目信息
{
	int		nExamID;
	std::string strZkzh;
	std::string strName;		//gb2312
	std::string strClassroom;	//gb2312
	std::string strSchool;		//gb2312
	L_SUBJECTSCANSTATUS lSubjectScanStatus;	//科目扫描状态
}ST_ALLSTUDENT, *pST_ALLSTUDENT;

typedef std::list<ST_ALLSTUDENT>		ALLSTUDENT_LIST;	//单个考试中所有科目的报名库列表
typedef std::map<int, ALLSTUDENT_LIST>	EXAMBMK_MAP;		//针对可能下载多个考试的报名库的情况
#endif

typedef struct _studentInfo_
{
	int			nScaned;
	std::string strZkzh;
	std::string strName;		//gb2312
	std::string strClassroom;	//gb2312
	std::string strSchool;		//gb2312
	_studentInfo_()
	{
		nScaned = 0;
	}
}ST_STUDENT, *pST_STUDENT;

class CBmkStudent
{
	CBmkStudent(){}
	CBmkStudent(std::string& strZkzh, std::string& strName, std::string& strClassroom, std::string& strSchool, int nScaned) :_strZkzh(strZkzh), _strName(strName), _strClassroom(strClassroom), _strSchool(strSchool), _nScaned(nScaned){}
	bool operator==(const CBmkStudent& other) const
	{
		return _strZkzh == other._strZkzh && _strName == other._strName && _strClassroom == other._strClassroom && _strSchool == other._strSchool;
	}

	bool operator < (const CBmkStudent& p) const
	{
		if (_strZkzh < p._strZkzh)
			return true;
		if (_strName < p._strName)
			return true;
		if (_strClassroom < p._strClassroom)
			return true;
		if (_strSchool < p._strSchool)
			return true;
		if (_nScaned < p._nScaned)
			return true;
	}

	const std::string& operator () () const
		/// This method is required so we can extract data to a map!
	{
		return _strZkzh;
	}
private:
	int			_nScaned;
	std::string _strZkzh;
	std::string _strName;
	std::string _strClassroom;
	std::string _strSchool;
};


namespace Poco {
	namespace Data {

		template <>
		class TypeHandler<ST_STUDENT>
		{
		public:
			static void bind(std::size_t pos, const ST_STUDENT& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
			{
				// the table is defined as Person (LastName VARCHAR(30), FirstName VARCHAR, Address VARCHAR, Age INTEGER(3))
				poco_assert_dbg(!pBinder.isNull());
				pBinder->bind(pos++, obj.strZkzh, dir);
				pBinder->bind(pos++, CMyCodeConvert::Gb2312ToUtf8(obj.strName), dir);
				pBinder->bind(pos++, CMyCodeConvert::Gb2312ToUtf8(obj.strClassroom), dir);
				pBinder->bind(pos++, CMyCodeConvert::Gb2312ToUtf8(obj.strSchool), dir);
				pBinder->bind(pos++, obj.nScaned, dir);
			}

			static void prepare(std::size_t pos, const ST_STUDENT& obj, AbstractPreparator::Ptr pPrepare)
			{
				// no-op (SQLite is prepare-less connector)
			}

			static std::size_t size()
			{
				return 5;
			}

			static void extract(std::size_t pos, ST_STUDENT& obj, const ST_STUDENT& defVal, AbstractExtractor::Ptr pExt)
			{
				poco_assert_dbg(!pExt.isNull());
				std::string strZkzh;
				std::string strName;
				std::string strClassroom;
				std::string strSchool;
				int			nScaned;

				if (pExt->extract(pos++, strZkzh))
					obj.strZkzh = strZkzh;
				else
					obj.strZkzh = defVal.strZkzh;

				if (pExt->extract(pos++, strName))
					obj.strName = CMyCodeConvert::Utf8ToGb2312(strName);
				else
					obj.strName = defVal.strName;

				if (pExt->extract(pos++, strClassroom))
					obj.strClassroom = CMyCodeConvert::Utf8ToGb2312(strClassroom);
				else
					obj.strClassroom = defVal.strClassroom;

				if (pExt->extract(pos++, strSchool))
					obj.strSchool = CMyCodeConvert::Utf8ToGb2312(strSchool);
				else
					obj.strSchool = defVal.strSchool;

				if (pExt->extract(pos++, nScaned))
					obj.nScaned = nScaned;
				else
					obj.nScaned = defVal.nScaned;
			}

		private:
			TypeHandler();
			~TypeHandler();
			TypeHandler(const TypeHandler&);
			TypeHandler& operator=(const TypeHandler&);
		};
	}
}

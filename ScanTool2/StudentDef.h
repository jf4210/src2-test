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

typedef struct _studentInfo_
{
	std::string strZkzh;
	std::string strName;		//gb2312
	std::string strClassroom;	//gb2312
	std::string strSchool;		//gb2312
}ST_STUDENT, *pST_STUDENT;
class CBmkStudent
{
	CBmkStudent(){}
	CBmkStudent(std::string& strZkzh, std::string& strName, std::string& strClassroom, std::string& strSchool) :_strZkzh(strZkzh), _strName(strName), _strClassroom(strClassroom), _strSchool(strSchool){}
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
	}

	const std::string& operator () () const
		/// This method is required so we can extract data to a map!
	{
		return _strZkzh;
	}
private:
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
			}

			static void prepare(std::size_t pos, const ST_STUDENT& obj, AbstractPreparator::Ptr pPrepare)
			{
				// no-op (SQLite is prepare-less connector)
			}

			static std::size_t size()
			{
				return 4;
			}

			static void extract(std::size_t pos, ST_STUDENT& obj, const ST_STUDENT& defVal, AbstractExtractor::Ptr pExt)
			{
				poco_assert_dbg(!pExt.isNull());
				std::string strZkzh;
				std::string strName;
				std::string strClassroom;
				std::string strSchool;

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
			}

		private:
			TypeHandler();
			~TypeHandler();
			TypeHandler(const TypeHandler&);
			TypeHandler& operator=(const TypeHandler&);
		};
	}
}

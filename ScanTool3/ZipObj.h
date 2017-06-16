#pragma once
#include <string>
#include "zip.h"
#include "unzip.h"
#include "MyCodeConvert.h"
#include "Poco/Exception.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/AutoPtr.h"  
#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"

class CZipObj
{
public:
	CZipObj();
	~CZipObj();

	bool	ZipFile(CString strSrcPath, CString strDstPath, CString strExtName = _T(".zip"));
	bool	UnZipFile(CString strZipPath, CString strUnzipPath = _T(""));
	void	setLogger(Poco::Logger* pLogger);
	void	setUsePwd(bool bUsePwd);
private:
	void	RecordLog(std::string& strLog);
private:
	std::string		_strPwd;
	Poco::Logger*	_pLogger;
	bool			_bUsePwd;
};


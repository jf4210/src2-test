#include "VCSDef.h"



std::string calcFileMd5(std::string strPath)
{
	std::string strResult;
	try
	{
		Poco::MD5Engine md5;
		Poco::DigestOutputStream dos(md5);

		std::string strgb = CMyCodeConvert::Utf8ToGb2312(strPath);

		std::ifstream istr(strgb, std::ios::binary);
		if (!istr)
		{
			string strLog = "calc MD5 failed 1: ";
			strLog.append(strPath);
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			return false;
		}
		Poco::StreamCopier::copyStream(istr, dos);
		dos.close();

		strResult = Poco::DigestEngine::digestToHex(md5.digest());
	}
	catch (...)
	{
		string strLog = "calc MD5 failed 3: ";
		strLog.append(strPath);
		g_Log.LogOutError(strLog);
		std::cout << strLog << std::endl;
		return strResult;
	}
	return strResult;
}

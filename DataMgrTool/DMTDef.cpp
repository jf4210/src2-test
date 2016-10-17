#include "DMTDef.h"




bool encString(std::string& strSrc, std::string& strDst)
{
	bool bResult = true;
	if (_strEncryptPwd_.empty())
	{
		strDst = strSrc;
		return bResult;
	}
	try
	{
		Poco::Crypto::Cipher::Ptr pCipher = Poco::Crypto::CipherFactory::defaultFactory().createCipher(Poco::Crypto::CipherKey("aes256", _strEncryptPwd_));

		strDst = pCipher->encryptString(strSrc, Poco::Crypto::Cipher::ENC_BINHEX);
	}
	catch (...)
	{
		bResult = false;
		std::string strLog = "���ݼ���ʧ�ܣ���ԭ���ݲ���";
		g_Log.LogOut(strLog);
	}
	return bResult;
}

bool decString(std::string& strSrc, std::string& strDst)
{
	bool bResult = true;
	if (_strEncryptPwd_.empty())
	{
		strDst = strSrc;
		return bResult;
	}
	try
	{
		Poco::Crypto::Cipher::Ptr pCipher = Poco::Crypto::CipherFactory::defaultFactory().createCipher(Poco::Crypto::CipherKey("aes256", _strEncryptPwd_));

		strDst = pCipher->decryptString(strSrc, Poco::Crypto::Cipher::ENC_BINHEX);
	}
	catch (...)
	{
		bResult = false;
		std::string strLog = "���ݽ���ʧ�ܣ���ԭ���ݲ���";
		g_Log.LogOut(strLog);
	}
	return bResult;
}



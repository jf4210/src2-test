#include "SysSet.h"
#include "global.h"

CSysSet CSysSet::m_SysSet;
CSysSet::CSysSet(void)
{
}

CSysSet::~CSysSet(void)
{
}

CSysSet& CSysSet::GetRef()
{
	return m_SysSet;
}

bool CSysSet::Load(std::string& strConfPath)
{
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strConfPath));
	m_sLocalIP			= pConf->getString("Net.LocalIP", "127.0.0.1");
	m_sPublicIP			= pConf->getString("Net.PublicIP", m_sLocalIP);
	m_nPaperUpLoadPort	= pConf->getInt("Net.FilePort", 19980);
	m_nCmdPort			= pConf->getInt("Net.CmdPort", 19981);
//	m_strUpLoadPath		= pConf->getString("Sys.upLoadPath");
//	m_strModelSavePath	= pConf->getString("Sys.modelSavePath");
	m_nDecompressThreads = pConf->getInt("Sys.decompressThreads", 2);
	m_nBackupPapers		= pConf->getInt("Sys.bPapersBackup", 0);
	m_strPapersBackupPath	= pConf->getString("Sys.papersBackupPath");
	m_nUpPicData	 = pConf->getInt("Sys.bUpPic", 1);
	m_nUpLoadOmrData = pConf->getInt("Sys.bUpOmrData", 1);
	m_nUpLoadZKZH		= pConf->getInt("Sys.bUpZkzh", 1);
	m_nUpLoadElectOmr	= pConf->getInt("Sys.bUpElectOmr", 1);
	m_nVerServerPort	= pConf->getInt("VerServerInfo.port", 19991);
	m_nServerMode		= pConf->getInt("Sys.serverMode", 0);

	m_nHandleCmdThreads = pConf->getInt("Cmd.sendThreads", 2);
	m_nSendHttpThreads	= pConf->getInt("UpHttp.sendThreads", 2);
	m_nHttpTimeOut		= pConf->getInt("UpHttp.sendTimeout", 60);
	m_nSendTimes		= pConf->getInt("UpHttp.sendTimes", 3);
	m_nIntervalTime		= pConf->getInt("UpHttp.intervalTime", 5);
	m_strUpLoadHttpUri	= pConf->getString("UpHttp.upUri");
	m_strBackUri		= pConf->getString("UpHttp.backUri");
	m_strEncryptPwd		= pConf->getString("Sys.encryptPwd", "yklxTest");
	m_strSessionName	= pConf->getString("Sys.sessionName", "ezs");
	m_strVerServerIP	= pConf->getString("VerServerInfo.addr", "116.211.105.45");
	m_strPicWwwNetAddr	= pConf->getString("UpHttp.picWwwNetAddr", "");
	m_nHeartPacketTime	= pConf->getInt("UpHttp.sessionKeepAlive", 300);
	m_nUseThirdPlatform = pConf->getInt("UpHttp.useThirdPlatform", 0);
	m_str3PlatformUrl	= pConf->getString("ThirdPlatform.url", "");
	m_strLoginYklxPwd	= pConf->getString("ThirdPlatform.loginYklxPwd", "12345678");

	m_nModifyPicOnFail = pConf->getInt("UpHttp.modifyPicUploadFail", 0);

	if (m_str3PlatformUrl.empty())	//若第3方平台的url为空，则不使用第3方登录平台登录
		m_nUseThirdPlatform = 0;

	//++获取不同类型文件上传的地址信息
	std::string strBaseExtName;
	if (m_nServerMode == 1)
		strBaseExtName = ".typkg";
	else
		strBaseExtName = ".pkg";
	std::string strAddr = Poco::format("@@@%s_%s_%d###", strBaseExtName, m_sPublicIP, m_nPaperUpLoadPort);
	m_strFileAddrs = strAddr;

	int nTypes = pConf->getInt("FileAddrs.nTypes", 1);
	for (int i = 1; i <= nTypes; i++)
	{
		std::string strBaseKey = Poco::format("FileType_%d", i);
		string strKey = strBaseKey + ".extName";
		string strExtName = pConf->getString(strKey, ".pkg");
		strKey = strBaseKey + ".ip";
		string strFileIP = pConf->getString(strKey, m_sLocalIP);
		strKey = strBaseKey + ".port";
		int nPort = pConf->getInt(strKey, m_nPaperUpLoadPort);
		std::string strAddr = Poco::format("@@@%s_%s_%d###", strExtName, strFileIP, nPort);
		m_strFileAddrs.append(strAddr);
	}
	//--
	return true;
}

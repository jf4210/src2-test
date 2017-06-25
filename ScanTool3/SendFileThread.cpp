#include "stdafx.h"
#include "SendFileThread.h"

CSendFileThread::CSendFileThread(std::string& strIP, int nPort)
	: _strIp(strIP), _nPort(nPort), m_pUpLoad(NULL)		//,m_upLoad(*this)
{
	g_pLogger->information("CSendFileThread start.");
	TRACE("CSendFileThread start.\n");
}

CSendFileThread::~CSendFileThread()
{
#ifdef TEST_MULTI_SENDER
// 	MAP_FILESENDER::iterator itSender = _mapSender.begin();
// 	for (; itSender != _mapSender.end();)
// 	{
// 		pST_SENDER pObjSender = itSender->second;
// 		CFileUpLoad* pUpLoad = pObjSender->pUpLoad;
// 		itSender = _mapSender.erase(itSender);
// 		SAFE_RELEASE(pUpLoad);
// 		SAFE_RELEASE(pObjSender);
// 	}
#else
	SAFE_RELEASE(m_pUpLoad);
	g_eFileUpLoadThreadExit.wait();
#endif
	g_pLogger->information("CSendFileThread exit.");
	TRACE("CSendFileThread exit.\n");
	g_eSendFileThreadExit.set();
}

void CSendFileThread::run()
{
	USES_CONVERSION;

#ifdef TEST_MULTI_SENDER
// 	std::string strKey = T2A(PAPERS_EXT_NAME);
// 	CFileUpLoad* pUpLoad = NULL;
// 	_fmMapSender_.lock();
// 	MAP_FILESENDER::iterator itSender = _mapSender_.find(strKey);
// 	if (itSender == _mapSender_.end())
// 	{
// 		pST_SENDER pObjSender = new ST_SENDER;
// 		pObjSender->strIP = _strIp;
// 		pObjSender->nPort = _nPort;
// 		pObjSender->pUpLoad = new CFileUpLoad(*this);
// 		_mapSender_.insert(MAP_FILESENDER::value_type(strKey, pObjSender));
// 
// 		pUpLoad = pObjSender->pUpLoad;
// 	}
// 	_fmMapSender_.unlock();
// 	if(pUpLoad)
// 		pUpLoad->InitUpLoadTcp(A2T(_strIp.c_str()), _nPort);
#else
	m_pUpLoad = new CFileUpLoad(*this);
	m_pUpLoad->InitUpLoadTcp(A2T(_strIp.c_str()), _nPort);
#endif
	while (!g_nExitFlag)
	{
	#ifndef TEST_MULTI_SENDER
		if (g_bFileNeedConnect)
		{
			if (_strIp != g_strFileIP || _nPort != g_nFilePort)
			{
				_strIp = g_strFileIP;
				_nPort = g_nFilePort;
				m_pUpLoad->ReConnectAddr(A2T(_strIp.c_str()), _nPort);
			}
		}
	#endif
		pSENDTASK pTask = NULL;
		g_fmSendLock.lock();
#if 1
		SENDTASKLIST::iterator it = g_lSendTask.begin();
		for (; it != g_lSendTask.end();)
		{
			if ((*it)->nSendState == 0)
			{
				(*it)->nSendState = 1;
				pTask = *it;
				break;
			}
			else
			{
				it++;
			}
		}
		g_fmSendLock.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(1000);
			continue;
		}

		HandleTask(pTask);
#else
		SENDTASKLIST::iterator it = g_lSendTask.begin();
		for (; it != g_lSendTask.end();)
		{
			pTask = *it;
			it = g_lSendTask.erase(it);
			break;
		}
		g_fmSendLock.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(1000);
			continue;
		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
#endif
	}
}

void CSendFileThread::HandleTask(pSENDTASK pTask)
{
	USES_CONVERSION;
	
	std::string strUtf8 = CMyCodeConvert::Gb2312ToUtf8(pTask->strPath);
	Poco::File fileSrcPath(strUtf8);
	if (!fileSrcPath.exists())
	{
		char szLog[500] = { 0 };
		sprintf_s(szLog, "发送文件(%s)失败,路径不存在: %s", pTask->strFileName.c_str(), pTask->strPath.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
		return;
	}
		
#ifdef TEST_MULTI_SENDER
	CFileUpLoad* pUpLoad = NULL;
	int nPos = pTask->strFileName.find(".");
	std::string strKey = pTask->strFileName.substr(nPos);


	std::string strPkg = T2A(PAPERS_EXT_NAME);
	std::string strTyPkg = T2A(PAPERS_EXT_NAME_4TY);
	if (strKey != strPkg && strKey != strTyPkg)
		strKey = strPkg;

	_fmMapSender_.lock();
	MAP_FILESENDER::iterator itSender = _mapSender_.find(strKey);
	if (itSender == _mapSender_.end())
	{
// 		std::string strPkg = T2A(PAPERS_EXT_NAME);
// 		std::string strTyPkg = T2A(PAPERS_EXT_NAME_4TY);
//		std::string strMod = ".mod";
		if (strKey == strPkg)	//strKey == strPkg || strKey == strMod
		{
			_strIp = g_strFileIP;
			_nPort = g_nFilePort;
		}
		else if (strKey == strTyPkg && !g_strFileIp4HandModel.empty() && g_nFilePort4HandModel > 0)
		{
			_strIp = g_strFileIp4HandModel;
			_nPort = g_nFilePort4HandModel;
		}
		else
		{
			char szLog[500] = { 0 };
			sprintf_s(szLog, "添加发送文件任务: %s失败(extName: %s, ip: %s, port: %d其中某些有错)", pTask->strFileName.c_str(), strKey.c_str(), g_strFileIp4HandModel.c_str(), g_nFilePort4HandModel);
			g_pLogger->information(szLog);
			return;
		}

		pST_SENDER pObjSender = new ST_SENDER;
		pObjSender->strIP = _strIp;
		pObjSender->nPort = _nPort;
		pObjSender->pUpLoad = new CFileUpLoad(*this);
		pObjSender->pUpLoad->InitUpLoadTcp(A2T(_strIp.c_str()), _nPort);
		_mapSender_.insert(MAP_FILESENDER::value_type(strKey, pObjSender));
		pUpLoad = pObjSender->pUpLoad;
	}
	else
		pUpLoad = itSender->second->pUpLoad;
	_fmMapSender_.unlock();


	char szLog[500] = { 0 };
	sprintf_s(szLog, "添加发送文件任务: %s, path: %s", pTask->strFileName.c_str(), pTask->strPath.c_str());
	g_pLogger->information(szLog);

	if(pUpLoad)
		pUpLoad->SendAnsFile(A2T(pTask->strPath.c_str()), A2T(pTask->strFileName.c_str()), pTask);
#else
	char szLog[500] = { 0 };
	sprintf_s(szLog, "添加发送文件任务: %s, path: %s", pTask->strFileName.c_str(), pTask->strPath.c_str());
	g_pLogger->information(szLog);

	m_pUpLoad->SendAnsFile(A2T(pTask->strPath.c_str()), A2T(pTask->strFileName.c_str()), pTask);
#endif
}

void CSendFileThread::SendFileComplete(char* pName, char* pSrcPath)
{
	char szLog[300] = { 0 };
	sprintf_s(szLog, "发送文件(%s)完成.", pName);
	g_pLogger->information(szLog);

	//移动文件
	std::string strName = pName;
	if (strName.find(".mod") != std::string::npos)
		return;

	std::string strFileNewPath = g_strPaperBackupPath + CMyCodeConvert::Gb2312ToUtf8(pName);
	try
	{
		Poco::File filePapers(CMyCodeConvert::Gb2312ToUtf8(pSrcPath));
		filePapers.moveTo(strFileNewPath);
		std::string strFileName = pName;
		std::string strLog = Poco::format("移动试卷袋(%s)完成", strFileName);
		g_pLogger->information(strLog);
	}
	catch (Poco::Exception& exc)
	{
		std::string strFileName = pName;
		std::string strErrInfo = Poco::format("移动试卷袋(%s)失败,%s", strFileName, exc.message());
		g_pLogger->information(strErrInfo);
	}
}



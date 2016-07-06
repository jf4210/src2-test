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
	SAFE_RELEASE(m_pUpLoad);
	g_eFileUpLoadThreadExit.wait();
	g_pLogger->information("CSendFileThread exit.");
	TRACE("CSendFileThread exit.\n");
	g_eSendFileThreadExit.set();
}

void CSendFileThread::run()
{
	USES_CONVERSION;
	m_pUpLoad = new CFileUpLoad(*this);
	m_pUpLoad->InitUpLoadTcp(A2T(_strIp.c_str()), _nPort);

	while (!g_nExitFlag)
	{
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
#else
		SENDTASKLIST::iterator it = g_lSendTask.begin();
		for (; it != g_lSendTask.end();)
		{
			pTask = *it;
			it = g_lSendTask.erase(it);
			break;
		}
#endif
		g_fmSendLock.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(1000);
			continue;
		}

		HandleTask(pTask);

// 		delete pTask;
// 		pTask = NULL;
	}
}

void CSendFileThread::HandleTask(pSENDTASK pTask)
{
	USES_CONVERSION;
	
	std::string strUtf8 = CMyCodeConvert::Gb2312ToUtf8(pTask->strPath);
	Poco::File fileSrcPath(strUtf8);
	if (!fileSrcPath.exists())
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "发送文件(%s)失败,路径不存在: %s", pTask->strFileName.c_str(), pTask->strPath.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
		return;
	}

	char szLog[300] = { 0 };
	sprintf_s(szLog, "添加发送文件任务: %s, path: %s", pTask->strFileName.c_str(), pTask->strPath.c_str());
	g_pLogger->information(szLog);

//	m_upLoad.SendAnsFile(A2T(pTask->strPath.c_str()), A2T(pTask->strFileName.c_str()));
	m_pUpLoad->SendAnsFile(A2T(pTask->strPath.c_str()), A2T(pTask->strFileName.c_str()), pTask);
}

void CSendFileThread::SendFileComplete(char* pName, char* pSrcPath)
{
	char szLog[300] = { 0 };
	sprintf_s(szLog, "发送文件(%s)完成.", pName);
	g_pLogger->information(szLog);
}



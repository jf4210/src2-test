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
//	m_upLoad.InitUpLoadTcp(A2T(_strIp.c_str()), _nPort);
	m_pUpLoad = new CFileUpLoad(*this);
	m_pUpLoad->InitUpLoadTcp(A2T(_strIp.c_str()), _nPort);

	while (!g_nExitFlag)
	{
		pSENDTASK pTask = NULL;
		g_fmSendLock.lock();
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
			Poco::Thread::sleep(500);
			continue;
		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}
//	m_upLoad.UnInit();
//	m_upLoad.m_bStop = TRUE;
}

void CSendFileThread::HandleTask(pSENDTASK pTask)
{
	USES_CONVERSION;
	
	std::string strUtf8 = CMyCodeConvert::Gb2312ToUtf8(pTask->strPath);
	Poco::File fileSrcPath(strUtf8);
	if (!fileSrcPath.exists())
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "�����ļ�(%s)ʧ��,·��������: %s", pTask->strFileName.c_str(), pTask->strPath.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
		return;
	}

	char szLog[300] = { 0 };
	sprintf_s(szLog, "��ӷ����ļ�����: %s, path: %s", pTask->strFileName.c_str(), pTask->strPath.c_str());
	g_pLogger->information(szLog);

//	m_upLoad.SendAnsFile(A2T(pTask->strPath.c_str()), A2T(pTask->strFileName.c_str()));
	m_pUpLoad->SendAnsFile(A2T(pTask->strPath.c_str()), A2T(pTask->strFileName.c_str()));
}

void CSendFileThread::SendFileComplete(char* pName, char* pSrcPath)
{
	char szLog[300] = { 0 };
	sprintf_s(szLog, "�����ļ�(%s)���.", pName);
	g_pLogger->information(szLog);
}



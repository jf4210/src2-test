#include "stdafx.h"
#include "CompressThread.h"
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"


CCompressThread::CCompressThread(void* pDlg) :m_pDlg(pDlg)
{
}

CCompressThread::~CCompressThread()
{
	g_eCompressThreadExit.set();
}

void CCompressThread::run()
{
	while (!g_nExitFlag)
	{
		pCOMPRESSTASK pTask = NULL;
		g_fmCompressLock.lock();
		COMPRESSTASKLIST::iterator it = g_lCompressTask.begin();
		for (; it != g_lCompressTask.end();)
		{
			pTask = *it;
			it = g_lCompressTask.erase(it);
			break;
		}
		g_fmCompressLock.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(200);
			continue;
		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}
}

void CCompressThread::HandleTask(pCOMPRESSTASK pTask)
{
	std::string strLog = "获得试卷包压缩任务: " + pTask->strSrcFilePath;
	g_Log.LogOut(strLog);

	USES_CONVERSION;
	CString strInfo;
	bool bWarn = false;
	bool bResult = ZipFile(pTask->strSavePath, pTask->strSrcFilePath, pTask->strExtName);
	if (!bResult)
	{
		bWarn = true;
		strInfo.Format(_T("压缩%s失败"), A2T(pTask->strCompressFileName.c_str()));
	}
	else
	{
		strInfo.Format(_T("压缩%s成功"), A2T(pTask->strCompressFileName.c_str()));
		
	}

	static_cast<CDataMgrToolDlg*>(m_pDlg)->showMsg(strInfo);

}

void CCompressThread::setDlg(void * pDlg)
{
	m_pDlg = pDlg;
}


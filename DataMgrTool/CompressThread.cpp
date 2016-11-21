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
	std::string strLog = "����Ծ��ѹ������: " + pTask->strSrcFilePath;
	g_Log.LogOut(strLog);

	USES_CONVERSION;
	CString strInfo;
	bool bWarn = false;
	bool bResult = ZipFile(pTask->strSavePath, pTask->strSrcFilePath, pTask->strExtName);
	if (!bResult)
	{
		bWarn = true;
		strInfo.Format(_T("ѹ��%sʧ��\r\n"), A2T(pTask->strCompressFileName.c_str()));
	}
	else
	{
		strInfo.Format(_T("ѹ��%s�ɹ�\r\n"), A2T(pTask->strCompressFileName.c_str()));
		
		//ɾ��Դ�ļ���
		try
		{
			Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pTask->strSrcFilePath));
			if (srcFileDir.exists())
				srcFileDir.remove(true);
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "ɾ���ļ���(" + pTask->strSrcFilePath + ")ʧ��: " + exc.message();
			g_Log.LogOutError(strErr);
		}
	}

	static_cast<CDataMgrToolDlg*>(m_pDlg)->showMsg(strInfo);
}

void CCompressThread::setDlg(void * pDlg)
{
	m_pDlg = pDlg;
}


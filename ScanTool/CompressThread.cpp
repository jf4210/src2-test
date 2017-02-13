#include "stdafx.h"
#include "CompressThread.h"
#include <algorithm>
#include "ScanTool.h"
#include "ScanToolDlg.h"


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
	g_pLogger->information(strLog);
	
	USES_CONVERSION;
	CString strInfo;
	bool bWarn = false;
	bool bResult = ZipFile(A2T(pTask->strSrcFilePath.c_str()), A2T(pTask->strSavePath.c_str()), A2T(pTask->strExtName.c_str()));
	if (!bResult)
	{
		bWarn = true;
		strInfo.Format(_T("����%sʧ��"), A2T(pTask->strCompressFileName.c_str()));
	}
	else
	{
		strInfo.Format(_T("����%s�ɹ�"), A2T(pTask->strCompressFileName.c_str()));
// 		SAFE_RELEASE(((CScanToolDlg*)m_pDlg)->m_pPapersInfo);
// 		((CScanToolDlg*)m_pDlg)->m_lcPicture.DeleteAllItems();
// 		((CScanToolDlg*)m_pDlg)->m_pCurrentShowPaper = NULL;
	}
	((CScanToolDlg*)m_pDlg)->SetStatusShowInfo(strInfo, bWarn);

	if (bWarn)
	{
		((CScanToolDlg*)m_pDlg)->m_bF2Enable = TRUE;
//		((CScanToolDlg*)m_pDlg)->m_bLastPkgSaveOK = FALSE;
		return;
	}

//	((CScanToolDlg*)m_pDlg)->m_bLastPkgSaveOK = TRUE;

	if (g_nManulUploadFile != 1)
	{
		char szFileFullPath[300] = { 0 };
		sprintf_s(szFileFullPath, "%s%s", pTask->strSavePath.c_str(), pTask->strExtName.c_str());
		pSENDTASK pSendTask = new SENDTASK;
		pSendTask->strFileName = pTask->strCompressFileName;
		pSendTask->strPath = szFileFullPath;
		g_fmSendLock.lock();
		g_lSendTask.push_back(pSendTask);
		g_fmSendLock.unlock();
	}
	((CScanToolDlg*)m_pDlg)->m_bF2Enable = TRUE;

	//Դ�ļ���ɾ��
	if (pTask->bDelSrcDir)
	{
		try
		{
			Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pTask->strSrcFilePath));
			if (srcFileDir.exists())
				srcFileDir.remove(true);

			strLog = "�ļ�[" + pTask->strCompressFileName + "]ѹ����ɣ�Դ�ļ���ɾ���ɹ�";
			g_pLogger->information(strLog);
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "�ļ�[" + pTask->strCompressFileName + "]ѹ����ɣ�ɾ��Դ�ļ���(" + pTask->strSrcFilePath + ")ʧ��: " + exc.message();
			g_pLogger->information(strErr);
		}
	}	
}

void CCompressThread::setDlg(void * pDlg)
{
	m_pDlg = pDlg;
}



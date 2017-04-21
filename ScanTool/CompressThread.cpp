#include "stdafx.h"
#include "CompressThread.h"
#include <algorithm>
#include "ScanTool.h"
#include "ScanToolDlg.h"
#include "PkgToPapers.h"

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
	if (pTask->nCompressType == 1)
	{
		std::string strLog = "����Ծ��ѹ������: " + pTask->strSrcFilePath;
		g_pLogger->information(strLog);

		USES_CONVERSION;
		CString strInfo;
		bool bWarn = false;
	#if 1
		bool bResult = false;
		CZipObj zipObj;
		zipObj.setLogger(g_pLogger);
		bResult = zipObj.ZipFile(A2T(pTask->strSrcFilePath.c_str()), A2T(pTask->strSavePath.c_str()), A2T(pTask->strExtName.c_str()));
	#else
		bool bResult = ZipFile(A2T(pTask->strSrcFilePath.c_str()), A2T(pTask->strSavePath.c_str()), A2T(pTask->strExtName.c_str()));
	#endif
		if (!bResult)
		{
			bWarn = true;
			strInfo.Format(_T("����%sʧ��"), A2T(pTask->strCompressFileName.c_str()));
		}
		else
		{
			strInfo.Format(_T("����%s�ɹ�"), A2T(pTask->strCompressFileName.c_str()));
		}
		((CScanToolDlg*)m_pDlg)->SetStatusShowInfo(strInfo, bWarn);

		if (bWarn)
		{
			((CScanToolDlg*)m_pDlg)->m_bF2Enable = TRUE;
			return;
		}

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
	else
	{
		//��pkg�ָ���Papers
		USES_CONVERSION;
		CString strInfo = _T("");
		strInfo.Format(_T("���ڻָ��Ծ��(%s), ���Ժ�..."), A2T(pTask->strCompressFileName.c_str()));
		((CScanToolDlg*)m_pDlg)->SetStatusShowInfo(strInfo);

		std::string strPkgPath = g_strPaperBackupPath + pTask->strCompressFileName;		//utf8
		CPkgToPapers pkg2PapersObj;
		pPAPERSINFO pPapers = pkg2PapersObj.Pkg2Papers(CMyCodeConvert::Utf8ToGb2312(strPkgPath));
		
		//��Ϣ���͸����߳�
		(static_cast<CScanToolDlg*>(m_pDlg))->SendMessage(MSG_Pkg2Papers_OK, (WPARAM)pPapers, NULL);
	}
}

void CCompressThread::setDlg(void * pDlg)
{
	m_pDlg = pDlg;
}



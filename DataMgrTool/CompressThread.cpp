#include "stdafx.h"
#include "CompressThread.h"
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"


CCompressThread::CCompressThread(void* pDlg) :m_pDlg(pDlg)
{
}

CCompressThread::~CCompressThread()
{
//	g_eCompressThreadExit.set();
}

void CCompressThread::run()
{
	eExit.reset();
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

		_fmCompress_.lock();
		_nCompress_++;
		_fmCompress_.unlock();

		delete pTask;
		pTask = NULL;
	}
	eExit.set();
}

void CCompressThread::HandleTask(pCOMPRESSTASK pTask)
{
	std::string strLog = "»ñµÃÊÔ¾í°üÑ¹ËõÈÎÎñ: " + pTask->strSrcFilePath;
	g_Log.LogOut(strLog);

	USES_CONVERSION;
	CString strInfo;
	bool bWarn = false;
	bool bResult = ZipFile(pTask->strSavePath, pTask->strSrcFilePath, pTask->strExtName);
	if (!bResult)
	{
		bWarn = true;
		strInfo.Format(_T("Ñ¹Ëõ%sÊ§°Ü\r\n"), A2T(pTask->strCompressFileName.c_str()));
	}
	else
	{
		strInfo.Format(_T("Ñ¹Ëõ%s³É¹¦\r\n"), A2T(pTask->strCompressFileName.c_str()));
		
		//É¾³ýÔ´ÎÄ¼þ¼Ð
		try
		{
			Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pTask->strSrcFilePath));
			if (srcFileDir.exists())
				srcFileDir.remove(true);
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "É¾³ýÎÄ¼þ¼Ð(" + pTask->strSrcFilePath + ")Ê§°Ü: " + exc.message();
			g_Log.LogOutError(strErr);
		}

	#if 0
		try
		{
			Poco::File srcPkgFile(CMyCodeConvert::Gb2312ToUtf8(pTask->pPapers->strSrcPapersPath));
			if (srcPkgFile.exists())
				srcPkgFile.remove(true);
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "É¾³ýÔ­Ê¼Ñ¹Ëõ°ü(" + pTask->pPapers->strSrcPapersPath + ")Ê§°Ü: " + exc.message();
			g_Log.LogOutError(strErr);
		}
	#endif
	}

	g_fmPapers.lock();			//ÊÍ·ÅÊÔ¾í´üÁÐ±í
	PAPERS_LIST::iterator itPapers = g_lPapers.begin();
	for (; itPapers != g_lPapers.end(); itPapers++)
	{
		pPAPERSINFO pPapersTask = *itPapers;
		if (pPapersTask == pTask->pPapers)
		{
			itPapers = g_lPapers.erase(itPapers);
			SAFE_RELEASE(pPapersTask);
			break;
		}
	}
	g_fmPapers.unlock();

	static_cast<CDataMgrToolDlg*>(m_pDlg)->showMsg(strInfo);
}

void CCompressThread::setDlg(void * pDlg)
{
	m_pDlg = pDlg;
}


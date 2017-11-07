#include "stdafx.h"
#include "CompressThread.h"
#include <algorithm>
#include "ScanTool3.h"
#include "ScanMgrDlg.h"
//#include "PkgToPapers.h"

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
		std::string strLog = "获得试卷包压缩任务: " + pTask->strSrcFilePath;
		g_pLogger->information(strLog);

		USES_CONVERSION;
		CString strInfo;
		bool bWarn = false;
	#if 1
		bool bResult = false;
		CZipObj zipObj;
		zipObj.setLogger(g_pLogger);
		if (pTask->strSrcFilePath.find("_UnPwd") != std::string::npos)		//手阅试卷袋以"_ToCompress_UnPwd"结尾
			zipObj.setUsePwd(false);

		bResult = zipObj.ZipFile(A2T(pTask->strSrcFilePath.c_str()), A2T(pTask->strSavePath.c_str()), A2T(pTask->strExtName.c_str()));
	#else
		bool bResult = ZipFile(A2T(pTask->strSrcFilePath.c_str()), A2T(pTask->strSavePath.c_str()), A2T(pTask->strExtName.c_str()));
	#endif
		if (!bResult)
		{
			bWarn = true;
			strInfo.Format(_T("保存%s失败"), A2T(pTask->strCompressFileName.c_str()));
		}
		else
		{
			strInfo.Format(_T("保存%s成功"), A2T(pTask->strCompressFileName.c_str()));
		}
		((CScanMgrDlg*)m_pDlg)->SendMessage(MSG_Compress_Done, (WPARAM)bWarn, (LPARAM)(LPCTSTR)strInfo);

		if (bWarn)
		{
//			((CScanToolDlg*)m_pDlg)->m_bF2Enable = TRUE;
			return;
		}

		std::string strDelDir;
		if (pTask->bDelSrcDir)
		{
			try
			{
				//删除会消耗较长的时间，此时关闭软件，会造成重启后重新压缩，但是内部文件是缺少不全的，
				//所以先将此文件夹重命名，防止重启后再次压缩
				int nPos = pTask->strSrcFilePath.find("_ToCompress");
				strDelDir = pTask->strSrcFilePath.substr(0, nPos);
				strDelDir.append("_NeedToDel");

				Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pTask->strSrcFilePath));
				srcFileDir.renameTo(CMyCodeConvert::Gb2312ToUtf8(strDelDir));

				if (bResult)
					strLog = "文件[" + pTask->strCompressFileName + "]压缩完成，源文件夹重命名" + strDelDir + "成功";
				else
					strLog = "文件[" + pTask->strCompressFileName + "]压缩失败，源文件夹重命名" + strDelDir + "成功";
				g_pLogger->information(strLog);
			}
			catch (Poco::Exception& exc)
			{
				strDelDir = pTask->strSrcFilePath;
				std::string strErr = "文件[" + pTask->strCompressFileName + "]压缩完成，重命名源文件夹(" + pTask->strSrcFilePath + ")失败: " + exc.message();
				g_pLogger->information(strErr);
			}
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
//		((CScanToolDlg*)m_pDlg)->m_bF2Enable = TRUE;

		//源文件夹删除
		if (pTask->bDelSrcDir)
		{
			try
			{
				Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(strDelDir));	//pTask->strSrcFilePath
				if (srcFileDir.exists())
					srcFileDir.remove(true);

				if (bResult)
					strLog = "文件[" + pTask->strCompressFileName + "]压缩完成，源文件夹删除成功";
				else
					strLog = "文件[" + pTask->strCompressFileName + "]压缩失败，源文件夹删除成功";
				g_pLogger->information(strLog);
			}
			catch (Poco::Exception& exc)
			{
				std::string strErr = "文件[" + pTask->strCompressFileName + "]压缩完成，删除源文件夹(" + pTask->strSrcFilePath + ")失败: " + exc.message();
				g_pLogger->information(strErr);
			}
		}
	}
	else
	{
#if 0
		//从pkg恢复到Papers
		USES_CONVERSION;
		CString strInfo = _T("");
		strInfo.Format(_T("正在恢复试卷包(%s), 请稍后..."), A2T(pTask->strCompressFileName.c_str()));
		((CScanToolDlg*)m_pDlg)->SetStatusShowInfo(strInfo);

		std::string strPkgPath = g_strPaperBackupPath + pTask->strCompressFileName;		//utf8
		CPkgToPapers pkg2PapersObj;
		pPAPERSINFO pPapers = pkg2PapersObj.Pkg2Papers(CMyCodeConvert::Utf8ToGb2312(strPkgPath));
		
		//消息发送给主线程
		(static_cast<CScanToolDlg*>(m_pDlg))->SendMessage(MSG_Pkg2Papers_OK, (WPARAM)pPapers, NULL);
#endif
	}
}

void CCompressThread::setDlg(void * pDlg)
{
	m_pDlg = pDlg;
}



#include "stdafx.h"
#include "SearchThread.h"
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"


CSearchThread::CSearchThread(void* pDlg)
: m_pDlg(pDlg)
{
}


CSearchThread::~CSearchThread()
{
}

void CSearchThread::run()
{
//	_E_StartSearch_.wait();
	
	while (!g_nExitFlag)
	{
		pST_SEARCH pTask = NULL;
		_fmSearchPathList_.lock();
		L_SearchTask::iterator it = _SearchPathList_.begin();
		for (; it != _SearchPathList_.end();)
		{
			pTask = *it;
			it = _SearchPathList_.erase(it);
			break;
		}
		_fmSearchPathList_.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(2000);
			continue;
		}

		HandleTask(pTask);

		if (_SearchPathList_.size() == 0)
		{
			USES_CONVERSION;
			CString strMsg;
			if (pTask->nSearchType == 1)
			{
				char szCompare[30] = { 0 };
				sprintf_s(szCompare, "_%d-%d_", _pModel_->nExamID, _pModel_->nSubjectID);

				strMsg.Format(_T("%s模板对应的试卷包搜索完成\r\n"), A2T(szCompare));
			}
			else
			{
				strMsg.Format(_T("试卷袋%s搜索完成\r\n"), A2T(pTask->strSearchName.c_str()));
			}
			
			((CDataMgrToolDlg*)m_pDlg)->showMsg(strMsg);
			TRACE("%s\n", strMsg);
		}
		
		delete pTask;
		pTask = NULL;
	}
	
}

void CSearchThread::HandleTask(pST_SEARCH pTask)
{
	try
	{
//		std::string strPkgPath = CMyCodeConvert::Gb2312ToUtf8(_strPkgSearchPath_);
		std::string strDecompressDir = CMyCodeConvert::Utf8ToGb2312(pTask->strSearchPath) + "\\tmpDecompress";
		TRACE("搜索目录: %s\n", CMyCodeConvert::Utf8ToGb2312(pTask->strSearchPath).c_str());

		char szCompare[20] = { 0 };
		if (pTask->nSearchType == 1)
			sprintf_s(szCompare, "%d-%d", _pModel_->nExamID, _pModel_->nSubjectID);

		Poco::DirectoryIterator it(pTask->strSearchPath);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			std::string  strPath = CMyCodeConvert::Utf8ToGb2312(p.toString());
			std::string strFileName = p.getFileName();
			std::string strBaseFileName = p.getBaseName();
			std::string strExtion = p.getExtension();
			if (it->isFile() && p.getExtension() == "pkg")
			{
				if (pTask->nSearchType == 1 && strFileName.find(szCompare) != std::string::npos)
				{
					pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
					pDecompressTask->nTaskType = 3;
					pDecompressTask->strFilePath = CMyCodeConvert::Utf8ToGb2312(p.toString());
					pDecompressTask->strFileBaseName = CMyCodeConvert::Utf8ToGb2312(p.getBaseName());
					pDecompressTask->strSrcFileName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());
					pDecompressTask->strDecompressDir = strDecompressDir;
					pDecompressTask->bRecogOmr = pTask->bRecogOmr;
					pDecompressTask->bRecogZkzh = pTask->bRecogZkzh;
					pDecompressTask->bRecogElectOmr = pTask->bRecogElectOmr;
					pDecompressTask->nSendEzs = pTask->nSendEzs;
					pDecompressTask->nNoNeedRecogVal = pTask->nNoNeedRecogVal;


					_fmDecompress_.lock();
					_nDecompress_++;
					_fmDecompress_.unlock();

					g_fmDecompressLock.lock();
					g_lDecompressTask.push_back(pDecompressTask);
					g_fmDecompressLock.unlock();
				}
				else if (pTask->nSearchType == 2 && pTask->strSearchName == strBaseFileName)
				{
					pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
					pDecompressTask->nTaskType = 6;
					pDecompressTask->strFilePath = CMyCodeConvert::Utf8ToGb2312(p.toString());
					pDecompressTask->strFileBaseName = CMyCodeConvert::Utf8ToGb2312(p.getBaseName());
					pDecompressTask->strSrcFileName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());
					pDecompressTask->strDecompressPaperFile = pTask->strPaperName;
					pDecompressTask->strDecompressDir = pTask->strDecompressPath;		//解压到的路径

					g_fmDecompressLock.lock();
					g_lDecompressTask.push_back(pDecompressTask);
					g_fmDecompressLock.unlock();
					break;
				}
			}
			else if (it->isDirectory())
			{
				_fmSearchPathList_.lock();
				pST_SEARCH pDirTask = new ST_SEARCH;
				if (pTask->nSearchType == 2)
				{
					pDirTask->nSearchType = 2;
					pDirTask->strSearchPath = p.toString();
					pDirTask->strSearchName = pTask->strSearchName;
					pDirTask->strPaperName = pTask->strPaperName;
					pDirTask->strDecompressPath = pTask->strDecompressPath;
				}
				else
				{					
					pDirTask->strSearchPath = p.toString();
					pDirTask->bRecogOmr = pTask->bRecogOmr;
					pDirTask->bRecogZkzh = pTask->bRecogZkzh;
					pDirTask->bRecogElectOmr = pTask->bRecogElectOmr;
					pDirTask->nSendEzs = pTask->nSendEzs;
					pDirTask->nNoNeedRecogVal = pTask->nNoNeedRecogVal;
				}				

				_SearchPathList_.push_back(pDirTask);
				_fmSearchPathList_.unlock();
			}
			it++;
		}
	}
	catch (Poco::Exception& exc)
	{
	}

}

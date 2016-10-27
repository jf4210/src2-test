
#include <Poco/Util/ServerApplication.h>
#include "ExamServerMgr.h"
#include "DCSDef.h"
#include "NetOperatorDll.h"
#include "DecompressThread.h"
#include "SendToHttpThread.h"
#include "ScanResquestHandler.h"
#include "crash_dumper_w32.h"


int		g_nExitFlag = 0;
CLog	g_Log;
Poco::FastMutex			g_fmDecompressLock;		//解压文件列表锁
DECOMPRESSTASKLIST		g_lDecompressTask;		//解压文件列表

Poco::FastMutex			g_fmPapers;
LIST_PAPERS_DETAIL		g_lPapers;				//试卷袋列表

Poco::FastMutex			g_fmHttpSend;
LIST_SEND_HTTP			g_lHttpSend;			//发送HTTP任务列表

Poco::FastMutex			g_fmScanReq;
LIST_SCAN_REQ			g_lScanReq;		//扫描端请求任务列表

Poco::FastMutex			_mapUserLock_;
MAP_USER				_mapUser_;					//用户映射

Poco::FastMutex	_mapModelLock_;
MAP_MODEL	_mapModel_;				//模板信息

Poco::FastMutex _mapPicAddrLock_;
MAP_PIC_ADDR	_mapPicAddr_;

class DCS : public Poco::Util::ServerApplication
{
protected:
	void initialize(Poco::Util::Application& self)
	{
		Application::ArgVec argv = self.argv();
		Poco::Path path(argv[0]);
//		chdir(path.parent().toString().c_str());
//		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}
	
	void  InitModelInfo()
	{
		std::string strModelPath = CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strModelSavePath);

		try
		{
			Poco::DirectoryIterator it(strModelPath);
			Poco::DirectoryIterator end;
			while (it != end)
			{
				Poco::Path p(it->path());
				if (it->isFile() && p.getExtension() == "mod")
				{
					std::string strName = p.getFileName();
					std::string strPath = p.toString();
					//在_mapModel_中把本地文件信息插入

					std::string strModelName = strName;
					int nPos = 0;
					int nOldPos = 0;
					nPos = strModelName.find("_");
					std::string strExamID = strModelName.substr(0, nPos);
					nOldPos = nPos;
					nPos = strModelName.find(".", nPos + 1);
					std::string strSubjectID = strModelName.substr(nOldPos + 1, nPos - nOldPos - 1);

					char szIndex[50] = { 0 };
					sprintf(szIndex, "%s_%s", strExamID.c_str(), strSubjectID.c_str());

					pMODELINFO pModelInfo = new MODELINFO;
					pModelInfo->nExamID = atoi(strExamID.c_str());
					pModelInfo->nSubjectID = atoi(strSubjectID.c_str());
					pModelInfo->strName = strName;
					pModelInfo->strPath = strPath;
					pModelInfo->strMd5 = calcFileMd5(strPath);

					_mapModelLock_.lock();
					_mapModel_.insert(MAP_MODEL::value_type(szIndex, pModelInfo));
					_mapModelLock_.unlock();
				}
				++it;
			}
		}
		catch (Poco::FileException& exc)
		{
			std::cerr << exc.displayText() << std::endl;
			return ;
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << exc.displayText() << std::endl;
			return ;
		}
	}

	//启动的时候，检测本地文件接收文件夹，将其中所有文件都加入压缩列表，目的：如果有异常退出，已接收文件还需要继续处理
	void  InitPapersList()
	{
		bool bFind = false;
		std::string strLog = "添加上次关闭未处理完成的试卷袋列表:";
		std::string strPapersPath = CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strUpLoadPath);
		Poco::DirectoryIterator it(strPapersPath);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			if (it->isFile())
			{
				if (p.getExtension() == "pkg")
				{
					bFind = true;
					pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
					pDecompressTask->strFilePath = CMyCodeConvert::Utf8ToGb2312(p.toString());
					pDecompressTask->strFileBaseName = CMyCodeConvert::Utf8ToGb2312(p.getBaseName());
					pDecompressTask->strSrcFileName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());

					strLog.append(pDecompressTask->strFileBaseName + " ");
					g_fmDecompressLock.lock();
					g_lDecompressTask.push_back(pDecompressTask);
					g_fmDecompressLock.unlock();
				}
			}
			it++;
		}
		if (bFind)
		{
			g_Log.LogOut(strLog);
			std::cout << strLog << std::endl;
		}		
	}

	int main(const std::vector < std::string > & args) 
	{
		Poco::Net::HTTPStreamFactory::registerFactory();
		
		std::string strCurrentPath = config().getString("application.dir");
		std::string strLogPath = strCurrentPath + "DCS.Log";
		std::string strDllLogPath = CMyCodeConvert::Utf8ToGb2312(strCurrentPath) + "DCS_Dll.Log";
		std::string strConfigPath = strCurrentPath + "DCS-config.ini";
				
		g_Log.SetFileName(strLogPath);
		SetLogFileName((char*)strDllLogPath.c_str());
		SysSet.Load(strConfigPath);
		SysSet.m_strCurrentDir = strCurrentPath;
		SysSet.m_strDecompressPath = strCurrentPath + "DecompressDir";
		SysSet.m_strUpLoadPath = CMyCodeConvert::Utf8ToGb2312(strCurrentPath) + "fileDispose";
		SysSet.m_strModelSavePath = CMyCodeConvert::Utf8ToGb2312(strCurrentPath) + "modelSave";
		SysSet.m_strRecvFilePath = CMyCodeConvert::Utf8ToGb2312(strCurrentPath + "tmpFileRecv\\");
		SysSet.m_strErrorPkg = CMyCodeConvert::Utf8ToGb2312(strCurrentPath + "errorPkg\\");
		
#ifdef POCO_OS_FAMILY_WINDOWS
		char szTitle[150] = { 0 };
		sprintf(szTitle, "%s <%s:%d - %d>", SOFT_VERSION, SysSet.m_sLocalIP.c_str(), SysSet.m_nCmdPort, SysSet.m_nPaperUpLoadPort);
		std::wstring wstrTitle;
		Poco::UnicodeConverter::toUTF16(szTitle, wstrTitle);
		SetConsoleTitle(wstrTitle.c_str());
#endif

		try
		{
			Poco::File decompressDir(SysSet.m_strDecompressPath);
			if (decompressDir.exists())
				decompressDir.remove(true);
			decompressDir.createDirectories();

			Poco::File fileRecvDir(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strUpLoadPath));
			if (!fileRecvDir.exists())
				fileRecvDir.createDirectories();

			Poco::File modelSaveDir(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strModelSavePath));
			if (!modelSaveDir.exists())
				modelSaveDir.createDirectories();

			Poco::File papersBackupDir(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strPapersBackupPath));
			if (!papersBackupDir.exists())
				papersBackupDir.createDirectories();

			Poco::File recvFileDir(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strRecvFilePath));
			if (recvFileDir.exists())
				recvFileDir.remove(true);

			recvFileDir.createDirectories();

			Poco::File errorPkgDir(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strErrorPkg));
			if (!errorPkgDir.exists())
				errorPkgDir.createDirectories();
		}
		catch (Poco::Exception& exc)
		{
			std::string strErrInfo;
			strErrInfo.append("**** 文件夹初始化创建失败，请检查路径设置是否正确！****");
			strErrInfo.append(exc.message());
			g_Log.LogOutError(strErrInfo);
			std::cout << strErrInfo << std::endl;
			waitForTerminationRequest();
			return 0;
		}

		InitModelInfo();
		InitPapersList();

		std::vector<CDecompressThread*> vecDecompressThreadObj;
		Poco::Thread* pDecompressThread = new Poco::Thread[SysSet.m_nDecompressThreads];
		for (int i = 0; i < SysSet.m_nDecompressThreads; i++)
		{
			CDecompressThread* pObj = new CDecompressThread;
			pDecompressThread[i].start(*pObj);
			vecDecompressThreadObj.push_back(pObj);
		}

		std::vector<CSendToHttpThread*> vecSendHttpThreadObj;
		Poco::Thread* pSendHttpThread = new Poco::Thread[SysSet.m_nSendHttpThreads];
		for (int i = 0; i < SysSet.m_nSendHttpThreads; i++)
		{
			CSendToHttpThread* pObj = new CSendToHttpThread;
			pSendHttpThread[i].start(*pObj);
			vecSendHttpThreadObj.push_back(pObj);
		}

// 		CScanResquestHandler scanReqHandler;
// 		Poco::Thread scanReqThread;
// 		scanReqThread.start(scanReqHandler);
		std::vector<CScanResquestHandler*> vecHandleCmdThreadObj;
		Poco::Thread* pHandleCmdThread = new Poco::Thread[SysSet.m_nHandleCmdThreads];
		for (int i = 0; i < SysSet.m_nHandleCmdThreads; i++)
		{
			CScanResquestHandler* pObj = new CScanResquestHandler;
			pHandleCmdThread[i].start(*pObj);
			vecHandleCmdThreadObj.push_back(pObj);
		}

		CExamServerMgr examServerMgr;
		if (examServerMgr.StartFileChannel())
			g_Log.LogOut("StartFileChannel success.");
		else
			g_Log.LogOut("StartFileChannel fail.");

		if (examServerMgr.StartCmdChannel())
			g_Log.LogOut("StartCmdChannel success.");
		else
			g_Log.LogOut("StartCmdChannel fail.");


#if 0	//test
		char szIndex[50] = { 0 };
		strcpy(szIndex, "1_1");
		pMODELINFO pModelInfo = NULL;
		MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
		if (itFind == _mapModel_.end())		//服务器上没有模板，请求后端提供数据生成模板
		{
			pModelInfo = new MODELINFO;
			pModelInfo->nExamID = 1;
			pModelInfo->nSubjectID = 1;

			_mapModelLock_.lock();
			_mapModel_.insert(MAP_MODEL::value_type(szIndex, pModelInfo));
			_mapModelLock_.unlock();

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = Poco::format("%s/sheet/data/1/1", SysSet.m_strBackUri);
//			pTask->pUser = pUser;
			pTask->strMsg = "createModel";
			pTask->nExamID = 1;
			pTask->nSubjectID = 1;

			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
		}
#endif
		
		waitForTerminationRequest();
		g_nExitFlag = 1;
		examServerMgr.StopFileChannel();
		examServerMgr.StopCmdChannel();

		//释放模板映射信息
		_mapModelLock_.lock();
		MAP_MODEL::iterator itModel = _mapModel_.begin();
		for (; itModel != _mapModel_.end();)
		{
			pMODELINFO pModel = itModel->second;
			itModel = _mapModel_.erase(itModel);
			SAFE_RELEASE(pModel);
		}
		_mapModelLock_.unlock();

		//释放扫描端请求命令列表
		g_fmScanReq.lock();
		LIST_SCAN_REQ::iterator itScanReq = g_lScanReq.begin();
		for (; itScanReq != g_lScanReq.end();)
		{
			pSCAN_REQ_TASK pTask = *itScanReq;
			SAFE_RELEASE(pTask);
			itScanReq = g_lScanReq.erase(itScanReq);
		}
		g_fmScanReq.unlock();

		//释放试卷袋信息列表
		g_fmPapers.lock();
		LIST_PAPERS_DETAIL::iterator itPapers = g_lPapers.begin();
		for (; itPapers != g_lPapers.end();)
		{
			pPAPERS_DETAIL pTask = *itPapers;
			SAFE_RELEASE(pTask);
			itPapers = g_lPapers.erase(itPapers);
		}
		g_fmPapers.unlock();

		//释放文件解压列表
		g_fmDecompressLock.lock();
		DECOMPRESSTASKLIST::iterator itDecomp = g_lDecompressTask.begin();
		for (; itDecomp != g_lDecompressTask.end();)
		{
			pDECOMPRESSTASK pTask = *itDecomp;
			SAFE_RELEASE(pTask);
			itDecomp = g_lDecompressTask.erase(itDecomp);
		}
		g_fmDecompressLock.unlock();

		//释放http请求列表
		g_fmHttpSend.lock();
		LIST_SEND_HTTP::iterator itHttp = g_lHttpSend.begin();
		for (; itHttp != g_lHttpSend.end();)
		{
			pSEND_HTTP_TASK pTask = *itHttp;
			SAFE_RELEASE(pTask);
			itHttp = g_lHttpSend.erase(itHttp);
		}
		g_fmHttpSend.unlock();

		std::vector<CDecompressThread*>::iterator itDecObj = vecDecompressThreadObj.begin();
		for (; itDecObj != vecDecompressThreadObj.end();)
		{
			CDecompressThread* pObj = *itDecObj;
			if (pObj)
			{
				delete pObj;
				pObj = NULL;
			}
			itDecObj = vecDecompressThreadObj.erase(itDecObj);
		}

		for (int i = 0; i < SysSet.m_nDecompressThreads; i++)
		{
			pDecompressThread[i].join();
		}
		delete[] pDecompressThread;

		std::vector<CSendToHttpThread*>::iterator itSendHttpObj = vecSendHttpThreadObj.begin();
		for (; itSendHttpObj != vecSendHttpThreadObj.end();)
		{
			CSendToHttpThread* pObj = *itSendHttpObj;
			if (pObj)
			{
				delete pObj;
				pObj = NULL;
			}
			itSendHttpObj = vecSendHttpThreadObj.erase(itSendHttpObj);
		}

		for (int i = 0; i < SysSet.m_nSendHttpThreads; i++)
		{
			pSendHttpThread[i].join();
		}
		delete[] pSendHttpThread;


//		scanReqThread.join();
		std::vector<CScanResquestHandler*>::iterator itHandleCmdObj = vecHandleCmdThreadObj.begin();
		for (; itHandleCmdObj != vecHandleCmdThreadObj.end();)
		{
			CScanResquestHandler* pObj = *itHandleCmdObj;
			if (pObj)
			{
				delete pObj;
				pObj = NULL;
			}
			itHandleCmdObj = vecHandleCmdThreadObj.erase(itHandleCmdObj);
		}

		for (int i = 0; i < SysSet.m_nHandleCmdThreads; i++)
		{
			pHandleCmdThread[i].join();
		}
		delete[] pHandleCmdThread;

		g_Log.LogOut("StopFileChannel complete.");
		g_Log.LogOut("StopCmdChannel complete.");
		return 0;
	}
};

POCO_SERVER_MAIN(DCS);

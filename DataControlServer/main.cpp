
#include <Poco/Util/ServerApplication.h>
#include "ExamServerMgr.h"
#include "DCSDef.h"
#include "NetOperatorDll.h"
#include "DecompressThread.h"
#include "SendToHttpThread.h"
#include "ScanResquestHandler.h"

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

	std::string calcFileMd5(std::string strPath)
	{
		std::string strResult;
		try
		{
			Poco::MD5Engine md5;
			Poco::DigestOutputStream dos(md5);

			std::ifstream istr(strPath, std::ios::binary);
			if (!istr)
			{
				string strLog = "calc MD5 failed 1: ";
				strLog.append(strPath);
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
				return false;
			}
			Poco::StreamCopier::copyStream(istr, dos);
			dos.close();

			strResult = Poco::DigestEngine::digestToHex(md5.digest());
			
		}
		catch (...)
		{
			string strLog = "calc MD5 failed 3: ";
			strLog.append(strPath);
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			return strResult;
		}
		return strResult;
	}

	void  InitModelInfo()
	{
		std::string strModelPath = SysSet.m_strModelSavePath;

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

					//在_mapModel_中把本地文件信息插入
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

	int main(const std::vector < std::string > & args) 
	{
		std::string strCurrentPath = config().getString("application.dir");
		std::string strLogPath = strCurrentPath + "DCS.Log";
		std::string strDllLogPath = CMyCodeConvert::Utf8ToGb2312(strCurrentPath) + "DCS_Dll.Log";
		std::string strConfigPath = strCurrentPath + "DCS-config.ini";
		
		g_Log.SetFileName(strLogPath);
		SetLogFileName((char*)strDllLogPath.c_str());
		SysSet.Load(strConfigPath);
		SysSet.m_strCurrentDir = strCurrentPath;
		SysSet.m_strDecompressPath = strCurrentPath + "DecompressDir";
		
		Poco::File decompressDir(SysSet.m_strDecompressPath);
		if (!decompressDir.exists())
			decompressDir.createDirectories();

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
		for (int i = 0; i < SysSet.m_nDecompressThreads; i++)
		{
			CSendToHttpThread* pObj = new CSendToHttpThread;
			pSendHttpThread[i].start(*pObj);
			vecSendHttpThreadObj.push_back(pObj);
		}

		CScanResquestHandler scanReqHandler;
		Poco::Thread scanReqThread;
		scanReqThread.start(scanReqHandler);

		CExamServerMgr examServerMgr;
		if (examServerMgr.StartFileChannel())
			g_Log.LogOut("StartFileChannel success.");
		else
			g_Log.LogOut("StartFileChannel fail.");

		if (examServerMgr.StartCmdChannel())
			g_Log.LogOut("StartCmdChannel success.");
		else
			g_Log.LogOut("StartCmdChannel fail.");


		//test
// 		pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
// 		pTask->strUri = SysSet.m_strScanReqUri;
// 		pTask->strUserName = "123";
// 		pTask->strPwd = "123";
// 		pTask->strMsg = "login";
// 		pTask->strRequest = "username=123&password=123";
// 		g_fmScanReq.lock();
// 		g_lScanReq.push_back(pTask);
// 		g_fmScanReq.unlock();
		//--
		waitForTerminationRequest();
		g_nExitFlag = 1;
		examServerMgr.StopFileChannel();
		examServerMgr.StopCmdChannel();

		g_fmScanReq.lock();
		LIST_SCAN_REQ::iterator itScanReq = g_lScanReq.begin();
		for (; itScanReq != g_lScanReq.end();)
		{
			pSCAN_REQ_TASK pTask = *itScanReq;
			SAFE_RELEASE(pTask);
			itScanReq = g_lScanReq.erase(itScanReq);
		}
		g_fmScanReq.unlock();

		g_fmPapers.lock();
		LIST_PAPERS_DETAIL::iterator itPapers = g_lPapers.begin();
		for (; itPapers != g_lPapers.end();)
		{
			pPAPERS_DETAIL pTask = *itPapers;
			SAFE_RELEASE(pTask);
			itPapers = g_lPapers.erase(itPapers);
		}
		g_fmPapers.unlock();

		g_fmDecompressLock.lock();
		DECOMPRESSTASKLIST::iterator itDecomp = g_lDecompressTask.begin();
		for (; itDecomp != g_lDecompressTask.end();)
		{
			pDECOMPRESSTASK pTask = *itDecomp;
			SAFE_RELEASE(pTask);
			itDecomp = g_lDecompressTask.erase(itDecomp);
		}
		g_fmDecompressLock.unlock();

		g_fmHttpSend.lock();
		LIST_SEND_HTTP::iterator itHttp = g_lHttpSend.begin();
		for (; itHttp != g_lHttpSend.end();)
		{
			pSEND_HTTP_TASK pTask = *itHttp;
			SAFE_RELEASE(pTask);
			itHttp = g_lHttpSend.erase(itHttp);
		}
		g_fmHttpSend.unlock();

		scanReqThread.join();
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

		g_Log.LogOut("StopFileChannel complete.");
		g_Log.LogOut("StopCmdChannel complete.");
		return 0;
	}
};

POCO_SERVER_MAIN(DCS);

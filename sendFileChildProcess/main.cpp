#include "global.h"
#include "MyCodeConvert.h"
#include "MulticastServer.h"
#include "SendFileThread.h"

std::string		_strServerIP_;
int				_nServerPort_;
std::string		_strMulticastIP_;
int				_nMulticastPort_;

std::string		_strCurPath_;
std::string		_strTaskName_;

bool				_bAllOK_ = true;
int					_nExitFlag_;					//退出标示
Poco::Logger*		_pLogger_;
Poco::Event*		_peStartMulticast_ = new Poco::Event(false);
Poco::Event			_eInitMulticast_;
Poco::Event			_eTaskCompleted_;

int					_nCurOKThreads_ = 0;
Poco::FastMutex		_fmCurOKLock_;

Poco::FastMutex		g_fmSendLock;
SENDTASKLIST		g_lSendTask;

int					_nThreads_= 5;
Poco::Thread*		_pRecogThread_;
std::vector<CSendFileThread*> _vecRecogThreadObj_;

void InitConfig()
{
	wchar_t szwPath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, szwPath, MAX_PATH);
	char szPath[MAX_PATH] = { 0 };
	int nLen = WideCharToMultiByte(CP_ACP, 0, szwPath, -1, NULL, 0, NULL, NULL);
	char* pszDst = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szwPath, -1, szPath, nLen, NULL, NULL);
	szPath[nLen - 1] = 0;
	delete[] pszDst;

	std::string strPath = szPath;
	strPath = strPath.substr(0, strPath.rfind('\\') + 1);
	_strCurPath_ = strPath;

	std::string strName = Poco::format("sendFile_Child_%d.log", (int)Poco::Thread::currentTid());

	std::string strLogPath = CMyCodeConvert::Gb2312ToUtf8(strPath + strName);	//_T("sendFile.log")
	Poco::AutoPtr<Poco::PatternFormatter> pFormatter(new Poco::PatternFormatter("%L%Y-%m-%d %H:%M:%S.%F %q:%t"));
	Poco::AutoPtr<Poco::FormattingChannel> pFCFile(new Poco::FormattingChannel(pFormatter));
	Poco::AutoPtr<Poco::FileChannel> pFileChannel(new Poco::FileChannel(strLogPath));
	pFCFile->setChannel(pFileChannel);
	pFCFile->open();
	pFCFile->setProperty("rotation", "1 M");
	pFCFile->setProperty("archive", "timestamp");
	pFCFile->setProperty("compress", "true");
	pFCFile->setProperty("purgeCount", "5");
	
	Poco::Logger& appLogger = Poco::Logger::create(strName, pFCFile, Poco::Message::PRIO_INFORMATION);
	_pLogger_ = &appLogger;

	std::string strConfigPath = strPath;
	strConfigPath.append("config_sendFileTest.ini");
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strConfigPath);
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
	_strServerIP_ = pConf->getString("SERVER.IP");
	_nServerPort_ = pConf->getInt("SERVER.Port", 19980);

	_strMulticastIP_ = pConf->getString("Multicast.IP");
	_nMulticastPort_ = pConf->getInt("Multicast.Port", 19980);

	_strTaskName_ = pConf->getString("Task.fileName");
}

void InitThreads()
{
	_pRecogThread_ = new Poco::Thread[_nThreads_];
	for (int i = 0; i < _nThreads_; i++)
	{
		CSendFileThread* pObj = new CSendFileThread(_strServerIP_, _nServerPort_);
		_pRecogThread_[i].start(*pObj);

		_vecRecogThreadObj_.push_back(pObj);
	}
}

void releaseData()
{
	delete _peStartMulticast_;
	_peStartMulticast_ = NULL;

	for (int i = 0; i < _vecRecogThreadObj_.size(); i++)
	{
		_pRecogThread_[i].join();
	}
	std::vector<CSendFileThread*>::iterator itRecogObj = _vecRecogThreadObj_.begin();
	for (; itRecogObj != _vecRecogThreadObj_.end();)
	{
		CSendFileThread* pObj = *itRecogObj;
		if (pObj)
		{
			delete pObj;
			pObj = NULL;
		}
		itRecogObj = _vecRecogThreadObj_.erase(itRecogObj);
	}
}

bool InitTask()
{
	std::string strSrcPath = _strCurPath_ + _strTaskName_;
	std::string strPath;
	std::string strExt;
	std::string strBaseName;
	try
	{
		Poco::Path filePath(CMyCodeConvert::Gb2312ToUtf8(strSrcPath));
		strPath = filePath.toString();
		strExt = filePath.getExtension();
		strBaseName = filePath.getBaseName();

		int nPos = strPath.rfind('.');
		strPath = strPath.substr(0, nPos);

		for (int i = 1; i <= _nThreads_; i++)
		{
			std::string strNewFilePath = Poco::format("%s_ChildProcess_%d_%d.%s", strPath, i, (int)Poco::Thread::currentTid(), strExt);

			Poco::File fileTask(CMyCodeConvert::Gb2312ToUtf8(strSrcPath));
			if (fileTask.exists())
				fileTask.copyTo(strNewFilePath);
			else
			{
				std::cout << "文件不存在: " << strSrcPath << std::endl; 
				std::string strLog = "文件不存在: " + strSrcPath;
				_pLogger_->information(strLog);
				return false;
			}
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "文件复制异常: " + CMyCodeConvert::Utf8ToGb2312(exc.displayText()) + "\n";
		_pLogger_->information(strLog);
		return false;
	}

	for (int i = 1; i <= _nThreads_; i++)
	{
		std::string strNewFilePath = Poco::format("%s_ChildProcess_%d_%d.%s", strPath, i, (int)Poco::Thread::currentTid(), strExt);
		std::string strNewName = Poco::format("%s_ChildProcess_%d_%d.%s", strBaseName, i, (int)Poco::Thread::currentTid(), strExt);

		pSENDTASK pTask = new SENDTASK;
		char szFilePath[200] = { 0 };
		pTask->strPath = strNewFilePath;
		pTask->strName = strNewName;

		g_lSendTask.push_back(pTask);
	}
	return true;
}

void afterExit()
{
// 	_pLogger_->shutdown();
// 	std::string strName = Poco::format("sendFile_Child_%d.log", (int)Poco::Thread::currentTid());
// 	std::string strLogPath = _strCurPath_ + strName;
// 	Poco::File logFile(CMyCodeConvert::Gb2312ToUtf8(strLogPath));
// 	if (_bAllOK_ && logFile.exists())
// 		logFile.remove();
}

int main()
{
	atexit(afterExit);

	std::cout << "文件发送子进程启动完成，等待命令中。。。" << std::endl;
	InitConfig();
	MulticastServer _MulticastServer(_strMulticastIP_, _nMulticastPort_);

	_eInitMulticast_.wait();
	InitThreads();
	
	if (!InitTask())
	{
		system("pause");
		return 0;
	}
	std::cout << "Init task OK" << std::endl;
	_eTaskCompleted_.wait();

	

	releaseData();

	_pLogger_->shutdown();
	std::string strName = Poco::format("sendFile_Child_%d.log", (int)Poco::Thread::currentTid());
	std::string strLogPath = _strCurPath_ + strName;
	Poco::File logFile(CMyCodeConvert::Gb2312ToUtf8(strLogPath));
	if (_bAllOK_ && logFile.exists())
		logFile.remove();

	exit(0);
	return 0;
}


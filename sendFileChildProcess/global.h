#pragma once
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <opencv2\opencv.hpp>

#include "Poco/Runnable.h"
#include "Poco/Exception.h"

#include "Poco/AutoPtr.h"  
#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"

#include "Poco/DirectoryIterator.h"
#include "Poco/File.h"
#include "Poco/Path.h"

#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"

#include "Poco/AutoPtr.h"  
#include "Poco/Util/IniFileConfiguration.h" 

#include "Poco/Random.h"

#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include "Poco/StreamCopier.h"

#include "Poco/Net/NetException.h"
#include "Poco/Net/TCPServer.h"

#include "Poco/Net/MulticastSocket.h"


#define MULTICAST_START		0x9001		//组播命令，开始发送文件
#define MULTICAST_INIT_PROCESS		0x9002		//组播命令，开始初始化进程
#define MULTICAST_INIT_THREAD		0x9003		//组播命令，开始初始化线程

extern bool				_bAllOK_;
extern int				_nExitFlag_;					//退出标示
extern Poco::Logger*	_pLogger_;
extern Poco::Event*		_peStartMulticast_;
extern Poco::Event		_eInitMulticast_;

extern int				_nThreads_;
extern Poco::Event		_eTaskCompleted_;

extern int					_nCurOKThreads_;
extern Poco::FastMutex		_fmCurOKLock_;

typedef struct _SendTask_
{
	int		nSendTimes;
	std::string strName;
	std::string strPath;
	_SendTask_()
	{
		nSendTimes = 0;
	}
}SENDTASK, *pSENDTASK;
typedef std::list<pSENDTASK> SENDTASKLIST;	//识别任务列表

extern Poco::FastMutex		g_fmSendLock;
extern SENDTASKLIST			g_lSendTask;



#pragma once
#include "global.h"
#include "Net_Cmd_Protocol.h"


class TimerKeepAliveObj : public Poco::Util::TimerTask
{
public:
	void run()
	{
		pTCP_TASK pTcpTask = new TCP_TASK;
		pTcpTask->usCmd = KEEPALIVE_PKG;
		pTcpTask->nPkgLen = _strEzs_.length();
		memcpy(pTcpTask->szSendBuf, (char*)_strEzs_.c_str(), _strEzs_.length());
		g_fmTcpTaskLock.lock();
		g_lTcpTask.push_back(pTcpTask);
		g_fmTcpTaskLock.unlock();		
	}
};

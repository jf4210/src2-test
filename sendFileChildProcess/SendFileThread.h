#pragma once
#include "global.h"
//#include "FileUpLoad.h"

class CSendFileThread :
	public Poco::Runnable
{
public:
	CSendFileThread(std::string& strIP, int nPort);
	~CSendFileThread();

	virtual void run();
	
	void HandleTask(pSENDTASK pTask);
	void SendFileComplete(char* pName, char* pSrcPath);

//	CFileUpLoad		m_upLoad;

	bool	connectServer();
	bool	sendMyData(char* szBuf, int nLen);
	bool	recvMyData(char* szBuf);
	bool	HandleCmd(char* szBuf, unsigned short usCmd);

	std::string calcMd5(std::string strFilePath);

	Poco::Net::StreamSocket m_ss;
	bool	_bConnect;
	char*	m_pRecvBuff;
	std::string _strIp;
	int		_nPort;
};


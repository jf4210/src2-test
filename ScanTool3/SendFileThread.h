#pragma once
#include "global.h"
#include "FileUpLoad.h"


#ifdef TEST_MULTI_SENDER
typedef struct _SendInfo_
{
	CFileUpLoad* pUpLoad;
	int			nPort;
	std::string strIP;
}ST_SENDER, *pST_SENDER;
typedef std::map<std::string, pST_SENDER> MAP_FILESENDER;
#endif

class CSendFileThread :
	public Poco::Runnable
{
public:
	CSendFileThread(std::string& strIP, int nPort);
	~CSendFileThread();


	virtual void run();

	void HandleTask(pSENDTASK pTask);
	void SendFileComplete(char* pName, char* pSrcPath);

	CFileUpLoad*	m_pUpLoad;

#ifdef TEST_MULTI_SENDER
	MAP_FILESENDER	_mapSender;
#endif

	std::string _strIp;
	int		_nPort;
};


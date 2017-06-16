#pragma once

#include "Thread.h"
#include <vector>
#include <list>
#include "NetCtrl.h"
#include "md5.h"
//#include "global.h"

#define FILE_BUFF	1024

//文件上传结构体
typedef struct ST_UPLOAD_ANS
{
	BOOL	bUpload;
	void*	pTask;
	CString strAnsName;
	CString strPath;
	
	ST_UPLOAD_ANS()
	{
		pTask	= NULL;
		strAnsName = _T("");
		strPath = _T("");
		bUpload = FALSE;
	}
}stUpLoadAns, *pStUpLoadAns;

class CSendFileThread;
class CFileUpLoad : public ITcpClientNotify,public CThread
{
public:
	CFileUpLoad(CSendFileThread& rNotify);
	~CFileUpLoad(void);

	void		OnTcpClientNotifyReceivedData(const char* pData,int nLen);
	void		OnTcpClientNotifyDisconnect(UINT uSocket);
	void		OnTcpClientNotifyTime(DWORD dwTime){};
	void		ThreadProcMain();

	BOOL		InitUpLoadTcp(CString strAddr,USHORT usPort);
	void		UnInit();
	BOOL		SendAnsFile(CString strFilePath, CString strFileName, void* pTask);
	bool		sendData(char * szBuff, DWORD nLen, stUpLoadAns* pTask);
	
	void		ProcessUpLoadResponse(BOOL bFlag);
	void		ProcessUpLoadDone(BOOL bFlag);
	BOOL		CheckUpLoadFile();

	void		ReConnectAddr(CString strAddr, USHORT usPort);

	std::list<stUpLoadAns*>		m_listFile; 
	std::vector<stUpLoadAns*> m_VecAns;

#ifdef TEST_MULTI_SENDER
	CMutex		mutexObj;
#endif

	ITcpClient	*m_pITcpClient;
	UINT		m_uThreadType;
	CString		m_strAddr;
	USHORT		m_usPort;
	char		m_szSendBuf[FILE_BUFF];		//发送缓存
 	CRITICAL_SECTION m_csCrit;
	BOOL		m_bConnect;
	BOOL		m_bUpLoad;
	HANDLE		m_hAddAnsEvent;
	HANDLE		m_hSendReadyEvent;
	HANDLE		m_hSendDoneEvent;
	BOOL		m_bSendOK;
	BOOL		m_bReadyOK;
	DWORD       staticTimer; //计时
	CSendFileThread& m_rNotify;
	BOOL		m_bStop;
};

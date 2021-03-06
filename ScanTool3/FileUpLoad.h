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
	int		nPriority;		//上传优先级，越高越优先上传，不能中断现有上传文件
	CString strAnsName;
	CString strPath;
	
	ST_UPLOAD_ANS()
	{
		pTask	= NULL;
		nPriority = 0;
		strAnsName = _T("");
		strPath = _T("");
		bUpload = FALSE;
	}
}stUpLoadAns, *pStUpLoadAns;

typedef struct ST_UpLoadAddr
{
	int		nPort;
	std::string strIP;
}stUpLoadAddr, *pstUpLoadAddr;

class CSendFileThread;
class CFileUpLoad : public ITcpClientNotify,public CThread
{
public:
	CFileUpLoad(CSendFileThread* rNotify);
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
	void		SetNotifyObj(CSendFileThread* rNotify);
	void		SetSendExtType(std::string strExtType);

	std::list<stUpLoadAns*>		m_listFile; 
	std::vector<stUpLoadAns*> m_VecAns;

#if 1
	CMutex		mutexObj;
#endif

	ITcpClient	*m_pITcpClient;
	UINT		m_uThreadType;
	CString		m_strAddr;		//当前使用的连接地址
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
	CSendFileThread* m_pNotifyObj;
	BOOL		m_bStop;

	int		m_nConnectFails;	//连接当前服务器地址失败的次数
	std::string	m_strSendExtType;	//此发送线程发送的文件类型，pkg  typkg
	std::list<pstUpLoadAddr>	m_UpLoadAddrList;	//上传的服务器地址，针对3大运营商的情况，电信不行换移动。。。
};

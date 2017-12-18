#pragma once
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Net_Cmd_Protocol.h"
#include "Net_Cmd_Protocol_Ver.h"
#include "md5.h"

#include <io.h>

#include "afxmt.h"
#include "MyCodeConvert.h"

#ifdef _DEBUG
	#define INTERVAL_TIME	30 * 1000	//间隔时间，秒60 * 1000
	#define CHECK_UPDATE_TIME	1 * 60 * 1000	//检查是否有更新的时间60 * 60 * 1000
#else
	#define INTERVAL_TIME	30 * 1000	//间隔时间，秒1 * 60 * 1000
	#define CHECK_UPDATE_TIME	1 * 60 * 1000	//检查是否有更新的时间60 * 60 * 1000
#endif

#define MSG_NOTIFY_UPDATE	(WM_APP + 101)

#define MCASTPORT 19970				//绑定的本地端口号。
#define BUFSIZE 1024				//接收数据缓冲大小。


#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}
#define SAFE_RELEASE_ARRY(pObj) if(pObj) {delete[] pObj; pObj = NULL;}

extern BOOL		bContinue;
extern CString	g_strAppPath;
extern CString	g_strInitServerIP;		//初始连接服务器地址
extern int		g_nInitServerPort;		//初始连接服务器端口
extern CString	g_strVerServerIP;
extern int		g_nVerServerPort;
extern BOOL		g_bConnect;
extern SOCKET	g_sock;

extern BOOL		g_bShowUpdateMsg;		//是否通知扫描程序进行版本更新，如果第一次打开扫描软件，提示时选择不更新，那么在扫描软件运行期间一直不提示，直到软件有关闭
extern CMutex			g_mutex_VSFL;
extern CMutex			g_mutex_LFM;
extern CMutex			g_mutex_DFL;
extern std::string		g_strFileVersion;

typedef struct _FileInfo_
{
	std::string strFileName;
	std::string strMd5;
	std::string strFilePath;
}ST_FILEINFO, *pST_FILEINFO;
typedef std::list<pST_FILEINFO> LIST_FILEINFO;

extern HANDLE			g_hMutex_Conn;
extern LIST_FILEINFO	g_VerServerFileList;

typedef std::list<std::string> LIST_NEED_DOWNLOAD;
extern LIST_NEED_DOWNLOAD	g_DownLoadFileList;
typedef std::map<std::string, pST_FILEINFO> MAP_FILEINFO;
extern MAP_FILEINFO		g_LocalFileMap;



BOOL DeleteDirectory(char* psDirName);

//自动启动
BOOL StartUp();
BOOL InitNetWork(BOOL bSender = FALSE);
void UnInitNetWork();
BOOL ConnectServer(SOCKET& sock, CString& strIP, int& nPort);
DWORD WINAPI MyWork(LPVOID lParam);

BOOL GetLocalFileList();


BOOL CheckProcessExist(CString &str);
BOOL UpdateFile(BOOL& bReplace, bool bSetupPkg);
void ReadConf();

void StartProcess(LPTSTR   lpImage);
BOOL EnablePrivilege(PCWSTR name);
HANDLE GetLSAToken();


BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid);
BOOL AddAceToDesktop(HDESK hdesk, PSID psid);
BOOL StartInteractiveClientProcess(LPTSTR lpszUsername, LPTSTR lpszDomain, LPTSTR lpszPassword, LPTSTR lpCommandLine, HANDLE Token = NULL);

DWORD __stdcall INTER_GetExplorerToken(OUT PHANDLE  phExplorerToken);

BOOL   RunProcess(LPTSTR   lpImage);


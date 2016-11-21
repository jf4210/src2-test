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
#if 0
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

#include "Poco/LocalDateTime.h"

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/URI.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/TCPServer.h"

#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/Crypto/X509Certificate.h"
#include "Poco/Crypto/CryptoStream.h"
#endif
#include "afxmt.h"
#include "MyCodeConvert.h"

#define MSG_NOTIFY_UPDATE	(WM_APP + 101)

#define MCASTPORT 19970				//�󶨵ı��ض˿ںš�
#define BUFSIZE 1024				//�������ݻ����С��
#define INTERVAL_TIME	1 * 60 * 1000	//���ʱ�䣬��60 * 1000
#define CHECK_UPDATE_TIME	60 * 60 * 1000	//����Ƿ��и��µ�ʱ��60 * 60 * 1000

#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}
#define SAFE_RELEASE_ARRY(pObj) if(pObj) {delete[] pObj; pObj = NULL;}

extern BOOL		bContinue;
extern CString	g_strAppPath;
extern CString	g_strInitServerIP;		//��ʼ���ӷ�������ַ
extern int		g_nInitServerPort;		//��ʼ���ӷ������˿�
extern CString	g_strVerServerIP;
extern int		g_nVerServerPort;
extern BOOL		g_bConnect;
extern SOCKET	g_sock;

extern BOOL		g_bShowUpdateMsg;		//�Ƿ�֪ͨɨ�������а汾���£������һ�δ�ɨ���������ʾʱѡ�񲻸��£���ô��ɨ����������ڼ�һֱ����ʾ��ֱ������йر�
extern CMutex			g_mutex_VSFL;
extern CMutex			g_mutex_LFM;
extern CMutex			g_mutex_DFL;

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

//�Զ�����
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


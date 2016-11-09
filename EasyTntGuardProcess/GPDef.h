#pragma once
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include "Net_Cmd_Protocol.h"
#include "Net_Cmd_Protocol_Ver.h"

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

#include "MyCodeConvert.h"


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



typedef struct _FileInfo_
{
	std::string strFileName;
	std::string strMd5;
	std::string strFilePath;
}ST_FILEINFO, *pST_FILEINFO;
typedef std::list<pST_FILEINFO> LIST_FILEINFO;

extern HANDLE			g_hMutex_Conn;
extern LIST_FILEINFO	g_VerServerFileList;


//自动启动
BOOL StartUp();
BOOL InitNetWork(BOOL bSender = FALSE);
void UnInitNetWork();
BOOL ConnectServer(SOCKET& sock, CString& strIP, int& nPort);
DWORD WINAPI MyWork(LPVOID lParam);


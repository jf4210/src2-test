#include "stdafx.h"
#include "GPDef.h"


struct sockaddr_in local;
struct sockaddr_in serverAddr;

BOOL StartUp()
{
	//写入注册表,开机自启动 
	HKEY hKey;
	//找到系统的启动项 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
	//打开启动项Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey);
	if (lRet == ERROR_SUCCESS)
	{
		TCHAR pFileName[MAX_PATH] = { 0 };
		//得到程序自身的全路径 
		DWORD dwRet = GetModuleFileNameW(NULL, pFileName, MAX_PATH);

		lRet = RegSetValueEx(hKey, _T("GuardProcess"), 0, REG_SZ, (BYTE *)(pFileName), MAX_PATH);

		RegCloseKey(hKey);
		if (lRet != ERROR_SUCCESS)
		{
			TRACE("系统参数错误,不能随系统启动");
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

BOOL InitNetWork(BOOL bSender /*= FALSE*/)
{
	WSADATA wsd;

	//初始化WinSock2.2
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		TRACE0("WSAStartup() failed\n");
		return FALSE;
	}
	
	g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_sock == INVALID_SOCKET)
	{
		TRACE0("socket failed with:%d\n", WSAGetLastError());
		WSACleanup();
		return FALSE;
	}

	//将sock绑定到本机某端口上。
	if (!bSender)
	{
		local.sin_family = AF_INET;
		local.sin_port = htons(MCASTPORT);
		local.sin_addr.s_addr = INADDR_ANY;
		if (bind(g_sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
		{
			TRACE0("bind failed with:%d \n", WSAGetLastError());
			closesocket(g_sock);
			WSACleanup();
			return FALSE;
		}
	}
	
	return TRUE;
}

void UnInitNetWork()
{
	closesocket(g_sock);
	WSACleanup();
}

BOOL ConnectServer(SOCKET& sock, CString& strIP, int& nPort)
{
	USES_CONVERSION;
	if (strIP == _T("") || nPort == 0)
		return FALSE;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr(T2A(strIP));	//inet_addr(SERVERIP)
	serverAddr.sin_port = htons(nPort);

	if (SOCKET_ERROR == connect(sock, (SOCKADDR *)&serverAddr, sizeof(SOCKADDR)))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL GetVerServerAddr()
{
	//获取版本控制服务器地址
	ST_CMD_HEADER header_login;
	header_login.usCmd = GET_VERSERVER_ADDR;

	char szSendBuf[200] = { 0 };
	memcpy(szSendBuf, (char*)&header_login, HEAD_SIZE);
	int sendLen = send(g_sock, szSendBuf, HEAD_SIZE, 0);
	if (sendLen == SOCKET_ERROR)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}
	char szRecvBuf[BUFSIZE] = { 0 };
	int recvLen = recv(g_sock, szRecvBuf, BUFSIZE, 0);
	if (recvLen <= 0)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}
	BOOL bGetVerServerAddr = FALSE;
	pStCmdHeader pHeader = (pStCmdHeader)szRecvBuf;
	if (pHeader->usCmd == RESPONSE_GET_VERSERVER_ADDR)
	{
		char* p = szRecvBuf + HEAD_SIZE;
		TRACE("版本服务器地址信息：%s\n", p);
		char* p2 = strstr(p, ":");
		int nLen = p2 - p;
		char szIP[50] = { 0 };
		strncpy_s(szIP, p, p2 - p);
		int nPort = atoi(p2 + 1);
		g_strVerServerIP = szIP;
		g_nVerServerPort = nPort;
		if (g_strVerServerIP != _T("") && g_nVerServerPort != 0)
			bGetVerServerAddr = TRUE;
	}
	if (!bGetVerServerAddr)
		return FALSE;
	return TRUE;
}

BOOL GetFileList()
{
	LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
	for (; it != g_VerServerFileList.end();)
	{
		pST_FILEINFO pFileInfo = *it;
		SAFE_RELEASE(pFileInfo);
		it = g_VerServerFileList.erase(it);
	}

	//从版本控制服务器获取列表
	ST_CMD_HEADER header_login;
	header_login.usCmd = GET_FILELIST;

	char szSendBuf[200] = { 0 };
	memcpy(szSendBuf, (char*)&header_login, HEAD_SIZE);
	int sendLen = send(g_sock, szSendBuf, HEAD_SIZE, 0);
	if (sendLen == SOCKET_ERROR)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}

	char szRecvBuf[BUFSIZE * 2] = { 0 };
	int recvLen = recv(g_sock, szRecvBuf, BUFSIZE * 2, 0);
	if (recvLen <= 0)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}
	pStCmdHeader pHeader = (pStCmdHeader)szRecvBuf;
	if (pHeader->usCmd == RESPONSE_GET_FILELIST)
	{
		if (pHeader->usResult == RESULT_SUCCESS)
		{
			char* p = szRecvBuf + HEAD_SIZE;
			char* p1 = strstr(p, "_");
			while (p1 != NULL)
			{
				char* p2 = strstr(p, ":");
				char szFileName[50] = { 0 };
				strncpy_s(szFileName, p, p2 - p);
				char szMd5[50] = { 0 };
				strncpy_s(szMd5, p2 + 1, p1 - p2 - 1);

				pST_FILEINFO pFileInfo = new ST_FILEINFO;
				pFileInfo->strFileName = szFileName;
				pFileInfo->strMd5 = szMd5;
				g_VerServerFileList.push_back(pFileInfo);

				p = p1 + 1;
				p1 = strstr(p, "_");
			}
		}
		else
			TRACE("文件列表为空，不需要更新");
	}
	return TRUE;
}

DWORD WINAPI MyWork(LPVOID lParam)
{
	char recvbuf[BUFSIZE] = { 0 };
	int ret;
	int nConnectFailTimes = 0;		//连接版本控制服务器失败次数，如果连续超过10次，重新请求版本控制服务器地址
	BOOL bGetVerServerAddr = FALSE;

	CString strServerIP = g_strInitServerIP;
	int nServerPort = g_nInitServerPort;
	while (bContinue)
	{
		if (!g_bConnect)
		{
			if (!ConnectServer(g_sock, strServerIP, nServerPort))
			{
				g_bConnect = FALSE;
				Sleep(1000 * 30);
				continue;
			}
			g_bConnect = TRUE;
		}

		if (!bGetVerServerAddr && !GetVerServerAddr())
			continue;

		if (!bGetVerServerAddr)
		{
			bGetVerServerAddr = TRUE;
			strServerIP = g_strVerServerIP;
			nServerPort = g_nVerServerPort;
			closesocket(g_sock);
			g_sock = INVALID_SOCKET;
			g_bConnect = FALSE;
		}
		else
		{
			BOOL bGetFileList = GetFileList();
			if (!bGetFileList)
				continue;
			

		}
	}
	
	return 0;
}

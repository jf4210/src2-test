#include "stdafx.h"
#include "GPDef.h"
#include <io.h>


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
		DWORD dwResult = WSAGetLastError();
		return FALSE;
	}

	return TRUE;
}

BOOL GetVerServerAddr()
{
	//获取版本控制服务器地址
	ST_CMD_HEADER header;
	header.usCmd = GET_VERSERVER_ADDR;

	char szSendBuf[200] = { 0 };
	memcpy(szSendBuf, (char*)&header, HEAD_SIZE);
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

BOOL GetLocalFileList()
{
	MAP_FILEINFO::iterator it = g_LocalFileMap.begin();
	for (; it != g_LocalFileMap.end();)
	{
		pST_FILEINFO pFileInfo = it->second;
		SAFE_RELEASE(pFileInfo);
		it = g_LocalFileMap.erase(it);
	}

	USES_CONVERSION;
	CFileFind ff;
	BOOL bFind = ff.FindFile(g_strAppPath + _T("*"), 0);
	while (bFind)
	{
		bFind = ff.FindNextFileW();
		if (ff.GetFileName() == _T(".") || ff.GetFileName() == _T(".."))
			continue;
		else if (ff.IsDirectory())
		{
			CString strDirName = ff.GetFileName();
			CString strPath = ff.GetFilePath();
			CFileFind ff2;
			BOOL bFind2 = ff2.FindFile(ff.GetFilePath() + _T("\\*"));
			while (bFind2)
			{
				bFind2 = ff2.FindNextFileW();
				if (ff2.GetFileName() == _T(".") || ff2.GetFileName() == _T(".."))
					continue;
				else if (ff2.IsArchived())	//!ff2.IsDirectory()
				{
					pST_FILEINFO pFileInfo = new ST_FILEINFO;
					pFileInfo->strFileName = T2A(strDirName + _T("\\") + ff2.GetFileName());
					pFileInfo->strFilePath = T2A(ff2.GetFilePath());
					char *pMd5 = MD5File(T2A(ff2.GetFilePath()));
					pFileInfo->strMd5 = pMd5;

					g_LocalFileMap.insert(MAP_FILEINFO::value_type(pFileInfo->strFileName, pFileInfo));
				}
			}
		}
		else
		{
			pST_FILEINFO pFileInfo = new ST_FILEINFO;
			pFileInfo->strFileName = T2A(ff.GetFileName());
			pFileInfo->strFilePath = T2A(ff.GetFilePath());
			char *pMd5 = MD5File(T2A(ff.GetFilePath()));
			pFileInfo->strMd5 = pMd5;

			g_LocalFileMap.insert(MAP_FILEINFO::value_type(pFileInfo->strFileName, pFileInfo));
		}
	}

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
	ST_CMD_HEADER header;
	header.usCmd = GET_FILELIST;

	char szSendBuf[200] = { 0 };
	memcpy(szSendBuf, (char*)&header, HEAD_SIZE);
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

BOOL GetDownLoadFileList()
{
	g_DownLoadFileList.clear();

	LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
	for (; it != g_VerServerFileList.end(); it++)
	{
		pST_FILEINFO pFileInfo = *it;
		MAP_FILEINFO::iterator itMap = g_LocalFileMap.find(pFileInfo->strFileName);
		if (itMap != g_LocalFileMap.end())
		{
			pST_FILEINFO pLocalFile = itMap->second;
			if (pFileInfo->strMd5 != pLocalFile->strMd5)
				g_DownLoadFileList.push_back(pFileInfo->strFileName);
		}
		else
			g_DownLoadFileList.push_back(pFileInfo->strFileName);
	}
	return TRUE;
}

BOOL DownLoadFile(std::string& strName)
{
	ST_CMD_HEADER header;
	header.usCmd = GET_FILE;
	header.uPackSize = strName.length();

	char szSendBuf[200] = { 0 };
	memcpy(szSendBuf, (char*)&header, HEAD_SIZE);
	memcpy(szSendBuf + HEAD_SIZE, strName.c_str(), strName.length());
	int sendLen = send(g_sock, szSendBuf, HEAD_SIZE + header.uPackSize, 0);
	if (sendLen == SOCKET_ERROR)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}

	char szRecvBuf[1024] = { 0 };
	int recvLen = recv(g_sock, szRecvBuf, HEAD_SIZE, 0);
	if (recvLen <= 0)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}

	BOOL bRecvResult = TRUE;
	pStCmdHeader pHeader = (pStCmdHeader)szRecvBuf;
	int nWantLen = pHeader->uPackSize;
	int nRecvLen = HEAD_SIZE;
	char* pData = new char[nWantLen + HEAD_SIZE];
	memcpy(pData, szRecvBuf, HEAD_SIZE);
	while (nWantLen > 0)
	{
		int recvLen = recv(g_sock, pData + nRecvLen, nWantLen, 0);
		if (recvLen <= 0)
		{
			bRecvResult = FALSE;
			break;
		}

		nRecvLen += recvLen;
		nWantLen -= recvLen;
	}
	if (!bRecvResult)
	{
		SAFE_RELEASE_ARRY(pData);
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}

	USES_CONVERSION;
	CString strUpdatePath = g_strAppPath + _T("updateVersion\\");
	DWORD dwAttr = GetFileAttributesA(T2A(strUpdatePath));
	if (dwAttr == 0xFFFFFFFF)
		CreateDirectoryA(T2A(strUpdatePath), NULL);

	CString strFilePath = strUpdatePath + A2T(strName.c_str());
	try
	{
		if (_access(T2A(strFilePath), 0) == 0)
		{
			remove(T2A(strFilePath));
			TRACE("移除已存在文件: %s\n", T2A(strFilePath));
		}
	}
	catch (...)
	{
		TRACE("移除文件(%s)异常\n", T2A(strFilePath));
		SAFE_RELEASE_ARRY(pData);
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}	

	ofstream out(strFilePath, std::ios::binary);
	if (!out)
	{
		TRACE("打开文件进行读写(%s)失败\n", T2A(strFilePath));
		SAFE_RELEASE_ARRY(pData);
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}
	std::stringstream buffer;
	buffer.write(pData + HEAD_SIZE, pHeader->uPackSize);
	int n = buffer.str().length();
	out << buffer.str();
	out.close();

	TRACE("文件下载完成：%s\n", T2A(strFilePath));
	SAFE_RELEASE_ARRY(pData);
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
			g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			g_bConnect = FALSE;
		}
		else
		{
			BOOL bGetVerServerFileList = GetFileList();
			if (!bGetVerServerFileList)
			{
				g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				continue;
			}
			
			BOOL bGetLocalFileList = GetLocalFileList();
			BOOL bGetDownloadFileList = GetDownLoadFileList();

			BOOL bUpdateError = FALSE;
			while (g_DownLoadFileList.size() > 0)
			{
				std::string strDLName = g_DownLoadFileList.front();
				
				BOOL bDLFile = DownLoadFile(strDLName);
				if (!bDLFile)
				{
					bUpdateError = TRUE;
					break;
				}
				g_DownLoadFileList.pop_front();
			}
			if (bUpdateError)
			{
				g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				continue;
			}
			TRACE("新版本文件下载完成\n");
			Sleep(1 * 10 * 1000);
		}
	}
	
	return 0;
}

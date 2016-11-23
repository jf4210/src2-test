#include "stdafx.h"
#include "GPDef.h"
#include <io.h>

#include <Tlhelp32.h>

#include <AccCtrl.h>
#include <Aclapi.h> 
#include <WtsApi32.h>
#include <Userenv.h>

struct sockaddr_in local;
struct sockaddr_in serverAddr;

BOOL DeleteDirectory(char* psDirName)
{
	USES_CONVERSION;
	CFileFind tempFind;
	char sTempFileFind[_MAX_PATH] = { 0 };
	sprintf(sTempFileFind, "%s//*.*", psDirName);
	BOOL IsFinded = tempFind.FindFile(A2T(sTempFileFind));
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots())
		{
			char sFoundFileName[_MAX_PATH] = { 0 };
			strcpy(sFoundFileName, T2A(tempFind.GetFileName().GetBuffer(200)));
			if (tempFind.IsDirectory())
			{
				char sTempDir[_MAX_PATH] = { 0 };
				sprintf(sTempDir, "%s//%s", psDirName, sFoundFileName);
				DeleteDirectory(sTempDir);
			}
			else
			{
				char sTempFileName[_MAX_PATH] = { 0 };
				sprintf(sTempFileName, "%s//%s", psDirName, sFoundFileName);
				DeleteFile(A2T(sTempFileName));
			}
		}
	}
	tempFind.Close();
	if (!RemoveDirectory(A2T(psDirName)))
	{
		return FALSE;
	}
	return TRUE;
}

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
		TRACE("连接服务器%s:%d失败\n", T2A(strIP), nPort);
		return FALSE;
	}
	TRACE("连接服务器%s:%d成功\n", T2A(strIP), nPort);

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
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}

	TRACE("获取版本服务器地址信息成功\n");

	return TRUE;
}

BOOL GetLocalFileList()
{
	g_mutex_LFM.Lock();
	MAP_FILEINFO::iterator it = g_LocalFileMap.begin();
	for (; it != g_LocalFileMap.end();)
	{
		pST_FILEINFO pFileInfo = it->second;
		if (_access(pFileInfo->strFilePath.c_str(), 0) != 0)
		{
			SAFE_RELEASE(pFileInfo);
			it = g_LocalFileMap.erase(it);
		}
		else
			it++;
// 		SAFE_RELEASE(pFileInfo);
// 		it = g_LocalFileMap.erase(it);
	}
	g_mutex_LFM.Unlock();

	TRACE("开始获取本地文件列表。。。\n");
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
			if (strDirName == _T("updateVersion"))
				continue;

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
					CString strName = ff2.GetFileName();
					if (strName.Find(_T(".pkg")) != -1 || strName.Find(_T(".typkg")) != -1 || strName.Find(_T(".mod")) != -1)
						continue;

					MAP_FILEINFO::iterator itFile = g_LocalFileMap.find(T2A(strDirName + _T("\\") + ff2.GetFileName()));
					if (itFile != g_LocalFileMap.end())
						continue;

					pST_FILEINFO pFileInfo = new ST_FILEINFO;
					pFileInfo->strFileName = T2A(strDirName + _T("\\") + ff2.GetFileName());
					pFileInfo->strFilePath = T2A(ff2.GetFilePath());
					char *pMd5 = MD5File(T2A(ff2.GetFilePath()));
					pFileInfo->strMd5 = pMd5;

					g_mutex_LFM.Lock();
					g_LocalFileMap.insert(MAP_FILEINFO::value_type(pFileInfo->strFileName, pFileInfo));
					g_mutex_LFM.Unlock();
				}
			}
			ff2.Close();
		}
		else
		{
			CString strName = ff.GetFileName();
			if (strName.Find(_T(".Log")) != -1 || strName.Find(_T(".log")) != -1)
				continue;

			MAP_FILEINFO::iterator itFile = g_LocalFileMap.find(T2A(ff.GetFileName()));
			if (itFile != g_LocalFileMap.end())
				continue;

			pST_FILEINFO pFileInfo = new ST_FILEINFO;
			pFileInfo->strFileName = T2A(ff.GetFileName());
			pFileInfo->strFilePath = T2A(ff.GetFilePath());
			char *pMd5 = MD5File(T2A(ff.GetFilePath()));
			pFileInfo->strMd5 = pMd5;

			g_mutex_LFM.Lock();
			g_LocalFileMap.insert(MAP_FILEINFO::value_type(pFileInfo->strFileName, pFileInfo));
			g_mutex_LFM.Unlock();
		}
	}
	ff.Close();

	TRACE("获取本地文件列表成功\n");
	return TRUE;
}

BOOL GetFileList()
{
	g_mutex_VSFL.Lock();
	LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
	for (; it != g_VerServerFileList.end();)
	{
		pST_FILEINFO pFileInfo = *it;
		SAFE_RELEASE(pFileInfo);
		it = g_VerServerFileList.erase(it);
	}
	g_mutex_VSFL.Unlock();

	TRACE("开始获取版本服务器文件列表。。。\n");
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
			char* p1 = strstr(p, "__");
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
				g_mutex_VSFL.Lock();
				g_VerServerFileList.push_back(pFileInfo);
				g_mutex_VSFL.Unlock();

				p = p1 + 2;
				p1 = strstr(p, "__");
			}
			TRACE("获取版本服务器文件列表成功\n");
		}
		else
			TRACE("文件列表为空，不需要更新");
	}
	return TRUE;
}

BOOL GetDownLoadFileList()
{
	g_mutex_DFL.Lock();
	g_DownLoadFileList.clear();
	g_mutex_DFL.Unlock();

	LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
	for (; it != g_VerServerFileList.end(); it++)
	{
		pST_FILEINFO pFileInfo = *it;
		MAP_FILEINFO::iterator itMap = g_LocalFileMap.find(pFileInfo->strFileName);
		if (itMap != g_LocalFileMap.end())
		{
			pST_FILEINFO pLocalFile = itMap->second;

			//守护进程不进行升级，如果需要升级，只能通过setup.exe的方式升级
			if (pFileInfo->strMd5 != pLocalFile->strMd5 && pFileInfo->strFileName.find("EasyTntGuardProcess.exe") == std::string::npos)	
			{
				g_mutex_DFL.Lock();
				g_DownLoadFileList.push_back(pFileInfo->strFileName);
				g_mutex_DFL.Unlock();
			}
		}
		else
		{
			g_mutex_DFL.Lock();
			g_DownLoadFileList.push_back(pFileInfo->strFileName);
			g_mutex_DFL.Unlock();
		}
	}
	TRACE("生成下载文件列表完成\n");
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
	int recvLen = recv(g_sock, szRecvBuf, sizeof(ST_CMD_HEADERTOVER), 0);
	if (recvLen <= 0)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}

	BOOL bRecvResult = TRUE;
	pStCmdHeader pHeader = (pStCmdHeader)szRecvBuf;
	if (pHeader->usCmd == RESPONSE_GET_FILE)
	{
		if (pHeader->usResult == RESULT_SUCCESS)
		{
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

			int nPos = strName.find("\\");
			if (nPos != std::string::npos)
			{
				CString strSubDir = strUpdatePath + A2T(strName.substr(0, nPos).c_str());
				DWORD dwAttr = GetFileAttributesA(T2A(strSubDir));
				if (dwAttr == 0xFFFFFFFF)
					CreateDirectoryA(T2A(strSubDir), NULL);
			}

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
				TRACE("移除文件(%s)异常\n", strFilePath);
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
		}
		else
		{
			if (pHeader->usResult == RESULT_ERROR_SEND)
				TRACE("服务器发送文件时发送出错\n");
			else if (pHeader->usResult == RESULT_ERROR_FILEIO)
				TRACE("服务器读取需要发送的文件时发生读写错误\n");
			bRecvResult = FALSE;
		}
	}
	if (!bRecvResult)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
	}
	return TRUE;
}

BOOL DownLoadAllFile()
{
	USES_CONVERSION;
	int nFailTimes = 0;
	BOOL bUpdateError = FALSE;

	CString strFilePath = g_strAppPath + _T("updateVersion");
	BOOL bDel = DeleteDirectory(T2A(strFilePath));
	if (bDel)
		TRACE("移除版本存放文件夹: %s\n", T2A(strFilePath));
	else
		TRACE("移除版本存放文件夹(%s)失败\n", T2A(strFilePath));
// 	try
// 	{
// 		if (_access(T2A(strFilePath), 0) == 0)
// 		{
// 			remove(T2A(strFilePath));
// 			TRACE("移除版本存放文件夹: %s\n", T2A(strFilePath));
// 		}
// 	}
// 	catch (...)
// 	{
// 		TRACE("移除版本存放文件夹(%s)异常\n", strFilePath);
// 	}

	while (g_DownLoadFileList.size() > 0)
	{
		std::string strDLName = g_DownLoadFileList.front();

		BOOL bDLFile = DownLoadFile(strDLName);
		if (!bDLFile)
		{
			bUpdateError = TRUE;
			break;
		}

		bool bFind = false;
		LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
		for (; it != g_VerServerFileList.end(); it++)
		{
			pST_FILEINFO pFileInfo = *it;
			if (pFileInfo->strFileName == strDLName)
			{
				bFind = true;
				CString strNewFilePath = g_strAppPath + _T("updateVersion\\") + A2T(strDLName.c_str());
				char *pMd5 = MD5File(T2A(strNewFilePath));
				if (pFileInfo->strMd5 == pMd5)
				{
					nFailTimes = 0;
					g_mutex_DFL.Lock();
					g_DownLoadFileList.pop_front();
					g_mutex_DFL.Unlock();
				}
				else
					nFailTimes++;
				break;
			}
		}
		if (!bFind) nFailTimes++;
		if (nFailTimes >= 10)		//同一文件下载超过10次
		{
			bUpdateError = TRUE;
			TRACE("下载文件%s失败次数超过10次，退出重来\n", strDLName.c_str());
			break;
		}
	}
	if (bUpdateError)
	{
		TRACE("下载所有新版本文件失败\n");
		return FALSE;
	}
	return TRUE;
}

BOOL SendUpdataResult(std::string& strResult)
{
	//从版本控制服务器获取列表
	ST_CMD_HEADER header;
	header.usCmd = RESULT_UPDATA;
	header.uPackSize = strResult.length();

	char szSendBuf[500] = { 0 };
	memcpy(szSendBuf, (char*)&header, HEAD_SIZE);
	memcpy(szSendBuf + HEAD_SIZE, strResult.c_str(), strResult.length());
	int sendLen = send(g_sock, szSendBuf, HEAD_SIZE + strResult.length(), 0);
	if (sendLen == SOCKET_ERROR)
	{
		g_bConnect = FALSE;
		closesocket(g_sock);
		g_sock = INVALID_SOCKET;
		return FALSE;
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
		if (nConnectFailTimes >= 4)
		{
			//重新读取配置文件，重头开始
			TRACE("重新读取配置文件，重新请求版本信息\n");
			ReadConf();
			if (g_sock != INVALID_SOCKET)
			{
				closesocket(g_sock);
				g_sock = INVALID_SOCKET;
			}
			g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			g_bConnect = FALSE;
			strServerIP = g_strInitServerIP;
			nServerPort = g_nInitServerPort;
			bGetVerServerAddr = FALSE;
		}
		if (!g_bConnect)
		{
			if (!ConnectServer(g_sock, strServerIP, nServerPort))
			{
				nConnectFailTimes++;
				g_bConnect = FALSE;
				Sleep(1000 * 30);
				g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				continue;
			}
			g_bConnect = TRUE;
		}

		if (!bGetVerServerAddr && !GetVerServerAddr())
		{
			nConnectFailTimes++;
			if (g_sock != INVALID_SOCKET)
			{
				closesocket(g_sock);
				g_sock = INVALID_SOCKET;
			}
			g_bConnect = FALSE;
			Sleep(INTERVAL_TIME);
			g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			continue;
		}

		if (!bGetVerServerAddr)
		{
			nConnectFailTimes = 0;
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
				nConnectFailTimes++;
				if (g_sock != INVALID_SOCKET)
				{
					closesocket(g_sock);
					g_sock = INVALID_SOCKET;
				}
				g_bConnect = FALSE;
				Sleep(INTERVAL_TIME);
				g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				continue;
			}
			
			BOOL bGetLocalFileList = GetLocalFileList();
			BOOL bGetDownloadFileList = GetDownLoadFileList();
			
			if (g_DownLoadFileList.size() == 0)
			{
				//没有需要更新的文件
				TRACE("没有需要更新的文件\n");
				std::string strResult = "没有需要更新的文件";
				SendUpdataResult(strResult);
				Sleep(1000);

				nConnectFailTimes = 0;
				closesocket(g_sock);
				g_sock = INVALID_SOCKET;

				Sleep(CHECK_UPDATE_TIME);

				g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				g_bConnect = FALSE;
				continue;
			}

			bool bSetupPkg = false;		//是否是单个安装文件
			if (g_DownLoadFileList.size() == 1)
			{
				std::string strDLName = g_DownLoadFileList.front();
				if (strDLName == "setup.exe")
					bSetupPkg = true;
			}

			BOOL bDownloadAllFile = DownLoadAllFile();
			if (!bDownloadAllFile)
			{
				std::string strResult = "下载所有版本文件失败";
				SendUpdataResult(strResult);
				Sleep(1000);

				nConnectFailTimes++;
				if (g_sock != INVALID_SOCKET)
				{
					closesocket(g_sock);
					g_sock = INVALID_SOCKET;
				}
				g_bConnect = FALSE;
				g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				continue;
			}
			TRACE("新版本文件下载完成\n");
			std::string strResult = "新版本文件下载完成";
			SendUpdataResult(strResult);

			//替换文件
			BOOL bReplace = FALSE;
			BOOL bResult = FALSE;
			int nReplaceTimes = 0;
			while (!bReplace && nReplaceTimes < 10)
			{
				bResult = UpdateFile(bReplace, bSetupPkg);
				if (bResult && bReplace)
					break;
				nReplaceTimes++;
				if (!bResult)
					Sleep(10 * 1000);
			}
			if (!bReplace)
			{
				TRACE("替换文件失败\n");
				std::string strResult = "替换文件失败";
				SendUpdataResult(strResult);
				Sleep(1000);

				nConnectFailTimes++;
				if (g_sock != INVALID_SOCKET)
				{
					closesocket(g_sock);
					g_sock = INVALID_SOCKET;
				}
				g_bConnect = FALSE;
				Sleep(INTERVAL_TIME);
				g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				continue;
			}

			TRACE("替换文件成功\n");
			strResult = "升级成功";
			SendUpdataResult(strResult);
			Sleep(1000);


			nConnectFailTimes = 0;
			if (g_sock != INVALID_SOCKET)
			{
				closesocket(g_sock);
				g_sock = INVALID_SOCKET;
			}
			g_bConnect = FALSE;
			Sleep(CHECK_UPDATE_TIME);
			g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
	}
	
	return 0;
}

BOOL ReplaceFile(bool bSetupPkg)
{
	CString strNewVersionPath = g_strAppPath + _T("updateVersion\\");
	DWORD dwAttr = GetFileAttributes(strNewVersionPath);
	if (dwAttr == 0xFFFFFFFF)
		return FALSE;

	USES_CONVERSION;
	TRACE("开始替换程序\n");
	CFileFind ff;
	BOOL bFind = ff.FindFile(strNewVersionPath + _T("*"), 0);
	while (bFind)
	{
		bFind = ff.FindNextFileW();
		if (ff.GetFileName() == _T(".") || ff.GetFileName() == _T(".."))
			continue;
		else if (ff.IsDirectory())
		{
			CString strDstPath = g_strAppPath + ff.GetFileName();
			DWORD dwAttr = GetFileAttributes(strDstPath);
			if (dwAttr != 0xFFFFFFFF)
			{
				CString strSrcPath = ff.GetFilePath();
				CFileFind ff2;
				BOOL bFind2 = ff2.FindFile(ff.GetFilePath() + _T("\\*"), 0);
				while (bFind2)
				{
					bFind2 = ff2.FindNextFileW();
					if (ff2.GetFileName() == _T(".") || ff2.GetFileName() == _T(".."))
						continue;
					else
					{
						CString strSrcPath2 = ff2.GetFilePath();
						CString strDstPath2 = strDstPath + _T("\\") + ff2.GetFileName();
						if (_access(T2A(strDstPath2), 0) == 0)
						{
							remove(T2A(strDstPath2));
							TRACE("移除已存在文件: %s\n", T2A(strDstPath2));
						}

						BOOL bMoveResult = MoveFile(ff2.GetFilePath(), strDstPath2);
						if (!bMoveResult)
						{
							int nError = GetLastError();
							TRACE("移动文件失败%s\n", T2A(ff2.GetFilePath()));
							return FALSE;
						}
						else
						{
							bool bFind = false;
							std::string strMd5;
							CString strName = ff.GetFileName() + _T("\\") + ff2.GetFileName();
							LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
							for (; it != g_VerServerFileList.end(); it++)
							{
								pST_FILEINFO pFileInfo = *it;
								if (pFileInfo->strFileName == T2A(strName))
								{
									bFind = true;
									strMd5 = pFileInfo->strMd5;
								}
							}
							if (bFind)
							{
								g_mutex_LFM.Lock();
								MAP_FILEINFO::iterator itFile = g_LocalFileMap.find(T2A(strName));
								if (itFile != g_LocalFileMap.end())
								{
									pST_FILEINFO pFile = itFile->second;
									pFile->strMd5 = strMd5;
								}
								g_mutex_LFM.Unlock();
							}
						}
					}
				}
			}
			else
			{
				BOOL bMoveResult = MoveFile(ff.GetFilePath(), strDstPath);
				if (!bMoveResult)
				{
					TRACE("移动文件夹失败%s\n", ff.GetFilePath());
					return FALSE;
				}
			}
		}
		else
		{
			CString strDstPath = _T("");
			
			if (bSetupPkg)
			{
				CString strSetupPkgPath = g_strAppPath + _T("\\newSetupPkg\\");
				DWORD dwAttr = GetFileAttributesA(T2A(strSetupPkgPath));
				if (dwAttr == 0xFFFFFFFF)
				{
					CreateDirectoryA(T2A(strSetupPkgPath), NULL);
				}
				strDstPath = strSetupPkgPath + ff.GetFileName();
			}
			else
				strDstPath = g_strAppPath + ff.GetFileName();


			if (_access(T2A(strDstPath), 0) == 0)
			{
				remove(T2A(strDstPath));
				TRACE("移除已存在文件: %s\n", T2A(strDstPath));
			}

			BOOL bMoveResult = MoveFile(ff.GetFilePath(), strDstPath);
			if (!bMoveResult)
			{
				int nError = GetLastError();
				TRACE("移动文件失败%s\n", ff.GetFilePath());
				return FALSE;
			}
			else
			{
				bool bFind = false;
				std::string strMd5;
				LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
				for (; it != g_VerServerFileList.end(); it++)
				{
					pST_FILEINFO pFileInfo = *it;
					if (pFileInfo->strFileName == T2A(ff.GetFileName()))
					{
						bFind = true;
						strMd5 = pFileInfo->strMd5;
					}
				}
				if (bFind)
				{
					g_mutex_LFM.Lock();
					MAP_FILEINFO::iterator itFile = g_LocalFileMap.find(T2A(ff.GetFileName()));
					if (itFile != g_LocalFileMap.end())
					{
						pST_FILEINFO pFile = itFile->second;
						pFile->strMd5 = strMd5;
					}
					g_mutex_LFM.Unlock();
				}				
			}
		}
	}
	return TRUE;
}

BOOL CheckProcessExist(CString &str, int& nProcessID)
{
	BOOL bResult;
	CString strTemp, strProcessName;
	HANDLE hSnapshot;               //内存进程的“快照”句柄      
	PROCESSENTRY32 ProcessEntry;    //描述进程的结构   
	//输入要结束的进程名称    
	strProcessName = str/*m_strProcessName*/;
	strProcessName.MakeLower();
	//返回内存所有进程的快照。参数为TH32CS_SNAPPROCESS取有的进程,忽略参数2；    
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	//获取要的进程名称对应的所有进程ID    
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	bResult = Process32First(hSnapshot, &ProcessEntry);//获取第一个进程 
	int processcount = 0;
	while (bResult)
	{
		//判断是否为要结束的进程    
		strTemp.Format(_T("%s"), ProcessEntry.szExeFile);
		strTemp.MakeLower();
		if (strTemp == strProcessName)
		{
			nProcessID = ProcessEntry.th32ProcessID;
			processcount++;
			return TRUE;
		}
		//获取下一个进程    
		bResult = Process32Next(hSnapshot, &ProcessEntry);
	}
	return FALSE;
}

BOOL UpdateFile(BOOL& bReplace, bool bSetupPkg)
{
	USES_CONVERSION;
	CString strProcessName = _T("");
	strProcessName.Format(_T("ScanTool.exe"));
	int nExeProcessID = 0;
	if (!CheckProcessExist(strProcessName, nExeProcessID))
	{
		bReplace = ReplaceFile(bSetupPkg);
		g_bShowUpdateMsg = TRUE;
		return TRUE;
	}
	else
	{
		//程序在运行
		TRACE("程序正在运行\n");
		if (g_bShowUpdateMsg)
		{
			CString str = _T("YKLX-ScanTool GuideDlg");
			TCHAR strTitle[MAX_PATH] = { 0 };
			HWND hwnd = NULL;
			HWND AfterHwnd = NULL;
			while (true)
			{
				hwnd = ::FindWindowEx(NULL, AfterHwnd, _T("#32770"), NULL);
				if (!hwnd)
					break;
				else
				{
					if (::GetWindowText(hwnd, strTitle, MAX_PATH))
					{
						if (StrStr(strTitle, str) != 0)
						{
							TRACE("开始通知扫描程序有版本更新\n");
							DWORD dwResult = SendMessage(hwnd, MSG_NOTIFY_UPDATE, NULL, NULL);
							TRACE("发送消息完成, %d\n", dwResult);
							if (dwResult == 1)
							{
								if (!bSetupPkg)
									Sleep(2 * 1000);		//等待程序退出

								bReplace = ReplaceFile(bSetupPkg);
								g_bShowUpdateMsg = TRUE;

								if (bReplace)
								{
									ULONG fMask = 0;
									CString strComm;
									CString strDir;
									if (bSetupPkg)
									{
										strProcessName = "newSetupPkg\\setup.exe";
										fMask = SEE_MASK_NOCLOSEPROCESS;				//直到调用的进程运行接收才返回

										TCHAR buf[MAX_PATH];
										GetTempPathW(MAX_PATH, buf);

										//将此安装文件移动到临时目录
										CString strTmpPath = _T("");
										strTmpPath.Format(_T("%ssetup.exe"), buf);
										if (_access(T2A(strTmpPath), 0) == 0)
											remove(T2A(strTmpPath));
										CString strTmpPath2 = _T("");
										strTmpPath2.Format(_T("%ssetup.exe"), g_strAppPath);
										if (_access(T2A(strTmpPath2), 0) == 0)
											remove(T2A(strTmpPath2));
										CopyFile(strProcessName, strTmpPath2, FALSE);

										if (MoveFile(strProcessName, strTmpPath))
										{
											strComm = strTmpPath;
											strDir = buf;
										}
										else
											return FALSE;
									}
									else
									{
										strComm.Format(_T("%s%s"), g_strAppPath, strProcessName);
										strDir = g_strAppPath;
									}
#if 1									
									SHELLEXECUTEINFOA TempInfo = {0};	

									TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
									TempInfo.fMask = fMask;
									TempInfo.hwnd = NULL;
									TempInfo.lpVerb = "runas";
									TempInfo.lpFile = T2A(strComm);
									TempInfo.lpParameters = "";
									TempInfo.lpDirectory = T2A(strDir);
									TempInfo.nShow = SW_NORMAL;

									::ShellExecuteExA(&TempInfo);
									TRACE("执行ShellExecuteExA完成1\n");
									if(bSetupPkg)
									{
										WaitForSingleObject(TempInfo.hProcess,INFINITE);
										TRACE("执行ShellExecuteExA完成2\n");
										CString strVerExePath = g_strAppPath + _T("newSetupPkg\\");
										BOOL bDel = DeleteDirectory(T2A(strVerExePath));
										if (bDel)
											TRACE("移除版本存放文件夹: %s\n", T2A(strVerExePath));
										else
											TRACE("移除版本存放文件夹(%s)失败\n", T2A(strVerExePath));
									}
#else
									STARTUPINFO si;
									PROCESS_INFORMATION pi;
									CString strComm;
									char szWrkDir[MAX_PATH];
									strComm.Format(_T("%s%s"), g_strAppPath, strProcessName);
									memset(&si, 0, sizeof(si));
									si.cb = sizeof(si);
									ZeroMemory(&pi, sizeof(pi));

									if (!CreateProcessW(NULL, (LPTSTR)(LPCTSTR)strComm, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
									{
										int nErrorCode = GetLastError();
										TRACE("启动程序失败: %d\n", nErrorCode);
									}
#endif
								}

								return TRUE;
							}
							else
								g_bShowUpdateMsg = FALSE;
						}
					}
				}
				AfterHwnd = hwnd;
			}
		}
	}
	return FALSE;
}

void ReadConf()
{
	TCHAR	szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	CString str = szPath;
	g_strAppPath = str.Left(str.ReverseFind('\\') + 1);
	CString strConfigPath = g_strAppPath + _T("config.ini");

	TCHAR szServerAddr[50] = { 0 };
	GetPrivateProfileString(_T("Server"), _T("cmdIP"), _T("116.211.105.45"), szServerAddr, 50, strConfigPath);
	g_nInitServerPort = GetPrivateProfileInt(_T("Server"), _T("cmdPort"), 19991, strConfigPath);
	g_strInitServerIP = szServerAddr;
}



///////////////////////////////////////////
#if 0
#pragma comment(lib, "wtsapi32")
HANDLE OpenSystemProcess()  

{
	HANDLE hSnapshot = NULL;  
	HANDLE hProc     = NULL;
	__try  
	{  
		// Get a snapshot of the processes in the system  

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  
		if (hSnapshot == NULL)  
		{  
			printf("OpenSystemProcess CreateToolhelp32Snapshot Failed");  
			__leave;  
		}  



		PROCESSENTRY32 pe32;  
		pe32.dwSize = sizeof(pe32);  

		// Find the "System" process  
		BOOL fProcess = Process32First(hSnapshot, &pe32);  
		while (fProcess && (lstrcmpi(pe32.szExeFile, TEXT("SYSTEM")) != 0))  
			fProcess = Process32Next(hSnapshot, &pe32);  

		if (!fProcess)  
		{  
			printf("OpenSystemProcess Not Found SYSTEM");  
			__leave;    // Didn’t find "System" process  
		}  
		
		// Open the process with PROCESS_QUERY_INFORMATION access  
		hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,  
							pe32.th32ProcessID);  
		if (hProc == NULL)  
		{  
			printf("OpenSystemProcess OpenProcess Failed");  

			__leave;  
		}  
	}   

	__finally  
	{  
		// Cleanup the snapshot  
		if (hSnapshot != NULL)  
			CloseHandle(hSnapshot);  

		return(hProc);  
	}  
}  

BOOL EnablePrivilege (PCWSTR name)  
{  
	HANDLE hToken;  
	BOOL rv;  

	TOKEN_PRIVILEGES priv = { 1, {0, 0, SE_PRIVILEGE_ENABLED} };  
	LookupPrivilegeValue (  
		0,  
		name,  
		&priv.Privileges[0].Luid  
		);  

	OpenProcessToken(  
		GetCurrentProcess (),  
		TOKEN_ADJUST_PRIVILEGES,  
		&hToken  
		);  
	
	AdjustTokenPrivileges (  
		hToken,  
		FALSE,  
		&priv,  
		sizeof priv,  
		0,  
		0  
		);  

	rv = GetLastError () == ERROR_SUCCESS;  

	CloseHandle (hToken);  
	return rv;  
}  

#define chDIMOF(Array) (sizeof(Array) / sizeof(Array[0]))  

BOOL ModifySecurity(HANDLE hProc, DWORD dwAccess)   
{  
	PACL pAcl        = NULL;  
	PACL pNewAcl     = NULL;  
	PACL pSacl       = NULL;  
	PSID pSidOwner   = NULL;  
	PSID pSidPrimary = NULL;  
	BOOL fSuccess    = TRUE;  

	PSECURITY_DESCRIPTOR pSD = NULL;  

	__try   
	{  
		// Find the length of the security object for the kernel object  
		DWORD dwSDLength;  
		if (GetKernelObjectSecurity(hProc, DACL_SECURITY_INFORMATION, pSD, 0,  
			&dwSDLength) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))  
		{  
			printf("ModifySecurity GetKernelObjectSecurity Size Failed");  

			__leave;  
		}  

		// Allocate a buffer of that length  
		pSD = LocalAlloc(LPTR, dwSDLength);  
		if (pSD == NULL)  
		{  
			printf("ModifySecurity LocalAlloc Failed");  

			__leave;  
		}  

		// Retrieve the kernel object  
		if (!GetKernelObjectSecurity(hProc, DACL_SECURITY_INFORMATION, pSD,  
			dwSDLength, &dwSDLength))  
		{  
			printf("ModifySecurity GetKernelObjectSecurity Failed");  

			__leave;  
		}  

		// Get a pointer to the DACL of the SD  
		BOOL fDaclPresent;  
		BOOL fDaclDefaulted;
		if (!GetSecurityDescriptorDacl(pSD, &fDaclPresent, &pAcl,  
			&fDaclDefaulted))  
		{  
			printf("ModifySecurity GetSecurityDescriptorDacl Failed");  

			__leave;  
		}  

		// Get the current user’s name  
		TCHAR szName[1024];  
		DWORD dwLen = chDIMOF(szName);  
		if (!GetUserName(szName, &dwLen))  
		{  
			printf("ModifySecurity GetUserName Failed");  

			__leave;  
		}  
		
		// Build an EXPLICIT_ACCESS structure for the ace we wish to add.  

		EXPLICIT_ACCESS ea;  
		BuildExplicitAccessWithName(&ea, szName, dwAccess, GRANT_ACCESS, 0);  
		ea.Trustee.TrusteeType = TRUSTEE_IS_USER;  
		
		// We are allocating a new ACL with a new ace inserted.  The new  
		// ACL must be LocalFree’d  

		if(ERROR_SUCCESS != SetEntriesInAcl(1, &ea, pAcl, &pNewAcl))   
		{  
			printf("ModifySecurity SetEntriesInAcl Failed");  

			pNewAcl = NULL;  

			__leave;  
		}  

		// Find the buffer sizes we would need to make our SD absolute  
		pAcl               = NULL;  
		dwSDLength         = 0;  
		DWORD dwAclSize    = 0;  
		DWORD dwSaclSize   = 0;  
		DWORD dwSidOwnLen  = 0;  
		DWORD dwSidPrimLen = 0;  

		PSECURITY_DESCRIPTOR pAbsSD = NULL;  

		if(MakeAbsoluteSD(pSD, pAbsSD, &dwSDLength, pAcl, &dwAclSize, pSacl,  
			&dwSaclSize, pSidOwner, &dwSidOwnLen, pSidPrimary, &dwSidPrimLen)  
			|| (GetLastError() != ERROR_INSUFFICIENT_BUFFER))  
		{  			
			printf("ModifySecurity MakeAbsoluteSD Size Failed");  

			__leave;  
		}  


		// Allocate the buffers  
		pAcl = (PACL) LocalAlloc(LPTR, dwAclSize);  
		pSacl = (PACL) LocalAlloc(LPTR, dwSaclSize);
		pSidOwner = (PSID) LocalAlloc(LPTR, dwSidOwnLen);  
		pSidPrimary = (PSID) LocalAlloc(LPTR, dwSidPrimLen);
		pAbsSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, dwSDLength);  

		if(!(pAcl && pSacl && pSidOwner && pSidPrimary && pAbsSD))  
		{  
			printf("ModifySecurity Invalid SID Found");  

			__leave;  
		}  

		// And actually make our SD absolute  
		if(!MakeAbsoluteSD(pSD, pAbsSD, &dwSDLength, pAcl, &dwAclSize, pSacl,  
			&dwSaclSize, pSidOwner, &dwSidOwnLen, pSidPrimary, &dwSidPrimLen)) 
		{  
			printf("ModifySecurity MakeAbsoluteSD Failed");  

			__leave;  
		}  

		// Now set the security descriptor DACL  
		if(!SetSecurityDescriptorDacl(pAbsSD, fDaclPresent, pNewAcl,  
			fDaclDefaulted))  
		{  
			printf("ModifySecurity SetSecurityDescriptorDacl Failed");  

			__leave;  
		}  

		// And set the security for the object  
		if(!SetKernelObjectSecurity(hProc, DACL_SECURITY_INFORMATION, pAbsSD))  
		{  
			printf("ModifySecurity SetKernelObjectSecurity Failed");  

			__leave;  
		}  

		fSuccess = TRUE;  
	}   

	__finally  
	{  
		// Cleanup  

		if (pNewAcl == NULL)  
			LocalFree(pNewAcl);

		if (pSD == NULL)  
			LocalFree(pSD);

		if (pAcl == NULL)  
			LocalFree(pAcl);

		if (pSacl == NULL)  
			LocalFree(pSacl);

		if (pSidOwner == NULL)
			LocalFree(pSidOwner);  

		if (pSidPrimary == NULL)  
			LocalFree(pSidPrimary);

		if(!fSuccess)  
		{  
			printf("ModifySecurity exception caught in __finally");  
		}  

		return(fSuccess);  
	}  
}  

HANDLE GetLSAToken()   
{  
	HANDLE hProc  = NULL;  
	HANDLE hToken = NULL;  
	BOOL bSuccess = FALSE;  

	__try  
	{  
		// Enable the SE_DEBUG_NAME privilege in our process token  
		if (!EnablePrivilege(SE_DEBUG_NAME))   
		{  
			printf("GetLSAToken EnablePrivilege Failed");  

			__leave;  
		}  

		// Retrieve a handle to the "System" process  
		hProc = OpenSystemProcess();  
		if(hProc == NULL)   
		{  
			printf("GetLSAToken OpenSystemProcess Failed");  

			__leave;  
		}  

		// Open the process token with READ_CONTROL and WRITE_DAC access.  We  
		// will use this access to modify the security of the token so that we  
		// retrieve it again with a more complete set of rights.  

		BOOL fResult = OpenProcessToken(hProc, READ_CONTROL | WRITE_DAC,  
										&hToken);  
		if(FALSE == fResult)    
		{  
			printf("GetLSAToken OpenProcessToken Failed");  

			__leave;  
		}  



		// Add an ace for the current user for the token.  This ace will add  

		// TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY rights.  

		if (!ModifySecurity(hToken, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY  
			| TOKEN_QUERY | TOKEN_ADJUST_SESSIONID))   
		{  
			printf("GetLSAToken ModifySecurity Failed");  

			__leave;  
		}  


		// Reopen the process token now that we have added the rights to  

		// query the token, duplicate it, and assign it.  

		fResult = OpenProcessToken(hProc, TOKEN_QUERY | TOKEN_DUPLICATE  
								   | TOKEN_ASSIGN_PRIMARY | READ_CONTROL | WRITE_DAC, &hToken);  
		if (FALSE == fResult)    
		{  
			printf("GetLSAT, oken OpenProcessToken Failed");  

			__leave;  
		}  

		bSuccess = TRUE;  
	}   

	__finally  
	{  
		// Close the System process handle  

		if (hProc != NULL)    CloseHandle(hProc);  
		if(bSuccess)  
			return hToken;  
		else  
		{  
			::CloseHandle(hToken);  

			return NULL;  
		}  
	}  
}  

#define DESKTOP_ALL (DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW | \
DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD | \
DESKTOP_JOURNALPLAYBACK | DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | \
DESKTOP_SWITCHDESKTOP | STANDARD_RIGHTS_REQUIRED)

#define WINSTA_ALL (WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | \
WINSTA_ACCESSCLIPBOARD | WINSTA_CREATEDESKTOP | WINSTA_WRITEATTRIBUTES | \
WINSTA_ACCESSGLOBALATOMS | WINSTA_EXITWINDOWS | WINSTA_ENUMERATE | \
WINSTA_READSCREEN | STANDARD_RIGHTS_REQUIRED)

#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL)  

BOOL GetLogonSID(HANDLE hToken, PSID *ppsid)  
{  
	PWTS_PROCESS_INFO pProcessInfo = NULL;  
	DWORD             ProcessCount = 0;  
	BOOL                ret=FALSE;  

	USES_CONVERSION;
	if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pProcessInfo, &ProcessCount))  
	{  
		// dump each process description  
		for (DWORD CurrentProcess = 0; CurrentProcess < ProcessCount; CurrentProcess++)  
		{  
			if (strcmp(T2A(pProcessInfo[CurrentProcess].pProcessName), "System") == 0)
			{
				//*ppsid = pProcessInfo[CurrentProcess].pUserSid;  
				DWORD dwLength = GetLengthSid(pProcessInfo[CurrentProcess].pUserSid);  
				*ppsid = (PSID) HeapAlloc(GetProcessHeap(),  
										  HEAP_ZERO_MEMORY, dwLength);  
				if (*ppsid == NULL)  
					break;  

				if (!CopySid(dwLength, *ppsid, pProcessInfo[CurrentProcess].pUserSid))   
				{  
					HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);  

					break;  
				}  

				ret=TRUE;  

				break;  
			}  
		}  

		WTSFreeMemory(pProcessInfo);  
	}  

	return ret;  
}  

BOOL GetLogonSID_1 (HANDLE hToken, PSID *ppsid)   
{  
	BOOL bSuccess = FALSE;  
	DWORD dwIndex;  
	DWORD dwLength = 0;  
	PTOKEN_GROUPS ptg = NULL;  

	// Verify the parameter passed in is not NULL.  

	if (NULL == ppsid)  
		goto Cleanup;  

	// Get required buffer size and allocate the TOKEN_GROUPS buffer.  



	if (!GetTokenInformation(  
		hToken,         // handle to the access token 
		TokenGroups,    // get information about the token’s groups   
		(LPVOID)ptg,   // pointer to TOKEN_GROUPS buffer  
		0,              // size of buffer  
		&dwLength       // receives required buffer size  
		))
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			goto Cleanup;

		ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(),
									   HEAP_ZERO_MEMORY, dwLength);
		if (ptg == NULL)
			goto Cleanup;
	}

	// Get the token group information from the access token.  

	if (!GetTokenInformation(
		hToken,         // handle to the access token  
		TokenGroups,    // get information about the token’s groups   
		(LPVOID)ptg,   // pointer to TOKEN_GROUPS buffer  
		dwLength,       // size of buffer  
		&dwLength       // receives required buffer size  
		))
	{
		goto Cleanup;
	}

	// Loop through the groups to find the logon SID.  

	for (dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++)
		if ((ptg->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID)
			== SE_GROUP_LOGON_ID)
		{
			// Found the logon SID; make a copy of it.  
			dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
			*ppsid = (PSID)HeapAlloc(GetProcessHeap(),
									 HEAP_ZERO_MEMORY, dwLength);
			if (*ppsid == NULL)
				goto Cleanup;

			if (!CopySid(dwLength, *ppsid, ptg->Groups[dwIndex].Sid))
			{
				HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
				goto Cleanup;
			}

			break;
		}

	bSuccess = TRUE;

Cleanup:
	// Free the buffer for the token groups.  

	if (ptg != NULL)
		HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);

	return bSuccess;
}

VOID FreeLogonSID(PSID *ppsid)
{
	HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}

BOOL StartInteractiveClientProcess(
	LPTSTR lpszUsername,    // client to log on  
	LPTSTR lpszDomain,      // domain of client’s account  
	LPTSTR lpszPassword,    // client’s password  
	LPTSTR lpCommandLine,    // command line to execute  
	HANDLE Token
	)
{
	HANDLE      hToken;
	HDESK       hdesk = NULL;
	HWINSTA     hwinsta = NULL, hwinstaSave = NULL;
	PROCESS_INFORMATION pi;
	PSID pSid = NULL;
	STARTUPINFO si;
	BOOL bResult = FALSE;

	// Log the client on to the local computer.  
	if (Token != NULL)
	{
		printf("%08x\n", Token);

		hToken = Token;
	}
	else if (!LogonUser(
		lpszUsername,
		lpszDomain,
		lpszPassword,
		LOGON32_LOGON_INTERACTIVE,
		LOGON32_PROVIDER_DEFAULT,
		&hToken))
	{
		goto Cleanup;
	}

	// Save a handle to the caller’s current window station.  
	if ((hwinstaSave = GetProcessWindowStation()) == NULL)
		goto Cleanup;

	// Get a handle to the interactive window station.  
	hwinsta = OpenWindowStation(
		_T("winsta0"),                   // the interactive window station  
		FALSE,                       // handle is not inheritable  
		READ_CONTROL | WRITE_DAC);   // ;rights to read/write the DACL  

	if (hwinsta == NULL)
		goto Cleanup;

	// To get the correct default desktop, set the caller’s   
	// window station to the interactive window station.  

	if (!SetProcessWindowStation(hwinsta))
		goto Cleanup;

	// Get a handle to the interactive desktop.  
	hdesk = OpenDesktop(
		_T("default"),     // the interactive window station   
		0,             // no interaction with other desktop processes  
		FALSE,         // handle is not inheritable  
		READ_CONTROL | // request the rights to read and write the DACL  
		WRITE_DAC |
		DESKTOP_WRITEOBJECTS |
		DESKTOP_READOBJECTS);

	// Restore the caller’s window station.  
	if (!SetProcessWindowStation(hwinstaSave))
		goto Cleanup;

	if (hdesk == NULL)
		goto Cleanup;

	// Get the SID for the client’s logon session.  
	if (!GetLogonSID(hToken, &pSid))
		goto Cleanup;

	// Allow logon SID full access to interactive window station.  
	if (!AddAceToWindowStation(hwinsta, pSid))
		goto Cleanup;

	// Allow logon SID full access to interactive desktop.  
	if (!AddAceToDesktop(hdesk, pSid))
		goto Cleanup;

	// Impersonate client to ensure access to executable file.  
	if (!ImpersonateLoggedOnUser(hToken))
		goto Cleanup;

	// Initialize the STARTUPINFO structure.  
	// Specify that the process runs in the interactive desktop.  
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = TEXT("winsta0\\default");  //You can use EnumWindowStations to enum desktop  

	// Launch the process in the client’s logon session.  
	bResult = CreateProcessAsUser(
		hToken,           // client’s access token  
	NULL,              // file to execute  
		lpCommandLine,     // command line  
		NULL,              // pointer to process SECURITY_ATTRIBUTES 
		NULL,              // pointer to thread SECURITY_ATTRIBUTES  
		FALSE,             // handles are not inheritable  
		NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,   // creation flags  
		NULL,              // pointer to new environment block   
		NULL,              // name of current directory   
		&si,               // pointer to STARTUPINFO structure  
		&pi                // receives information about new process  
		);

	// End impersonation of client.  
	RevertToSelf();

	goto Cleanup;
	//return bResult; <------------------------------------------------------------------------  

	if (bResult && pi.hProcess != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
	}

	if (pi.hThread != INVALID_HANDLE_VALUE)
		CloseHandle(pi.hThread);
Cleanup:
	if (hwinstaSave != NULL)
		SetProcessWindowStation(hwinstaSave);

	// Free the buffer for the logon SID.  
	if (pSid)
		FreeLogonSID(&pSid);
	// Close the handles to the interactive window station and desktop.  

	if (hwinsta)
		CloseWindowStation(hwinsta);

	if (hdesk)
		CloseDesktop(hdesk);

	// Close the handle to the client’s access token.  
	if (hToken != INVALID_HANDLE_VALUE)
		CloseHandle(hToken);     return bResult;
}

BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid)
{
	ACCESS_ALLOWED_ACE   *pace;
	ACL_SIZE_INFORMATION aclSizeInfo;
	BOOL                 bDaclExist;
	BOOL                 bDaclPresent;
	BOOL                 bSuccess = FALSE;
	DWORD                dwNewAclSize;
	DWORD                dwSidSize = 0;
	DWORD                dwSdSizeNeeded;
	PACL                 pacl;
	PACL                 pNewAcl;
	PSECURITY_DESCRIPTOR psd = NULL;
	PSECURITY_DESCRIPTOR psdNew = NULL;
	PVOID                pTempAce;
	SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
	unsigned int         i;

	__try
	{
		// Obtain the DACL for the window station.  
		if (!GetUserObjectSecurity(
			hwinsta,
			&si,
			psd,
			dwSidSize,
			&dwSdSizeNeeded)
			)

			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				psd = (PSECURITY_DESCRIPTOR)HeapAlloc(
					GetProcessHeap(),
					HEAP_ZERO_MEMORY,
					dwSdSizeNeeded);

				if (psd == NULL)
					__leave;

				psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(
					GetProcessHeap(),
					HEAP_ZERO_MEMORY,
					dwSdSizeNeeded);

				if (psdNew == NULL)
					__leave;

				dwSidSize = dwSdSizeNeeded;
				if (!GetUserObjectSecurity(
					hwinsta,
					&si,
					psd,
					dwSidSize,
					&dwSdSizeNeeded)
					)
					__leave;
			}
			else
				__leave;

		// Create a new DACL.  
		if (!InitializeSecurityDescriptor(
			psdNew,
			SECURITY_DESCRIPTOR_REVISION)
			)
			__leave;

		// Get the DACL from the security descriptor.  
		
		if (!GetSecurityDescriptorDacl(
			psd,
			&bDaclPresent,
			&pacl,
			&bDaclExist)
			)
			__leave;

		// Initialize the ACL.  

		ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
		aclSizeInfo.AclBytesInUse = sizeof(ACL);

		// Call only if the DACL is not NULL.  
		if (pacl != NULL)
		{
			// get the file ACL size info  
			if (!GetAclInformation(
				pacl,
				(LPVOID)&aclSizeInfo,
				sizeof(ACL_SIZE_INFORMATION),
				AclSizeInformation)
				)
				__leave;
		}

		// Compute the size of the new ACL.  
		dwNewAclSize = aclSizeInfo.AclBytesInUse +
			(2 * sizeof(ACCESS_ALLOWED_ACE)) + (2 * GetLengthSid(psid)) - (2 * sizeof(DWORD));
		
		// Allocate memory for the new ACL.  
		pNewAcl = (PACL)HeapAlloc(
			GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			dwNewAclSize);

		if (pNewAcl == NULL)
			__leave;

		// Initialize the new DACL.  
		if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
			__leave;

		// If DACL is present, copy it to a new DACL.  
		if (bDaclPresent)
		{
			// Copy the ACEs to the new ACL.  
			if (aclSizeInfo.AceCount)
			{
				for (i = 0; i < aclSizeInfo.AceCount; i++)
				{
					// Get an ACE.  
					if (!GetAce(pacl, i, &pTempAce))
						__leave;

					// Add the ACE to the new ACL.  
					if (!AddAce(
						pNewAcl,
						ACL_REVISION,
						MAXDWORD,
						pTempAce,
						((PACE_HEADER)pTempAce)->AceSize)
						)
						__leave;
				}
			}
		}

		// Add the first ACE to the window station.  
		pace = (ACCESS_ALLOWED_ACE *)HeapAlloc(
			GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid) -
			sizeof(DWORD));

		if (pace == NULL)
			__leave;

		pace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
		pace->Header.AceFlags = CONTAINER_INHERIT_ACE |
			INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
		pace->Header.AceSize = sizeof(ACCESS_ALLOWED_ACE) +
			GetLengthSid(psid) - sizeof(DWORD);
		pace->Mask = GENERIC_ACCESS;

		if (!CopySid(GetLengthSid(psid), &pace->SidStart, psid))
			__leave;

		if (!AddAce(
			pNewAcl,
			ACL_REVISION,
			MAXDWORD,
			(LPVOID)pace,
			pace->Header.AceSize)
			)
			__leave;
		// Add the second ACE to the window station.  
		pace->Header.AceFlags = NO_PROPAGATE_INHERIT_ACE;
		pace->Mask = WINSTA_ALL;

		if (!AddAce(
			pNewAcl,
			ACL_REVISION,
			MAXDWORD,
			(LPVOID)pace,
			pace->Header.AceSize)
			)
			__leave;

		// Set a new DACL for the security descriptor.  
		if (!SetSecurityDescriptorDacl(
			psdNew,
			TRUE,
			pNewAcl,
			FALSE)
			)
			__leave;

		// Set the new security descriptor for the window station.  
		if (!SetUserObjectSecurity(hwinsta, &si, psdNew))
			__leave;

		// Indicate success.  

		bSuccess = TRUE;
	}

	__finally
	{
		// Free the allocated buffers.  
		if (pace != NULL)
			HeapFree(GetProcessHeap(), 0, (LPVOID)pace);
		if (pNewAcl != NULL)
			HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);
		if (psd != NULL)
			HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
		if (psdNew != NULL)
			HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
	}
	
	return bSuccess;
}

BOOL AddAceToDesktop(HDESK hdesk, PSID psid)
{
	ACL_SIZE_INFORMATION aclSizeInfo;
	BOOL                 bDaclExist;
	BOOL                 bDaclPresent;
	BOOL                 bSuccess = FALSE;
	DWORD                dwNewAclSize;
	DWORD                dwSidSize = 0;
	DWORD                dwSdSizeNeeded;
	PACL                 pacl;
	PACL                 pNewAcl;
	PSECURITY_DESCRIPTOR psd = NULL;
	PSECURITY_DESCRIPTOR psdNew = NULL;
	PVOID                pTempAce;
	SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
	unsigned int         i;

	__try
	{
		// Obtain the security descriptor for the desktop object. 
		if (!GetUserObjectSecurity(
			hdesk,
			&si,
			psd,
			dwSidSize,
			&dwSdSizeNeeded))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				psd = (PSECURITY_DESCRIPTOR)HeapAlloc(
					GetProcessHeap(),
					HEAP_ZERO_MEMORY,
					dwSdSizeNeeded);

				if (psd == NULL)
					__leave;

				psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(
					GetProcessHeap(),
					HEAP_ZERO_MEMORY,
					dwSdSizeNeeded);

				if (psdNew == NULL)
					__leave;

				dwSidSize = dwSdSizeNeeded;

				if (!GetUserObjectSecurity(
					hdesk,
					&si,
					psd,
					dwSidSize,
					&dwSdSizeNeeded)
					)
					__leave;
			}
			else
				__leave;
		}

		// Create a new security descriptor.  
		if (!InitializeSecurityDescriptor(
			psdNew,
			SECURITY_DESCRIPTOR_REVISION)
			)
			__leave;

		// Obtain the DACL from the security descriptor.  
		if (!GetSecurityDescriptorDacl(
			psd,
			&bDaclPresent,
			&pacl,
			&bDaclExist)
			)
			__leave;

		// Initialize.  
		ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
		aclSizeInfo.AclBytesInUse = sizeof(ACL);

		// Call only if NULL DACL.  
		if (pacl != NULL)
		{
			// Determine the size of the ACL information.  
			if (!GetAclInformation(
				pacl,
				(LPVOID)&aclSizeInfo,
				sizeof(ACL_SIZE_INFORMATION),
				AclSizeInformation)
				)
				__leave;
		}

		// Compute the size of the new ACL.  
		dwNewAclSize = aclSizeInfo.AclBytesInUse +
			sizeof(ACCESS_ALLOWED_ACE) +
			GetLengthSid(psid) - sizeof(DWORD);

		// Allocate buffer for the new ACL.  
		pNewAcl = (PACL)HeapAlloc(
			GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			dwNewAclSize);

		if (pNewAcl == NULL)
			__leave;

		// Initialize the new ACL.  
		if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
			__leave;

		// If DACL is present, copy it to a new DACL.  
		if (bDaclPresent)
		{
			// Copy the ACEs to the new ACL.  
			if (aclSizeInfo.AceCount)
			{
				for (i = 0; i < aclSizeInfo.AceCount; i++)
				{
					// Get an ACE.  
					if (!GetAce(pacl, i, &pTempAce))
						__leave;

					// Add the ACE to the new ACL.  
					if (!AddAce(
						pNewAcl,
						ACL_REVISION,
						MAXDWORD,
						pTempAce,
						((PACE_HEADER)pTempAce)->AceSize)
						)
						__leave;
				}
			}
		}

		// Add ACE to the DACL.  
		if (!AddAccessAllowedAce(
			pNewAcl,
			ACL_REVISION,
			DESKTOP_ALL,
			psid)
			)
			__leave;

		// Set new DACL to the new security descriptor.  
		if (!SetSecurityDescriptorDacl(
			psdNew,
			TRUE,
			pNewAcl,
			FALSE)
			)
			__leave;



		// Set the new security descriptor for the desktop object.  
		if (!SetUserObjectSecurity(hdesk, &si, psdNew))
			__leave;
		// Indicate success.  
		bSuccess = TRUE;
	}
	__finally
	{
		// Free buffers.  
		if (pNewAcl != NULL)
			HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);
		if (psd != NULL)
			HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
		if (psdNew != NULL)
			HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
	}

	return bSuccess;
}


#endif

//////////////////////////////////////////////////
#if 0
#pragma comment(lib, "wtsapi32")  
#pragma comment(lib, "Userenv")

void StartProcess(LPTSTR   lpImage)
{
	// 为了显示更加复杂的用户界面，我们需要从Session 0创建 
	// 一个进程，但是这个进程是运行在用户环境下。 
	// 我们可以使用CreateProcessAsUser实现这一功能。 

	BOOL bSuccess = FALSE;
	STARTUPINFO si = { 0 };
	// 进程信息 
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);

	// 获得当前Session ID 
	DWORD dwSessionID = WTSGetActiveConsoleSessionId();

	HANDLE hToken = NULL;
	// 获得当前Session的用户令牌 
	if (WTSQueryUserToken(dwSessionID, &hToken) == FALSE)
	{
		goto Cleanup;
	}

	// 复制令牌 
	HANDLE hDuplicatedToken = NULL;
	if (DuplicateTokenEx(hToken,
		MAXIMUM_ALLOWED, NULL,
		SecurityIdentification, TokenPrimary,
		&hDuplicatedToken) == FALSE)
	{
		goto Cleanup;
	}

	// 创建用户Session环境 
	LPVOID lpEnvironment = NULL;
	if (CreateEnvironmentBlock(&lpEnvironment,
		hDuplicatedToken, FALSE) == FALSE)
	{
		goto Cleanup;
	}

	// 在复制的用户Session下执行应用程序，创建进程。 
	// 通过这个进程，就可以显示各种复杂的用户界面了 
	if (CreateProcessAsUser(hDuplicatedToken,
		lpImage, NULL, NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
		lpEnvironment, NULL, &si, &pi) == FALSE)
	{
		goto Cleanup;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	bSuccess = TRUE;

	// 清理工作 

Cleanup:
	/*if (!bSuccess)
	{
	ShowMessage(L"无法创建复杂UI", L"错误");
	} */

	if (hToken != NULL)
		CloseHandle(hToken);
	if (hDuplicatedToken != NULL)
		CloseHandle(hDuplicatedToken);
	if (lpEnvironment != NULL)
		DestroyEnvironmentBlock(lpEnvironment);
}

#endif

#if 0
DWORD __stdcall INTER_GetExplorerToken(OUT PHANDLE  phExplorerToken)
{
	DWORD       dwStatus = ERROR_FILE_NOT_FOUND;
	BOOL        bRet = FALSE;
	HANDLE      hProcess = NULL;
	HANDLE      hProcessSnap = NULL;
	char        szExplorerPath[MAX_PATH] = { 0 };
	char        FileName[MAX_PATH] = { 0 };
	PROCESSENTRY32 pe32 = { 0 };

	USES_CONVERSION;
	__try
	{
		GetWindowsDirectory(A2T(szExplorerPath), MAX_PATH);
		strcat(szExplorerPath, "explorer.exe");	////Explorer.EXE
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
			dwStatus = GetLastError();
			__leave;
		}
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hProcessSnap, &pe32))
		{
			dwStatus = GetLastError();
			__leave;
		}
		do {
			hProcess = OpenProcess(
				PROCESS_ALL_ACCESS,
				FALSE,
				pe32.th32ProcessID);
			if (NULL != hProcess)
			{
			#if 1
				if (!strcmpi(T2A(pe32.szExeFile), szExplorerPath))
				{
					HANDLE  hToken;
					if (OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
					{
						*phExplorerToken = hToken;
						dwStatus = 0;
					}
					break;
				}
			#else
				DWORD(__stdcall *GMFNE)(HANDLE hProcess,
										HMODULE hModule,
										LPTSTR lpFilename,
										DWORD nSize);
				HMODULE hPsapi = LoadLibrary(_T("PSAPI"));
				if (!hPsapi)
				{
					dwStatus = GetLastError();
					break;
				}
				GMFNE = (DWORD(__stdcall *) (HANDLE, HMODULE, LPTSTR, DWORD))GetProcAddress(hPsapi, "GetModuleFileNameExW");//GetModuleFileNameExA
				if (GMFNE(hProcess, NULL, A2T(FileName), MAX_PATH))
				{
					if (!strcmpi(FileName, szExplorerPath))
					{
						HANDLE  hToken;
						if (OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
						{
							*phExplorerToken = hToken;
							dwStatus = 0;
						}
						break;
					}
				}
				CloseHandle(hProcess);
				hProcess = NULL;
			#endif
			}

		} while (Process32Next(hProcessSnap, &pe32));
	}
	__finally
	{
		if (NULL != hProcess)
		{
			CloseHandle(hProcess);
		}
		if (NULL != hProcessSnap)
		{
			CloseHandle(hProcessSnap);
		}
	}
	return dwStatus;
}
#endif

HANDLE GetToken(DWORD dwProcessID)
{
	HANDLE hProcess = NULL;
	HANDLE hToken = NULL;
	HANDLE g_hToken = NULL;

	PSECURITY_DESCRIPTOR pSD = NULL;

	try {
		// Get the handle to the process
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);
		if (hProcess == NULL) 	return NULL;

		// Get the token (All access so we can change and launch things
		if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
		{
			CloseHandle(hProcess);
			return NULL;
		}

		// Get memory for an SD
		pSD = (PSECURITY_DESCRIPTOR)GlobalAlloc(GPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (pSD == NULL)
		{
			CloseHandle(hToken);
			CloseHandle(hProcess);
			return NULL;
		}

		// Initialize it
		if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
		{
			GlobalFree(pSD);
			CloseHandle(hToken);
			CloseHandle(hProcess);
			return NULL;
		}

		// Add a NULL DACL to the security descriptor..
		if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL)NULL, FALSE))
		{
			GlobalFree(pSD);
			CloseHandle(hToken);
			CloseHandle(hProcess);
			return NULL;
		}

		// We made the security descriptor just in case they want a duplicate.
		// We make the duplicate have all access to everyone.
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = pSD;
		sa.bInheritHandle = TRUE;

		// If the user chooses not to copy the token, then changes made to it
		// will effect the owning process
		//        if (IDNO == ::MessageBox(NULL, TEXT("Would you like to make a copy of ")
		//            TEXT("this process token?\n(Selecting \"No\" will cause the ")
		//            TEXT("\"AdjustToken\" and \"SetToken\"\nfeatures to affect the ")
		//            TEXT("owning process.) "), TEXT("Duplicate Token?"), MB_YESNO)) 

		// Duplicate the token
		if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, &sa,
			SecurityImpersonation, TokenPrimary, &g_hToken))
		{
			CloseHandle(hToken);
			CloseHandle(hProcess);
			GlobalFree(pSD);
			return NULL;
		}
	}
	catch (...)
	{
	}

	CloseHandle(hToken);
	CloseHandle(hProcess);
	GlobalFree(pSD);
	return g_hToken;
}

DWORD GetProcessIdByName(CString& strName)
{
	BOOL bResult;
	DWORD dwProcessID = 0;
	CString strTemp, strProcessName;
	HANDLE hSnapshot;               //内存进程的“快照”句柄      
	PROCESSENTRY32 ProcessEntry;    //描述进程的结构   
	//输入要结束的进程名称    
	strProcessName = strName/*m_strProcessName*/;
	strProcessName.MakeLower();
	//返回内存所有进程的快照。参数为TH32CS_SNAPPROCESS取有的进程,忽略参数2；    
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	//获取要的进程名称对应的所有进程ID    
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	bResult = Process32First(hSnapshot, &ProcessEntry);//获取第一个进程 
	int processcount = 0;
	while (bResult)
	{
		//判断是否为要结束的进程    
		strTemp.Format(_T("%s"), ProcessEntry.szExeFile);
		strTemp.MakeLower();
		if (strTemp == strProcessName)
		{
			dwProcessID = ProcessEntry.th32ProcessID;
			processcount++;
			return dwProcessID;
		}
		//获取下一个进程    
		bResult = Process32Next(hSnapshot, &ProcessEntry);
	}
	return dwProcessID;
}

BOOL   RunProcess(LPTSTR   lpImage)
{
	BOOL bResult = FALSE;
	if (!lpImage)
	{
		return   FALSE;
	}
	//RaisePrivleges();
	CString strName = _T("explorer.exe");
	DWORD dwPID = GetProcessIdByName(strName);
	if (dwPID != 0)
	{
		HANDLE hToken = GetToken(dwPID);
		if (hToken != NULL)
		{
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);

			TCHAR szFilePath[MAX_PATH];
			::GetModuleFileName(NULL, szFilePath, MAX_PATH);
			*_tcsrchr(szFilePath, _T('\\')) = 0;
			_tcscat_s(szFilePath, MAX_PATH, _T("\\setup.exe"));

			// Create a new process with our current token
			PROCESS_INFORMATION pi;
			if (CreateProcessAsUser(hToken, NULL, szFilePath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
			{
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
			else
			{
				int nErrorID = GetLastError();
				TRACE("CreateProcessAsUser error = %d\n", nErrorID);
			}
			CloseHandle(hToken);
			bResult = TRUE;
		}
	}
	return bResult;
}


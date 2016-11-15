#include "stdafx.h"
#include "GPDef.h"
#include <io.h>

#include <Tlhelp32.h>

struct sockaddr_in local;
struct sockaddr_in serverAddr;

BOOL StartUp()
{
	//д��ע���,���������� 
	HKEY hKey;
	//�ҵ�ϵͳ�������� 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
	//��������Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey);
	if (lRet == ERROR_SUCCESS)
	{
		TCHAR pFileName[MAX_PATH] = { 0 };
		//�õ����������ȫ·�� 
		DWORD dwRet = GetModuleFileNameW(NULL, pFileName, MAX_PATH);

		lRet = RegSetValueEx(hKey, _T("GuardProcess"), 0, REG_SZ, (BYTE *)(pFileName), MAX_PATH);

		RegCloseKey(hKey);
		if (lRet != ERROR_SUCCESS)
		{
			TRACE("ϵͳ��������,������ϵͳ����");
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

BOOL InitNetWork(BOOL bSender /*= FALSE*/)
{
	WSADATA wsd;

	//��ʼ��WinSock2.2
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

	//��sock�󶨵�����ĳ�˿��ϡ�
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
		TRACE("���ӷ�����%s:%dʧ��\n", T2A(strIP), nPort);
		return FALSE;
	}
	TRACE("���ӷ�����%s:%d�ɹ�\n", T2A(strIP), nPort);

	return TRUE;
}

BOOL GetVerServerAddr()
{
	//��ȡ�汾���Ʒ�������ַ
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
		TRACE("�汾��������ַ��Ϣ��%s\n", p);
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

	TRACE("��ȡ�汾��������ַ��Ϣ�ɹ�\n");

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

	TRACE("��ʼ��ȡ�����ļ��б�����\n");
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
		}
		else
		{
			CString strName = ff.GetFileName();
			if (strName.Find(_T(".Log")) != -1)
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

	TRACE("��ȡ�����ļ��б�ɹ�\n");
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

	TRACE("��ʼ��ȡ�汾�������ļ��б�����\n");
	//�Ӱ汾���Ʒ�������ȡ�б�
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
			TRACE("��ȡ�汾�������ļ��б�ɹ�\n");
		}
		else
			TRACE("�ļ��б�Ϊ�գ�����Ҫ����");
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
			if (pFileInfo->strMd5 != pLocalFile->strMd5)
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
	TRACE("���������ļ��б����\n");
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
					TRACE("�Ƴ��Ѵ����ļ�: %s\n", T2A(strFilePath));
				}
			}
			catch (...)
			{
				TRACE("�Ƴ��ļ�(%s)�쳣\n", strFilePath);
				SAFE_RELEASE_ARRY(pData);
				g_bConnect = FALSE;
				closesocket(g_sock);
				g_sock = INVALID_SOCKET;
				return FALSE;
			}

			ofstream out(strFilePath, std::ios::binary);
			if (!out)
			{
				TRACE("���ļ����ж�д(%s)ʧ��\n", T2A(strFilePath));
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

			TRACE("�ļ�������ɣ�%s\n", T2A(strFilePath));
			SAFE_RELEASE_ARRY(pData);
		}
		else
		{
			if (pHeader->usResult == RESULT_ERROR_SEND)
				TRACE("�����������ļ�ʱ���ͳ���\n");
			else if (pHeader->usResult == RESULT_ERROR_FILEIO)
				TRACE("��������ȡ��Ҫ���͵��ļ�ʱ������д����\n");
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
	try
	{
		if (_access(T2A(strFilePath), 0) == 0)
		{
			remove(T2A(strFilePath));
			TRACE("�Ƴ��汾����ļ���: %s\n", T2A(strFilePath));
		}
	}
	catch (...)
	{
		TRACE("�Ƴ��汾����ļ���(%s)�쳣\n", strFilePath);
	}

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
		if (nFailTimes >= 10)		//ͬһ�ļ����س���10��
		{
			bUpdateError = TRUE;
			TRACE("�����ļ�%sʧ�ܴ�������10�Σ��˳�����\n", strDLName.c_str());
			break;
		}
	}
	if (bUpdateError)
	{
		TRACE("���������°汾�ļ�ʧ��\n");
		return FALSE;
	}
	return TRUE;
}

BOOL SendUpdataResult(std::string& strResult)
{
	//�Ӱ汾���Ʒ�������ȡ�б�
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
	int nConnectFailTimes = 0;		//���Ӱ汾���Ʒ�����ʧ�ܴ����������������10�Σ���������汾���Ʒ�������ַ
	BOOL bGetVerServerAddr = FALSE;

	CString strServerIP = g_strInitServerIP;
	int nServerPort = g_nInitServerPort;
	while (bContinue)
	{
		if (nConnectFailTimes >= 4)
		{
			//���¶�ȡ�����ļ�����ͷ��ʼ
			TRACE("���¶�ȡ�����ļ�����������汾��Ϣ\n");
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
				//û����Ҫ���µ��ļ�
				TRACE("û����Ҫ���µ��ļ�\n");
				std::string strResult = "û����Ҫ���µ��ļ�";
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

			bool bSetupPkg = false;		//�Ƿ��ǵ�����װ�ļ�
			if (g_DownLoadFileList.size() == 1)
			{
				std::string strDLName = g_DownLoadFileList.front();
				if (strDLName == "setup.exe")
					bSetupPkg = true;
			}

			BOOL bDownloadAllFile = DownLoadAllFile();
			if (!bDownloadAllFile)
			{
				std::string strResult = "�������а汾�ļ�ʧ��";
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
			TRACE("�°汾�ļ��������\n");
			std::string strResult = "�°汾�ļ��������";
			SendUpdataResult(strResult);

			//�滻�ļ�
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
				TRACE("�滻�ļ�ʧ��\n");
				std::string strResult = "�滻�ļ�ʧ��";
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

			TRACE("�滻�ļ��ɹ�\n");
			strResult = "�����ɹ�";
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

BOOL ReplaceFile()
{
	CString strNewVersionPath = g_strAppPath + _T("updateVersion\\");
	DWORD dwAttr = GetFileAttributes(strNewVersionPath);
	if (dwAttr == 0xFFFFFFFF)
		return FALSE;

	USES_CONVERSION;
	TRACE("��ʼ�滻����\n");
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
							TRACE("�Ƴ��Ѵ����ļ�: %s\n", T2A(strDstPath2));
						}

						BOOL bMoveResult = MoveFile(ff2.GetFilePath(), strDstPath2);
						if (!bMoveResult)
						{
							int nError = GetLastError();
							TRACE("�ƶ��ļ�ʧ��%s\n", T2A(ff2.GetFilePath()));
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
					TRACE("�ƶ��ļ���ʧ��%s\n", ff.GetFilePath());
				}
			}
		}
		else
		{
			CString strDstPath = g_strAppPath + ff.GetFileName();
			if (_access(T2A(strDstPath), 0) == 0)
			{
				remove(T2A(strDstPath));
				TRACE("�Ƴ��Ѵ����ļ�: %s\n", T2A(strDstPath));
			}

			BOOL bMoveResult = MoveFile(ff.GetFilePath(), strDstPath);
			if (!bMoveResult)
			{
				int nError = GetLastError();
				TRACE("�ƶ��ļ�ʧ��%s\n", ff.GetFilePath());
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
	HANDLE hSnapshot;               //�ڴ���̵ġ����ա����      
	PROCESSENTRY32 ProcessEntry;    //�������̵Ľṹ   
	//����Ҫ�����Ľ�������    
	strProcessName = str/*m_strProcessName*/;
	strProcessName.MakeLower();
	//�����ڴ����н��̵Ŀ��ա�����ΪTH32CS_SNAPPROCESSȡ�еĽ���,���Բ���2��    
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	//��ȡҪ�Ľ������ƶ�Ӧ�����н���ID    
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	bResult = Process32First(hSnapshot, &ProcessEntry);//��ȡ��һ������ 
	int processcount = 0;
	while (bResult)
	{
		//�ж��Ƿ�ΪҪ�����Ľ���    
		strTemp.Format(_T("%s"), ProcessEntry.szExeFile);
		strTemp.MakeLower();
		if (strTemp == strProcessName)
		{
			nProcessID = ProcessEntry.th32ProcessID;
			processcount++;
			return TRUE;
		}
		//��ȡ��һ������    
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
		bReplace = ReplaceFile();
		g_bShowUpdateMsg = TRUE;
		return TRUE;
	}
	else
	{
		//����������
		TRACE("������������\n");
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
							TRACE("��ʼ֪ͨɨ������а汾����\n");
							DWORD dwResult = SendMessage(hwnd, MSG_NOTIFY_UPDATE, NULL, NULL);
							TRACE("������Ϣ���, %d\n", dwResult);
							if (dwResult == 1)
							{
								bReplace = ReplaceFile();
								g_bShowUpdateMsg = TRUE;

								if (bReplace)
								{
									if (bSetupPkg)
										strProcessName = "setup.exe";

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
									}
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

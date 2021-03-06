#include "stdafx.h"
#include "global.h"
#include "FileUpLoad.h"
#include "Net_Cmd_Protocol.h"
#include "SendFileThread.h"

Poco::Event		g_eFileUpLoadThreadExit;
CMutex	mutexObj(FALSE, _T("mutex1"));

CFileUpLoad::CFileUpLoad(CSendFileThread& rNotify)
 :m_pITcpClient(NULL)
,m_uThreadType(0)
,m_bConnect(FALSE)
,m_bUpLoad(TRUE)
,m_bSendOK(FALSE)
,m_bReadyOK(FALSE)
, m_rNotify(rNotify)
, m_bStop(FALSE)
{
	ZeroMemory(m_szSendBuf, FILE_BUFF);
	m_hAddAnsEvent=CreateEventW(NULL,TRUE,FALSE,NULL);
	m_hSendReadyEvent=CreateEventW(NULL,TRUE,FALSE,NULL);
	m_hSendDoneEvent=CreateEventW(NULL,TRUE,FALSE,NULL);
	staticTimer = 0; //计时
}

CFileUpLoad::~CFileUpLoad(void)
{
	UnInit();
	std::string strLog = "CFileUpLoad exit.";
	g_pLogger->information(strLog);
	TRACE(strLog.c_str());
	g_eFileUpLoadThreadExit.set();
}

void CFileUpLoad::OnTcpClientNotifyReceivedData( const char* pData,int nLen )
{
	//接受到文件服务器的答案上传成功指令
	//通知主程序文件标志更改
	ST_CMD_HEADER stHead;
	memcpy(&stHead,pData,HEAD_SIZE);
	switch(stHead.usCmd)
	{
	case RESPONSE_UPLOADANS:
		ProcessUpLoadResponse(stHead.usResult == RESULT_SUCCESS);
		break;
	case NOTIFY_RECVANSWERFIN:
		ProcessUpLoadDone(stHead.usResult == RESULT_SUCCESS);
		break;
	}
}

void CFileUpLoad::OnTcpClientNotifyDisconnect( UINT uSocket )
{
	TRACE0("file server disconnect!\n");
	m_uThreadType = 1;
	m_bConnect = FALSE;
	g_bFileConnect = false;

	std::string strLog = "文件发送线程与服务器连接断开";
	g_pLogger->information(strLog);
}

BOOL CFileUpLoad::SendAnsFile(CString strFilePath, CString strFileName, void* pTask)
{
	stUpLoadAns* stAns	= new stUpLoadAns();
	stAns->strAnsName	= strFileName;
	stAns->strPath	= strFilePath;
	stAns->bUpload	= FALSE;
	stAns->pTask	= pTask;
#if 1
	mutexObj.Lock();
	m_listFile.push_back(stAns);
	mutexObj.Unlock();
#else
	m_VecAns.push_back(stAns);
#endif

	TRACE("add ans file %s\n", strFileName);
	SetEvent(m_hAddAnsEvent);
	return TRUE;
	
}

void CFileUpLoad::ThreadProcMain()
{
	USES_CONVERSION;
RESTART:
	if (m_uThreadType == 1)
	{
		while(m_bConnect == FALSE)
		{
			if (m_bStop)
				goto END;

			DWORD dwTimeLatest=0;
			DWORD dwTimeWait=0;
			dwTimeLatest = GetTickCount();
			dwTimeWait = dwTimeLatest - staticTimer;
			if (dwTimeWait > 3000)
			{
				staticTimer = dwTimeLatest;

				if (m_pITcpClient)
				{
					m_pITcpClient->ReleaseConnections();
					m_pITcpClient = NULL;
				}

				m_pITcpClient = CreateTcpClient(*this, T2A(m_strAddr), m_usPort);
				if (m_pITcpClient == NULL)
				{
					//TRACE0("\nconect to File Server failed!\n");
					g_bFileConnect = false;
					Sleep(100);
					continue;
				}
				else
				{
					TRACE0("\nconect to File Server Success!\n");
					m_bConnect = TRUE;
					m_uThreadType = 2;
					g_bFileConnect = true;
					goto RESTART;
				}
			}
			else
				Sleep(300);
		}
	}
	else if (m_uThreadType == 2)
	{
		while (m_bUpLoad)
		{
			if (!m_bConnect)
			{
				m_uThreadType = 1;
				g_bFileConnect = false;
				goto RESTART;
			}
			bool bFindTask = false;
			std::list<stUpLoadAns*>::iterator it = m_listFile.begin();
			for (; it != m_listFile.end(); it++)
			{
				if (m_bStop)
					goto END;

				stUpLoadAns* pTask = *it;

				if (pTask->bUpload == FALSE)
				{
					if (_access(T2A(pTask->strPath), 0) != 0)
					{
						continue;
					}
					bFindTask = true;
#if 1
					char	*szFileBuff = NULL;
					std::string strAnsName = T2A(pTask->strAnsName);
					DWORD Length = 0;
					try
					{
						CFile MyFileSend(pTask->strPath, CFile::modeRead);
						Length = MyFileSend.GetLength();
						
						szFileBuff = new char[Length];

						MyFileSend.Seek(0, CFile::begin);
						MyFileSend.Read(szFileBuff, Length);
						MyFileSend.Close();
					}
					catch (CMemoryException* e)
					{
						char szLog[100] = { 0 };
						sprintf_s(szLog, "上传文件(%s)时内存申请失败，需要重新尝试。", strAnsName.c_str());
						g_pLogger->information(szLog);
						Sleep(500);
						continue;
					}
					catch (CFileException* e)
					{
						CString strErr = _T("");
						UINT uErr;
						e->GetErrorMessage((LPTSTR)(LPCTSTR)strErr, uErr);
						char szLog[300] = { 0 };
						sprintf_s(szLog, "上传文件(%s)时发生文件异常，需要重新尝试。%s", strAnsName.c_str(), strErr);
						g_pLogger->information(szLog);
						Sleep(500);
						continue;
					}
					catch (...)
					{
						char szLog[100] = { 0 };
						sprintf_s(szLog, "上传文件(%s)时发生异常，需要重新尝试。", strAnsName.c_str());
						g_pLogger->information(szLog);
						Sleep(500);
						continue;
					}
					
#else
					CFile MyFileSend(pTask->strPath, CFile::modeRead);
					DWORD Length = MyFileSend.GetLength();
					char	*szFileBuff = NULL;
					std::string strAnsName = T2A(pTask->strAnsName);
					try
					{
						szFileBuff = new char[Length];
					}
					catch (...)
					{
						char szLog[100] = { 0 };
						sprintf_s(szLog, "上传文件(%s)时内存申请失败，需要重新尝试。", strAnsName);
						g_pLogger->information(szLog);
						Sleep(500);
						continue;
					}
					
					MyFileSend.Seek(0, CFile::begin);
					MyFileSend.Read(szFileBuff, Length);
					MyFileSend.Close();
#endif
					TRACE0("start send ans file\n");
					char szLog[300] = { 0 };
					sprintf_s(szLog, "开始上传文件: %s", T2A(pTask->strAnsName));
					g_pLogger->information(szLog);
					//上传文件
					ST_CMD_HEADER stHead;
					stHead.usCmd = REQUEST_UPLOADANS;
					stHead.uPackSize = sizeof(ST_FILE_INFO)+Length;
					ST_FILE_INFO stAnsInfo;
					strcpy(stAnsInfo.szFileName, T2A(pTask->strAnsName));

					char *pMd5 = MD5File(T2A(pTask->strPath));
					memcpy(stAnsInfo.szMD5, pMd5, LEN_MD5);
					stAnsInfo.dwFileLen = Length;
					memcpy(m_szSendBuf, &stHead, HEAD_SIZE);
					memcpy(m_szSendBuf + HEAD_SIZE, &stAnsInfo, sizeof(ST_FILE_INFO));
				RESENDHEAD:
					ResetEvent(m_hSendReadyEvent);
					ResetEvent(m_hSendDoneEvent);
					if (!m_bConnect)
					{
						delete szFileBuff;
						m_uThreadType = 1;
						g_bFileConnect = false;
						break;
					}
					if (m_bStop)
					{
						sprintf_s(szLog, "发送文件(%s)时检测到系统退出标志，停止发送数据", T2A(pTask->strAnsName));
						g_pLogger->information(szLog);
						(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = 0;
						delete szFileBuff;
						break;
					}
					if (!sendData(m_szSendBuf, HEAD_SIZE + sizeof(ST_FILE_INFO), pTask))
					{
						std::string strLog = "sendData失败，需要重连";
						g_pLogger->information(strLog);

						(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = 0;
						delete szFileBuff;

						m_bConnect = FALSE;
						break;
					}
//					DWORD dwResult1 = WaitForSingleObject(m_hSendReadyEvent, INFINITE);
					bool bRecvResult = false;	//接收服务器是否接收完成数据的结果命令
					DWORD dwResult1 = WaitForSingleObject(m_hSendReadyEvent, 5 * 1000);
					switch (dwResult1)
					{
						case WAIT_OBJECT_0:
							bRecvResult = true;
							break;
						case WAIT_TIMEOUT:
							sprintf_s(szLog, "文件头数据发送完成，接收服务器回复命令失败(%s)失败(5s超时), 需要重传", T2A(pTask->strAnsName));
							g_pLogger->information(szLog);
							break;
					}
					if (!bRecvResult)
					{
						(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = 0;
						m_bConnect = FALSE;
						goto RESENDHEAD;
					}
					ResetEvent(m_hSendReadyEvent);
					if (m_bReadyOK == FALSE)
					{
						sprintf_s(szLog, "上传文件(%s)的反馈信息失败，重传", T2A(pTask->strAnsName));
						g_pLogger->information(szLog);

						(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = 0;
						m_bConnect = FALSE;
						goto RESENDHEAD;
					}
					if (!sendData(szFileBuff, Length, pTask))
					{
						std::string strLog = "sendData失败2，需要重连";
						g_pLogger->information(strLog);

						(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = 0;
						delete szFileBuff;

						m_bConnect = FALSE;
						break;
					}
//					DWORD dwResult = WaitForSingleObject(m_hSendDoneEvent, INFINITE);
					bRecvResult = false;	//接收服务器是否接收完成数据的结果命令
					DWORD dwResult = WaitForSingleObject(m_hSendDoneEvent, 10 * 1000);
					switch (dwResult)
					{
						case WAIT_OBJECT_0:
							bRecvResult = true;
							break;
						case WAIT_TIMEOUT:
							sprintf_s(szLog, "文件数据发送完成，接收服务器回复命令失败(%s)失败(10s超时)", T2A(pTask->strAnsName));
							g_pLogger->information(szLog);
							break;
					}
					if (!bRecvResult)
					{
						(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = 0;
						m_bConnect = FALSE;
						goto RESENDHEAD;
					}
					ResetEvent(m_hSendDoneEvent);
					if (m_bSendOK == FALSE)
					{
						sprintf_s(szLog, "上传文件(%s)失败，重传", T2A(pTask->strAnsName));
						g_pLogger->information(szLog);

						(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = 0;
						m_bConnect = FALSE;
						goto RESENDHEAD;
					}
					pTask->bUpload = TRUE;

					(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = 100.0;
					(reinterpret_cast<pSENDTASK>(pTask->pTask))->nSendState = 2;

					delete szFileBuff;
					delete pMd5;
					TRACE0("end send ans file\n");
					m_rNotify.SendFileComplete(stAnsInfo.szFileName, T2A(pTask->strPath));
				}
			}
			if (!bFindTask)	Sleep(1000);
		}
	}
END:
	return;
}

BOOL CFileUpLoad::InitUpLoadTcp(CString strAddr,USHORT usPort)
{
	m_strAddr = strAddr;
	m_usPort  = usPort;
	m_uThreadType=1;
	return CThread::StartThread();
}

bool CFileUpLoad::sendData( char * szBuff, DWORD nLen, stUpLoadAns* pTask)
{
	ULONG uOffset = 0;
	if(m_pITcpClient == NULL || m_bConnect == FALSE)
		return false;

	bool bResult = true;
	while (uOffset < nLen && !m_bStop)
	{
		if ((nLen-uOffset)<TCP_PACKET_MAXSIZE)
		{
			int nSendLen = m_pITcpClient->SendData(szBuff + uOffset, nLen-uOffset);
			if (nSendLen < 0)
			{
				bResult = false;
				break;
			}
			uOffset=nLen;

			(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = (float)uOffset / nLen * 100;
		}
		else
		{
			int nSendLen = m_pITcpClient->SendData(szBuff + uOffset,TCP_PACKET_MAXSIZE);
			if (nSendLen < 0)
			{
				bResult = false;
				break;
			}
			uOffset+=TCP_PACKET_MAXSIZE;

			(reinterpret_cast<pSENDTASK>(pTask->pTask))->fSendPercent = (float)uOffset / nLen * 100;
		}
	}
	if (m_bStop)
		return false;

	return bResult;
}

//反馈结果成功则记录，失败则重发
void CFileUpLoad::ProcessUpLoadResponse(BOOL bFlag )
{
	m_bReadyOK=bFlag;
	SetEvent(m_hSendReadyEvent);
}

BOOL CFileUpLoad::CheckUpLoadFile()
{
	if (m_VecAns.size() < 5)
	{
		return FALSE;
	}
	std::vector<stUpLoadAns*>::iterator it = m_VecAns.begin();
	for (;it != m_VecAns.end();it++)
	{
		stUpLoadAns *stAns = (stUpLoadAns*)*it;
		if (stAns->bUpload == FALSE)
		{
			return FALSE;
		}
	}
	return TRUE;
}

void CFileUpLoad::ProcessUpLoadDone( BOOL bFlag )
{	
	m_bSendOK = bFlag;
	SetEvent(m_hSendDoneEvent);
}

void CFileUpLoad::UnInit()
{
	m_bStop = TRUE;
	m_bConnect=FALSE;
	g_bFileConnect = false;
	m_bUpLoad = FALSE;

	g_pLogger->information("CFileUpLoad UnInit==>WaitForStop().");
	WaitForStop();
	g_pLogger->information("CFileUpLoad UnInit==>WaitForStop() completed.");

	if (m_pITcpClient)
	{
		m_pITcpClient->ReleaseConnections();
		m_pITcpClient=NULL;
	}

	mutexObj.Lock();
	std::list<stUpLoadAns*>::iterator it1 = m_listFile.begin();
	for (; it1 != m_listFile.end(); )
	{
		stUpLoadAns* itUp = *it1;
		it1 = m_listFile.erase(it1);
		SAFE_RELEASE(itUp);
	}
	m_listFile.clear();
	mutexObj.Unlock();
	// 	m_VecAns.clear();
	std::vector<stUpLoadAns*>::iterator it = m_VecAns.begin();
	for (;it!=m_VecAns.end();it++)
	{
		if (*it)
		{
			delete (*it);
			(*it)=NULL;
		}

	}
}

void CFileUpLoad::ReConnectAddr(CString strAddr, USHORT usPort)
{
	m_strAddr = strAddr;
	m_usPort = usPort;
	m_uThreadType = 1;
	m_bConnect = FALSE;
}

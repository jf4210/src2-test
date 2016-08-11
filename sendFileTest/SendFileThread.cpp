#include "stdafx.h"
#include "SendFileThread.h"
#include "MyCodeConvert.h"
#include "Net_Cmd_Protocol.h"

CSendFileThread::CSendFileThread(std::string& strIP, int nPort)
	: _strIp(strIP), _nPort(nPort), m_upLoad(*this), m_pRecvBuff(NULL), _bConnect(false)
{
	connectServer();
}


CSendFileThread::~CSendFileThread()
{
	if (m_pRecvBuff)
	{
		delete m_pRecvBuff;
		m_pRecvBuff = NULL;
	}
}

void CSendFileThread::run()
{
	USES_CONVERSION;
//	m_upLoad.InitUpLoadTcp(A2T(_strIp.c_str()), _nPort);

	while (!g_nExitFlag)
	{
		pSENDTASK pTask = NULL;
		g_fmSendLock.lock();
		SENDTASKLIST::iterator it = g_lSendTask.begin();
		for (; it != g_lSendTask.end();)
		{
			pTask = *it;
			it = g_lSendTask.erase(it);
			break;
		}
		g_fmSendLock.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(100);
			continue;
		}

// 		try
// 		{
// 			int nThreadId = Poco::Thread::currentTid();
// 
// 			Poco::Path filePath(CMyCodeConvert::Gb2312ToUtf8(pTask->strPath));
// 			std::string strPath = filePath.toString();
// 
// 			int nPos = strPath.rfind('.');
// 			strPath = strPath.substr(0, nPos);
// 			std::string strNewFilePath = Poco::format("%s_%d.%s", strPath, nThreadId, filePath.getExtension());
// 
// 			Poco::File fileTask(CMyCodeConvert::Gb2312ToUtf8(pTask->strPath));
// 			fileTask.copyTo(strNewFilePath);
// 
// 			std::string strNewName = Poco::format("%s_%d.%s", filePath.getBaseName(), nThreadId, filePath.getExtension());
// 			pTask->strPath = strNewFilePath;
// 			pTask->strName = strNewName;
// 		}
// 		catch (Poco::Exception& exc)
// 		{
// 			std::string strLog = "文件复制异常: " + CMyCodeConvert::Utf8ToGb2312(exc.displayText()) + "\n";
// 			TRACE(strLog.c_str());
// 		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}
}

void CSendFileThread::HandleTask(pSENDTASK pTask)
{
#if 1
	bool bSendOK = false;
	while (!g_nExitFlag && !bSendOK && pTask->nSendTimes <= 3)
	{
		while (!_bConnect)
		{
			if (!connectServer())
				Poco::Thread::sleep(1000);
		}

		Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(pTask->strPath));
		int nLen = fileList.getSize();

		ST_CMD_HEADER stHead;
		stHead.usCmd = REQUEST_UPLOADANS;
		stHead.uPackSize = sizeof(ST_FILE_INFO) + nLen;
		ST_FILE_INFO stFileInfo;
		USES_CONVERSION;
		strcpy(stFileInfo.szFileName, pTask->strName.c_str());

		char *pMd5 = MD5File((char*)pTask->strPath.c_str());
		memcpy(stFileInfo.szMD5, pMd5, LEN_MD5);
		stFileInfo.dwFileLen = nLen;

		char szSendBuf[1024] = { 0 };
		memcpy(szSendBuf, &stHead, HEAD_SIZE);
		memcpy(szSendBuf + HEAD_SIZE, &stFileInfo, sizeof(ST_FILE_INFO));

		TRACE("发送文件头\n");
		if (!sendMyData(szSendBuf, HEAD_SIZE + sizeof(ST_FILE_INFO)))
		{
			pTask->nSendTimes++;
			std::string strLog = Poco::format("发送文件头失败(发送时),次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			g_pLogger->information(strLog);
			continue;
		}

		TRACE("接收发送文件命令\n");
		char szRecvBuf[1024] = { 0 };
		if (!recvMyData(szRecvBuf))
		{
			pTask->nSendTimes++;
			std::string strLog = Poco::format("接收文件头成功与否命令失败,次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			g_pLogger->information(strLog);
			continue;
		}

		if (!HandleCmd(szRecvBuf, RESPONSE_UPLOADANS))
		{
			pTask->nSendTimes++;
			std::string strLog = Poco::format("发送文件头失败(发送结果),次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			g_pLogger->information(strLog);
			continue;
		}

		TRACE("读取文件内容\n");
		std::string strFileData;
		std::ifstream fin(pTask->strPath, std::ifstream::binary);
		if (!fin)
		{
			pTask->nSendTimes++;
			std::string strLog = Poco::format("打开文件失败,次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			g_pLogger->information(strLog);
			continue;
		}
		std::stringstream buffer;
		buffer << fin.rdbuf();
		strFileData = buffer.str();
		fin.close();

		clock_t start, end;
		start = clock();

		TRACE("发送文件内容\n");
		if (!sendMyData(const_cast<char*>(strFileData.c_str()), nLen))
		{
			pTask->nSendTimes++;
			std::string strLog = Poco::format("发送文件失败(发送中),次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			g_pLogger->information(strLog);
			continue;
		}

		TRACE("接收是否发送成功命令\n");
		ZeroMemory(szRecvBuf, sizeof(szRecvBuf));
		if (!recvMyData(szRecvBuf))
		{
			pTask->nSendTimes++;
			std::string strLog = Poco::format("接收文件发送成功与否命令失败,次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			g_pLogger->information(strLog);
			continue;
		}

		if (!HandleCmd(szRecvBuf, NOTIFY_RECVANSWERFIN))
		{
			pTask->nSendTimes++;
			std::string strLog = Poco::format("发送文件失败(发送结果),次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			g_pLogger->information(strLog);
			continue;
		}

		end = clock();


		std::string strLog = Poco::format("------>发送文件成功,次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
		g_pLogger->information(strLog);
		TRACE("发送完成，时间：%.2f\n", (end - start)/1000.0);
		bSendOK = true;
	}
	
	if(bSendOK)
	{
		Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(pTask->strPath));
		if (fileList.exists())
			fileList.remove();
	}


#else
	USES_CONVERSION;
	std::string strDesPath = "E:\\myWorkspace\\yklx\\bin\\debug\\sendFileTest\\";

	Poco::File fileSrcPath(CMyCodeConvert::Gb2312ToUtf8(pTask->strPath));
	Poco::File fileDesPath(strDesPath);
	if (!fileDesPath.exists())
		fileDesPath.createDirectories();

	Poco::Random rnd;
	rnd.seed();
	char szDesFileName[50] = { 0 };
	sprintf_s(szDesFileName, "测试_%d_%05d.zip", Poco::Thread::currentTid(), rnd.next(99999));
	strDesPath.append(szDesFileName);
	fileSrcPath.copyTo(CMyCodeConvert::Gb2312ToUtf8(strDesPath));

	m_upLoad.SendAnsFile(A2T(strDesPath.c_str()), A2T(szDesFileName));

// 	if (fileDesPath.exists())
// 		fileDesPath.remove(true);

	Sleep(1000);
#endif
}

void CSendFileThread::SendFileComplete(char* pName, char* pSrcPath)
{
// 	std::string strTest = pName;
// 	std::string strLog1 = Poco::format("%c", pName);
// 	std::string strLog2 = Poco::format("%s", strTest);
// 	std::string strLog3 = Poco::format("%d", 33);
// 	std::string strLog = Poco::format("发送文件(%s)完成.", pName);

	char szLog[300] = { 0 };
	sprintf_s(szLog, "发送文件(%s)完成.\n", pName);
	TRACE(szLog);

	Poco::File fileDesPath(pSrcPath);
	if (fileDesPath.exists())
		fileDesPath.remove(true);
}

bool CSendFileThread::connectServer()
{
	Poco::Net::SocketAddress sa(_strIp, _nPort);
	m_ss.close();
	try
	{
		m_ss.connect(sa);					//重新连接服务器
// 		m_ss.setBlocking(false);
// 		m_ss.setNoDelay(true);
		_bConnect = true;
		std::string strLog = "连接服务器成功\n";
		TRACE(strLog.c_str());
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "连接服务器失败 ==> " + exc.displayText();
		//		TRACE(strLog.c_str());
		_bConnect = false;
	}
	return _bConnect;
}

bool CSendFileThread::sendMyData(char* szBuf, int nLen)
{
	bool bResult = false;
	try
	{
		int nSended = 0;
		int nWantSend = nLen;
		while (nSended < nLen)
		{
			int nSend = m_ss.sendBytes(szBuf + nSended, nWantSend);
			if (nSend == 0)
			{
				TRACE("the peer has closed.\n");
				std::string strLog = "the peer has closed";
				g_pLogger->information(strLog);
				_bConnect = false;
				if (nSended == nLen)	bResult = true;
				return bResult;
			}
			nSended += nSend;
			nWantSend -= nSend;
		}
		bResult = true;
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "发送数据异常 ==> " + exc.displayText();
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		_bConnect = false;
	}
	return bResult;
}

bool CSendFileThread::recvMyData(char* szBuf)
{
	int nCount = 0;
	int nRecvLen = 0;
	int nWantLen = 0;
	int nBaseLen = HEAD_SIZE;
	bool bResult = false;
	try
	{
		while (true)
		{
			if (nRecvLen < nBaseLen)
			{
				nWantLen = nBaseLen - nRecvLen;
				int nLen = m_ss.receiveBytes(szBuf + nRecvLen, nWantLen);
				if (nLen > 0)
				{
					nRecvLen += nLen;
					if (nRecvLen == nBaseLen)
					{
						if (nCount == 0)
						{
							ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)szBuf;
							nBaseLen += pstHead->uPackSize;
						}
						nCount++;
					}
				}
				else if (nLen == 0)
				{
					TRACE("the peer has closed.\n");
					std::string strLog = "the peer has closed";
					g_pLogger->information(strLog);
					_bConnect = false;
					return bResult;
				}
				else
					Poco::Thread::sleep(1);
			}
			else
				break;
		}
		bResult = true;
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "接收数据异常 ==> " + exc.displayText();
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		_bConnect = false;
	}
	
	return bResult;
}

bool CSendFileThread::HandleCmd(char* szBuf, unsigned short usCmd)
{
	bool bResult = false;
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)szBuf;	//m_pRecvBuff
	if (pstHead->usCmd == usCmd)
	{
		if (pstHead->usResult == RESULT_SUCCESS)
		{
			bResult = true;
		}
	}

	return bResult;
}


#include "stdafx.h"
#include "SendFileThread.h"
#include "MyCodeConvert.h"
#include "Net_Cmd_Protocol.h"

CSendFileThread::CSendFileThread(std::string& strIP, int nPort)
	: _strIp(strIP), _nPort(nPort), m_upLoad(*this), m_pRecvBuff(NULL), _bConnect(false)
{
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

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}
}

void CSendFileThread::HandleTask(pSENDTASK pTask)
{
#if 1
START:
	while (!_bConnect)
	{
		connectServer();
	}
	
	std::string strFileData;
	std::ifstream fin(pTask->strPath, std::ifstream::binary);
	if (!fin)
	{
		pTask->nSendTimes++;
		if (pTask->nSendTimes > 3)
			goto END;
		else
			goto START;
	}
	std::stringstream buffer;
	buffer << fin.rdbuf();
	strFileData = buffer.str();
	fin.close();

	int nLen = strFileData.length();


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

	if (!sendMyData(szSendBuf, HEAD_SIZE + sizeof(ST_FILE_INFO)))
	{
		pTask->nSendTimes++;
		if (pTask->nSendTimes > 3)
			goto END;
		else
			goto START;
	}

END:


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
		TRACE(strLog.c_str());
		_bConnect = false;
	}
	return bResult;
}

bool CSendFileThread::recvMyData()
{
	int nCount = 0;
	int nRecvLen = 0;
	int nWantLen = 0;
	int nBaseLen = HEAD_SIZE;
	if (!m_pRecvBuff)
	{
		m_pRecvBuff = new char[1024 + HEAD_SIZE];
	}

	while (true)
	{
		if (nRecvLen < nBaseLen)
		{
			nWantLen = nBaseLen - nRecvLen;
			int nLen = m_ss.receiveBytes(m_pRecvBuff + nRecvLen, nWantLen);
			if (nLen > 0)
			{
				nRecvLen += nLen;
				if (nRecvLen == nBaseLen)
				{
					if (nCount == 0)
					{
						ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_pRecvBuff;
						nBaseLen += pstHead->uPackSize;
					}
					nCount++;
				}
			}
			else if (nLen == 0)
			{
				TRACE("the peer has closed.\n");
				return 0;
			}
			else
				Poco::Thread::sleep(1);
		}
	}
	
}


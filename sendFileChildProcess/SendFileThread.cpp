//#include "stdafx.h"
#include "SendFileThread.h"
#include "MyCodeConvert.h"
#include "Net_Cmd_Protocol.h"

#define SAFE_RELEASE(pObj)	if(pObj){delete pObj; pObj = NULL;}

CSendFileThread::CSendFileThread(std::string& strIP, int nPort)
	: _strIp(strIP), _nPort(nPort), m_pRecvBuff(NULL), _bConnect(false)
{
	connectServer();
}


CSendFileThread::~CSendFileThread()
{
	std::string strInfo = "发送线程退出";
	_pLogger_->information(strInfo);
	if (m_pRecvBuff)
	{
		delete m_pRecvBuff;
		m_pRecvBuff = NULL;
	}
}

void CSendFileThread::run()
{
	while (!_nExitFlag_)
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

		break;
	}
	_fmCurOKLock_.lock();
	_nCurOKThreads_++;
	if (_nCurOKThreads_ == _nThreads_)
		_eTaskCompleted_.set();
	_fmCurOKLock_.unlock();
}

void CSendFileThread::HandleTask(pSENDTASK pTask)
{
	_peStartMulticast_->wait();

	std::string strInfo = "开始处理任务，文件名: " + pTask->strName;
	_pLogger_->information(strInfo);
	std::cout << strInfo << std::endl;

	bool bSendOK = false;
	while (!_nExitFlag_ && !bSendOK && pTask->nSendTimes <= 3)
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
		strcpy(stFileInfo.szFileName, pTask->strName.c_str());

		strInfo = Poco::format("calcMd5, 次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
		_pLogger_->information(strInfo);
		std::string strMd5 = calcMd5(pTask->strPath);
		memcpy(stFileInfo.szMD5, strMd5.c_str(), LEN_MD5);
		stFileInfo.dwFileLen = nLen;

		char szSendBuf[1024] = { 0 };
		memcpy(szSendBuf, &stHead, HEAD_SIZE);
		memcpy(szSendBuf + HEAD_SIZE, &stFileInfo, sizeof(ST_FILE_INFO));

//		TRACE("发送文件头\n");
		strInfo = Poco::format("发送文件头, 次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
		_pLogger_->information(strInfo);
		if (!sendMyData(szSendBuf, HEAD_SIZE + sizeof(ST_FILE_INFO)))
		{
			_bAllOK_ = false;
			pTask->nSendTimes++;
			std::string strLog = Poco::format("发送文件头失败(发送时),次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;
			continue;
		}

//		TRACE("接收发送文件命令\n");
		strInfo = Poco::format("接收发送文件命令, 次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
		_pLogger_->information(strInfo);
		char szRecvBuf[1024] = { 0 };
		if (!recvMyData(szRecvBuf))
		{
			_bAllOK_ = false;
			pTask->nSendTimes++;
			std::string strLog = Poco::format("接收文件头成功与否命令失败,次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;
			continue;
		}

		if (!HandleCmd(szRecvBuf, RESPONSE_UPLOADANS))
		{
			_bAllOK_ = false;
			pTask->nSendTimes++;
			std::string strLog = Poco::format("发送文件头失败(发送结果),次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;
			continue;
		}

//		TRACE("读取文件内容\n");
		strInfo = Poco::format("读取文件内容, 次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
		_pLogger_->information(strInfo);
//		std::string strFileData;
//		std::stringstream buffer;
		char* pFileData = new char[nLen];
		std::ifstream fin(pTask->strPath, std::ifstream::binary);
		if (!fin)
		{
			SAFE_RELEASE(pFileData);
			_bAllOK_ = false;
			pTask->nSendTimes++;
			std::string strLog = Poco::format("打开文件失败,次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;
			continue;
		}
		try
		{
			std::stringstream buffer;
			buffer << fin.rdbuf();
//			strFileData = buffer.str();
			memcpy(pFileData, buffer.str().c_str(), nLen);
			fin.close();
		}
		catch (Poco::Exception& exc)
		{
			SAFE_RELEASE(pFileData);
			_bAllOK_ = false;
			pTask->nSendTimes++;
			std::string strLog = Poco::format("读取文件内容失败,次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;
			continue;
		}
		
		clock_t start, end;
		start = clock();

//		TRACE("发送文件内容\n");
		strInfo = Poco::format("发送文件内容, 次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
		_pLogger_->information(strInfo);
		if (!sendMyData(pFileData, nLen))	//const_cast<char*>(strFileData.c_str())
		{
			SAFE_RELEASE(pFileData);
			_bAllOK_ = false;
			pTask->nSendTimes++;
			std::string strLog = Poco::format("发送文件失败(发送中),次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;
			continue;
		}

//		TRACE("接收是否发送成功命令\n");
		strInfo = Poco::format("接收是否发送成功命令, 次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
		_pLogger_->information(strInfo);
		ZeroMemory(szRecvBuf, sizeof(szRecvBuf));
		if (!recvMyData(szRecvBuf))
		{
			SAFE_RELEASE(pFileData);
			_bAllOK_ = false;
			pTask->nSendTimes++;
			std::string strLog = Poco::format("接收文件发送成功与否命令失败,次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;

//			m_ss.shutdownSend();

			continue;
		}

		if (!HandleCmd(szRecvBuf, NOTIFY_RECVANSWERFIN))
		{
			SAFE_RELEASE(pFileData);
			_bAllOK_ = false;
			pTask->nSendTimes++;
			std::string strLog = Poco::format("发送文件失败(发送结果),次数%d, 文件名:%s", pTask->nSendTimes, pTask->strName);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;
			continue;
		}

		end = clock();

		SAFE_RELEASE(pFileData);

		std::string strLog = Poco::format("------>发送文件成功,次数%d, 文件名:%s，时间: %.2fs", pTask->nSendTimes, pTask->strName, (end - start) / 1000.0);
		_pLogger_->information(strLog);
		std::cout << strLog << std::endl;
//		TRACE("发送完成，时间：%.2f\n", (end - start)/1000.0);
		bSendOK = true;
	}
	
	if(bSendOK)
	{
		Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(pTask->strPath));
		if (fileList.exists())
			fileList.remove();
	}
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
//	TRACE(szLog);

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
		std::cout << strLog << std::endl;
//		TRACE(strLog.c_str());
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
//				TRACE("the peer has closed.\n");
				std::string strLog = "the peer has closed";
				_pLogger_->information(strLog);
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
		_pLogger_->information(strLog);
//		TRACE(strLog.c_str());
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
//					TRACE("the peer has closed.\n");
					std::string strLog = "the peer has closed";
					_pLogger_->information(strLog);
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
		_pLogger_->information(strLog);
//		TRACE(strLog.c_str());
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

std::string CSendFileThread::calcMd5(std::string strFilePath)
{
	std::string strMd5;
	try
	{
		Poco::MD5Engine md5;
		Poco::DigestOutputStream dos(md5);

		std::ifstream istr(strFilePath, std::ios::binary);
		if (!istr)
		{
			string strLog = "calc MD5 failed 1: ";
			strLog.append(strFilePath);
			_pLogger_->information(strLog);
			std::cout << strLog << std::endl;
			return false;
		}
		Poco::StreamCopier::copyStream(istr, dos);
		dos.close();

		strMd5 = Poco::DigestEngine::digestToHex(md5.digest());
	}
	catch (...)
	{
		string strLog = "calc MD5 failed 3: ";
		strLog.append(strFilePath);
		_pLogger_->information(strLog);
		std::cout << strLog << std::endl;
	}
	return strMd5;
}


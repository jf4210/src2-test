#include "stdafx.h"
#include "TcpClient.h"
#include "Net_Cmd_Protocol.h"

//extern Poco::Net::StreamSocket *pSs;

CTcpClient::CTcpClient(std::string strIp, int nPort)
	: _strIP(strIp), _nPort(nPort), _bConnect(false), _nRecvLen(0), _nWantLen(0)
{
}


CTcpClient::~CTcpClient()
{
}

void CTcpClient::run()
{
	while (!g_nExitFlag)
	{
		if (!_bConnect)
		{
			connectServer();
		}

		if (!receiveData())
		{
			connectServer();
		}

		if (!_bConnect)
		{
			Poco::Thread::sleep(3000);
			continue;
		}

		pTCP_TASK pTask = NULL;
		if (g_lTcpTask.size() > 0)
		{
			g_fmTcpTaskLock.lock();
			pTask = g_lTcpTask.front();
			g_lTcpTask.pop_front();
			g_fmTcpTaskLock.unlock();
		}
		if (pTask == NULL)
		{
			Sleep(300);
			continue;
		}

		HandleTask(pTask);


		SAFE_RELEASE(pTask);
	}
}

bool CTcpClient::connectServer()
{
	Poco::Net::SocketAddress sa(_strIP, _nPort);
	m_ss.close();
	try
	{
		m_ss.connect(sa);					//重新连接服务器
		m_ss.setBlocking(false);
		m_ss.setNoDelay(true);
		_bConnect = true;
		TRACE("连接服务器成功\n"); 
		std::string strLog = "连接服务器成功\n";
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "连接服务器失败 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		_bConnect = false;
	}
	return _bConnect;
}

bool CTcpClient::receiveData()
{
	if (!_bConnect)
		return false;

	bool bGetCmd = false;
	int nCount = 0;
	int nBaseLen = HEAD_SIZE;
	try
	{
		int nLen;
		if (_nRecvLen < nBaseLen)
		{
			_nWantLen = nBaseLen - _nRecvLen;
			nLen = m_ss.receiveBytes(m_szRecvBuff + _nRecvLen, _nWantLen);
			if (nLen > 0)
			{
				_nRecvLen += nLen;
				if (_nRecvLen == nBaseLen)
				{
					ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
					if (pstHead->uPackSize == 0)
					{
						bGetCmd = true;
					}
				}
			}
			else if (nLen == 0)
			{
				TRACE("the peer has closed.\n");
				return false;
			}
		}
		else
		{
			ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
			nBaseLen += pstHead->uPackSize;
			_nWantLen = nBaseLen - _nRecvLen;
			nLen = m_ss.receiveBytes(m_szRecvBuff + _nRecvLen, _nWantLen);
			if (nLen > 0)
			{
				_nRecvLen += nLen;
				if (_nRecvLen == nBaseLen)
				{
					//完整命令接收完成
					bGetCmd = true;
				}
			}
			else if (nLen == 0)
			{
				TRACE("the peer has closed.\n");
				return false;
			}
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "接收数据异常 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		_bConnect = false;
		return false;
	}

	if (bGetCmd)
	{
		HandleCmd();
		memmove(m_szRecvBuff, m_szRecvBuff + _nRecvLen, _nRecvLen);
		_nRecvLen = 0;
		_nWantLen = 0;
//		memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
	}

	return true;
}

void CTcpClient::HandleCmd()
{
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_szRecvBuff;
	if (pstHead->usCmd == USER_RESPONSE_LOGIN)
	{
	}
	else if (pstHead->usCmd == USER_RESPONSE_EXAMINFO)
	{
		switch (pstHead->usResult)
		{
		case RESULT_EXAMINFO_SUCCESS:
			{
				char szExamData[1024 * 10] = { 0 };
				strncpy(szExamData, m_szRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				std::string strExamData = szExamData;
				std::string strUtf = CMyCodeConvert::Utf8ToGb2312(szExamData);

				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result;
				try
				{
					result = parser.parse(szExamData);
					Poco::JSON::Object::Ptr examObj = result.extract<Poco::JSON::Object::Ptr>();

					Poco::JSON::Array::Ptr arryObj = examObj->getArray("exams");

					for (int i = 0; i < arryObj->size(); i++)
					{
						Poco::JSON::Object::Ptr objExamInfo = arryObj->getObject(i);
						EXAMINFO examInfo;
						examInfo.nExamID = objExamInfo->get("id").convert<int>();
						examInfo.strExamName = CMyCodeConvert::Utf8ToGb2312(objExamInfo->get("name").convert<std::string>());

						if (!objExamInfo->isNull("examType"))
						{
							Poco::JSON::Object::Ptr objExamType = objExamInfo->getObject("examType");
							if (objExamType->has("name"))
								examInfo.strExamTypeName = CMyCodeConvert::Utf8ToGb2312(objExamType->get("name").convert<std::string>());
						}
						if (!objExamInfo->isNull("grade"))
						{
							Poco::JSON::Object::Ptr objGrade = objExamInfo->getObject("grade");
							if (objGrade->has("id"))
								examInfo.nExamGrade = objGrade->get("id").convert<int>();
							if (objGrade->has("name"))
								examInfo.strGradeName = CMyCodeConvert::Utf8ToGb2312(objGrade->get("name").convert<std::string>());
						}
						examInfo.nExamState = objExamInfo->get("state").convert<int>();

						Poco::JSON::Array::Ptr arrySubjects = objExamInfo->getArray("examSubjects");
						for (int j = 0; j < arrySubjects->size(); j++)
						{
							Poco::JSON::Object::Ptr objSubject = arrySubjects->getObject(j);
							EXAM_SUBJECT subjectInfo;
							subjectInfo.nSubjID = objSubject->get("id").convert<int>();
							subjectInfo.nSubjCode = objSubject->get("code").convert<int>();
							subjectInfo.strSubjName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("name").convert<std::string>());
							subjectInfo.strModelName = objSubject->get("scanTemplateName").convert<std::string>();
							examInfo.lSubjects.push_back(subjectInfo);
						}
						g_lExamList.push_back(examInfo);
					}
				}
				catch (Poco::JSON::JSONException& jsone)
				{
					std::string strErrInfo;
					strErrInfo.append("Error when parse json: ");
					strErrInfo.append(jsone.message() + "\tData:" + strExamData);
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());
				}
				catch (Poco::Exception& exc)
				{
					std::string strErrInfo;
					strErrInfo.append("Error: ");
					strErrInfo.append(exc.message() + "\tData:");
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());
				}
				catch (...)
				{
					std::string strErrInfo;
					strErrInfo.append("Unknown error.\tData:" + strExamData);
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());
				}
			}
			break;
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_MODELINFO)
	{
		switch (pstHead->usResult)
		{
		    case RESULT_SETMODELINFO_SEND:
		       {
				   pST_MODELINFO pstModelInfo = (pST_MODELINFO)(m_szRecvBuff + HEAD_SIZE);
				   USES_CONVERSION;
				   std::string strModelPath = T2A(g_strCurrentPath);
				   strModelPath.append("Model\\");
				   strModelPath.append(pstModelInfo->szModelName);

				   pSENDTASK pTask = new SENDTASK;
				   pTask->strFileName = pstModelInfo->szModelName;
				   pTask->strPath = strModelPath;
				   g_fmSendLock.lock();
				   g_lSendTask.push_back(pTask);
				   g_fmSendLock.unlock();

				   std::string strLog = "需要重新发送模板文件: " + strModelPath;
				   TRACE(strLog.c_str());
				   g_pLogger->information(strLog);
		       }
		       break;
			case RESULT_SETMODELINFO_NO:
			  {
				  std::string strLog = "不需要重新发送模板文件";
				  TRACE(strLog.c_str());
				  g_pLogger->information(strLog);
			  }
			  break;
		}
	}
}

void CTcpClient::HandleTask(pTCP_TASK pTask)
{
	ST_CMD_HEADER stHead;
	stHead.usCmd = pTask->usCmd;
	stHead.uPackSize = pTask->nPkgLen;

	char szSendBuf[1024 + HEAD_SIZE] = { 0 };
	memcpy(szSendBuf, (char*)&stHead, HEAD_SIZE);
	memcpy(szSendBuf + HEAD_SIZE, pTask->szSendBuf, pTask->nPkgLen);

	try
	{
		m_ss.sendBytes(szSendBuf, HEAD_SIZE + pTask->nPkgLen);
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "发送数据异常 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		_bConnect = false;
	}
}

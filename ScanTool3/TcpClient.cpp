#include "stdafx.h"
#include "TcpClient.h"
#include "Net_Cmd_Protocol.h"
#include "StudentMgr.h"

//extern Poco::Net::StreamSocket *pSs;

CTcpClient::CTcpClient(std::string strIp, int nPort)
: _strIP(strIp), _nPort(nPort), _bConnect(false), _nRecvLen(0), _nWantLen(0), m_pRecvBuff(NULL)
{
}


CTcpClient::~CTcpClient()
{
	SAFE_RELEASE_ARRY(m_pRecvBuff);
	g_pLogger->information("CTcpClient exit.");
	TRACE("CTcpClient�˳�\n");
	g_eTcpThreadExit.set();
}

void CTcpClient::run()
{
	while (!g_nExitFlag)
	{
		if (g_bCmdNeedConnect)
		{
			if (_strIP != g_strCmdIP || _nPort != g_nCmdPort)
			{
				_strIP = g_strCmdIP;
				_nPort = g_nCmdPort;
				_bConnect = false;
			}
		}
		if (!_bConnect)
		{
			connectServer();
		}
		
		if (!_bConnect)
		{
			Poco::Thread::sleep(3000);
			continue;
		}

		if (!receiveData())
		{
			connectServer();
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

	try
	{
		if (_bConnect) m_ss.shutdown();
		m_ss.close();
		TRACE("Poco::Net::StreamSocket close.\n");
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "CTcpClient socket exception: " + exc.displayText();
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
	}
	_bConnect = false;
	g_bCmdConnect = _bConnect;
}

bool CTcpClient::connectServer()
{
	Poco::Net::SocketAddress sa(_strIP, _nPort);
	m_ss.close();
	try
	{
		m_ss.connect(sa);					//�������ӷ�����
		m_ss.setBlocking(false);
		m_ss.setNoDelay(true);
		_bConnect = true;
		g_bCmdConnect = _bConnect;
		TRACE("���ӷ������ɹ�\n"); 
		std::string strLog = "���ӷ������ɹ�\n";
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "���ӷ�����ʧ�� ==> " + exc.displayText();
//		TRACE(strLog.c_str());
//		g_pLogger->information(strLog);
		_bConnect = false;
		g_bCmdConnect = _bConnect;
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

	if (!m_pRecvBuff)
	{
		m_pRecvBuff = new char[1024 + HEAD_SIZE];
		_nRecvBuffSize = 1024 + HEAD_SIZE;
	}
	try
	{
		int nLen;
		if (_nRecvLen < nBaseLen)
		{
			_nWantLen = nBaseLen - _nRecvLen;
			nLen = m_ss.receiveBytes(m_pRecvBuff + _nRecvLen, _nWantLen);
			if (nLen > 0)
			{
				_nRecvLen += nLen;
				if (_nRecvLen == nBaseLen)
				{
					ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_pRecvBuff;
					if (pstHead->uPackSize == 0)
					{
						bGetCmd = true;
					}
					else
					{
						if (pstHead->uPackSize > 1024)	//1024
						{
							char* pOld = m_pRecvBuff;
							m_pRecvBuff = new char[pstHead->uPackSize + HEAD_SIZE + 1];
							m_pRecvBuff[pstHead->uPackSize + HEAD_SIZE] = '\0';
							_nRecvBuffSize = pstHead->uPackSize + HEAD_SIZE + 1;
							memcpy(m_pRecvBuff, pOld, _nRecvLen);
							SAFE_RELEASE_ARRY(pOld);
						}
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
			ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_pRecvBuff;
			nBaseLen += pstHead->uPackSize;
			_nWantLen = nBaseLen - _nRecvLen;
			nLen = m_ss.receiveBytes(m_pRecvBuff + _nRecvLen, _nWantLen);
			if (nLen > 0)
			{
				_nRecvLen += nLen;
				if (_nRecvLen == nBaseLen)
				{
					//��������������
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
		std::string strLog = "���������쳣 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		_bConnect = false;
		g_bCmdConnect = _bConnect;
		return false;
	}

	if (bGetCmd)
	{
		HandleCmd();
//		memmove(m_pRecvBuff, m_pRecvBuff + _nRecvLen, _nRecvLen);
		ZeroMemory(m_pRecvBuff, _nRecvBuffSize);
		_nRecvLen = 0;
		_nWantLen = 0;
	}

	return true;
}

void CTcpClient::HandleCmd()
{
	ST_CMD_HEADER* pstHead = (ST_CMD_HEADER*)m_pRecvBuff;	//m_pRecvBuff
	if (pstHead->usCmd == USER_RESPONSE_LOGIN)
	{
	}
	else if (pstHead->usCmd == USER_RESPONSE_LOGOUT)
	{
		if (pstHead->usResult == RESULT_SUCCESS)
		{
			char szData[1024] = { 0 };
			strncpy(szData, m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
			std::string strUser = szData;

			std::string strLog = "�˳���¼�˺�(" + strUser + ")�ɹ�";
			g_pLogger->information(strLog);
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_EXAMINFO)
	{
		switch (pstHead->usResult)
		{
		case RESULT_EXAMINFO_SUCCESS:
		{
			char* pBuff = new char[pstHead->uPackSize + 1];
			pBuff[pstHead->uPackSize] = '\0';
			strncpy(pBuff, m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
			std::string strExamData = pBuff;
			std::string strUtf = CMyCodeConvert::Utf8ToGb2312(pBuff);

			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result;
			try
			{
				result = parser.parse(pBuff);
				Poco::JSON::Object::Ptr examObj = result.extract<Poco::JSON::Object::Ptr>();
				Poco::JSON::Object::Ptr statusObj = examObj->getObject("status");
				bool bSuccess = statusObj->get("success").convert<bool>();
				if (!bSuccess)
				{
					std::string strMsg = CMyCodeConvert::Utf8ToGb2312(statusObj->get("msg").convert<std::string>());
					std::string strLog = "��ȡ������Ϣʧ��: " + strMsg;
					g_pLogger->information(strLog);
					USES_CONVERSION;
					AfxMessageBox(A2T(strLog.c_str()));
					SAFE_RELEASE_ARRY(pBuff);
					return;
				}

				Poco::JSON::Array::Ptr arryObj = examObj->getArray("exams");
				g_lfmExamList.lock();
				for (int i = 0; i < arryObj->size(); i++)
				{
					Poco::JSON::Object::Ptr objExamInfo = arryObj->getObject(i);
					//EXAMINFO examInfo;
					pEXAMINFO pExamInfo = new EXAMINFO;
				#ifndef TO_WHTY
					pExamInfo->nExamID = objExamInfo->get("id").convert<int>();
				#else
					pExamInfo->strExamID = objExamInfo->get("id").convert<std::string>();
				#endif
					pExamInfo->strExamName = CMyCodeConvert::Utf8ToGb2312(objExamInfo->get("name").convert<std::string>());

					if (!objExamInfo->isNull("examType"))
					{
						Poco::JSON::Object::Ptr objExamType = objExamInfo->getObject("examType");
						if (objExamType->has("name"))
							pExamInfo->strExamTypeName = CMyCodeConvert::Utf8ToGb2312(objExamType->get("name").convert<std::string>());
					}
					if (!objExamInfo->isNull("grade"))
					{
						Poco::JSON::Object::Ptr objGrade = objExamInfo->getObject("grade");
						if (objGrade->has("id"))
							pExamInfo->nExamGrade = objGrade->get("id").convert<int>();
						if (objGrade->has("name"))
							pExamInfo->strGradeName = CMyCodeConvert::Utf8ToGb2312(objGrade->get("name").convert<std::string>());
					}
					pExamInfo->nExamState = objExamInfo->get("state").convert<int>();

					Poco::JSON::Array::Ptr arrySubjects = objExamInfo->getArray("examSubjects");
					for (int j = 0; j < arrySubjects->size(); j++)
					{
						Poco::JSON::Object::Ptr objSubject = arrySubjects->getObject(j);
						//EXAM_SUBJECT subjectInfo;
						pEXAM_SUBJECT pSubjectInfo = new EXAM_SUBJECT;
						pSubjectInfo->nSubjID = objSubject->get("id").convert<int>();
//						pSubjectInfo->nSubjCode = objSubject->get("code").convert<int>();
						pSubjectInfo->strSubjName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("name").convert<std::string>());
						if (!objSubject->isNull("scanTemplateName"))
							pSubjectInfo->strModelName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("scanTemplateName").convert<std::string>());

						pExamInfo->lSubjects.push_back(pSubjectInfo);
					}
					g_lExamList.push_back(pExamInfo);
				}
				g_lfmExamList.unlock();
			}
			catch (Poco::JSON::JSONException& jsone)
			{
				std::string strErrInfo;
				strErrInfo.append("Error when parse json: ");
				strErrInfo.append(jsone.message() + "\tData:" + strUtf);
				g_pLogger->information(strErrInfo);
				TRACE(_T("%s\n"), strErrInfo.c_str());
			}
			catch (Poco::Exception& exc)
			{
				std::string strErrInfo;
				strErrInfo.append("Error: ");
				strErrInfo.append(exc.message() + "\tData:" + strUtf);
				g_pLogger->information(strErrInfo);
				TRACE(_T("%s\n"), strErrInfo.c_str());
			}
			catch (...)
			{
				std::string strErrInfo;
				strErrInfo.append("Unknown error.\tData:" + strUtf);
				g_pLogger->information(strErrInfo);
				TRACE(_T("%s\n"), strErrInfo.c_str());
			}
			SAFE_RELEASE_ARRY(pBuff);
			g_eGetExamList.set();		//��ȡ����������Ϣ
		}
		break;
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_NEEDDOWN)
	{
		switch (pstHead->usResult)
		{
			case RESULT_DOWNMODEL_OK:
			{
				pST_DOWN_MODEL pstModelInfo = (pST_DOWN_MODEL)(m_szRecvBuff + HEAD_SIZE);

				ST_DOWN_MODEL stModelInfo;
				ZeroMemory(&stModelInfo, sizeof(ST_DOWN_MODEL));
				stModelInfo.nExamID = _pCurrExam_->nExamID;
				stModelInfo.nSubjectID = _pCurrSub_->nSubjID;
				sprintf_s(stModelInfo.szUserNo, "%s", _strUserName_.c_str());
				sprintf_s(stModelInfo.szModelName, "%s", _pCurrSub_->strModelName.c_str());

				pTCP_TASK pTcpTask = new TCP_TASK;
				pTcpTask->usCmd = USER_DOWN_MODEL;
				pTcpTask->nPkgLen = sizeof(ST_DOWN_MODEL);
				memcpy(pTcpTask->szSendBuf, (char*)&stModelInfo, sizeof(ST_DOWN_MODEL));
				g_fmTcpTaskLock.lock();
				g_lTcpTask.push_back(pTcpTask);
				g_fmTcpTaskLock.unlock();
				g_nDownLoadModelStatus = 1;
			}
			break;
			case RESULT_DOWNMODEL_FAIL:
			{
				std::string strLog = "�������˿�Ŀģ�岻����";
				g_pLogger->information(strLog);
				g_nDownLoadModelStatus = -1;
				g_eDownLoadModel.set();			//����ģ���ļ�������������
			}
			break;
			case RESULT_DOWNMODEL_NONEED:
			{
				std::string strLog = "���ش��ڴ��ļ�������Ҫ����";
				g_pLogger->information(strLog);
				g_nDownLoadModelStatus = 3;
				g_eDownLoadModel.set();			//����ģ���ļ�������������
			}
			break;
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_DOWNMODEL)
	{
		USES_CONVERSION;
		switch (pstHead->usResult)
		{
			case RESULT_DOWNMODEL_RECV:
			{
				//���Ǳ����ļ�
				std::string strModelPath = T2A(g_strCurrentPath);
				strModelPath.append("Model\\");

				Poco::File fileModelPath(g_strModelSavePath);
				fileModelPath.createDirectories();

				strModelPath.append(_pCurrSub_->strModelName);
				Poco::File fileModel(strModelPath);
				if (fileModel.exists())
					fileModel.remove();

				ofstream out(strModelPath, std::ios::binary);
				if (!out)
				{
					break;
				}
				std::stringstream buffer;
				buffer.write(m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				int n = buffer.str().length();
				out << buffer.str();
				out.close();
				g_nDownLoadModelStatus = 2;
			}
			break;
			case RESULT_ERROR_FILEIO:
			{
				//��������ȡ�ļ�ʧ��
				g_nDownLoadModelStatus = -2;
			}
			break;
		}
		g_eDownLoadModel.set();			//����ģ���ļ�������������
	}
	else if (pstHead->usCmd == USER_RESPONSE_MODELINFO)
	{
		switch (pstHead->usResult)
		{
			case RESULT_SETMODELINFO_SEND:
			{
				pST_MODELINFO pstModelInfo = (pST_MODELINFO)(m_pRecvBuff + HEAD_SIZE);
				//				   pST_MODELINFO pstModelInfo = (pST_MODELINFO)(m_pRecvBuff + HEAD_SIZE);
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

				std::string strLog = "��Ҫ���·���ģ���ļ�: " + strModelPath;
				TRACE(strLog.c_str());
				g_pLogger->information(strLog);
			}
			break;
			case RESULT_SETMODELINFO_NO:
			{
				std::string strLog = "����Ҫ���·���ģ���ļ�";
				TRACE(strLog.c_str());
				g_pLogger->information(strLog);
			}
			break;
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_GET_BMK)
	{
		char* pBuff = new char[pstHead->uPackSize + 1];
		pBuff[pstHead->uPackSize] = '\0';
		strncpy(pBuff, m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
		std::string strResult = CMyCodeConvert::Utf8ToGb2312(pBuff);
		switch (pstHead->usResult)
		{
			case RESULT_GET_BMK_SUCCESS:
			{
				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result;
				try
				{
					result = parser.parse(pBuff);
					Poco::JSON::Object::Ptr examObj = result.extract<Poco::JSON::Object::Ptr>();
					Poco::JSON::Array::Ptr arryObj = examObj->getArray("students");

					g_lBmkStudent.clear();
					for (int i = 0; i < arryObj->size(); i++)
					{
						Poco::JSON::Object::Ptr objItem = arryObj->getObject(i);
						ST_STUDENT stData;
						stData.strZkzh = objItem->get("zkzh").convert<std::string>();
						stData.strName = CMyCodeConvert::Utf8ToGb2312(objItem->get("name").convert<std::string>());
						if (objItem->has("classRoom"))
							stData.strClassroom = CMyCodeConvert::Utf8ToGb2312(objItem->get("classRoom").convert<std::string>());
						if (objItem->has("school"))
							stData.strSchool = CMyCodeConvert::Utf8ToGb2312(objItem->get("school").convert<std::string>());
						g_lBmkStudent.push_back(stData);
					}
					std::string strLog = Poco::format("��ȡ���������(%d��)", (int)g_lBmkStudent.size());
					g_pLogger->information(strLog);
					TRACE(_T("%s\n"), strLog.c_str());

					//�������ݿ�
					USES_CONVERSION;
					std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
					CStudentMgr studentMgr;
					bool bResult = studentMgr.InitDB(strDbPath);
					std::string strTableName = "student";
					if (bResult) bResult = studentMgr.InitTable(strTableName);
					if (bResult) bResult = studentMgr.InsertData(g_lBmkStudent, strTableName);
				}
				catch (Poco::JSON::JSONException& jsone)
				{
					std::string strErrInfo;
					strErrInfo.append("Error when parse json(��ȡ������): ");
					strErrInfo.append(jsone.message() + "\tData:" + strResult);
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());
				}
			}
			break;
			case RESULT_GET_BMK_FAIL:
			{
				std::string strLog = "����������ʧ��: " + strResult;
				TRACE(strLog.c_str());
				g_pLogger->information(strLog);
			}
			break;
		}
		SAFE_RELEASE_ARRY(pBuff);
	}
}

void CTcpClient::HandleTask(pTCP_TASK pTask)
{
	ST_CMD_HEADER stHead;
	stHead.usCmd = pTask->usCmd;
	stHead.uPackSize = pTask->nPkgLen;

	char szSendBuf[2500 + HEAD_SIZE] = { 0 };
	memcpy(szSendBuf, (char*)&stHead, HEAD_SIZE);
	memcpy(szSendBuf + HEAD_SIZE, pTask->szSendBuf, pTask->nPkgLen);

	try
	{
		m_ss.sendBytes(szSendBuf, HEAD_SIZE + pTask->nPkgLen);
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "���������쳣 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		_bConnect = false;
		g_bCmdConnect = _bConnect;
	}
}

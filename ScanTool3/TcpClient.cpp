#include "stdafx.h"
#include "TcpClient.h"
#include "Net_Cmd_Protocol.h"
#include "StudentMgr.h"
#include "ScanTool3.h"
#include "ScanTool3Dlg.h"
#include "NewMessageBox.h"
#include "NewMakeModelDlg.h"

//extern Poco::Net::StreamSocket *pSs;

CTcpClient::CTcpClient(std::string strIp, int nPort)
: _strIP(strIp), _nPort(nPort), _bConnect(false), _nRecvLen(0), _nWantLen(0), m_pRecvBuff(NULL), m_pSendBuff(NULL)
{
}


CTcpClient::~CTcpClient()
{
	SAFE_RELEASE_ARRY(m_pRecvBuff);
	g_pLogger->information("CTcpClient exit.");
	TRACE("CTcpClient退出\n");
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
		m_ss.connect(sa);					//重新连接服务器
		m_ss.setBlocking(false);
		m_ss.setNoDelay(true);
		_bConnect = true;
		g_bCmdConnect = _bConnect;
		TRACE("连接服务器成功\n"); 
		std::string strLog = "连接服务器成功\n";
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "连接服务器失败 ==> " + exc.displayText();
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
		g_bCmdConnect = _bConnect;

		CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
		pDlg->PostMessage(MSG_CMD_CONNECTION_FAIL, NULL, NULL);
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

			std::string strLog = "退出登录账号(" + strUser + ")成功";
			g_pLogger->information(strLog);
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_EXAMINFO)
	{
		char* pBuff = new char[pstHead->uPackSize + 1];
		pBuff[pstHead->uPackSize] = '\0';
		strncpy(pBuff, m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
		std::string strExamData = pBuff;
		std::string strUtf = CMyCodeConvert::Utf8ToGb2312(pBuff);
		switch (pstHead->usResult)
		{
		case RESULT_EXAMINFO_SUCCESS:
		{
			g_lfmExamList.lock();
			EXAM_LIST::iterator itExam = g_lExamList.begin();
			for (; itExam != g_lExamList.end();)
			{
				pEXAMINFO pExam = *itExam;
				itExam = g_lExamList.erase(itExam);
				SAFE_RELEASE(pExam);
			}

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
					std::string strLog = "获取考试信息失败: " + strMsg;
					g_pLogger->information(strLog);

					g_lfmExamList.unlock();
					USES_CONVERSION;
					CNewMessageBox	dlg;
					dlg.setShowInfo(2, 1, strLog);
					dlg.DoModal();
					SAFE_RELEASE_ARRY(pBuff);
					return;
				}

				Poco::JSON::Array::Ptr arryObj = examObj->getArray("exams");
				for (int i = 0; i < arryObj->size(); i++)
				{
					Poco::JSON::Object::Ptr objExamInfo = arryObj->getObject(i);
					//EXAMINFO examInfo;
					pEXAMINFO pExamInfo = new EXAMINFO;
					
					if (objExamInfo->get("id").isString())	//直接从天喻服务器返回手阅数据
					{
						pExamInfo->strExamID = objExamInfo->get("id").convert<std::string>();
						pExamInfo->nModel = 1;
						pExamInfo->strPersonID = _strPersonID_;
					}
					else
						pExamInfo->nExamID = objExamInfo->get("id").convert<int>();
					pExamInfo->strExamName = CMyCodeConvert::Utf8ToGb2312(objExamInfo->get("name").convert<std::string>());
					
					if (objExamInfo->has("examID"))
						pExamInfo->strExamID = objExamInfo->get("examID").convert<std::string>();
					if (objExamInfo->has("examModel"))
						pExamInfo->nModel = objExamInfo->get("examModel").convert<int>();
					if (objExamInfo->has("examTime"))
						pExamInfo->strExamTime = objExamInfo->get("examTime").convert<std::string>();

					if (pExamInfo->nModel == 1)
					{
						if (objExamInfo->has("personid"))
						{
							pExamInfo->strPersonID = objExamInfo->get("personid").convert<std::string>();
							int nPos = pExamInfo->strPersonID.find(",");
							if (nPos != std::string::npos)
							{
								std::string stdTmpPersonId = pExamInfo->strPersonID.substr(0, nPos);
								std::string strLog = Poco::format("原始personID字符串(%s), 截取后(%s)", pExamInfo->strPersonID, stdTmpPersonId);
								g_pLogger->information(strLog);

								pExamInfo->strPersonID = stdTmpPersonId;
							}
						}
					}
					if (!objExamInfo->isNull("examType"))
					{
						Poco::JSON::Object::Ptr objExamType = objExamInfo->getObject("examType");
						if (objExamType->has("name"))
							pExamInfo->strExamTypeName = CMyCodeConvert::Utf8ToGb2312(objExamType->get("name").convert<std::string>());
					}
					if (!objExamInfo->isNull("grade"))
					{
						Poco::JSON::Object::Ptr objGrade = objExamInfo->getObject("grade");
// 						if (pExamInfo->nModel == 1)
// 						{
// 							if (objGrade->has("id"))
// 								pExamInfo->nExamGrade = objGrade->get("id").convert<int>();
// 						}
// 						else
// 						{
// 							if (objGrade->has("id"))
// 								pExamInfo->nExamGrade = objGrade->get("id").convert<int>();
// 						}
						if (objGrade->has("name"))
							pExamInfo->strGradeName = CMyCodeConvert::Utf8ToGb2312(objGrade->get("name").convert<std::string>());
					}
					pExamInfo->nExamState = objExamInfo->get("state").convert<int>();

					Poco::JSON::Array::Ptr arrySubjects = objExamInfo->getArray("examSubjects");
					for (int j = 0; j < arrySubjects->size(); j++)
					{
						Poco::JSON::Object::Ptr objSubject = arrySubjects->getObject(j);
						pEXAM_SUBJECT pSubjectInfo = new EXAM_SUBJECT;
						if (pExamInfo->nModel == 1)
						{
							pSubjectInfo->nSubjID = 10 * (j + 1);
							pSubjectInfo->strSubjName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("name").convert<std::string>());
						}
						else
						{
							pSubjectInfo->nSubjID = objSubject->get("id").convert<int>();
							pSubjectInfo->strSubjName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("name").convert<std::string>());
							if (!objSubject->isNull("scanTemplateName"))
								pSubjectInfo->strModelName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("scanTemplateName").convert<std::string>());
						}
						pExamInfo->lSubjects.push_back(pSubjectInfo);
					}
					g_lExamList.push_back(pExamInfo);
				}
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
			g_lfmExamList.unlock();
			g_eGetExamList.set();		//获取到考试列表信息
		}
		break;
		case RESULT_EXAMINFO_FAIL:
		{
			std::string strLog = "获取考试列表失败: " + CMyCodeConvert::Utf8ToGb2312(strUtf);
			TRACE(strLog.c_str());
			g_pLogger->information(strLog);
			g_eGetExamList.set();		//获取到考试列表信息

			SAFE_RELEASE_ARRY(pBuff);
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
//				pST_DOWN_MODEL pstModelInfo = (pST_DOWN_MODEL)(m_szRecvBuff + HEAD_SIZE);

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
				std::string strLog = "服务器此科目模板不存在";
				g_pLogger->information(strLog);
				g_nDownLoadModelStatus = -1;
				g_eDownLoadModel.set();			//下载模板文件请求命令处理完毕
			}
			break;
			case RESULT_DOWNMODEL_NONEED:
			{
				std::string strLog = "本地存在此文件，不需要下载";
				g_pLogger->information(strLog);
				g_nDownLoadModelStatus = 3;
				g_eDownLoadModel.set();			//下载模板文件请求命令处理完毕
			}
			break;
		}
		if (g_nDownLoadModelStatus != 1)
		{
#if 0
			CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
			pDlg->HandleModel();
#else
			CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
			pDlg->PostMessage(MSG_CMD_DL_MODEL_OK, NULL, NULL);
#endif
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_DOWNMODEL)
	{
		USES_CONVERSION;
		switch (pstHead->usResult)
		{
			case RESULT_DOWNMODEL_RECV:
			{
				//覆盖本地文件
				std::string strModelPath = T2A(g_strCurrentPath);
				strModelPath.append("Model\\");

				try
				{
					Poco::File fileModelPath(g_strModelSavePath);
					fileModelPath.createDirectories();

					strModelPath.append(_pCurrSub_->strModelName);
					Poco::File fileModel(strModelPath);
					if (fileModel.exists())
						fileModel.remove();
				}
				catch (Poco::Exception &e)
				{
					TRACE("获取模板异常%s\n", e.displayText().c_str());
				}
				
				ofstream out(strModelPath, std::ios::binary);
				if (!out)
				{
					g_nDownLoadModelStatus = -2;
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
				//服务器读取文件失败
				g_nDownLoadModelStatus = -2;
			}
			break;
		}
		g_eDownLoadModel.set();			//下载模板文件请求命令处理完毕
#if 0
		CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
		pDlg->HandleModel();
#else
		CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
		pDlg->PostMessage(MSG_CMD_DL_MODEL_OK, NULL, NULL);
#endif
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

					Poco::JSON::Object::Ptr examInfoObj = examObj->getObject("examInfo");
					int nExamID = examInfoObj->get("examId").convert<int>();
					int nSubID = examInfoObj->get("subjectId").convert<int>();

					Poco::JSON::Array::Ptr arryObj = examObj->getArray("students");

					g_lBmkStudent.clear();
					_bGetBmk_ = false;
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

					USES_CONVERSION;

					std::string strLog = Poco::format("获取报名库完成(%d人)", (int)g_lBmkStudent.size());
					g_pLogger->information(strLog);
					TRACE(_T("%s\n"), A2T(strLog.c_str()));

					//先通知主窗口进行后面下载模板操作，插入数据库这块较耗时
					CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
					pDlg->PostMessage(MSG_CMD_GET_BMK_OK, NULL, NULL);
					
					//插入数据库
					std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
					CStudentMgr studentMgr;
					bool bResult = studentMgr.InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
					std::string strTableName = Poco::format("T%d_%d", nExamID, nSubID);			//"student";
					if (bResult) bResult = studentMgr.InitTable(strTableName);
					if (bResult) bResult = studentMgr.InsertData(g_lBmkStudent, strTableName);

					if (g_lBmkStudent.size() > 0)
						_bGetBmk_ = true;
				}
				catch (Poco::Exception& jsone)
				{
					std::string strErrInfo;
					strErrInfo.append("Error when parse json(获取报名库): ");
					strErrInfo.append(jsone.message() + "\tData:" + strResult);
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());

					CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
					pDlg->PostMessage(MSG_CMD_GET_BMK_OK, NULL, NULL);
				}
			}
			break;
			case RESULT_GET_BMK_FAIL:
			{
				std::string strLog = "报名库下载失败: " + CMyCodeConvert::Utf8ToGb2312(strResult);
				TRACE(strLog.c_str());
				g_pLogger->information(strLog);

				CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
				pDlg->PostMessage(MSG_CMD_GET_BMK_OK, NULL, NULL);
			}
			break;
			default:
			{
				std::string strLog = "报名库下载失败(无法解析结果): " + CMyCodeConvert::Utf8ToGb2312(strResult);
				TRACE(strLog.c_str());
				g_pLogger->information(strLog);

				CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
				pDlg->PostMessage(MSG_CMD_GET_BMK_OK, NULL, NULL);
			}
		}
		SAFE_RELEASE_ARRY(pBuff);
		g_eGetBmk.set();
// 		CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
// 		pDlg->PostMessage(MSG_CMD_GET_BMK_OK, NULL, NULL);
	}
	else if (pstHead->usCmd == USER_RESPONSE_GET_EXAM_BMK)		//获取到整个考试的报名库
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

					Poco::JSON::Object::Ptr examInfoObj = examObj->getObject("examInfo");
					int nExamID = examInfoObj->get("examId").convert<int>();
					int nSubID = examInfoObj->get("subjectId").convert<int>();

					Poco::JSON::Array::Ptr arryObj = examObj->getArray("students");

					std::vector<int> vecSubjectID;
					ALLSTUDENT_LIST _AllStudent;
					
					for (int i = 0; i < arryObj->size(); i++)
					{
						Poco::JSON::Object::Ptr objItem = arryObj->getObject(i);
						ST_ALLSTUDENT stData;
						stData.nExamID = nExamID;
						stData.strZkzh = objItem->get("zkzh").convert<std::string>();
						stData.strName = CMyCodeConvert::Utf8ToGb2312(objItem->get("name").convert<std::string>());
						if (objItem->has("classRoom"))
							stData.strClassroom = CMyCodeConvert::Utf8ToGb2312(objItem->get("classRoom").convert<std::string>());
						if (objItem->has("school"))
							stData.strSchool = CMyCodeConvert::Utf8ToGb2312(objItem->get("school").convert<std::string>());

						Poco::JSON::Array::Ptr arrySub = objItem->getArray("scanStatus");
						for (int j = 0; j < arrySub->size(); j++)
						{
							Poco::JSON::Object::Ptr objSubjectItem = arrySub->getObject(j);
							ST_SubjectScanStatus _SubjectScan;
							_SubjectScan.nSubjectID = objSubjectItem->get("subjectID").convert<int>();
							_SubjectScan.nScaned	= objSubjectItem->get("scaned").convert<int>();
							stData.lSubjectScanStatus.push_back(_SubjectScan);

							bool bFind = false;
							for (int k = 0; k < vecSubjectID.size(); k++)
							{
								if (_SubjectScan.nSubjectID == vecSubjectID[k])
								{
									bFind = true;
									break;
								}
							}
							if (!bFind) vecSubjectID.push_back(_SubjectScan.nSubjectID);
						}
						_AllStudent.push_back(stData);
					}

					EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(nExamID);
					if (itFindExam != g_mapBmkMgr.end())
					{
						itFindExam->second.clear();
						itFindExam->second = _AllStudent;
					}
					else
					{
						if(_AllStudent.size() > 0)
							g_mapBmkMgr.insert(EXAMBMK_MAP::value_type(nExamID, _AllStudent));	//************************
					}

					USES_CONVERSION;

					std::string strLog = Poco::format("获取考试(%d)报名库完成(%d人)", nExamID, (int)_AllStudent.size());
					g_pLogger->information(strLog);
					TRACE(_T("%s\n"), A2T(strLog.c_str()));

					//先通知主窗口进行后面下载模板操作，插入数据库这块较耗时
					CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
					pDlg->PostMessage(MSG_CMD_GET_BMK_OK, 1, NULL);
					
					//初始化当前科目报名库
					g_lBmkStudent.clear();
					_bGetBmk_ = false;
					for (auto student : _AllStudent)
					{
						for (auto subject : student.lSubjectScanStatus)
						{
							if (subject.nSubjectID == nSubID)
							{
								ST_STUDENT _currSubjectStudent;
								_currSubjectStudent.strZkzh = student.strZkzh;
								_currSubjectStudent.strName = student.strName;
								_currSubjectStudent.strClassroom = student.strClassroom;
								_currSubjectStudent.strSchool = student.strSchool;
								_currSubjectStudent.nScaned = subject.nScaned;
								g_lBmkStudent.push_back(_currSubjectStudent);
							}
						}
					}
					//插入数据库
				#if 1
					std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
					CStudentMgr studentMgr;
					bool bResult = studentMgr.InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
					for (int k = 0; k < vecSubjectID.size(); k++)
					{
						STUDENT_LIST _tmpListBmkStudent;
						for (auto student : _AllStudent)
						{
							for (auto subject : student.lSubjectScanStatus)
							{
								if (subject.nSubjectID == vecSubjectID[k])
								{
									ST_STUDENT _currSubjectStudent;
									_currSubjectStudent.strZkzh = student.strZkzh;
									_currSubjectStudent.strName = student.strName;
									_currSubjectStudent.strClassroom = student.strClassroom;
									_currSubjectStudent.strSchool = student.strSchool;
									_currSubjectStudent.nScaned = subject.nScaned;
									_tmpListBmkStudent.push_back(_currSubjectStudent);
								}
							}
						}
						
						std::string strTableName = Poco::format("T%d_%d", nExamID, vecSubjectID[k]);			//"student";
						if (bResult) bResult = studentMgr.InsertMemData(_tmpListBmkStudent, strTableName);
// 						if (bResult) bResult = studentMgr.InitTable(strTableName);
// 						if (bResult) bResult = studentMgr.InsertData(_tmpListBmkStudent, strTableName);
					}
				#else
					if (nSubID > 0)
					{
						std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
						CStudentMgr studentMgr;
						bool bResult = studentMgr.InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
						std::string strTableName = Poco::format("T%d_%d", nExamID, nSubID);			//"student";
						if (bResult) bResult = studentMgr.InitTable(strTableName);
						if (bResult) bResult = studentMgr.InsertData(g_lBmkStudent, strTableName);
					}
				#endif
					if (g_lBmkStudent.size() > 0)
						_bGetBmk_ = true;
					else
					{
						if (nSubID != 0)	//从考试列表中查看扫描进度窗口进入的，不发送科目报名库获取信息	nSubID != 0
						{
							std::string strErrInfo;
							strErrInfo.append("获取报名库失败: 无当前科目信息");
							g_pLogger->information(strErrInfo);
							CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
							pDlg->PostMessage(MSG_CMD_GET_BMK_OK, 1, 1);
						}
					}
				}
				catch (Poco::Exception& jsone)	//Poco::JSON::JSONException
				{
					std::string strErrInfo;
					strErrInfo.append("Error when parse json(获取报名库): ");
					strErrInfo.append(jsone.message() + "\tData:" + strResult);
					g_pLogger->information(strErrInfo);
					TRACE(_T("%s\n"), strErrInfo.c_str());

					CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
					pDlg->PostMessage(MSG_CMD_GET_BMK_OK, 1, NULL);
				}
			}
			break;
			case RESULT_GET_BMK_FAIL:
			{
				std::string strLog = "报名库下载失败: " + CMyCodeConvert::Utf8ToGb2312(strResult);
				TRACE(strLog.c_str());
				g_pLogger->information(strLog);

				CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
				pDlg->PostMessage(MSG_CMD_GET_BMK_OK, 1, NULL);
			}
			break;
			default:
			{
				std::string strLog = "报名库下载失败2: " + CMyCodeConvert::Utf8ToGb2312(strResult);
				TRACE(strLog.c_str());
				g_pLogger->information(strLog);

				CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
				pDlg->PostMessage(MSG_CMD_GET_BMK_OK, 1, NULL);
			}
		}
		SAFE_RELEASE_ARRY(pBuff);
		g_eGetBmk.set();
		// 		CScanTool3Dlg* pDlg = (CScanTool3Dlg*)_pMainDlg;
		// 		pDlg->PostMessage(MSG_CMD_GET_BMK_OK, NULL, NULL);
	}
	else if (pstHead->usCmd == USER_RESPONSE_GET_FILE_UPLOAD_ADDR)
	{
		char* pBuff = new char[pstHead->uPackSize + 1];
		pBuff[pstHead->uPackSize] = '\0';
		strncpy(pBuff, m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
		std::string strResult = CMyCodeConvert::Utf8ToGb2312(pBuff);
		TRACE("获取文件服务器地址数据: %s\n", strResult.c_str());
		switch (pstHead->usResult)
		{
			case RESULT_GET_FILE_ADDR_SUCCESS:
			{
				//解析格式数据
				USES_CONVERSION;
				std::string strPkg = T2A(PAPERS_EXT_NAME);
				std::string strTyPkg = T2A(PAPERS_EXT_NAME_4TY);

				std::string strData = pBuff;
				int nPos = strData.find("@@@");
				while (nPos != std::string::npos)
				{
					int nEnd = strData.find("###");
					std::string strInfo = strData.substr(nPos + 3, nEnd - nPos - 3);
					
					int nPos1 = strInfo.find("_");
					std::string strExtName = strInfo.substr(0, nPos1);
					int nPos2 = strInfo.find("_", nPos1 + 1);
					std::string strIP = strInfo.substr(nPos1 + 1, nPos2 - nPos1 - 1);
					std::string strPort = strInfo.substr(nPos2 + 1);
					int nPort = atoi(strPort.c_str());
					TRACE("地址信息: %s-%s-%d\n", strExtName.c_str(), strIP.c_str(), nPort);

					if (strExtName == strTyPkg)
					{
						g_strFileIp4HandModel = strIP;
						g_nFilePort4HandModel = nPort;
						TRACE(_T("get file addr: %s:%d\n"), A2T(strIP.c_str()), nPort);
					}
					else if (strExtName == strPkg)
					{
						g_strFileIP = strIP;
						g_nFilePort = nPort;
						TRACE(_T("get file addr: %s:%d\n"), A2T(strIP.c_str()), nPort);
					}

					//修改发送子线程
				#ifdef TEST_MULTI_SENDER
					_fmMapSender_.lock();
					MAP_FILESENDER::iterator itSender = _mapSender_.find(strExtName);
					if(itSender != _mapSender_.end())
					{
						pST_SENDER pObjSender = itSender->second;
						pObjSender->strIP = strIP;
						pObjSender->nPort = nPort;
						pObjSender->pUpLoad->ReConnectAddr(A2T(strIP.c_str()), nPort);
					}
					else
					{
// 						pST_SENDER pObjSender = new ST_SENDER;
// 						pObjSender->strIP = strIP;
// 						pObjSender->nPort = nPort;
// 						pObjSender->pUpLoad = new CFileUpLoad(*this);
// 						pObjSender->pUpLoad->InitUpLoadTcp(A2T(strIP.c_str()), nPort);
// 						_mapSender_.insert(MAP_FILESENDER::value_type(strExtName, pObjSender));
					}
					_fmMapSender_.unlock();
				#endif
					nPos = strData.find("@@@", nPos + 3);
				}
			}
			break;
			case RESULT_GET_FILE_ADDR_FAIL:
			{
				//无数据
				g_strFileIp4HandModel = g_strFileIP;
				g_nFilePort4HandModel = g_nFilePort;
				TRACE(_T("get file addr: %s:%d\n"), g_strFileIp4HandModel.c_str(), g_nFilePort4HandModel);
			}
			break;
			default:
				break;
		}
		SAFE_RELEASE_ARRY(pBuff);
	}
	else if (pstHead->usCmd == USER_RESPONSE_NEED_UP_MODEL_PIC)
	{
		pST_MODELPIC pstModelPic = (pST_MODELPIC)(m_pRecvBuff + HEAD_SIZE);
		switch (pstHead->usResult)
		{
			case RESULT_UP_MODEL_PIC_SEND:
			{
				TRACE("可以发送模板图片: %s\n", pstModelPic->szPicPath);

				std::string strLog = "可以发送模板图片: ";
				strLog.append(pstModelPic->szPicPath);
				g_pLogger->information(strLog);
			#if 1
				char szModelPicName[100] = { 0 };
				sprintf_s(szModelPicName, "%d_%d_%d_#_%s", pstModelPic->nExamID, pstModelPic->nSubjectID, pstModelPic->nIndex, pstModelPic->szPicName);
//				std::string strModelPicName = Poco::format("%d_%d_%s", pstModelPic->nExamID, pstModelPic->nSubjectID, pstModelPic->szPicName);
				pSENDTASK pTask = new SENDTASK;
				pTask->strFileName = szModelPicName;	//pstModelPic->szPicName;
				pTask->strPath = pstModelPic->szPicPath;
				g_fmSendLock.lock();
				g_lSendTask.push_back(pTask);
				g_fmSendLock.unlock();
			#else
				std::string strFileData;

				std::ifstream fin(pstModelPic->szPicPath, std::ifstream::binary);
				if (!fin)
				{
					return ;
				}
				std::stringstream buffer;
				buffer << fin.rdbuf();
				strFileData = buffer.str();
				fin.close();

				int nLen = strFileData.length();

				ST_SENDMODELPIC stSendPic;
				stSendPic.nExamID = pstModelPic->nExamID;
				stSendPic.nSubjectID = pstModelPic->nSubjectID;
				stSendPic.nPicLen = nLen;
				strncpy(stSendPic.szMD5, pstModelPic->szMD5, strlen(pstModelPic->szMD5));
				strncpy(stSendPic.szFileName, pstModelPic->szPicName, strlen(pstModelPic->szPicName));

				pTCP_TASK pTcpTask = new TCP_TASK;
				pTcpTask->usCmd = USER_MODEL_PIC_SEND;
				pTcpTask->nPkgLen = sizeof(ST_SENDMODELPIC) + stSendPic.nPicLen;
				if (pTcpTask->nPkgLen > strlen(pTcpTask->szSendBuf))
				{
					pTcpTask->pszSendBuf = new char[pTcpTask->nPkgLen + 1];
					ZeroMemory(pTcpTask->pszSendBuf, pTcpTask->nPkgLen + 1);
					memcpy(pTcpTask->pszSendBuf, (char*)&stSendPic, sizeof(ST_SENDMODELPIC));
				}
				else
					memcpy(pTcpTask->szSendBuf, (char*)&stSendPic, sizeof(ST_SENDMODELPIC));
				g_fmTcpTaskLock.lock();
				g_lTcpTask.push_back(pTcpTask);
				g_fmTcpTaskLock.unlock();
			#endif
			}
				break;
			case RESULT_UP_MODEL_PIC_NONEED:
			{
				TRACE("不需要发送模板图片: %s\n", pstModelPic->szPicPath);
				std::string strLog = "不需要重新发送模板文件";
				strLog.append(pstModelPic->szPicPath);
				g_pLogger->information(strLog);
			}
				break;
			default:
				break;
		}
	}
	else if (pstHead->usCmd == USER_RESPONSE_GET_MODEL_PIC)
	{
		switch (pstHead->usResult)
		{
			case RESULT_GET_MODEL_PIC_SUCCESS:
			{				
				USES_CONVERSION;
				std::stringstream buffData;
				buffData.write(m_pRecvBuff + HEAD_SIZE, pstHead->uPackSize);
				std::string strData = buffData.str();
				int nPos = strData.find("#_#_#_#_");
				while (nPos != std::string::npos)
				{
					int nPos2 = strData.find("_*_", nPos + 8);
					string strFileName = strData.substr(nPos + 8, nPos2 - nPos - 8);
					int nPos3 = strData.find("_#####_", nPos2 + 3);
					string sFileLen = strData.substr(nPos2 + 3, nPos3 - nPos2 - 3);
					int nLen = atoi(sFileLen.c_str());
//					string strFileData = strData.substr(nPos3 + 7, nLen);
					
					TRACE("收到模板图片: %s\n", strFileName.c_str()); 
					std::string strLog = "收到模板图片: " + strFileName;
					g_pLogger->information(strLog);
					
					std::string strExamID = strFileName.substr(0, strFileName.find("_"));

					//覆盖本地文件
					std::string strModelDir = g_strModelSavePath + "\\TmpModelPic\\" + strExamID + "\\";
					std::string strModelPicNewPath = strModelDir + strFileName;
					try
					{
						Poco::File fileModelPath(strModelDir);
						fileModelPath.createDirectories();

						Poco::File fileModel(strModelPicNewPath);
						if (fileModel.exists())
							fileModel.remove(true);
					}
					catch (Poco::Exception &e)
					{
						TRACE("获取模板异常%s\n", e.displayText().c_str());
					}

					ofstream out(CMyCodeConvert::Utf8ToGb2312(strModelPicNewPath), std::ios::binary);
					if (!out)
					{
						continue;
					}
					std::stringstream buffer;
					buffer.write(m_pRecvBuff + HEAD_SIZE + nPos3 + 7, nLen);
					int n = buffer.str().length();
					out << buffer.str();
					out.close();

					MODELPICPATH picInfo;
					picInfo.strName = A2T(strFileName.c_str());
					picInfo.strPath = A2T(CMyCodeConvert::Utf8ToGb2312(strModelPicNewPath).c_str());
					_vecModelPicPath_.push_back(picInfo);

					//next
					nPos = strData.find("#_#_#_#_", nPos3 + 7 + nLen);
				}
				_nGetModelPic_ = 2;
			}
				break;
			case RESULT_ERROR_FILEIO:
			{
				TRACE("服务器读取模板图片失败\n");
				std::string strLog = "服务器读取模板图片失败";
				g_pLogger->information(strLog);
				_nGetModelPic_ = 4;
			}
				break;
			case RESULT_GET_MODEL_PIC_NOPIC:
			{
				TRACE("服务器不存在模板图像");
				std::string strLog = "服务器不存在模板图像";
				g_pLogger->information(strLog);
				_nGetModelPic_ = 3;
			}
				break;
			default:
				break;
		}
		g_eGetModelPic.set();
	}
}

void CTcpClient::HandleTask(pTCP_TASK pTask)
{
	ST_CMD_HEADER stHead;
	stHead.usCmd = pTask->usCmd;
	stHead.uPackSize = pTask->nPkgLen;

	bool  bSendNewBuff = false;
	char* pSendBuff = NULL;
	char szSendBuf[2500 + HEAD_SIZE] = { 0 };
	if (pTask->pszSendBuf != NULL)
	{
		bSendNewBuff = true;
		pSendBuff = new char[pTask->nPkgLen + 1];
		ZeroMemory(pSendBuff, pTask->nPkgLen + 1);
		memcpy(pSendBuff, (char*)&stHead, HEAD_SIZE);
		memcpy(pSendBuff + HEAD_SIZE, pTask->szSendBuf, pTask->nPkgLen);

// 		m_ss.setBlocking(true);
// 		m_ss.setNoDelay(false);
	}
	else
	{
		memcpy(szSendBuf, (char*)&stHead, HEAD_SIZE);
		memcpy(szSendBuf + HEAD_SIZE, pTask->szSendBuf, pTask->nPkgLen);
	}

// 	int nWantSend = HEAD_SIZE + pTask->nPkgLen;
// 	int nSended = 0;
// 	while (nWantSend > 0)
// 	{
// 		int nSendLen = -1;
// 		try
// 		{
// 			if (bSendNewBuff)
// 				nSendLen = m_ss.sendBytes(pSendBuff + nSended, 1500);
// 			else
// 				nSendLen = m_ss.sendBytes(szSendBuf + nSended, nWantSend);
// 
// 			nWantSend -= nSendLen;
// 			nSended += nSendLen;
// 		}
// 		catch (Poco::Exception& exc)
// 		{
// 			std::string strLog = "发送数据异常 ==> " + exc.displayText();
// 			TRACE(strLog.c_str());
// 			g_pLogger->information(strLog);
// 			_bConnect = false;
// 			g_bCmdConnect = _bConnect;
// 		}
// 	}
	try
	{
		if (bSendNewBuff)
			m_ss.sendBytes(pSendBuff, HEAD_SIZE + pTask->nPkgLen);
		else
			m_ss.sendBytes(szSendBuf, HEAD_SIZE + pTask->nPkgLen);
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "发送数据异常 ==> " + exc.displayText();
		TRACE(strLog.c_str());
		g_pLogger->information(strLog);
		_bConnect = false;
		g_bCmdConnect = _bConnect;
 	}
	if (bSendNewBuff)
	{
		SAFE_RELEASE(pSendBuff);
// 		m_ss.setBlocking(false);
// 		m_ss.setNoDelay(true);
	}
}

void CTcpClient::SetMainWnd(void* p)
{
	_pMainDlg = p;
}

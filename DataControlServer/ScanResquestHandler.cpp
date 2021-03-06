#include "ScanResquestHandler.h"
#include "miniunz/minizip.c"


CScanResquestHandler::CScanResquestHandler()
{
	std::cout << "CScanResquestHandler start" << std::endl;
}

CScanResquestHandler::~CScanResquestHandler()
{
	std::cout << "CScanResquestHandler exit." << std::endl;
}

void CScanResquestHandler::run()
{
	while (!g_nExitFlag)
	{
		pSCAN_REQ_TASK pTask = NULL;
		g_fmScanReq.lock();
		LIST_SCAN_REQ::iterator it = g_lScanReq.begin();
		for (; it != g_lScanReq.end();)
		{
			pTask = *it;
			it = g_lScanReq.erase(it);
			break;
		}
		g_fmScanReq.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(200);
			continue;
		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}
}

void CScanResquestHandler::HandleTask(pSCAN_REQ_TASK pTask)
{
	Poco::URI uri(pTask->strUri);				//pTask->strUri
	Poco::Net::HTTPClientSession session;
	Poco::Net::HTTPRequest request;
	Poco::Net::HTTPResponse response;

	try
	{
		std::string path(uri.getPathAndQuery());
		if (path.empty()) path = "/";

		request.setURI(path);
		request.setMethod(Poco::Net::HTTPRequest::HTTP_POST);
		request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
		request.setKeepAlive(true);

		session.setHost(uri.getHost());
		session.setPort(uri.getPort());
		session.setKeepAlive(true);
		session.setTimeout(Poco::Timespan(SysSet.m_nHttpTimeOut, 0));

		request.set("User-Agent", "Go-http-client/1.1");
		request.set("Accept-Encoding", "gzip");	//gzip	//text/html

		std::string strBody;
		strBody = pTask->strRequest;
		if (pTask->strMsg == "login")
		{
			request.setContentType("application/x-www-form-urlencoded");
			request.set("EZ-Scanning-Program", "yes");
		}
		else
		{
			request.set("Accept", "application/json");
			request.setContentType("application/json");
			request.set("Cookie", pTask->strEzs);
		}
		request.setContentLength(strBody.length());

		std::ostream& ostr = session.sendRequest(request);
		ostr << strBody;


		std::istream& iStr = session.receiveResponse(response);
		if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK)
		{
			std::string strResultStatus;
			std::string strContentEncoding;
			if (response.has("Content-Encoding"))
			{
				strContentEncoding = response.get("Content-Encoding");
			}
			if (strContentEncoding == "gzip")
			{
				std::ostringstream outStringStream;
				Poco::InflatingInputStream inflater(iStr, Poco::InflatingStreamBuf::STREAM_GZIP);
				outStringStream << inflater.rdbuf();
				strResultStatus = outStringStream.str();
			}
			else
			{
				std::ostringstream outStringStream;
				outStringStream << iStr.rdbuf();
				strResultStatus = outStringStream.str();
			}

			if (ParseResult(strResultStatus, pTask))
			{
				//解析返回的处理结果
			}

		}
		else
		{
//			std::string strLog = "发送数据:" + pTask->strRequest + "\t\t后端数据返回错误,不是200 OK";
			std::string strLog = Poco::format("命令: %s\turi: %s\t发送数据: %s\n[ezs = %s]后端数据返回错误,不是200 OK,返回http代码: %d", pTask->strMsg, pTask->strUri, pTask->strRequest, pTask->strEzs, (int)response.getStatus());
			g_Log.LogOut(strLog);
			if(pTask->strMsg != "sessionAlive")
				std::cout << strLog << std::endl;
		#if 1
			std::string strSendData;
			strSendData = Poco::format("请求失败，错误代码%d", (int)response.getStatus());
			HandleHttpFail(pTask, strSendData);
		#else
			int nCmd = 0;
			if (pTask->strMsg == "login")
				nCmd = USER_RESPONSE_LOGIN;
			else if (pTask->strMsg == "login2Ty")
				nCmd = USER_RESPONSE_LOGIN;
			else if (pTask->strMsg == "getUserInfo4Ty")
				nCmd = USER_RESPONSE_LOGIN;
			else if (pTask->strMsg == "ezs")
				nCmd = USER_RESPONSE_EXAMINFO;
			else if (pTask->strMsg == "createModel")
				nCmd = USER_RESPONSE_CREATE_MODEL;
			else if (pTask->strMsg == "setElectOmrInfo")
				nCmd = USER_RESPONSE_ELECTOMR_MODEL;
			else if (pTask->strMsg == "getBmk")
				nCmd = USER_RESPONSE_GET_BMK;
			else if (pTask->strMsg == "getExamBmk")
				nCmd = USER_RESPONSE_GET_EXAM_BMK;


			int ret = RESULT_ERROR_UNKNOWN;
			if (pTask->strMsg == "getBmk" || pTask->strMsg == "getExamBmk")
			{
				ret = RESULT_GET_BMK_FAIL;				
			}
			else if (pTask->strMsg == "ezs")
			{
				ret = RESULT_EXAMINFO_FAIL;
			}

			std::string strSendData;
			strSendData = Poco::format("请求失败，错误代码%d", (int)response.getStatus());
			if (pTask->pUser)
				pTask->pUser->SendResponesInfo(nCmd, ret, (char*)strSendData.c_str(), strSendData.length());
		#endif
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrorInfo;
		strErrorInfo.append(exc.displayText());
		strErrorInfo.append("\tDetail: " + pTask->strRequest);

		std::cout << strErrorInfo << std::endl;
		g_Log.LogOutError(strErrorInfo);

		std::string strSendData;
		strSendData = Poco::format("请求失败，详情:%s", exc.displayText());
		HandleHttpFail(pTask, strSendData);
	}
	catch (...)
	{
		std::string strErrorInfo;
		strErrorInfo.append("**** Unknown error ***\tDetail: " + pTask->strRequest);

		std::cout << strErrorInfo << std::endl;
		g_Log.LogOutError(strErrorInfo);

		std::string strSendData = "请求失败2，详情:Unknown error";
		HandleHttpFail(pTask, strSendData);
	}
}

bool CScanResquestHandler::ParseResult(std::string& strInput, pSCAN_REQ_TASK pTask)
{
	bool bResult = false;
	int ret = 0;
	std::string strSendData;
	std::string strGb2312 = CMyCodeConvert::Utf8ToGb2312(strInput);
	std::string strLog = "收到后端数据: " + strGb2312;
	g_Log.LogOut(strLog);

	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strInput);
		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
		if (pTask->strMsg == "login")
		{
			bool bResult = object->get("success").convert<bool>();
			std::string strResult;
			if (bResult)
				strResult = object->get("result").convert<std::string>();
			else
				strResult = object->get("msg").convert<std::string>();
			strResult = CMyCodeConvert::Utf8ToGb2312(strResult);

			if (pTask->nLoginFlag == 1)
			{
				std::string strTmpLog = "易考乐学后端平台二次登录(" + pTask->strUser + "\t" + pTask->strPwd + ")完成: " + strResult;
				std::cout << strTmpLog << std::endl;
				g_Log.LogOut(strTmpLog);
			}

			if (bResult)
			{
				ret = RESULT_SUCCESS;
				std::string strEzs = object->get("ezs").convert<std::string>();
				std::string strUserInfo = object->get("user").convert<std::string>();
				int nTeacher = object->get("teacherId").convert<int>();
				int nUserID = -1;
				if (!object->isNull("user"))
				{
					Poco::JSON::Object::Ptr objUser = object->getObject("user");
					nUserID = objUser->get("id").convert<int>();
				}

				ST_LOGIN_RESULT stResult;
				ZeroMemory(&stResult, sizeof(stResult));
				stResult.nTeacherId = nTeacher;
				stResult.nUserId = nUserID;
				strncpy(stResult.szEzs, strEzs.c_str(), strEzs.length());
				strncpy(stResult.szUserInfo, strUserInfo.c_str(), strUserInfo.length());
				
				char szLoginResult[1024] = { 0 };
				memcpy(szLoginResult, (char*)&stResult, sizeof(stResult));

				pTask->pUser->UpdateLogonInfo((char*)pTask->strUser.c_str(), (char*)pTask->strPwd.c_str());

				_mapUserLock_.lock();
				MAP_USER::iterator itFind = _mapUser_.find(pTask->strUser);
				if (itFind == _mapUser_.end())
				{
					_mapUser_.insert(MAP_USER::value_type(pTask->strUser, pTask->pUser));
				}
				else
				{
					//重复登录提醒，
					itFind->second = pTask->pUser;
				}
				_mapUserLock_.unlock();

				//记录session映射关系
				std::string strSessionKey = Poco::format("%s:%d", std::string(pTask->pUser->m_pIPAddress), (int)pTask->pUser->m_wPort);
				_mapSessionLock_.lock();
				ST_USER_SESSION stUserSession;
				stUserSession.nTeacherID = nTeacher;
				stUserSession.nUserID = nUserID;
				stUserSession.strUser = pTask->strUser;
				stUserSession.tmStamp.update();
				_mapSession_[strEzs] = stUserSession;
// 				MAP_SESSION::iterator itFind2 = _mapSession_.find(strSessionKey);
// 				if (itFind2 == _mapSession_.end())
// 				{
// 					_mapSession_.insert(MAP_SESSION::value_type(strSessionKey, strEzs));
// 				}
				_mapSessionLock_.unlock();

				pTask->pUser->SendResponesInfo(USER_RESPONSE_LOGIN, ret, szLoginResult, sizeof(stResult));
				return bResult;
			}
			else
			{
				ret = RESULT_LOGIN_FAIL;
				strSendData = strResult;
			}
		}
		else if (pTask->strMsg == "login_SanXi_huanshuo")
		{
			int nResult = object->get("code").convert<int>();
			bResult = (nResult == 200) ? true : false;
			std::string strResult;
			strResult = object->get("msg").convert<std::string>();
			strResult = CMyCodeConvert::Utf8ToGb2312(strResult);

			if (bResult)
			{
				ret = RESULT_SUCCESS;
				
				//登录第3方平台成功，开始登录易考乐学后端平台
				pSCAN_REQ_TASK pNewTask = new SCAN_REQ_TASK;
				pNewTask->pUser = pTask->pUser;
				pNewTask->strUser = pTask->strUser;
				pNewTask->strPwd = SysSet.m_strLoginYklxPwd;	//默认密码123456， pTask->strPwd;
				pNewTask->nLoginFlag = 1;

				pNewTask->strUri = SysSet.m_strBackUri + "/login";
				pNewTask->strMsg = "login";
				pNewTask->strRequest = "username=" + pNewTask->strUser + "&password=" + pNewTask->strPwd;

				g_fmScanReq.lock();
				g_lScanReq.push_back(pNewTask);
				g_fmScanReq.unlock();

				std::string strTmpLog = pTask->strMsg + ": 登录成功，开始易考乐学后端平台二次登录";
				std::cout << strTmpLog << std::endl;
				g_Log.LogOut(strTmpLog);
				return bResult;
			}
			else
			{
				ret = RESULT_LOGIN_FAIL;
				strSendData = strResult;
			}
		}
		else if (pTask->strMsg == "login2Ty")
		{
			std::string strResult = object->get("result").convert<std::string>();
			if (strResult == "000000")		//用户只在单平台存在的返回
			{
				if (SysSet.m_nServerMode == 0)	//非天喻服务器模式，则进入此处为登录第3方天喻平台
				{
					//登录第3方平台成功，开始登录易考乐学后端平台
					pSCAN_REQ_TASK pNewTask = new SCAN_REQ_TASK;
					pNewTask->pUser = pTask->pUser;
					pNewTask->strUser = pTask->strUser;
					pNewTask->strPwd = SysSet.m_strLoginYklxPwd;	//默认密码123456， pTask->strPwd;
					pNewTask->nLoginFlag = 1;

					pNewTask->strUri = SysSet.m_strBackUri + "/login";
					pNewTask->strMsg = "login";
					pNewTask->strRequest = "username=" + pNewTask->strUser + "&password=" + pNewTask->strPwd;

					g_fmScanReq.lock();
					g_lScanReq.push_back(pNewTask);
					g_fmScanReq.unlock();

					std::string strTmpLog = pTask->strMsg + ": 登录成功，开始易考乐学后端平台二次登录";
					std::cout << strTmpLog << std::endl;
					g_Log.LogOut(strTmpLog);
					return bResult;
				}
				else	//天喻服务器模式
				{
					std::string strSessionId = object->get("sessionId").convert<std::string>();
					GetUserInfo4Ty(strSessionId, pTask);
				}
				return true;
			}
			else if (strResult == "301000")		//用户在多个平台的返回
			{
				Poco::JSON::Array::Ptr jsnDetailArry = object->getArray("platList");
				std::stringstream jsnString;
				jsnDetailArry->stringify(jsnString, 0);

				//发送平台信息给客户端
				ret = RESULT_LOGIN_PLATFORM_TY;
				std::string strPlatform = jsnString.str();
				pTask->pUser->SendResponesInfo(USER_RESPONSE_LOGIN, ret, (char*)jsnString.str().c_str(), jsnString.str().length());
				return true;
			}
			else
			{
				std::string strResult = object->get("msg").convert<std::string>();
				strResult = CMyCodeConvert::Utf8ToGb2312(strResult);
				ret = RESULT_LOGIN_FAIL;
				strSendData = strResult;
			}
		}
		else if (pTask->strMsg == "getUserInfo4Ty")
		{
			if (object->has("name"))	//有name说明获取用户信息成功
			{
				std::string strName = object->get("name").convert<std::string>();
				std::string strPersonId = object->get("personid").convert<std::string>();
				std::string strAccount = object->get("account").convert<std::string>();

				ret = RESULT_SUCCESS;
				int nTeacher = -1;
				int nUserID = -1;

				Poco::JSON::Object jsUserInfo;
				jsUserInfo.set("personid", strPersonId);
				jsUserInfo.set("name", strName);
				jsUserInfo.set("account", strAccount);
				std::stringstream jsnString;
				jsUserInfo.stringify(jsnString, 0);

				ST_LOGIN_RESULT stResult;
				ZeroMemory(&stResult, sizeof(stResult));
				stResult.nTeacherId = nTeacher;
				stResult.nUserId = nUserID;
				strncpy(stResult.szUserInfo, jsnString.str().c_str(), jsnString.str().length());

				char szLoginResult[1024] = { 0 };
				memcpy(szLoginResult, (char*)&stResult, sizeof(stResult));

				pTask->pUser->UpdateLogonInfo((char*)pTask->strUser.c_str(), (char*)pTask->strPwd.c_str());

				_mapUserLock_.lock();
				MAP_USER::iterator itFind = _mapUser_.find(pTask->strUser);
				if (itFind == _mapUser_.end())
				{
					_mapUser_.insert(MAP_USER::value_type(pTask->strUser, pTask->pUser));
				}
				else
				{
					//重复登录提醒，
					itFind->second = pTask->pUser;
				}
				_mapUserLock_.unlock();

				pTask->pUser->SendResponesInfo(USER_RESPONSE_LOGIN, ret, szLoginResult, sizeof(stResult));
				return bResult;
			}
			else
			{
				ret = RESULT_LOGIN_FAIL;
				strSendData = "获取用户信息失败";
			}
		}
		else if (pTask->strMsg == "ezs")
		{
			ret = RESULT_EXAMINFO_SUCCESS;
			strSendData = strInput;

			//=========================		针对模板上传到服务器了，但是后台设置模板失败的情况
// 			if (SysSet.m_nServerMode == 0)
// 			{
// 				Poco::JSON::Object::Ptr statusObj = object->getObject("status");
// 				bool bSuccess = statusObj->get("success").convert<bool>();
// 				if (bSuccess)
// 				{
// 					bool bModifyData = false;
// 					Poco::JSON::Array::Ptr arryObj = object->getArray("exams");
// 					for (int i = 0; i < arryObj->size(); i++)
// 					{
// 						Poco::JSON::Object::Ptr objExamInfo = arryObj->getObject(i);
// 
// 						int nExamID = objExamInfo->get("id").convert<int>();
// 						std::string strExamName = CMyCodeConvert::Utf8ToGb2312(objExamInfo->get("name").convert<std::string>());
// 						Poco::JSON::Array::Ptr arrySubjects = objExamInfo->getArray("examSubjects");
// 						for (int j = 0; j < arrySubjects->size(); j++)
// 						{
// 							Poco::JSON::Object::Ptr objSubject = arrySubjects->getObject(j);
// 
// 							int nSubjID = objSubject->get("id").convert<int>();
// 							std::string strSubjName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("name").convert<std::string>());
// 
// 							if (!objSubject->isNull("scanTemplateName"))
// 							{
// 								std::string strModelName = CMyCodeConvert::Utf8ToGb2312(objSubject->get("scanTemplateName").convert<std::string>());
// 								if (strModelName.empty())
// 								{
// 									char szIndex[50] = { 0 };
// 									sprintf(szIndex, "%d_%d", nExamID, nSubjID);
// 
// 									MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
// 									if (itFind != _mapModel_.end())		//服务器上存在此模板文件
// 									{
// 										//char szModelName[150] = { 0 };
// 										//sprintf_s(szModelName, "%s_%s_N_%d_%d.mod", strExamName.c_str(), strSubjName.c_str(), nExamID, nSubjID);
// 										strModelName = itFind->second->strName;
// 										objSubject->set("scanTemplateName", CMyCodeConvert::Gb2312ToUtf8(strModelName));
// 										objSubject->set("isMyServerSet", true);
// 										if(!bModifyData) bModifyData = true;
// 									}
// 								}
// 							}
// 						}
// 					}
// 					if (bModifyData)
// 					{
// 						std::stringstream jsnSnString;
// 						object->stringify(jsnSnString, 0);
// 						strSendData = jsnSnString.str();
// 					}
// 				}
// 			}
		}
		else if (pTask->strMsg == "setScanModel")
		{
			//**************	这里要考虑要不要推送给客户端	****************************
			Poco::JSON::Object::Ptr objResult = object->getObject("status");
			bResult = objResult->get("success").convert<bool>();
			if (!bResult)
			{
				std::string strMsg;
				if (!objResult->isNull("msg"))
				{
					strMsg = CMyCodeConvert::Utf8ToGb2312(objResult->get("msg").convert<std::string>());
				}
				std::string strLog = "设置考试科目的扫描模板名称失败，失败原因: " + strMsg;
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
			}
			else
			{
				std::string strLog = Poco::format("设置考试科目(%d_%d)的扫描模板名称成功", pTask->nExamID, pTask->nSubjectID);
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}
			return bResult;
		}
		else if (pTask->strMsg == "getBmk")
		{
			Poco::JSON::Object::Ptr objResult = object->getObject("status");
			bResult = objResult->get("success").convert<bool>();
			if (bResult)
			{
				//++添加考试ID和科目ID到结果信息中
				Poco::JSON::Object objExam;
				objExam.set("examId", pTask->nExamID);
				objExam.set("subjectId", pTask->nSubjectID);
				object->set("examInfo", objExam);
				std::stringstream jsnSnString;
				object->stringify(jsnSnString, 0);
				//--
				ret = RESULT_GET_BMK_SUCCESS;
				strSendData = jsnSnString.str();
			}
			else
			{
				std::string strResult = object->get("msg").convert<std::string>();
				strResult = CMyCodeConvert::Utf8ToGb2312(strResult);

				ret = RESULT_GET_BMK_FAIL;
				strSendData = strResult;
			}
		}
		else if (pTask->strMsg == "getExamBmk")
		{
			Poco::JSON::Object::Ptr objResult = object->getObject("status");
			bResult = objResult->get("success").convert<bool>();
			if (bResult)
			{
				//++添加考试ID和科目ID到结果信息中
				Poco::JSON::Object objExam;
				objExam.set("examId", pTask->nExamID);
				objExam.set("subjectId", pTask->nSubjectID);
				object->set("examInfo", objExam);
				std::stringstream jsnSnString;
				object->stringify(jsnSnString, 0);
				//--
				ret = RESULT_GET_BMK_SUCCESS;
				strSendData = jsnSnString.str();
			}
			else
			{
				std::string strResult = object->get("msg").convert<std::string>();
				strResult = CMyCodeConvert::Utf8ToGb2312(strResult);

				ret = RESULT_GET_BMK_FAIL;
				strSendData = strResult;
			}
			std::cout << "获取考试报名库命令操作完成" << std::endl;
		}
		else if (pTask->strMsg == "createModel")
		{
			Poco::JSON::Object::Ptr objResult = object->getObject("status");
			bResult = objResult->get("success").convert<bool>();
			if (!bResult)
			{
				//后端不存在此模板数据
				ret = RESULT_CREATE_MODEL_NOFIND;
				_mapModelLock_.lock();
				char szIndex[50] = { 0 };
				sprintf(szIndex, "%d_%d", pTask->nExamID, pTask->nSubjectID);
				pMODELINFO pModelInfo = NULL;
				MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
				if (itFind != _mapModel_.end())
					_mapModel_.erase(itFind);
				_mapModelLock_.unlock();
			}
			else
			{
				char szIndex[150] = { 0 };
//				sprintf(szIndex, "%d_%d", pTask->nExamID, pTask->nSubjectID);
				sprintf(szIndex, "%s_%s_N_%d_%d", pTask->strExamName.c_str(), pTask->strSubjectName.c_str(), pTask->nExamID, pTask->nSubjectID);
				std::string strModelPath = SysSet.m_strModelSavePath + "\\";
				strModelPath.append(szIndex);
				strModelPath.append(".mod");
				Poco::File modelFile(CMyCodeConvert::Gb2312ToUtf8(strModelPath));

				if (!objResult->isNull("code"))
				{
					std::string strCode = objResult->get("code").convert<std::string>();
					if (strCode == "NO_UPDATED")	//题卡数据没有修改
					{
						//先判断本地是否存在此模板，如果没有，则重新生成
						if (modelFile.exists())
							ret = RESULT_CREATE_MODEL_NONEED;
						else
							ret = modelHandle(pTask, object);
					}
				}
				else
				{
					//删除本地可能已经存在的模板
					char szModelName[150] = { 0 };
//					sprintf(szModelName, "%d_%d", pTask->nExamID, pTask->nSubjectID);
					sprintf(szModelName, "%s_%s_N_%d_%d", pTask->strExamName.c_str(), pTask->strSubjectName.c_str(), pTask->nExamID, pTask->nSubjectID);
					std::string strModelPath = SysSet.m_strModelSavePath + "\\";
					strModelPath.append(szModelName);
					strModelPath.append(".mod");
					try
					{
						if (modelFile.exists())
							modelFile.remove(true);
					}
					catch (Poco::Exception& exc)
					{
					}
					ret = modelHandle(pTask, object);
				}
			}
		}
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append(pTask->strMsg + " ==> Error when parse json: ");
		strErrInfo.append(jsone.message() + "\tData:" + strGb2312);
		g_Log.LogOutError(strErrInfo);
		strSendData = "异常失败";
		std::cout << strErrInfo << std::endl;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append(pTask->strMsg + " ==> Error: ");
		strErrInfo.append(exc.message() + "\tData:" + strGb2312);
		g_Log.LogOutError(strErrInfo);
		strSendData = "异常失败";
		std::cout << strErrInfo << std::endl;
	}
	catch (...)
	{
		std::string strErrInfo;
		strErrInfo.append(pTask->strMsg + " ==> Unknown error.\tData:" + strGb2312);
		g_Log.LogOutError(strErrInfo);
		strSendData = "异常失败";
		std::cout << strErrInfo << std::endl;
	}

	int nCmd = 0;
	if (pTask->strMsg == "login")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "login_SanXi_huanshuo")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "login2Ty")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "getUserInfo4Ty")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "ezs")
		nCmd = USER_RESPONSE_EXAMINFO;
	else if (pTask->strMsg == "createModel")
		nCmd = USER_RESPONSE_CREATE_MODEL;
	else if (pTask->strMsg == "setElectOmrInfo")
		nCmd = USER_RESPONSE_ELECTOMR_MODEL;
	else if (pTask->strMsg == "getBmk")
		nCmd = USER_RESPONSE_GET_BMK;
	else if (pTask->strMsg == "getExamBmk")
		nCmd = USER_RESPONSE_GET_EXAM_BMK;

	if (pTask->pUser)
		pTask->pUser->SendResponesInfo(nCmd, ret, (char*)strSendData.c_str(), strSendData.length());
	return bResult;
}

void CScanResquestHandler::HandleHttpFail(pSCAN_REQ_TASK pTask, std::string strSendData)
{
	int nCmd = 0;
	if (pTask->strMsg == "login")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "login_SanXi_huanshuo")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "login2Ty")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "getUserInfo4Ty")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "ezs")
		nCmd = USER_RESPONSE_EXAMINFO;
	else if (pTask->strMsg == "createModel")
		nCmd = USER_RESPONSE_CREATE_MODEL;
	else if (pTask->strMsg == "setElectOmrInfo")
		nCmd = USER_RESPONSE_ELECTOMR_MODEL;
	else if (pTask->strMsg == "getBmk")
		nCmd = USER_RESPONSE_GET_BMK;
	else if (pTask->strMsg == "getExamBmk")
		nCmd = USER_RESPONSE_GET_EXAM_BMK;

	int ret = RESULT_ERROR_UNKNOWN;
	if (pTask->strMsg == "getBmk" || pTask->strMsg == "getExamBmk")
	{
		ret = RESULT_GET_BMK_FAIL;
	}
	else if (pTask->strMsg == "ezs")
	{
		ret = RESULT_EXAMINFO_FAIL;
	}

	if (pTask->pUser)
		pTask->pUser->SendResponesInfo(nCmd, ret, (char*)strSendData.c_str(), strSendData.length());
}

int CScanResquestHandler::modelHandle(pSCAN_REQ_TASK pTask, Poco::JSON::Object::Ptr object)
{
	clock_t start, end;
	start = clock();

	int ret = RESULT_ERROR_UNKNOWN;
	
	char szIndex[50] = { 0 };
	sprintf(szIndex, "%d_%d", pTask->nExamID, pTask->nSubjectID);
// 	char szModelName[150] = { 0 };
// 	sprintf(szModelName, "%s_%s_N_%d_%d", pTask->nExamID, pTask->nSubjectID);
	pMODELINFO pModelInfo = NULL;
	MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
	if (itFind != _mapModel_.end())
	{
		pModelInfo = itFind->second;
		if (pModelInfo->pModel)
			SAFE_RELEASE(pModelInfo->pModel);
		
		//创建基本模板
		std::vector<std::vector<int>> vecSheets;
		pMODEL pModel = CreateModel(object, pTask, vecSheets);
		pModelInfo->pModel = pModel;

		//下载PDF
		char szModelName[150] = { 0 };
		sprintf(szModelName, "%s_%s_N_%d_%d", pTask->strExamName.c_str(), pTask->strSubjectName.c_str(), pTask->nExamID, pTask->nSubjectID);
		std::string strModelPath = SysSet.m_strModelSavePath + "\\";
		strModelPath.append(szModelName);
		try
		{
			Poco::File modelDir(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
			if (modelDir.exists())
				modelDir.remove(true);

			modelDir.createDirectories();
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "模板路径(" + strModelPath + ")判断异常: " + exc.message();
			g_Log.LogOutError(strErr);
		}

		bool bResult = GetPdf(object, strModelPath);
		if (bResult) bResult = Pdf2Jpg(strModelPath, vecSheets);
		if (bResult) bResult = InitModelRecog(pModel);				//初始化模板识别参数
		if (bResult) bResult = SaveModel(pModel, strModelPath);
		if (bResult) bResult = ZipModel(pModel, strModelPath);
		if (bResult)
		{
			pModelInfo->strName = szModelName;
			pModelInfo->strName.append(".mod");
			pModelInfo->strPath = strModelPath + ".mod";
			pModelInfo->strMd5 = calcFileMd5(CMyCodeConvert::Gb2312ToUtf8(pModelInfo->strPath));

			std::string strLog;
			Poco::File modelDir(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
			if (modelDir.exists())
			{
				modelDir.remove(true);
				strLog = "模板文件夹(" + strModelPath + ")移除完成";
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}

			//设置后端数据库中扫描模板的名称
			Poco::JSON::Object jsnModel;
			jsnModel.set("examId", pTask->nExamID);
			jsnModel.set("subjectId", pTask->nSubjectID);
			jsnModel.set("tmplateName", CMyCodeConvert::Gb2312ToUtf8(pModelInfo->strName));

			std::stringstream jsnString;
			jsnModel.stringify(jsnString, 0);

			std::string strEzs = pTask->strEzs;
			pSCAN_REQ_TASK pNewTask = new SCAN_REQ_TASK;
			pNewTask->nExamID = pTask->nExamID;
			pNewTask->nSubjectID = pTask->nSubjectID;
			pNewTask->strUri = SysSet.m_strBackUri + "/scanTemplate";
			pNewTask->pUser = pTask->pUser;
			pNewTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
			pNewTask->strMsg = "setScanModel";
			pNewTask->strRequest = jsnString.str();
			g_fmScanReq.lock();
			g_lScanReq.push_back(pNewTask);
			g_fmScanReq.unlock();
			g_Log.LogOut("设置科目扫描模板名称任务已发出。");

			end = clock();
			strLog = Poco::format("模板生成全程完成，耗时(%dms)", (int)(end - start));
			g_Log.LogOut(strLog);
			std::cout << strLog << std::endl;

			ret = RESULT_CREATE_MODEL_SUCCESS;
		}
		else
		{
			ret = RESULT_CREATE_MODEL_FAIL;
			//删除模板列表中预存的模板信息
			_mapModelLock_.lock();
			char szIndex[50] = { 0 };
			sprintf(szIndex, "%d_%d", pTask->nExamID, pTask->nSubjectID);
			pMODELINFO pModelInfo = NULL;
			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind != _mapModel_.end())
				_mapModel_.erase(itFind);
			_mapModelLock_.unlock();

			std::string strLog;
			Poco::File modelDir(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
			if (modelDir.exists())
			{
				modelDir.remove(true);
				strLog = "模板创建失败，删除未完成的模板文件夹(" + strModelPath + ")";
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
			}			
		}
		SAFE_RELEASE(pModel);
	}
	return ret;
}

typedef struct _RectPos_
{
	int nIndex;
	cv::Rect rt;
}RECTPOS;

bool SortByIndex(RECTPOS& rc1, RECTPOS& rc2)
{
	return rc1.nIndex < rc2.nIndex;
}

bool GetHeader(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	Poco::JSON::Object::Ptr objHeader = objTK->getObject("syncHeader");
	Poco::JSON::Object::Ptr objAnchorPoint = objTK->getObject("anchorPoint");

	int nPaperType = 1;		//纸张类型：A3-1，A4-2
	nPaperType = objHeader->get("sheetType").convert<int>();
	pPaperModel->nPaperType = nPaperType;

	//同步头
	std::vector<RECTPOS> vecHeader_H;
	std::vector<RECTPOS> vecHeader_V;
	//获取定点中的同步头信息
	Poco::JSON::Array::Ptr arryItems = objAnchorPoint->getArray("items");
	for (int j = 0; j < arryItems->size(); j++)
	{
		Poco::JSON::Object::Ptr objItem = arryItems->getObject(j);
		int nHor = objItem->get("horIndex").convert<int>();
		int nVer = objItem->get("verIndex").convert<int>();
		RECTPOS rc;
		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
		if (nPaperType == 2)	//A4垂直同步头在右边，A3垂直同步头在左边
		{
			if (nHor == 0 && nVer == 0)
			{
				rc.nIndex = 0;
				vecHeader_H.push_back(rc);
			}
			else if (nHor != 0 && nVer == 0)
			{
				rc.nIndex = nHor;
				vecHeader_H.push_back(rc);
				rc.nIndex = 0;		//交叉定位点，对垂直同步头来说索引是0
				vecHeader_V.push_back(rc);
			}
			else if (nHor != 0 && nVer != 0)
			{
				rc.nIndex = nVer;
				vecHeader_V.push_back(rc);
			}
		}
		else
		{
			if (nHor == 0 && nVer == 0)
			{
				rc.nIndex = 0;
				vecHeader_H.push_back(rc);
				vecHeader_V.push_back(rc);
			}
			else if (nHor != 0 && nVer == 0)
			{
				rc.nIndex = nHor;
				vecHeader_H.push_back(rc);
			}
			else if (nHor == 0 && nVer != 0)
			{
				rc.nIndex = nVer;
				vecHeader_V.push_back(rc);
			}
		}
	}
	Poco::JSON::Array::Ptr arryHorHeaders = objHeader->getArray("horHeaders");
	for (int k = 0; k < arryHorHeaders->size(); k++)
	{
		Poco::JSON::Object::Ptr objItem = arryHorHeaders->getObject(k);
		RECTPOS rc;
		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
		rc.nIndex = objItem->get("index").convert<int>();
		vecHeader_H.push_back(rc);
	}
	Poco::JSON::Array::Ptr arryVerHeaders = objHeader->getArray("verHeaders");
	for (int k = 0; k < arryVerHeaders->size(); k++)
	{
		Poco::JSON::Object::Ptr objItem = arryVerHeaders->getObject(k);
		RECTPOS rc;
		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
		rc.nIndex = objItem->get("index").convert<int>();
		vecHeader_V.push_back(rc);
	}
	std::sort(vecHeader_H.begin(), vecHeader_H.end(), SortByIndex);
	std::sort(vecHeader_V.begin(), vecHeader_V.end(), SortByIndex);

	//设置同步头橡皮筋大小
	cv::Rect rcTracker_H, rcTracker_V;
	cv::Point pt1 = vecHeader_H[0].rt.tl() - cv::Point(50, 50);
	cv::Point pt2 = vecHeader_H[vecHeader_H.size() - 1].rt.br() + cv::Point(50, 50);
	cv::Point pt3 = vecHeader_V[0].rt.tl() - cv::Point(50, 50);
	cv::Point pt4 = vecHeader_V[vecHeader_V.size() - 1].rt.br() + cv::Point(50, 50);
	pPaperModel->rtHTracker = cv::Rect(pt1, pt2);
	pPaperModel->rtVTracker = cv::Rect(pt3, pt4);

	RECTINFO rtSelHTracker, rtSelVTracker;
	rtSelHTracker.eCPType = H_HEAD;
	rtSelHTracker.rt = pPaperModel->rtHTracker;
	rtSelVTracker.eCPType = V_HEAD;
	rtSelVTracker.rt = pPaperModel->rtVTracker;
	pPaperModel->lSelHTracker.push_back(rtSelHTracker);		//识别时查找同步头
	pPaperModel->lSelVTracker.push_back(rtSelVTracker);		//识别时查找同步头

	//设置同步头
	for (int m = 0; m < vecHeader_H.size(); m++)
	{
		RECTINFO rc;
		rc.eCPType = H_HEAD;
		rc.nHItem = vecHeader_H[m].nIndex;
		rc.nVItem = 0;
		rc.rt = vecHeader_H[m].rt;
		pPaperModel->lH_Head.push_back(rc);
	}
	for (int m = 0; m < vecHeader_V.size(); m++)
	{
		RECTINFO rc;
		rc.eCPType = V_HEAD;
		rc.nHItem = 0;
		rc.nVItem = vecHeader_V[m].nIndex;
		rc.rt = vecHeader_V[m].rt;
		pPaperModel->lV_Head.push_back(rc);
	}
	return true;
}

bool GetFixPoint(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	Poco::JSON::Object::Ptr objHeader = objTK->getObject("syncHeader");
	Poco::JSON::Object::Ptr objAnchorPoint = objTK->getObject("anchorPoint");

	int nPaperType = 1;		//纸张类型：A3-1，A4-2
	nPaperType = objHeader->get("sheetType").convert<int>();
	pPaperModel->nPaperType = nPaperType;

	//定点
	std::vector<RECTPOS> vecFixPoint;
	//获取定点中的同步头信息
	Poco::JSON::Array::Ptr arryItems = objAnchorPoint->getArray("items");
	for (int j = 0; j < arryItems->size(); j++)
	{
		Poco::JSON::Object::Ptr objItem = arryItems->getObject(j);
		int nHor = objItem->get("horIndex").convert<int>();
		int nVer = objItem->get("verIndex").convert<int>();
		RECTPOS rc;
		rc.rt.x = objItem->get("x").convert<int>() + 0.5;
		rc.rt.y = objItem->get("y").convert<int>() + 0.5;
		rc.rt.width = objItem->get("width").convert<int>() + 0.5;
		rc.rt.height = objItem->get("height").convert<int>() + 0.5;
		rc.nIndex = j;
		vecFixPoint.push_back(rc);

		cv::Point pt1, pt2;
		pt1 = rc.rt.tl() - cv::Point(80, 80);
		pt2 = rc.rt.br() + cv::Point(80, 80);
		RECTINFO rcFixSel;
		rcFixSel.rt = cv::Rect(pt1, pt2);
		pPaperModel->lSelFixRoi.push_back(rcFixSel);
	}

	//设置定点
	for (auto p : vecFixPoint)
	{
		RECTINFO rc;
		rc.eCPType = Fix_CP;
		rc.rt = p.rt;
		pPaperModel->lFix.push_back(rc);
	}
	return true;
}

bool GetZkzh(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel, pMODEL pModel)
{
	cv::Point ptZkzh1, ptZkzh2;
	if (!objTK->isNull("baseInfo"))
	{
		Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
		if (objBaseInfo->has("zkzh"))
		{
			Poco::JSON::Object::Ptr objZKZH = objBaseInfo->getObject("zkzh");
			Poco::JSON::Array::Ptr arryZkzhItems = objZKZH->getArray("items");
			for (int n = 0; n < arryZkzhItems->size(); n++)
			{
				Poco::JSON::Object::Ptr objItem = arryZkzhItems->getObject(n);
				Poco::JSON::Object::Ptr objPanel = objItem->getObject("panel");
				int nItem = objPanel->get("index").convert<int>();
				if (nItem == 0)
				{
					ptZkzh1.x = objPanel->get("x").convert<int>();
					ptZkzh1.y = objPanel->get("y").convert<int>();
				}
				if (nItem == arryZkzhItems->size() - 1)
				{
					ptZkzh2.x = objPanel->get("x").convert<int>() + objPanel->get("width").convert<int>();
					ptZkzh2.y = objPanel->get("y").convert<int>() + objPanel->get("height").convert<int>();
				}

				//获取每列的准考证信息
				pSN_ITEM pSnItem = new SN_ITEM;
				pSnItem->nItem = nItem;

				Poco::JSON::Array::Ptr arryZkzhGrids = objItem->getArray("grids");
				for (int k = 0; k < arryZkzhGrids->size(); k++)
				{
					Poco::JSON::Object::Ptr objGrids = arryZkzhGrids->getObject(k);
					RECTINFO rc;
					rc.eCPType = SN;
					rc.rt.x = objGrids->get("x").convert<int>();
					rc.rt.y = objGrids->get("y").convert<int>();
					rc.rt.width = objGrids->get("width").convert<int>();
					rc.rt.height = objGrids->get("height").convert<int>();
					rc.nTH = nItem;
					rc.nSnVal = objGrids->get("index").convert<int>();
					rc.nHItem = objGrids->get("horIndex").convert<int>();
					rc.nVItem = objGrids->get("verIndex").convert<int>();
					rc.nRecogFlag = 10;
					pSnItem->lSN.push_back(rc);
				}
				pPaperModel->lSNInfo.push_back(pSnItem);
			}
			ptZkzh1 += cv::Point(2, 2);			//准考证橡皮筋缩放，防止选框太大
			ptZkzh2 -= cv::Point(2, 2);			//准考证橡皮筋缩放，防止选框太大
//			pPaperModel->rtSNTracker = cv::Rect(ptZkzh1, ptZkzh2);
			pPaperModel->rcSNTracker.rt = cv::Rect(ptZkzh1, ptZkzh2);
			pPaperModel->rcSNTracker.nRecogFlag = 10;
		}
		else if (objBaseInfo->has("studentCode"))
		{
			Poco::JSON::Object::Ptr objZKZH = objBaseInfo->getObject("studentCode");
			Poco::JSON::Object::Ptr objPanel = objZKZH->getObject("panel");
			ptZkzh1.x = objPanel->get("x").convert<int>();
			ptZkzh1.y = objPanel->get("y").convert<int>();
			ptZkzh2.x = objPanel->get("x").convert<int>() + objPanel->get("width").convert<int>();
			ptZkzh2.y = objPanel->get("y").convert<int>() + objPanel->get("height").convert<int>();

			ptZkzh1 += cv::Point(2, 2);			//准考证橡皮筋缩放，防止选框太大
			ptZkzh2 -= cv::Point(2, 2);			//准考证橡皮筋缩放，防止选框太大
			//			pPaperModel->rtSNTracker = cv::Rect(ptZkzh1, ptZkzh2);
			pPaperModel->rcSNTracker.rt = cv::Rect(ptZkzh1, ptZkzh2);
			
			pSN_ITEM pSnItem = new SN_ITEM;
			pSnItem->nItem = 0;
			RECTINFO rc;
			rc.eCPType = SN;
			rc.nZkzhType = 2;
			rc.rt = cv::Rect(ptZkzh1, ptZkzh2);
			pSnItem->lSN.push_back(rc);
			pPaperModel->lSNInfo.push_back(pSnItem);
			pModel->nZkzhType = 2;
		}
	}
	return true;
}

bool GetOMR(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	Poco::JSON::Array::Ptr arryElement = objTK->getArray("elements");
	
	for (int m = 0; m < arryElement->size(); m++)
	{
		Poco::JSON::Object::Ptr objElement = arryElement->getObject(m);
		Poco::JSON::Object::Ptr objItem = objElement->getObject("item");		
		int nQuestionType = objElement->get("type").convert<int>();
		if (nQuestionType == 11)	//11表示选择题
		{
			int nOrientation = objElement->get("orientation").convert<int>();
			int nRecogFlag;
			if (nOrientation == 1)
				nRecogFlag = 26;
			else
				nRecogFlag = 42;
			Poco::JSON::Array::Ptr arryQuestions = objItem->getArray("questions");
			for (int n = 0; n < arryQuestions->size(); n++)
			{
				Poco::JSON::Object::Ptr objQuestion = arryQuestions->getObject(n);
				OMR_QUESTION omrItem;
				omrItem.nTH = objQuestion->get("num").convert<int>();
				omrItem.nSingle = objQuestion->get("choiceType").convert<int>() - 1;

				Poco::JSON::Array::Ptr arryOptions = objQuestion->getArray("options");
				for (int k = 0; k < arryOptions->size(); k++)
				{
					Poco::JSON::Object::Ptr objOptions = arryOptions->getObject(k);
					Poco::JSON::Object::Ptr objPanel = objOptions->getObject("panel");
					RECTINFO rc;
					rc.eCPType = OMR;
					rc.rt.x = objPanel->get("x").convert<int>();
					rc.rt.y = objPanel->get("y").convert<int>();
					rc.rt.width = objPanel->get("width").convert<int>();
					rc.rt.height = objPanel->get("height").convert<int>();
					rc.nHItem = objPanel->get("horIndex").convert<int>();
					rc.nVItem = objPanel->get("verIndex").convert<int>();
					rc.nAnswer = (int)objOptions->get("label").convert<char>() - 65;
					rc.nTH = omrItem.nTH;
					rc.nSingle = omrItem.nSingle;
					rc.nRecogFlag = nRecogFlag;
					omrItem.lSelAnswer.push_back(rc);
				}
				pPaperModel->lOMR2.push_back(omrItem);
			}
		}
		else if (nQuestionType == 16)	//选做题
		{
			ELECTOMR_QUESTION sElectOmrQuestion;
			sElectOmrQuestion.sElectOmrGroupInfo.nAllCount = objItem->get("totalNum").convert<int>();
			sElectOmrQuestion.sElectOmrGroupInfo.nRealCount = objItem->get("chooseNum").convert<int>();
			sElectOmrQuestion.sElectOmrGroupInfo.nGroupID = (pPaperModel->nPaper + 1) * 100 + m;		//******************************	这里要改成201,202这样的，加上页数，属于第几页的信息

			Poco::JSON::Array::Ptr arryOptions = objItem->getArray("options");
			for (int k = 0; k < arryOptions->size(); k++)
			{
				Poco::JSON::Object::Ptr objOptions = arryOptions->getObject(k);
				RECTINFO rc;
				rc.eCPType = ELECT_OMR;
				rc.rt.x = objOptions->get("x").convert<int>();
				rc.rt.y = objOptions->get("y").convert<int>();
				rc.rt.width = objOptions->get("width").convert<int>();
				rc.rt.height = objOptions->get("height").convert<int>();
				rc.nHItem = objOptions->get("horIndex").convert<int>();
				rc.nVItem = objOptions->get("verIndex").convert<int>();
				rc.nTH = objOptions->get("num").convert<int>();
				sElectOmrQuestion.lItemInfo.push_back(rc);
			}
			pPaperModel->lElectOmr.push_back(sElectOmrQuestion);
		}
	}
	return true;
}

bool GetCourse(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	Poco::JSON::Object::Ptr objSubject = objTK->getObject("subject");
	Poco::JSON::Array::Ptr arryCourse = objSubject->getArray("items");
	for (int k = 0; k < arryCourse->size(); k++)
	{
		Poco::JSON::Object::Ptr objItem = arryCourse->getObject(k);
		RECTINFO rcCourse;
		rcCourse.eCPType = COURSE;
		rcCourse.rt.x = objItem->get("x").convert<int>();
		rcCourse.rt.y = objItem->get("y").convert<int>();
		rcCourse.rt.width = objItem->get("width").convert<int>();
		rcCourse.rt.height = objItem->get("height").convert<int>();
		rcCourse.nHItem = objItem->get("horIndex").convert<int>();
		rcCourse.nVItem = objItem->get("verIndex").convert<int>();
		pPaperModel->lCourse.push_back(rcCourse);
	}
	return true;
}

bool GetQK(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	if (!objTK->isNull("baseInfo"))
	{
		Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
		Poco::JSON::Object::Ptr objAbsentBreach = objBaseInfo->getObject("absentAndBreach");
		Poco::JSON::Object::Ptr objAbsent = objAbsentBreach->getObject("absent");
		RECTINFO rcAbsent;
		rcAbsent.eCPType = QK_CP;
		rcAbsent.rt.x = objAbsent->get("x").convert<int>();
		rcAbsent.rt.y = objAbsent->get("y").convert<int>();
		rcAbsent.rt.width = objAbsent->get("width").convert<int>();
		rcAbsent.rt.height = objAbsent->get("height").convert<int>();
		rcAbsent.nHItem = objAbsent->get("horIndex").convert<int>();
		rcAbsent.nVItem = objAbsent->get("verIndex").convert<int>();
		pPaperModel->lQK_CP.push_back(rcAbsent);
	}
	return true;
}

bool GetGray(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	Poco::JSON::Object::Ptr objPageNum = objTK->getObject("pageNum");
	Poco::JSON::Array::Ptr arryItems = objPageNum->getArray("items");
	for (int k = 0; k < arryItems->size(); k++)
	{
		Poco::JSON::Object::Ptr objItem = arryItems->getObject(k);
		RECTINFO rcGray;
		rcGray.eCPType = GRAY_CP;
		rcGray.rt.x = objItem->get("x").convert<int>();
		rcGray.rt.y = objItem->get("y").convert<int>();
		rcGray.rt.width = objItem->get("width").convert<int>();
		rcGray.rt.height = objItem->get("height").convert<int>();
		rcGray.nHItem = objItem->get("horIndex").convert<int>();
		rcGray.nVItem = objItem->get("verIndex").convert<int>();
		pPaperModel->lGray.push_back(rcGray);
	}
	return true;
}
pMODEL CScanResquestHandler::CreateModel(Poco::JSON::Object::Ptr object, pSCAN_REQ_TASK pTask, std::vector<std::vector<int>>& vecSheets)
{
	pMODEL pModel = NULL;
	Poco::JSON::Array::Ptr arrySheets = object->getArray("sheets");
	for (int k = 0; k < arrySheets->size(); k++)
	{
		Poco::JSON::Object::Ptr objSheet = arrySheets->getObject(k);
		
		std::vector<int> vecPage;
		Poco::JSON::Array::Ptr arryContents = objSheet->getArray("contents");
		for (int i = 0; i < arryContents->size(); i++)
		{
			int nPage = arryContents->get(i).convert<int>();
			vecPage.push_back(nPage);
		}
		vecSheets.push_back(vecPage);

		std::string strContent = objSheet->get("content").convert<std::string>();
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result;
		result = parser.parse(strContent);		//strJsnData
		Poco::JSON::Array::Ptr arryData = result.extract<Poco::JSON::Array::Ptr>();

		//++test
// 		std::stringstream jsnString;
// 		arryData->stringify(jsnString, 0);
// 		std::string strVal = jsnString.str();
// 		g_Log.LogOut(strVal);
		//--

		pModel = new MODEL;
		for (int i = 0; i < arryData->size(); i++)
		{
			Poco::JSON::Object::Ptr objTK = arryData->getObject(i);
			Poco::JSON::Object::Ptr objSubject = objTK->getObject("subject");
			Poco::JSON::Object::Ptr objPageNum = objTK->getObject("pageNum");

			if (i == 0)
			{
				Poco::JSON::Object::Ptr objCurSubject = objSubject->getObject("curSubject");
//				pModel->strModelName = CMyCodeConvert::Utf8ToGb2312(objCurSubject->get("name").convert<std::string>());		//使用考试ID和科目ID做模板名称
				char szModelName[150] = { 0 };
				sprintf(szModelName, "%s_%s_N_%d_%d", pTask->strExamName.c_str(), pTask->strSubjectName.c_str(), pTask->nExamID, pTask->nSubjectID);
				pModel->strModelName = szModelName;
			}

			std::string strName = Poco::format("model%d.jpg", vecPage[i]);

			pPAPERMODEL pPaperModel = new PAPERMODEL;
			pPaperModel->nPaper = objPageNum->get("curPageNum").convert<int>() - 1;			//add from 0
			pPaperModel->strModelPicName = strName;	//图片名称，目前不知道，考虑从PDF直接转图片然后命名			//**********	test	*****************

			//同步头
			if (pModel->nHasHead)
				GetHeader(objTK, pPaperModel);
			else		//定点
				GetFixPoint(objTK, pPaperModel);

			//准考证号
			GetZkzh(objTK, pPaperModel, pModel);

			//OMR设置
			GetOMR(objTK, pPaperModel);

			//添加科目点
			GetCourse(objTK, pPaperModel);

			//添加缺考点
			GetQK(objTK, pPaperModel);

			//添加灰度点,通过页码点来设置	**********************************************************************************
			GetGray(objTK, pPaperModel);

			//添加白校验点

			//添加试卷模板到总模板
			pModel->vecPaperModel.push_back(pPaperModel);
			if (pPaperModel->nPaperType == 1)				//A3用150dpi扫描
			{
				pModel->nScanDpi = 200;
				pModel->nScanSize = 2;
			}
			else if (pPaperModel->nPaperType == 2)			//A4用200dpi扫描
			{
				pModel->nScanDpi = 200;
				pModel->nScanSize = 1;
			}
		}
		pModel->nPicNum = arryData->size();
		pModel->nType = 1;
		break;					//************	注意：先不支持AB卷模板	****************
	}
	return pModel;
}

bool CScanResquestHandler::GetPdf(Poco::JSON::Object::Ptr object, std::string& strSavePath)
{
	bool bResult = true;
	Poco::JSON::Array::Ptr arrySheets = object->getArray("sheets");
	for (int i = 0; i < arrySheets->size(); i++)
	{
		Poco::JSON::Object::Ptr objSheet = arrySheets->getObject(i);

		Poco::JSON::Array::Ptr arryContents = objSheet->getArray("contents");
		for (int k = 0; k < arryContents->size(); k++)
		{
			int nPage = arryContents->get(k).convert<int>();

			std::string strUri = Poco::format("%s/sheet/download/%d", SysSet.m_strBackUri, nPage);

			std::string strLog;
			clock_t start, end;
			try
			{
				start = clock();
				Poco::URI uri(strUri);
				std::auto_ptr<std::istream> pStr(Poco::URIStreamOpener::defaultOpener().open(uri));

				std::string strPicPath = Poco::format("%s\\model%d.pdf", strSavePath, nPage);
				std::ofstream out(strPicPath, std::ios::binary);
				Poco::StreamCopier::copyStream(*pStr.get(), out);

				out.close();
				end = clock();
				strLog = Poco::format("下载PDF试卷: %s, time: %d", strUri, (int)(end - start));
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}
			catch (Poco::Exception &exc)
			{
				std::string strErr = "下载PDF试卷(" + strUri + ")失败: " + exc.message();
				g_Log.LogOutError(strErr);
				std::cout << strErr << std::endl;
				bResult = false;
			}			
		}
	}
	return bResult;
}

bool CScanResquestHandler::Pdf2Jpg(std::string& strModelPath, std::vector<std::vector<int>>& vecSheets)
{
	bool bResult = true;
	for (int i = 0; i < vecSheets.size(); i++)
	{
		for (int j = 0; j < vecSheets[i].size(); j++)
		{
			std::string strPdfPath = Poco::format("%s\\model%d.pdf", strModelPath, vecSheets[i][j]);
			std::string strUtfPdfPath = CMyCodeConvert::Gb2312ToUtf8(strPdfPath);
			Poco::File picFile(strUtfPdfPath);
			if (!picFile.exists())
			{
				std::string strLog = "pdf试卷文件(" + strPdfPath + ")不存在，停止PDF与jpg转换";
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
				bResult = false;
				return bResult;
			}
			int nPos = strPdfPath.rfind('.');
			std::string strPicOutPath = strPdfPath.substr(0, nPos);
			strPicOutPath.append(".jpg");

			char szPdfPath[250] = { 0 };
			strcpy_s(szPdfPath, strUtfPdfPath.c_str());
			std::wstring strWSrc;
			Poco::UnicodeConverter::toUTF16(szPdfPath, strWSrc);
			
			clock_t start, end;
			start = clock();
			CMuPDFConvert pdfConvert;
			int nNum = 0;
			bResult = pdfConvert.Pdf2Png(strWSrc.c_str(), strPicOutPath.c_str(), nNum);
			end = clock();
			std::string strLog;
			if (bResult)
				strLog = Poco::format("pdf转jpg成功: %s, time: %d", strPicOutPath, (int)(end - start));
			else
				strLog = Poco::format("pdf转jpg失败: %s, time: %d", strPicOutPath, (int)(end - start));
			g_Log.LogOut(strLog);
			std::cout << strLog << std::endl;

			try
			{
				picFile.remove(true);
				std::string  strLog = "删除pdf试卷文件(" + strPdfPath + ")成功";
				g_Log.LogOut(strLog);
			}
			catch (Poco::Exception& exc)
			{
				std::string strErr = "删除pdf试卷文件(" + strPdfPath + ")失败: " + exc.message();
				g_Log.LogOutError(strErr);
			}
		}
	}
	return bResult;
}

//++ 识别相关
void SharpenImage(const cv::Mat &image, cv::Mat &result)
{
	//创建并初始化滤波模板
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = 5;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//对图像进行滤波
	cv::filter2D(image, result, image.depth(), kernel);
}

bool SortByPositionX(RECTINFO& rc1, RECTINFO& rc2)
{
	bool bResult = true;
	bResult = rc1.rt.x < rc2.rt.x ? true : false;
	if (!bResult)
	{
		if (rc1.rt.x == rc2.rt.x)
			bResult = rc1.rt.y < rc2.rt.y ? true : false;
	}
	return bResult;
}
bool SortByPositionY(RECTINFO& rc1, RECTINFO& rc2)
{
	bool bResult = true;
	bResult = rc1.rt.y < rc2.rt.y ? true : false;
	if (!bResult)
	{
		if (rc1.rt.y == rc2.rt.y)
			bResult = rc1.rt.x < rc2.rt.x ? true : false;
	}
	return bResult;
}

bool RecogHHead(int nPic, cv::Mat& matCompPic, pPAPERMODEL pPicModel, RECTINFO rc)
{
	bool bResult = true;
	std::vector<RECTINFO> vecH_Head;

	std::string strErrDesc;

	std::vector<cv::Rect>RectCompList;
	try
	{
		if (rc.rt.x < 0) rc.rt.x = 0;
		if (rc.rt.y < 0) rc.rt.y = 0;
		if (rc.rt.br().x > matCompPic.cols)
		{
			rc.rt.width = matCompPic.cols - rc.rt.x;
		}
		if (rc.rt.br().y > matCompPic.rows)
		{
			rc.rt.height = matCompPic.rows - rc.rt.y;
		}

		cv::Mat matCompRoi;
		matCompRoi = matCompPic(rc.rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
		SharpenImage(matCompRoi, matCompRoi);

		int nThreshold = 100;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);

		cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));	//Size(6, 6)	普通空白框可识别
		cv::dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;
			rm.x = rm.x + rc.rt.x;
			rm.y = rm.y + rc.rt.y;

			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "识别水平同步头异常: " + exc.msg;
		g_Log.LogOutError(strLog);

		bResult = false;
	}
	if (RectCompList.size() == 0)
	{
		bResult = false;
		strErrDesc = "水平同步头数量为0.";
	}
	else
	{
		for (int i = 0; i < RectCompList.size(); i++)
		{
			RECTINFO rcHead;
			rcHead.rt = RectCompList[i];

			rcHead.nGaussKernel = 5;
			rcHead.nSharpKernel = 5;
			rcHead.nCannyKernel = 90;
			rcHead.nDilateKernel = 3;

			rcHead.eCPType = H_HEAD;
			vecH_Head.push_back(rcHead);
		}
		std::sort(vecH_Head.begin(), vecH_Head.end(), SortByPositionX);

		for (int i = 0; i < vecH_Head.size(); i++)
			pPicModel->lH_Head.push_back(vecH_Head[i]);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别水平同步头失败, 原因: %s\n", strErrDesc.c_str());
		g_Log.LogOutError(std::string(szLog));
	}
	return bResult;
}

bool RecogVHead(int nPic, cv::Mat& matCompPic, pPAPERMODEL pPicModel, RECTINFO rc)
{
	bool bResult = true;
	std::vector<RECTINFO> vecV_Head;

	std::string strErrDesc;
	std::vector<cv::Rect>RectCompList;
	try
	{
		if (rc.rt.x < 0) rc.rt.x = 0;
		if (rc.rt.y < 0) rc.rt.y = 0;
		if (rc.rt.br().x > matCompPic.cols)
		{
			rc.rt.width = matCompPic.cols - rc.rt.x;
		}
		if (rc.rt.br().y > matCompPic.rows)
		{
			rc.rt.height = matCompPic.rows - rc.rt.y;
		}

		cv::Mat matCompRoi;
		matCompRoi = matCompPic(rc.rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
		SharpenImage(matCompRoi, matCompRoi);

		int nThreshold = 100;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);

		cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));	//Size(6, 6)	普通空白框可识别
		cv::dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;
			rm.x = rm.x + rc.rt.x;
			rm.y = rm.y + rc.rt.y;

			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "识别垂直同步头异常: " + exc.msg;
		g_Log.LogOutError(strLog);

		bResult = false;
	}
	if (RectCompList.size() == 0)
	{
		bResult = false;
		strErrDesc = "垂直同步头数量为0.";
	}
	else
	{
		for (int i = 0; i < RectCompList.size(); i++)
		{
			RECTINFO rcHead;
			rcHead.rt = RectCompList[i];

			rcHead.nGaussKernel = 5;
			rcHead.nSharpKernel = 5;
			rcHead.nCannyKernel = 90;
			rcHead.nDilateKernel = 3;

			rcHead.eCPType = V_HEAD;
			vecV_Head.push_back(rcHead);
		}
		std::sort(vecV_Head.begin(), vecV_Head.end(), SortByPositionY);

		for (int i = 0; i < vecV_Head.size(); i++)
			pPicModel->lV_Head.push_back(vecV_Head[i]);
	}

	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别垂直同步头失败, 原因: %s\n", strErrDesc.c_str());
		g_Log.LogOutError(std::string(szLog));
	}
	return bResult;
}

bool GetPositionByHead(pPAPERMODEL pPicModel, int nH, int nV, cv::Rect& rt)
{
	RECTLIST::iterator itItemH = pPicModel->lH_Head.begin();
	for (int i = 0; itItemH != pPicModel->lH_Head.end(); itItemH++, i++)
	{
		if (i == nH)
			break;
	}
	RECTLIST::iterator itItemV = pPicModel->lV_Head.begin();
	for (int i = 0; itItemV != pPicModel->lV_Head.end(); itItemV++, i++)
	{
		if (i == nV)
			break;
	}
	rt.x = itItemH->rt.x;
	rt.y = itItemV->rt.y;
	rt.width = itItemH->rt.width;
	rt.height = itItemV->rt.height;
	return true;
}
//--

inline bool RecogGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc)
{
	cv::cvtColor(matSrcRoi, matSrcRoi, CV_BGR2GRAY);
	cv::GaussianBlur(matSrcRoi, matSrcRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);
	SharpenImage(matSrcRoi, matSrcRoi);

	const int channels[1] = { 0 };
	const float* ranges[1];
	const int histSize[1] = { 1 };
	float hranges[2];
	if (rc.eCPType != WHITE_CP)
	{
		hranges[0] = g_nRecogGrayMin;
		hranges[1] = static_cast<float>(rc.nThresholdValue);
		ranges[0] = hranges;
	}
	else
	{
		hranges[0] = static_cast<float>(rc.nThresholdValue);
		hranges[1] = g_nRecogGrayMax_White;	//255			//256时可统计完全空白的点，即RGB值为255的完全空白点;255时只能统计到RGB为254的值，255的值统计不到
		ranges[0] = hranges;
	}
	cv::MatND src_hist;
	cv::calcHist(&matSrcRoi, 1, channels, cv::Mat(), src_hist, 1, histSize, ranges, false);

	rc.fStandardValue = src_hist.at<float>(0);
	rc.fStandardArea = rc.rt.area();
	rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea;



	cv::MatND src_hist2;
	const int histSize2[1] = { 256 };	//rc.nThresholdValue - g_nRecogGrayMin
	const float* ranges2[1];
	float hranges2[2];
	hranges2[0] = 0;
	hranges2[1] = 255;
	ranges2[0] = hranges2;
	cv::calcHist(&matSrcRoi, 1, channels, cv::Mat(), src_hist2, 1, histSize2, ranges2, true, false);
	int nCount = 0;
	for (int i = 0; i < 256; i++)
	{
		nCount += i * src_hist2.at<float>(i);
	}
	rc.fStandardMeanGray = nCount / rc.fStandardArea;
	return true;
}

bool CScanResquestHandler::InitModelRecog(pMODEL pModel)
{
	bool bResult = true;
	std::string strLog;
	clock_t start, end;
	start = clock();

	for (int i = 0; i < pModel->vecPaperModel.size(); i++)
	{
		std::string strModelPicPath = SysSet.m_strModelSavePath + "\\" + pModel->strModelName + "\\" + pModel->vecPaperModel[i]->strModelPicName;

		cv::Mat matSrc = cv::imread(strModelPicPath);

		pPAPERMODEL pPicModel = pModel->vecPaperModel[i];

		//++ test	识别模板的同步头大小
		if (pModel->nHasHead)
		{
			RECTLIST::iterator itBegin = pPicModel->lH_Head.begin();
			RECTINFO rcFist = *itBegin;
			RECTLIST::reverse_iterator itLast = pPicModel->lH_Head.rbegin();
			RECTINFO rcLast = *(itLast);
			RECTINFO rcH_Tracker;
			cv::Point pt1, pt2;
			pt1 = rcFist.rt.tl() - cv::Point(15, 15);
			pt2 = rcLast.rt.br() + cv::Point(15, 15);
			rcH_Tracker.rt = cv::Rect(pt1, pt2);
			pPicModel->lH_Head.clear();
			RecogHHead(i, matSrc, pPicModel, rcH_Tracker);

			itBegin = pPicModel->lV_Head.begin();
			itLast = pPicModel->lV_Head.rbegin();
			rcFist = *itBegin;
			rcLast = *itLast;
			RECTINFO rcV_Tracker;
			pt1 = rcFist.rt.tl() - cv::Point(15, 15);
			pt2 = rcLast.rt.br() + cv::Point(15, 15);
			rcV_Tracker.rt = cv::Rect(pt1, pt2);
			pPicModel->lV_Head.clear();
			RecogVHead(i, matSrc, pPicModel, rcV_Tracker);
		}		
		//--
		for (auto rcFix : pPicModel->lFix)
		{
			rcFix.nThresholdValue = _nFixVal_;
			rcFix.fStandardValuePercent = _fFixThresholdPercent_;
			
			rcFix.nGaussKernel = _nGaussKernel_;
			rcFix.nSharpKernel = _nSharpKernel_;
			rcFix.nCannyKernel = _nCannyKernel_;
			rcFix.nDilateKernel = _nDilateKernel_;

			cv::Mat matComp = matSrc(rcFix.rt);
			RecogGrayValue(matComp, rcFix);
		}
		RECTLIST::iterator itHead_H = pPicModel->lH_Head.begin();
		for (; itHead_H != pPicModel->lH_Head.end(); itHead_H++)
		{
			itHead_H->nThresholdValue = _nHeadVal_;
			itHead_H->fStandardValuePercent = _fHeadThresholdPercent_;

			cv::Mat matComp = matSrc(itHead_H->rt);
			RecogGrayValue(matComp, *itHead_H);
		}
		RECTLIST::iterator itHead_V = pPicModel->lV_Head.begin();
		for (; itHead_V != pPicModel->lV_Head.end(); itHead_V++)
		{
			itHead_V->nThresholdValue = _nHeadVal_;
			itHead_V->fStandardValuePercent = _fHeadThresholdPercent_;

			cv::Mat matComp = matSrc(itHead_V->rt);
			RecogGrayValue(matComp, *itHead_V);
		}
		RECTLIST::iterator itABModel = pPicModel->lABModel.begin();
		for (; itABModel != pPicModel->lABModel.end(); itABModel++)
		{
			itABModel->nThresholdValue = _nABModelVal_;
			itABModel->fStandardValuePercent = _fABModelThresholdPercent_;

			itABModel->nGaussKernel = _nGaussKernel_;
			itABModel->nSharpKernel = _nSharpKernel_;
			itABModel->nCannyKernel = _nCannyKernel_;
			itABModel->nDilateKernel = _nDilateKernel_;

			if (pModel->nHasHead)
				GetPositionByHead(pPicModel, itABModel->nHItem, itABModel->nVItem, itABModel->rt);

			cv::Mat matComp = matSrc(itABModel->rt);
			RecogGrayValue(matComp, *itABModel);
		}
		RECTLIST::iterator itCourse = pPicModel->lCourse.begin();
		for (; itCourse != pPicModel->lCourse.end(); itCourse++)
		{
			itCourse->nThresholdValue = _nCourseVal_;
			itCourse->fStandardValuePercent = _fCourseThresholdPercent_;

			itCourse->nGaussKernel = _nGaussKernel_;
			itCourse->nSharpKernel = _nSharpKernel_;
			itCourse->nCannyKernel = _nCannyKernel_;
			itCourse->nDilateKernel = _nDilateKernel_;

			if (pModel->nHasHead)
				GetPositionByHead(pPicModel, itCourse->nHItem, itCourse->nVItem, itCourse->rt);

			cv::Mat matComp = matSrc(itCourse->rt);
			RecogGrayValue(matComp, *itCourse);
		}
		RECTLIST::iterator itQK = pPicModel->lQK_CP.begin();
		for (; itQK != pPicModel->lQK_CP.end(); itQK++)
		{
			itQK->nThresholdValue = _nQK_CPVal_;
			if (pModel->nHasHead)
				itQK->fStandardValuePercent = _dQKThresholdPercent_Head_;
			else
				itQK->fStandardValuePercent = _dQKThresholdPercent_Fix_;

			itQK->nGaussKernel = _nGaussKernel_;
			itQK->nSharpKernel = _nSharpKernel_;
			itQK->nCannyKernel = _nCannyKernel_;
			itQK->nDilateKernel = _nDilateKernel_;

			if (pModel->nHasHead)
				GetPositionByHead(pPicModel, itQK->nHItem, itQK->nVItem, itQK->rt);

			cv::Mat matComp = matSrc(itQK->rt);
			RecogGrayValue(matComp, *itQK);
		}
		RECTLIST::iterator itGray = pPicModel->lGray.begin();
		for (; itGray != pPicModel->lGray.end(); itGray++)
		{
			itGray->nThresholdValue = _nGrayVal_;
			itGray->fStandardValuePercent = _fGrayThresholdPercent_;

			itGray->nGaussKernel = _nGaussKernel_;
			itGray->nSharpKernel = _nSharpKernel_;
			itGray->nCannyKernel = _nCannyKernel_;
			itGray->nDilateKernel = _nDilateKernel_;

			if (pModel->nHasHead)
				GetPositionByHead(pPicModel, itGray->nHItem, itGray->nVItem, itGray->rt);

			cv::Mat matComp = matSrc(itGray->rt);
			RecogGrayValue(matComp, *itGray);
		}
		RECTLIST::iterator itWhite = pPicModel->lWhite.begin();
		for (; itWhite != pPicModel->lWhite.end(); itWhite++)
		{
			itWhite->nThresholdValue = _nWhiteVal_;
			itWhite->fStandardValuePercent = _fWhiteThresholdPercent_;

			itWhite->nGaussKernel = _nGaussKernel_;
			itWhite->nSharpKernel = _nSharpKernel_;
			itWhite->nCannyKernel = _nCannyKernel_;
			itWhite->nDilateKernel = _nDilateKernel_;

			if (pModel->nHasHead)
				GetPositionByHead(pPicModel, itWhite->nHItem, itWhite->nVItem, itWhite->rt);

			cv::Mat matComp = matSrc(itWhite->rt);
			RecogGrayValue(matComp, *itWhite);
		}
		if (pModel->nZkzhType == 1)
		{
			SNLIST::iterator itSN = pPicModel->lSNInfo.begin();
			for (; itSN != pPicModel->lSNInfo.end(); itSN++)
			{
				pSN_ITEM pSNItem = *itSN;
				RECTLIST::iterator itSNItem = pSNItem->lSN.begin();
				for (; itSNItem != pSNItem->lSN.end(); itSNItem++)
				{
					itSNItem->nThresholdValue = _nSN_;
					if (pModel->nHasHead)
						itSNItem->fStandardValuePercent = _dSnThresholdPercent_Head_;
					else
						itSNItem->fStandardValuePercent = _dSnThresholdPercent_Fix_;

					itSNItem->nGaussKernel = _nGaussKernel_;
					itSNItem->nSharpKernel = _nSharpKernel_;
					itSNItem->nCannyKernel = _nCannyKernel_;
					itSNItem->nDilateKernel = _nDilateKernel_;

					if (pModel->nHasHead)
						GetPositionByHead(pPicModel, itSNItem->nHItem, itSNItem->nVItem, itSNItem->rt);

					cv::Mat matComp = matSrc(itSNItem->rt);
					RecogGrayValue(matComp, *itSNItem);
				}
			}
		}
		OMRLIST::iterator itOmr = pPicModel->lOMR2.begin();
		for (; itOmr != pPicModel->lOMR2.end(); itOmr++)
		{
			RECTLIST::iterator itOmrItem = itOmr->lSelAnswer.begin();
			for (; itOmrItem != itOmr->lSelAnswer.end(); itOmrItem++)
			{
				itOmrItem->nThresholdValue = _nOMR_;
				if (pModel->nHasHead)
					itOmrItem->fStandardValuePercent = _dOmrThresholdPercent_Head_;
				else
					itOmrItem->fStandardValuePercent = _dOmrThresholdPercent_Fix_;

				itOmrItem->nGaussKernel = _nGaussKernel_;
				itOmrItem->nSharpKernel = _nSharpKernel_;
				itOmrItem->nCannyKernel = _nCannyKernel_;
				itOmrItem->nDilateKernel = _nDilateKernel_;

				if (pModel->nHasHead)
					GetPositionByHead(pPicModel, itOmrItem->nHItem, itOmrItem->nVItem, itOmrItem->rt);

				cv::Mat matComp = matSrc(itOmrItem->rt);
				RecogGrayValue(matComp, *itOmrItem);
			}
		}

		ELECTOMR_LIST::iterator itElectOmr = pPicModel->lElectOmr.begin();
		for (; itElectOmr != pPicModel->lElectOmr.end(); itElectOmr++)
		{
			RECTLIST::iterator itOmrItem = itElectOmr->lItemInfo.begin();
			for (; itOmrItem != itElectOmr->lItemInfo.end(); itOmrItem++)
			{
				itOmrItem->nThresholdValue = 235;
				itOmrItem->fStandardValuePercent = 1.1;

				itOmrItem->nGaussKernel = _nGaussKernel_;
				itOmrItem->nSharpKernel = _nSharpKernel_;
				itOmrItem->nCannyKernel = _nCannyKernel_;
				itOmrItem->nDilateKernel = _nDilateKernel_;

				if (pModel->nHasHead)
					GetPositionByHead(pPicModel, itOmrItem->nHItem, itOmrItem->nVItem, itOmrItem->rt);

				cv::Mat matComp = matSrc(itOmrItem->rt);
				RecogGrayValue(matComp, *itOmrItem);
			}
		}
	}

	end = clock();
	strLog = Poco::format("模板图片校验信息识别完成(%dms)", (int)(end - start));
	g_Log.LogOut(strLog);
	std::cout << strLog << std::endl;
	return bResult;
}

bool CScanResquestHandler::SaveModel(pMODEL pModel, std::string& strModelPath)
{
	Poco::JSON::Object jsnModel;
	Poco::JSON::Array  jsnPicModel;
	Poco::JSON::Array  jsnPaperModel;				//卷形模板，AB卷用
	for (int i = 0; i < pModel->nPicNum; i++)
	{
		Poco::JSON::Object jsnPaperObj;

		std::string strPicName = pModel->vecPaperModel[i]->strModelPicName;

		Poco::JSON::Array jsnSNArry;
		Poco::JSON::Array jsnSelHTrackerArry;
		Poco::JSON::Array jsnSelVTrackerArry;
		Poco::JSON::Array jsnSelRoiArry;
		Poco::JSON::Array jsnFixCPArry;
		Poco::JSON::Array jsnHHeadArry;
		Poco::JSON::Array jsnVHeadArry;
		Poco::JSON::Array jsnABModelArry;
		Poco::JSON::Array jsnCourseArry;
		Poco::JSON::Array jsnQKArry;
		Poco::JSON::Array jsnGrayCPArry;
		Poco::JSON::Array jsnWhiteCPArry;
		Poco::JSON::Array jsnOMRArry;
		Poco::JSON::Array jsnElectOmrArry;
		RECTLIST::iterator itFix = pModel->vecPaperModel[i]->lFix.begin();
		for (; itFix != pModel->vecPaperModel[i]->lFix.end(); itFix++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itFix->eCPType);
			jsnObj.set("left", itFix->rt.x);
			jsnObj.set("top", itFix->rt.y);
			jsnObj.set("width", itFix->rt.width);
			jsnObj.set("height", itFix->rt.height);
			jsnObj.set("thresholdValue", itFix->nThresholdValue);
			jsnObj.set("standardValPercent", itFix->fStandardValuePercent);
			jsnObj.set("standardVal", itFix->fStandardValue);
			jsnObj.set("standardArea", itFix->fStandardArea);
			jsnObj.set("standardDensity", itFix->fStandardDensity);
			jsnObj.set("standardMeanGray", itFix->fStandardMeanGray);
			jsnObj.set("standardStddev", itFix->fStandardStddev);

			jsnObj.set("gaussKernel", itFix->nGaussKernel);
			jsnObj.set("sharpKernel", itFix->nSharpKernel);
			jsnObj.set("cannyKernel", itFix->nCannyKernel);
			jsnObj.set("dilateKernel", itFix->nDilateKernel);
			jsnFixCPArry.add(jsnObj);
		}
		RECTLIST::iterator itHHead = pModel->vecPaperModel[i]->lH_Head.begin();
		for (; itHHead != pModel->vecPaperModel[i]->lH_Head.end(); itHHead++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itHHead->eCPType);
			jsnObj.set("left", itHHead->rt.x);
			jsnObj.set("top", itHHead->rt.y);
			jsnObj.set("width", itHHead->rt.width);
			jsnObj.set("height", itHHead->rt.height);
			jsnObj.set("thresholdValue", itHHead->nThresholdValue);
			jsnObj.set("standardValPercent", itHHead->fStandardValuePercent);
			jsnObj.set("standardVal", itHHead->fStandardValue);
			jsnObj.set("standardArea", itHHead->fStandardArea);
			jsnObj.set("standardDensity", itHHead->fStandardDensity);
			jsnObj.set("standardMeanGray", itHHead->fStandardMeanGray);
			jsnObj.set("standardStddev", itHHead->fStandardStddev);

			jsnObj.set("gaussKernel", itHHead->nGaussKernel);
			jsnObj.set("sharpKernel", itHHead->nSharpKernel);
			jsnObj.set("cannyKernel", itHHead->nCannyKernel);
			jsnObj.set("dilateKernel", itHHead->nDilateKernel);
			jsnHHeadArry.add(jsnObj);
		}
		RECTLIST::iterator itVHead = pModel->vecPaperModel[i]->lV_Head.begin();
		for (; itVHead != pModel->vecPaperModel[i]->lV_Head.end(); itVHead++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itVHead->eCPType);
			jsnObj.set("left", itVHead->rt.x);
			jsnObj.set("top", itVHead->rt.y);
			jsnObj.set("width", itVHead->rt.width);
			jsnObj.set("height", itVHead->rt.height);
			jsnObj.set("thresholdValue", itVHead->nThresholdValue);
			jsnObj.set("standardValPercent", itVHead->fStandardValuePercent);
			jsnObj.set("standardVal", itVHead->fStandardValue);
			jsnObj.set("standardArea", itVHead->fStandardArea);
			jsnObj.set("standardDensity", itVHead->fStandardDensity);
			jsnObj.set("standardMeanGray", itVHead->fStandardMeanGray);
			jsnObj.set("standardStddev", itVHead->fStandardStddev);

			jsnObj.set("gaussKernel", itVHead->nGaussKernel);
			jsnObj.set("sharpKernel", itVHead->nSharpKernel);
			jsnObj.set("cannyKernel", itVHead->nCannyKernel);
			jsnObj.set("dilateKernel", itVHead->nDilateKernel);
			jsnVHeadArry.add(jsnObj);
		}
		RECTLIST::iterator itABModel = pModel->vecPaperModel[i]->lABModel.begin();
		for (; itABModel != pModel->vecPaperModel[i]->lABModel.end(); itABModel++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itABModel->eCPType);
			jsnObj.set("left", itABModel->rt.x);
			jsnObj.set("top", itABModel->rt.y);
			jsnObj.set("width", itABModel->rt.width);
			jsnObj.set("height", itABModel->rt.height);
			jsnObj.set("hHeadItem", itABModel->nHItem);
			jsnObj.set("vHeadItem", itABModel->nVItem);
			jsnObj.set("thresholdValue", itABModel->nThresholdValue);
			jsnObj.set("standardValPercent", itABModel->fStandardValuePercent);
			jsnObj.set("standardVal", itABModel->fStandardValue);
			jsnObj.set("standardArea", itABModel->fStandardArea);
			jsnObj.set("standardDensity", itABModel->fStandardDensity);
			jsnObj.set("standardMeanGray", itABModel->fStandardMeanGray);
			jsnObj.set("standardStddev", itABModel->fStandardStddev);

			jsnObj.set("gaussKernel", itABModel->nGaussKernel);
			jsnObj.set("sharpKernel", itABModel->nSharpKernel);
			jsnObj.set("cannyKernel", itABModel->nCannyKernel);
			jsnObj.set("dilateKernel", itABModel->nDilateKernel);
			jsnABModelArry.add(jsnObj);
		}
		RECTLIST::iterator itCourse = pModel->vecPaperModel[i]->lCourse.begin();
		for (; itCourse != pModel->vecPaperModel[i]->lCourse.end(); itCourse++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itCourse->eCPType);
			jsnObj.set("left", itCourse->rt.x);
			jsnObj.set("top", itCourse->rt.y);
			jsnObj.set("width", itCourse->rt.width);
			jsnObj.set("height", itCourse->rt.height);
			jsnObj.set("hHeadItem", itCourse->nHItem);
			jsnObj.set("vHeadItem", itCourse->nVItem);
			jsnObj.set("thresholdValue", itCourse->nThresholdValue);
			jsnObj.set("standardValPercent", itCourse->fStandardValuePercent);
			jsnObj.set("standardVal", itCourse->fStandardValue);
			jsnObj.set("standardArea", itCourse->fStandardArea);
			jsnObj.set("standardDensity", itCourse->fStandardDensity);
			jsnObj.set("standardMeanGray", itCourse->fStandardMeanGray);
			jsnObj.set("standardStddev", itCourse->fStandardStddev);

			jsnObj.set("gaussKernel", itCourse->nGaussKernel);
			jsnObj.set("sharpKernel", itCourse->nSharpKernel);
			jsnObj.set("cannyKernel", itCourse->nCannyKernel);
			jsnObj.set("dilateKernel", itCourse->nDilateKernel);
			jsnCourseArry.add(jsnObj);
		}
		RECTLIST::iterator itQKCP = pModel->vecPaperModel[i]->lQK_CP.begin();
		for (; itQKCP != pModel->vecPaperModel[i]->lQK_CP.end(); itQKCP++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itQKCP->eCPType);
			jsnObj.set("left", itQKCP->rt.x);
			jsnObj.set("top", itQKCP->rt.y);
			jsnObj.set("width", itQKCP->rt.width);
			jsnObj.set("height", itQKCP->rt.height);
			jsnObj.set("hHeadItem", itQKCP->nHItem);
			jsnObj.set("vHeadItem", itQKCP->nVItem);
			jsnObj.set("thresholdValue", itQKCP->nThresholdValue);
			jsnObj.set("standardValPercent", itQKCP->fStandardValuePercent);
			jsnObj.set("standardVal", itQKCP->fStandardValue);
			jsnObj.set("standardArea", itQKCP->fStandardArea);
			jsnObj.set("standardDensity", itQKCP->fStandardDensity);
			jsnObj.set("standardMeanGray", itQKCP->fStandardMeanGray);
			jsnObj.set("standardStddev", itQKCP->fStandardStddev);

			jsnObj.set("gaussKernel", itQKCP->nGaussKernel);
			jsnObj.set("sharpKernel", itQKCP->nSharpKernel);
			jsnObj.set("cannyKernel", itQKCP->nCannyKernel);
			jsnObj.set("dilateKernel", itQKCP->nDilateKernel);
			jsnQKArry.add(jsnObj);
		}
		RECTLIST::iterator itGrayCP = pModel->vecPaperModel[i]->lGray.begin();
		for (; itGrayCP != pModel->vecPaperModel[i]->lGray.end(); itGrayCP++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itGrayCP->eCPType);
			jsnObj.set("left", itGrayCP->rt.x);
			jsnObj.set("top", itGrayCP->rt.y);
			jsnObj.set("width", itGrayCP->rt.width);
			jsnObj.set("height", itGrayCP->rt.height);
			jsnObj.set("hHeadItem", itGrayCP->nHItem);
			jsnObj.set("vHeadItem", itGrayCP->nVItem);
			jsnObj.set("thresholdValue", itGrayCP->nThresholdValue);
			jsnObj.set("standardValPercent", itGrayCP->fStandardValuePercent);
			jsnObj.set("standardVal", itGrayCP->fStandardValue);
			jsnObj.set("standardArea", itGrayCP->fStandardArea);
			jsnObj.set("standardDensity", itGrayCP->fStandardDensity);
			jsnObj.set("standardMeanGray", itGrayCP->fStandardMeanGray);
			jsnObj.set("standardStddev", itGrayCP->fStandardStddev);

			jsnObj.set("gaussKernel", itGrayCP->nGaussKernel);
			jsnObj.set("sharpKernel", itGrayCP->nSharpKernel);
			jsnObj.set("cannyKernel", itGrayCP->nCannyKernel);
			jsnObj.set("dilateKernel", itGrayCP->nDilateKernel);
			jsnGrayCPArry.add(jsnObj);
		}
		RECTLIST::iterator itWhiteCP = pModel->vecPaperModel[i]->lWhite.begin();
		for (; itWhiteCP != pModel->vecPaperModel[i]->lWhite.end(); itWhiteCP++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itWhiteCP->eCPType);
			jsnObj.set("left", itWhiteCP->rt.x);
			jsnObj.set("top", itWhiteCP->rt.y);
			jsnObj.set("width", itWhiteCP->rt.width);
			jsnObj.set("height", itWhiteCP->rt.height);
			jsnObj.set("hHeadItem", itWhiteCP->nHItem);
			jsnObj.set("vHeadItem", itWhiteCP->nVItem);
			jsnObj.set("thresholdValue", itWhiteCP->nThresholdValue);
			jsnObj.set("standardValPercent", itWhiteCP->fStandardValuePercent);
			jsnObj.set("standardVal", itWhiteCP->fStandardValue);
			jsnObj.set("standardArea", itWhiteCP->fStandardArea);
			jsnObj.set("standardDensity", itWhiteCP->fStandardDensity);
			jsnObj.set("standardMeanGray", itWhiteCP->fStandardMeanGray);
			jsnObj.set("standardStddev", itWhiteCP->fStandardStddev);

			jsnObj.set("gaussKernel", itWhiteCP->nGaussKernel);
			jsnObj.set("sharpKernel", itWhiteCP->nSharpKernel);
			jsnObj.set("cannyKernel", itWhiteCP->nCannyKernel);
			jsnObj.set("dilateKernel", itWhiteCP->nDilateKernel);
			jsnWhiteCPArry.add(jsnObj);
		}
		RECTLIST::iterator itSelRoi = pModel->vecPaperModel[i]->lSelFixRoi.begin();
		for (; itSelRoi != pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itSelRoi->eCPType);
			jsnObj.set("left", itSelRoi->rt.x);
			jsnObj.set("top", itSelRoi->rt.y);
			jsnObj.set("width", itSelRoi->rt.width);
			jsnObj.set("height", itSelRoi->rt.height);
			jsnObj.set("thresholdValue", itSelRoi->nThresholdValue);
			jsnObj.set("standardValPercent", itSelRoi->fStandardValuePercent);
			//			jsnObj.set("standardVal", itSelRoi->fStandardValue);

			jsnObj.set("gaussKernel", itSelRoi->nGaussKernel);
			jsnObj.set("sharpKernel", itSelRoi->nSharpKernel);
			jsnObj.set("cannyKernel", itSelRoi->nCannyKernel);
			jsnObj.set("dilateKernel", itSelRoi->nDilateKernel);
			jsnSelRoiArry.add(jsnObj);
		}
		RECTLIST::iterator itSelHTracker = pModel->vecPaperModel[i]->lSelHTracker.begin();
		for (; itSelHTracker != pModel->vecPaperModel[i]->lSelHTracker.end(); itSelHTracker++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itSelHTracker->eCPType);
			jsnObj.set("left", itSelHTracker->rt.x);
			jsnObj.set("top", itSelHTracker->rt.y);
			jsnObj.set("width", itSelHTracker->rt.width);
			jsnObj.set("height", itSelHTracker->rt.height);
			jsnObj.set("thresholdValue", itSelHTracker->nThresholdValue);
			jsnObj.set("standardValPercent", itSelHTracker->fStandardValuePercent);
			//			jsnObj.set("standardVal", itSelHTracker->fStandardValue);

			jsnObj.set("gaussKernel", itSelHTracker->nGaussKernel);
			jsnObj.set("sharpKernel", itSelHTracker->nSharpKernel);
			jsnObj.set("cannyKernel", itSelHTracker->nCannyKernel);
			jsnObj.set("dilateKernel", itSelHTracker->nDilateKernel);
			jsnSelHTrackerArry.add(jsnObj);
		}
		RECTLIST::iterator itSelVTracker = pModel->vecPaperModel[i]->lSelVTracker.begin();
		for (; itSelVTracker != pModel->vecPaperModel[i]->lSelVTracker.end(); itSelVTracker++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itSelVTracker->eCPType);
			jsnObj.set("left", itSelVTracker->rt.x);
			jsnObj.set("top", itSelVTracker->rt.y);
			jsnObj.set("width", itSelVTracker->rt.width);
			jsnObj.set("height", itSelVTracker->rt.height);
			jsnObj.set("thresholdValue", itSelVTracker->nThresholdValue);
			jsnObj.set("standardValPercent", itSelVTracker->fStandardValuePercent);
			//			jsnObj.set("standardVal", itSelVTracker->fStandardValue);

			jsnObj.set("gaussKernel", itSelVTracker->nGaussKernel);
			jsnObj.set("sharpKernel", itSelVTracker->nSharpKernel);
			jsnObj.set("cannyKernel", itSelVTracker->nCannyKernel);
			jsnObj.set("dilateKernel", itSelVTracker->nDilateKernel);
			jsnSelVTrackerArry.add(jsnObj);
		}
		OMRLIST::iterator itOmr = pModel->vecPaperModel[i]->lOMR2.begin();
		for (; itOmr != pModel->vecPaperModel[i]->lOMR2.end(); itOmr++)
		{
			Poco::JSON::Object jsnTHObj;
			Poco::JSON::Array  jsnArry;
			RECTLIST::iterator itOmrSel = itOmr->lSelAnswer.begin();
			for (; itOmrSel != itOmr->lSelAnswer.end(); itOmrSel++)
			{
				Poco::JSON::Object jsnObj;
				jsnObj.set("eType", (int)itOmrSel->eCPType);
				jsnObj.set("nTH", itOmrSel->nTH);
				jsnObj.set("nAnswer", itOmrSel->nAnswer);
				jsnObj.set("nSingle", itOmrSel->nSingle);
				jsnObj.set("nOmrRecogFlag", itOmrSel->nRecogFlag);
				jsnObj.set("left", itOmrSel->rt.x);
				jsnObj.set("top", itOmrSel->rt.y);
				jsnObj.set("width", itOmrSel->rt.width);
				jsnObj.set("height", itOmrSel->rt.height);
				jsnObj.set("hHeadItem", itOmrSel->nHItem);
				jsnObj.set("vHeadItem", itOmrSel->nVItem);
				jsnObj.set("thresholdValue", itOmrSel->nThresholdValue);
				jsnObj.set("standardValPercent", itOmrSel->fStandardValuePercent);
				jsnObj.set("standardVal", itOmrSel->fStandardValue);
				jsnObj.set("standardArea", itOmrSel->fStandardArea);
				jsnObj.set("standardDensity", itOmrSel->fStandardDensity);
				jsnObj.set("standardMeanGray", itOmrSel->fStandardMeanGray);
				jsnObj.set("standardStddev", itOmrSel->fStandardStddev);

				jsnObj.set("gaussKernel", itOmrSel->nGaussKernel);
				jsnObj.set("sharpKernel", itOmrSel->nSharpKernel);
				jsnObj.set("cannyKernel", itOmrSel->nCannyKernel);
				jsnObj.set("dilateKernel", itOmrSel->nDilateKernel);
				jsnArry.add(jsnObj);
			}
			jsnTHObj.set("nTH", itOmr->nTH);
			jsnTHObj.set("nSingle", itOmr->nSingle);
			jsnTHObj.set("omrlist", jsnArry);
			jsnOMRArry.add(jsnTHObj);
		}
		SNLIST::iterator itSn = pModel->vecPaperModel[i]->lSNInfo.begin();
		for (; itSn != pModel->vecPaperModel[i]->lSNInfo.end(); itSn++)
		{
			Poco::JSON::Object jsnSNObj;
			Poco::JSON::Array  jsnArry;
			RECTLIST::iterator itSnDetail = (*itSn)->lSN.begin();
			for (; itSnDetail != (*itSn)->lSN.end(); itSnDetail++)
			{
				Poco::JSON::Object jsnObj;
				jsnObj.set("eType", (int)itSnDetail->eCPType);
				jsnObj.set("nTH", itSnDetail->nTH);
				jsnObj.set("nSnVal", itSnDetail->nSnVal);
				jsnObj.set("nAnswer", itSnDetail->nAnswer);
				jsnObj.set("nSingle", itSnDetail->nSingle);
				jsnObj.set("nSnRecogFlag", itSnDetail->nRecogFlag);
				jsnObj.set("left", itSnDetail->rt.x);
				jsnObj.set("top", itSnDetail->rt.y);
				jsnObj.set("width", itSnDetail->rt.width);
				jsnObj.set("height", itSnDetail->rt.height);
				jsnObj.set("hHeadItem", itSnDetail->nHItem);
				jsnObj.set("vHeadItem", itSnDetail->nVItem);
				jsnObj.set("thresholdValue", itSnDetail->nThresholdValue);
				jsnObj.set("standardValPercent", itSnDetail->fStandardValuePercent);
				jsnObj.set("standardVal", itSnDetail->fStandardValue);
				jsnObj.set("standardArea", itSnDetail->fStandardArea);
				jsnObj.set("standardDensity", itSnDetail->fStandardDensity);
				jsnObj.set("standardMeanGray", itSnDetail->fStandardMeanGray);
				jsnObj.set("standardStddev", itSnDetail->fStandardStddev);

				jsnObj.set("gaussKernel", itSnDetail->nGaussKernel);
				jsnObj.set("sharpKernel", itSnDetail->nSharpKernel);
				jsnObj.set("cannyKernel", itSnDetail->nCannyKernel);
				jsnObj.set("dilateKernel", itSnDetail->nDilateKernel);
				jsnArry.add(jsnObj);
			}
			jsnSNObj.set("nItem", (*itSn)->nItem);
			jsnSNObj.set("nRecogVal", (*itSn)->nRecogVal);
			jsnSNObj.set("snList", jsnArry);
			jsnSNArry.add(jsnSNObj);
		}
		ELECTOMR_LIST::iterator itElectOmr = pModel->vecPaperModel[i]->lElectOmr.begin();
		for (; itElectOmr != pModel->vecPaperModel[i]->lElectOmr.end(); itElectOmr++)
		{
			pModel->nHasElectOmr = 1;		//设置标识

			Poco::JSON::Object jsnTHObj;
			Poco::JSON::Array  jsnArry;
			RECTLIST::iterator itOmrSel = itElectOmr->lItemInfo.begin();
			for (; itOmrSel != itElectOmr->lItemInfo.end(); itOmrSel++)
			{
				Poco::JSON::Object jsnObj;
				jsnObj.set("eType", (int)itOmrSel->eCPType);
				jsnObj.set("nTH", itOmrSel->nTH);
				jsnObj.set("nAnswer", itOmrSel->nAnswer);
				jsnObj.set("left", itOmrSel->rt.x);
				jsnObj.set("top", itOmrSel->rt.y);
				jsnObj.set("width", itOmrSel->rt.width);
				jsnObj.set("height", itOmrSel->rt.height);
				jsnObj.set("hHeadItem", itOmrSel->nHItem);
				jsnObj.set("vHeadItem", itOmrSel->nVItem);
				jsnObj.set("thresholdValue", itOmrSel->nThresholdValue);
				jsnObj.set("standardValPercent", itOmrSel->fStandardValuePercent);
				jsnObj.set("standardVal", itOmrSel->fStandardValue);
				jsnObj.set("standardArea", itOmrSel->fStandardArea);
				jsnObj.set("standardDensity", itOmrSel->fStandardDensity);
				jsnObj.set("standardMeanGray", itOmrSel->fStandardMeanGray);
				jsnObj.set("standardStddev", itOmrSel->fStandardStddev);

				jsnObj.set("gaussKernel", itOmrSel->nGaussKernel);
				jsnObj.set("sharpKernel", itOmrSel->nSharpKernel);
				jsnObj.set("cannyKernel", itOmrSel->nCannyKernel);
				jsnObj.set("dilateKernel", itOmrSel->nDilateKernel);
				jsnArry.add(jsnObj);
			}
			jsnTHObj.set("nGroupID", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnTHObj.set("nAllCount", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnTHObj.set("nRealCount", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnTHObj.set("omrlist", jsnArry);
			jsnElectOmrArry.add(jsnTHObj);
		}
		jsnPaperObj.set("paperNum", i);
		jsnPaperObj.set("modelPicName", CMyCodeConvert::Gb2312ToUtf8(strPicName));		//CMyCodeConvert::Gb2312ToUtf8(T2A(strPicName))
		jsnPaperObj.set("FixCP", jsnFixCPArry);
		jsnPaperObj.set("H_Head", jsnHHeadArry);
		jsnPaperObj.set("V_Head", jsnVHeadArry);
		jsnPaperObj.set("ABModel", jsnABModelArry);
		jsnPaperObj.set("Course", jsnCourseArry);
		jsnPaperObj.set("QKCP", jsnQKArry);
		jsnPaperObj.set("GrayCP", jsnGrayCPArry);
		jsnPaperObj.set("WhiteCP", jsnWhiteCPArry);
		jsnPaperObj.set("selRoiRect", jsnSelRoiArry);
		jsnPaperObj.set("hTrackerRect", jsnSelHTrackerArry);
		jsnPaperObj.set("vTrackerRect", jsnSelVTrackerArry);
		jsnPaperObj.set("selOmrRect", jsnOMRArry);
		jsnPaperObj.set("snList", jsnSNArry);
		jsnPaperObj.set("electOmrList", jsnElectOmrArry);

		jsnPaperObj.set("picW", pModel->vecPaperModel[i]->nPicW);		//add on 16.8.29
		jsnPaperObj.set("picH", pModel->vecPaperModel[i]->nPicH);		//add on 16.8.29
		jsnPaperObj.set("rtHTracker.x", pModel->vecPaperModel[i]->rtHTracker.x);
		jsnPaperObj.set("rtHTracker.y", pModel->vecPaperModel[i]->rtHTracker.y);
		jsnPaperObj.set("rtHTracker.width", pModel->vecPaperModel[i]->rtHTracker.width);
		jsnPaperObj.set("rtHTracker.height", pModel->vecPaperModel[i]->rtHTracker.height);
		jsnPaperObj.set("rtVTracker.x", pModel->vecPaperModel[i]->rtVTracker.x);
		jsnPaperObj.set("rtVTracker.y", pModel->vecPaperModel[i]->rtVTracker.y);
		jsnPaperObj.set("rtVTracker.width", pModel->vecPaperModel[i]->rtVTracker.width);
		jsnPaperObj.set("rtVTracker.height", pModel->vecPaperModel[i]->rtVTracker.height);
		jsnPaperObj.set("rtSNTracker.x", pModel->vecPaperModel[i]->rcSNTracker.rt.x);
		jsnPaperObj.set("rtSNTracker.y", pModel->vecPaperModel[i]->rcSNTracker.rt.y);
		jsnPaperObj.set("rtSNTracker.width", pModel->vecPaperModel[i]->rcSNTracker.rt.width);
		jsnPaperObj.set("rtSNTracker.height", pModel->vecPaperModel[i]->rcSNTracker.rt.height);
		jsnPaperObj.set("rtSNTracker.nRecogFlag", pModel->vecPaperModel[i]->rcSNTracker.nRecogFlag);

		jsnPicModel.add(jsnPaperObj);
	}

	jsnModel.set("modelName", CMyCodeConvert::Gb2312ToUtf8(pModel->strModelName));		//CMyCodeConvert::Gb2312ToUtf8(T2A(pModel->strModelName))
	jsnModel.set("modelDesc", CMyCodeConvert::Gb2312ToUtf8(pModel->strModelDesc));
	jsnModel.set("modelType", pModel->nType);
	jsnModel.set("modeSaveMode", pModel->nSaveMode);
	jsnModel.set("paperModelCount", pModel->nPicNum);			//此模板有几页试卷(图片)
	jsnModel.set("enableModify", pModel->nEnableModify);		//是否可以修改标识
	jsnModel.set("abPaper", pModel->nABModel);					//是否是AB卷					*************	暂时没加入AB卷的模板	**************
	jsnModel.set("hasHead", pModel->nHasHead);					//是否有同步头
	jsnModel.set("hasElectOmr", pModel->nHasElectOmr);			//是否有选做题
	jsnModel.set("nZkzhType", pModel->nZkzhType);				//准考证号识别类型
	jsnModel.set("nScanDpi", pModel->nScanDpi);					//扫描的dpi设置
	jsnModel.set("nScanSize", pModel->nScanSize);				//扫描用的纸张类型，1-a4, 2-a3, 3-定制
	jsnModel.set("nScanAutoCut", pModel->nAutoCut);				//扫描仪是否自动裁剪，超长卡不能裁剪

	jsnModel.set("nExamId", pModel->nExamID);
	jsnModel.set("nSubjectId", pModel->nSubjectID);
	jsnModel.set("paperInfo", jsnPicModel);

	std::stringstream jsnString;
	jsnModel.stringify(jsnString, 0);

	std::string strFileData;
#ifdef USES_FILE_CRYPTOGRAM
	if (!encString(jsnString.str(), strFileData))
		strFileData = jsnString.str();
#else
	strFileData = jsnString.str();
#endif

	std::string strJsnFile = strModelPath;
	strJsnFile += "\\model.dat";
	ofstream out(strJsnFile);
	if (!out)	return false;
	out << strFileData.c_str();
	out.close();

	return true;
}

int CScanResquestHandler::ZipModel(pMODEL pModel, std::string& strModelPath)
{
	bool bResult = true;
	std::string strZipName = strModelPath + ".mod";

	try
	{
		Poco::File zipModel(CMyCodeConvert::Gb2312ToUtf8(strZipName));
		if (zipModel.exists())
			zipModel.remove(true);
	}
	catch (Poco::Exception& exc)
	{
		std::string strErr = "模板文件(" + strZipName + ")判断异常: " + exc.message();
		g_Log.LogOutError(strErr);
	}

	clock_t startTime, endTime;
	startTime = clock();
	std::string strLog = "开始模板文件压缩: " + strZipName;
	g_Log.LogOut(strLog);
	std::cout << strLog << std::endl;

	zipFile zf = NULL;
#ifdef USEWIN32IOAPI
	zlib_filefunc64_def ffunc = { 0 };
#endif
	char *zipfilename = const_cast<char*>(strZipName.c_str());
	const char* password = NULL;
	password = "static";
	
	void* buf = NULL;
	int size_buf = WRITEBUFFERSIZE;
	int err = 0;
	int errclose = 0;

	int opt_overwrite = APPEND_STATUS_CREATE;
	int opt_compress_level = Z_DEFAULT_COMPRESSION;

	buf = (void*)malloc(size_buf);
	if (buf == NULL)
	{
		printf("Error allocating memory\n");
		return ZIP_INTERNALERROR;
	}

#ifdef USEWIN32IOAPI
	fill_win32_filefunc64A(&ffunc);
	zf = zipOpen2_64(zipfilename, opt_overwrite, NULL, &ffunc);
#else
	zf = zipOpen64(zipfilename, opt_overwrite);
#endif

	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strModelPath);
	Poco::DirectoryIterator it(strUtf8Path);
	Poco::DirectoryIterator end;
	while (it != end)
	{
		Poco::Path p(it->path());
		std::string strFilePath = CMyCodeConvert::Utf8ToGb2312(it->path());
		if (it->isFile())
		{
			Poco::File modelFile(it->path());
			
			FILE *fin = NULL;
			int size_read = 0;
			const char* filenameinzip = strFilePath.c_str();
			const char *savefilenameinzip;
			zip_fileinfo zi = { 0 };
			unsigned long crcFile = 0;
			int zip64 = 0;
			
			/* Get information about the file on disk so we can store it in zip */
			filetime(filenameinzip, &zi.tmz_date, &zi.dosDate);

			std::string strName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());
			savefilenameinzip = strName.c_str();

			/* Add to zip file */
			err = zipOpenNewFileInZip3_64(zf, savefilenameinzip, &zi,
										  NULL, 0, NULL, 0, NULL /* comment*/,
										  (opt_compress_level != 0) ? Z_DEFLATED : 0,
										  opt_compress_level, 0,
										  -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
										  password, crcFile, zip64);

			if (err != ZIP_OK)
				printf("error in opening %s in zipfile (%d)\n", filenameinzip, err);
			else
			{
				fin = FOPEN_FUNC(filenameinzip, "rb");
				if (fin == NULL)
				{
					err = ZIP_ERRNO;
					printf("error in opening %s for reading\n", filenameinzip);
				}
			}

			if (err == ZIP_OK)
			{
				/* Read contents of file and write it to zip */
				do
				{
					size_read = (int)fread(buf, 1, size_buf, fin);
					if ((size_read < size_buf) && (feof(fin) == 0))
					{
						printf("error in reading %s\n", filenameinzip);
						err = ZIP_ERRNO;
					}

					if (size_read > 0)
					{
						err = zipWriteInFileInZip(zf, buf, size_read);
						if (err < 0)
							printf("error in writing %s in the zipfile (%d)\n", filenameinzip, err);
					}
				} while ((err == ZIP_OK) && (size_read > 0));
			}

			if (fin)
				fclose(fin);

			if (err < 0)
			{
				err = ZIP_ERRNO;
				strLog = "压缩文件失败(" + strName + ")";
				g_Log.LogOutError(strLog);
			}
			else
			{
				err = zipCloseFileInZip(zf);
				if (err != ZIP_OK)
					printf("error in closing %s in the zipfile (%d)\n", filenameinzip, err);
			}
		}
		it++;
	}

	errclose = zipClose(zf, NULL);
	if (errclose != ZIP_OK)
		printf("error in closing %s (%d)\n", zipfilename, errclose);

	free(buf);

	endTime = clock();
	strLog = Poco::format("模板文件压缩完成: %s, 时间: %dms", strZipName, (int)(endTime - startTime));
	g_Log.LogOut(strLog);
	std::cout << strLog << std::endl;
	return bResult;
}

void CScanResquestHandler::GetUserInfo4Ty(std::string& strSessionId, pSCAN_REQ_TASK pOldTask)
{
	std::string strAppKey = "SP0000000TEST";
	std::string strAppValue = "63d4311d46714a39-a54cf2b0537a79b6TEST";
	std::string strMsgFormat = "json";
	std::string strMethod = "getAamUser";
	std::string strSha1Src = Poco::format("%sappKey%smessageFormat%smethod%ssessionId%sv1.0%s", strAppValue, strAppKey, strMsgFormat, strMethod, strSessionId, strAppValue);
//	std::string strSha1Src = Poco::format("%sappKey%smethod%ssession%sv1.0%s", strAppValue, strAppKey, strMethod, strSessionId, strAppValue);
	Poco::SHA1Engine engine;
	engine.update(strSha1Src);
	std::string strSHA1 = Poco::DigestEngine::digestToHex(engine.digest());
	std::string strSHA1_Up = Poco::toUpper(strSHA1);
	std::string strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&sessionId=%s&sign=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strSessionId, strSHA1_Up);
//	std::string strUriValue = Poco::format("/router?appKey=%s&method=%s&session=%s&sign=%s&v=1.0", strAppKey, strMethod, strSessionId, strSHA1_Up);

	pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
	pTask->strUri = SysSet.m_strBackUri + strUriValue;
	pTask->strMsg = "getUserInfo4Ty";
	pTask->pUser = pOldTask->pUser;
	pTask->strUser = pOldTask->strUser;
	pTask->strPwd = pOldTask->strPwd;
	std::string strLog = Poco::format("天喻获取用户信息: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
	g_Log.LogOut(strLog);

	g_fmScanReq.lock();
	g_lScanReq.push_back(pTask);
	g_fmScanReq.unlock();
}

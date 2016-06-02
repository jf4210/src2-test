#include "ScanResquestHandler.h"


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
			std::string strLog = "发送数据:" + pTask->strRequest + "\t\t后端数据返回错误,不是200 OK";
			g_Log.LogOut(strLog);
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrorInfo;
		strErrorInfo.append(exc.displayText());
		strErrorInfo.append("\tDetail: " + pTask->strRequest);

		std::cout << strErrorInfo << std::endl;
		g_Log.LogOutError(strErrorInfo);
	}
	catch (...)
	{
		std::string strErrorInfo;
		strErrorInfo.append("**** Unknown error ***\tDetail: " + pTask->strRequest);

		std::cout << strErrorInfo << std::endl;
		g_Log.LogOutError(strErrorInfo);
	}
}

bool CScanResquestHandler::ParseResult(std::string& strInput, pSCAN_REQ_TASK pTask)
{
	bool bResult = false;
	int ret = 0;
	std::string strSendData;
	std::string strUtf8 = CMyCodeConvert::Utf8ToGb2312(strInput);
	std::string strLog = "收到后端数据: " + strUtf8;
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
			std::string strResult = object->get("result").convert<std::string>();
			strResult = CMyCodeConvert::Utf8ToGb2312(strResult);

			if (bResult)
			{
				ret = RESULT_SUCCESS;
				std::string strEzs = object->get("ezs").convert<std::string>();
				std::string strUserInfo = object->get("user").convert<std::string>();
				int nTeacher = object->get("teacherId").convert<int>();


				ST_LOGIN_RESULT stResult;
				stResult.nTeacherId = nTeacher;
				strncpy(stResult.szEzs, strEzs.c_str(), strEzs.length());
				strncpy(stResult.szUserInfo, strUserInfo.c_str(), strUserInfo.length());

				int n = sizeof(stResult);

				char szLoginResult[1024] = { 0 };
				memcpy(szLoginResult, (char*)&stResult, sizeof(stResult));

				strSendData = szLoginResult;
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
				strSendData = strResult;
			}

			if (ret == RESULT_SUCCESS)
			{
				pTask->pUser->UpdateLogonInfo((char*)pTask->strUser.c_str(), (char*)pTask->strPwd.c_str());
			}
		}
		else if (pTask->strMsg == "ezs")
		{
			ret = RESULT_EXAMINFO_SUCCESS;
			strSendData = strInput;
		}
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("Error when parse json: ");
		strErrInfo.append(jsone.message() + "\tData:" + strUtf8);
		g_Log.LogOutError(strErrInfo);
		strSendData = "异常失败";
		std::cout << strErrInfo << std::endl;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("Error: ");
		strErrInfo.append(exc.message() + "\tData:" + strUtf8);
		g_Log.LogOutError(strErrInfo);
		strSendData = "异常失败";
		std::cout << strErrInfo << std::endl;
	}
	catch (...)
	{
		std::string strErrInfo;
		strErrInfo.append("Unknown error.\tData:" + strUtf8);
		g_Log.LogOutError(strErrInfo);
		strSendData = "异常失败";
		std::cout << strErrInfo << std::endl;
	}

	int nCmd = 0;
	if (pTask->strMsg == "login")
		nCmd = USER_RESPONSE_LOGIN;
	else if (pTask->strMsg == "ezs")
		nCmd = USER_RESPONSE_EXAMINFO;

	pTask->pUser->SendResponesInfo(nCmd, ret, (char*)strSendData.c_str(), strSendData.length());
	return bResult;
}

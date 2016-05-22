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
#if 0
		Poco::Net::HTMLForm form;
		form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);

		form.set("username", pTask->strUserName);
		form.set("password", pTask->strPwd);
		form.prepareSubmit(request);

//		session.setTimeout(Poco::Timespan(60, 0));
		request.set("EZ-Scanning-Program", "yes");	//gzip	//text/html
		request.setContentLength(form.calculateContentLength());
//		request.setChunkedTransferEncoding(false);
		form.write(session.sendRequest(request));
#else
		std::string strBody;
		strBody = pTask->strRequest;
//		strBody = "username=" + pTask->strUserName + "&password=" + pTask->strPwd;
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
#endif

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
	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strInput);
		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
		std::string strTmp = object->get("message").convert<std::string>();
		strTmp = CMyCodeConvert::Utf8ToGb2312(strTmp);
		std::string strEzs = object->get("ezs").convert<std::string>();

		pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
		pTask->strUri = SysSet.m_strScanReqUri;
		pTask->strEzs = "ezs=" + strEzs;
		pTask->strMsg = "ezs";
		g_fmScanReq.lock();
		g_lScanReq.push_back(pTask);
		g_fmScanReq.unlock();



// 		MAP_USER::iterator itFind = _mapUser_.find(LoginInfo.szUserNo);
// 		if (itFind == _mapUser_.end())
// 		{
// 			_mapUserLock_.lock();
// 			_mapUser_.insert(MAP_USER::value_type(LoginInfo.szUserNo, pTask->pUser));
// 			_mapUserLock_.unlock();
// 		}
// 		else
// 		{
// 			//重复登录提醒，
// 			itFind->second = pUser;
// 		}

	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("Error when parse json: ");
		strErrInfo.append(jsone.message() + "\tData:" + strInput);
		g_Log.LogOutError(strErrInfo);
		std::cout << strErrInfo << std::endl;
		return false;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("Error: ");
		strErrInfo.append(exc.message() + "\tData:" + strInput);
		g_Log.LogOutError(strErrInfo);
		std::cout << strErrInfo << std::endl;
		return false;
	}
	catch (...)
	{
		std::string strErrInfo;
		strErrInfo.append("Unknown error.\tData:" + strInput);
		g_Log.LogOutError(strErrInfo);
		std::cout << strErrInfo << std::endl;
		return false;
	}

	return bResult;
}

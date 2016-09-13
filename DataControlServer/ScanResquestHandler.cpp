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

				pTask->pUser->SendResponesInfo(USER_RESPONSE_LOGIN, ret, szLoginResult, sizeof(stResult));
				return bResult;
			}
			else
			{
				ret = RESULT_LOGIN_FAIL;
				strSendData = strResult;
			}
		}
		else if (pTask->strMsg == "ezs")
		{
			ret = RESULT_EXAMINFO_SUCCESS;
			strSendData = strInput;
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
		}
		else if (pTask->strMsg == "createModel")
		{
			Poco::JSON::Object::Ptr objResult = object->getObject("status");
			bResult = objResult->get("success").convert<bool>();
			if (!bResult)
			{
				//后端不存在此模板数据
			}
			else
			{
				pMODEL pModel = CreateModel(object);

				char szIndex[50] = { 0 };
				sprintf(szIndex, "%d_%d", pTask->nExamID, pTask->nSubjectID);
				pMODELINFO pModelInfo = NULL;
				MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
				if (itFind != _mapModel_.end())
				{
					pModelInfo = itFind->second;
					if (pModelInfo->pModel)
						SAFE_RELEASE(pModelInfo->pModel);
					pModelInfo->pModel = pModel;

					//下载PDF
					std::string strModelPath = Poco::format("%s/%s", SysSet.m_strModelSavePath, szIndex);
					Poco::File modelDir(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
					if (!modelDir.exists())
						modelDir.createDirectories();

					GetPdf(object, strModelPath);
				}
				else
					SAFE_RELEASE(pModel);
			}
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

bool GetZkzh(Poco::JSON::Object::Ptr objTK, pPAPERMODEL pPaperModel)
{
	cv::Point ptZkzh1, ptZkzh2;
	if (!objTK->isNull("baseInfo"))
	{
		Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
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
		pPaperModel->rtSNTracker = cv::Rect(ptZkzh1, ptZkzh2);
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
					omrItem.lSelAnswer.push_back(rc);
				}
				pPaperModel->lOMR2.push_back(omrItem);
			}
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

pMODEL CScanResquestHandler::CreateModel(Poco::JSON::Object::Ptr object)
{
	pMODEL pModel = NULL;
	Poco::JSON::Array::Ptr arrySheets = object->getArray("sheets");
	for (int k = 0; k < arrySheets->size(); k++)
	{
		Poco::JSON::Object::Ptr objSheet = arrySheets->getObject(k);
		
		int nArry[10] = { 0 };
		Poco::JSON::Array::Ptr arryContents = objSheet->getArray("contents");
		for (int k = 0; k < arryContents->size(); k++)
		{
			int nPage = arryContents->get(k).convert<int>();
			nArry[k] = nPage;
		}

		std::string strContent = objSheet->get("content").convert<std::string>();
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result;
		result = parser.parse(strContent);		//strJsnData
		Poco::JSON::Array::Ptr arryData = result.extract<Poco::JSON::Array::Ptr>();

		pModel = new MODEL;
		for (int i = 0; i < arryData->size(); i++)
		{
			Poco::JSON::Object::Ptr objTK = arryData->getObject(i);
			Poco::JSON::Object::Ptr objSubject = objTK->getObject("subject");
			Poco::JSON::Object::Ptr objPageNum = objTK->getObject("pageNum");

			if (i == 0)
			{
				Poco::JSON::Object::Ptr objCurSubject = objSubject->getObject("curSubject");
				pModel->strModelName = CMyCodeConvert::Utf8ToGb2312(objCurSubject->get("name").convert<std::string>());
			}

			std::string strName = Poco::format("model%d.jpg", nArry[i]);

			pPAPERMODEL pPaperModel = new PAPERMODEL;
			pPaperModel->nPaper = objPageNum->get("curPageNum").convert<int>() - 1;			//add from 0
			pPaperModel->strModelPicName = strName;	//图片名称，目前不知道，考虑从PDF直接转图片然后命名			//**********	test	*****************

			//同步头
			GetHeader(objTK, pPaperModel);

			//准考证号
			GetZkzh(objTK, pPaperModel);

			//OMR设置
			GetOMR(objTK, pPaperModel);

			//添加科目点
			GetCourse(objTK, pPaperModel);

			//添加缺考点
			GetQK(objTK, pPaperModel);

			//添加灰度点

			//添加白校验点

			//添加试卷模板到总模板
			pModel->vecPaperModel.push_back(pPaperModel);
		}
		pModel->nType = 1;
		break;					//************	注意：先不支持AB卷模板	****************
	}
	return pModel;
}

bool CScanResquestHandler::GetPdf(Poco::JSON::Object::Ptr object, std::string strSavePath)
{
	Poco::JSON::Array::Ptr arrySheets = object->getArray("sheets");
	for (int i = 0; i < arrySheets->size(); i++)
	{
		Poco::JSON::Object::Ptr objSheet = arrySheets->getObject(i);

		Poco::JSON::Array::Ptr arryContents = objSheet->getArray("contents");
		for (int k = 0; k < arryContents->size(); k++)
		{
			int nPage = arryContents->get(k).convert<int>();

			std::string strUri = Poco::format("%s/sheet/download/%d", SysSet.m_strBackUri, nPage);
			Poco::URI uri(strUri);
			std::auto_ptr<std::istream> pStr(Poco::URIStreamOpener::defaultOpener().open(uri));

			std::string strPicPath = Poco::format("%s\\model%d.pdf", strSavePath, nPage);
			std::ofstream out(strPicPath, std::ios::binary);
			Poco::StreamCopier::copyStream(*pStr.get(), out);

			out.close();
		}
	}
}

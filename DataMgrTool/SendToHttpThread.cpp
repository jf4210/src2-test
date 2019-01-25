#include "stdafx.h"
#include "SendToHttpThread.h"
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"
#include "RecogResultMgr.h"

CSendToHttpThread::CSendToHttpThread(void* pDlg)
: m_pDlg(pDlg)
{
//	std::cout << "HttpSendThread start.\n";
}


CSendToHttpThread::~CSendToHttpThread()
{
//	std::cout << "HttpSendThread exit.\n";
}

void CSendToHttpThread::run()
{
	while (!g_nExitFlag)	//!g_exit.tryWait(0)
	{
		pSEND_HTTP_TASK pTask = NULL;
		try
		{
			g_fmHttpSend.lock();
			LIST_SEND_HTTP::iterator it = g_lHttpSend.begin();
			for (; it != g_lHttpSend.end();)
			{
				if (!(*it)->nSendFlag || ((*it)->nSendFlag > 0 && (*it)->nSendFlag < 3 && (*it)->sTime.elapsed() > 5 * 1000000))
				{
					pTask = *it;
					it = g_lHttpSend.erase(it);
					break;
				}
				else if ((*it)->nSendFlag >= 3)		//此任务连续发送3次，每次间隔5秒，都失败了，将它删除
				{
					pTask = *it;
					std::string strEraseInfo;
					if (pTask->nTaskType == 3)
					{
						strEraseInfo = "Erase task(提交 OMR 结果信息给后端),试卷袋: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask, 0);
					}
					else if (pTask->nTaskType == 4)
					{
						strEraseInfo = "Erase task(提交 ZKZH 结果信息给后端),试卷袋: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask, 0);
					}
					else if (pTask->nTaskType == 5)
					{
						strEraseInfo = "Erase task(提交 选做题 结果信息给后端),试卷袋: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask, 0);
					}
					g_Log.LogOutError(strEraseInfo);

					it = g_lHttpSend.erase(it);
					delete pTask;
					pTask = NULL;
				}
				else
					it++;
			}
			g_fmHttpSend.unlock();

			if (pTask == NULL)
			{
				Poco::Thread::sleep(1000);
				continue;
			}
			if (pTask->nTaskType == 3)
				std::cout << "post Omr 数据给后端服务器" << std::endl;
			else if (pTask->nTaskType == 4)
				std::cout << "post ZKZH 数据给后端服务器" << std::endl;
			else if (pTask->nTaskType == 7)
			{
//				std::cout << "post Omr、ZKZH、选做题 数据给后端服务器" << std::endl;
				HandleOmrTask(pTask);
				continue;
			}
			else if (pTask->nTaskType == 8)
			{
				RecogResultRecord(pTask);
				continue;
			}

			Poco::URI uri(pTask->strUri);				//pTask->strUri
			Poco::Net::HTTPClientSession session;
			Poco::Net::HTTPRequest request;
			Poco::Net::HTTPResponse response;

			if (!doRequest(session, request, uri, pTask))
			{
				std::string strLog = "post图片数据给后端服务器失败";
				g_Log.LogOutError(strLog);
//				std::cout << strLog << std::endl;
			}

			std::istream& iStr = session.receiveResponse(response);  // get the response from server

			if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK)
			{
				std::string strResultStatus;
				std::string strContentEncoding;
				if (response.has("Content-Encoding"))
				{
					strContentEncoding = response.get("Content-Encoding");
				}
//				std::cout << strContentEncoding << std::endl;
				if (strContentEncoding == "gzip")
				{
					std::ostringstream outStringStream;
					Poco::InflatingInputStream inflater(iStr, Poco::InflatingStreamBuf::STREAM_GZIP);
					outStringStream << inflater.rdbuf();
					strResultStatus = outStringStream.str();
//					std::cout << strResultStatus << std::endl;
				}
				else
				{
					std::ostringstream outStringStream;
					outStringStream << iStr.rdbuf();
					strResultStatus = outStringStream.str();
//					std::cout << strResultStatus << std::endl;
				}

				if (!ParseResult(strResultStatus, pTask))
				{
					pTask->nSendFlag++;
					pTask->sTime.update();
					//发送失败，再放入队列
					g_fmHttpSend.lock();
					g_lHttpSend.push_back(pTask);
					g_fmHttpSend.unlock();
					continue;
				}
				else
				{
					if (pTask->nTaskType == 3)
					{
						std::string strLog = "发送OMR信息给后端成功, 试卷袋名: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers OMR result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask, 1);
					}
					else if (pTask->nTaskType == 4)
					{
						std::string strLog = "发送ZKZH信息给后端成功, 试卷袋名: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers ZKZH result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//zkzh
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask, 1);
					}
					else if (pTask->nTaskType == 5)
					{
						std::string strLog = "发送选做题信息给后端成功, 试卷袋名: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers ElectOmr result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//electomr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask, 1);
					}
				}
			}
			else
			{
				std::string strLog;
				int nCode = response.getStatus();
				stringstream ssLog;
				if (pTask->nTaskType == 3)
				{
// 					strLog = "post omr result failed: " + pTask->pPapers->strPapersName + "\tErrCode: " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
// 					g_Log.LogOutError(strLog);
					ssLog << "post omr result failed: " << pTask->pPapers->strPapersName << "\tErrCode(Http错误码): " << response.getReason() << "(" << nCode << ")\tName: " << pTask->pPapers->strPapersName << "\nResult info: " << pTask->strResult;
					g_Log.LogOutError(ssLog.str());
//					std::cout << strLog << std::endl;
				}
				else if (pTask->nTaskType == 4)
				{
// 					strLog = "post zkzh result failed: " + pTask->pPapers->strPapersName + "\tErrCode: " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
// 					g_Log.LogOutError(strLog);
					ssLog << "post zkzh result failed: " << pTask->pPapers->strPapersName << "\tErrCode(Http错误码): " << response.getReason() << "(" << nCode << ")\tName: " << pTask->pPapers->strPapersName << "\nResult info: " << pTask->strResult;
					g_Log.LogOutError(ssLog.str());
//					std::cout << strLog << std::endl;
				}
				else if (pTask->nTaskType == 5)
				{
// 					strLog = "post ElectOmr result failed: " + pTask->pPapers->strPapersName + "\tErrCode: " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
// 					g_Log.LogOutError(strLog);
					ssLog << "post ElectOmr result failed: " << pTask->pPapers->strPapersName << "\tErrCode(Http错误码): " << response.getReason() << "(" << nCode << ")\tName: " << pTask->pPapers->strPapersName << "\nResult info: " << pTask->strResult;
					g_Log.LogOutError(ssLog.str());
//					std::cout << strLog << std::endl;
				}
				pTask->nSendFlag++;
				pTask->sTime.update();
				//发送失败，再放入队列
				g_fmHttpSend.lock();
				g_lHttpSend.push_back(pTask);
				g_fmHttpSend.unlock();
				continue;
			}
			delete pTask;
			pTask = NULL;
		}
		catch (Poco::Exception& exc)
		{
			std::string strErrorInfo;
			char szFailTimes[30] = { 0 };
			sprintf(szFailTimes, " --- Send fail times : %d", pTask->nSendFlag);

			strErrorInfo.append(exc.displayText());
			strErrorInfo.append(szFailTimes);

			std::cout << "\n";
			std::cout << strErrorInfo << std::endl;
			g_Log.LogOutError(strErrorInfo);

			pTask->nSendFlag++;
			pTask->sTime.update();
			//发送失败，再放入队列
			g_fmHttpSend.lock();
			g_lHttpSend.push_back(pTask);
			g_fmHttpSend.unlock();
		}
		catch (...)
		{
			std::string strErrorInfo;
			char szFailTimes[5] = { 0 };
			sprintf(szFailTimes, "%d\n", pTask->nSendFlag);

			if (pTask->nTaskType == 3)
			{
				strErrorInfo.append("**** Unknown error ***\t提交OMR异常");
			}
			else if (pTask->nTaskType == 4)
			{
				strErrorInfo.append("**** Unknown error ***\t提交ZKZH异常");
			}
			else if (pTask->nTaskType == 5)
			{
				strErrorInfo.append("**** Unknown error ***\t提交选做题异常");
			}

			std::cout << "\n";
//			std::cout << strErrorInfo << std::endl;
			g_Log.LogOutError(strErrorInfo);

			pTask->nSendFlag++;
			pTask->sTime.update();
			//发送失败，再放入队列
			g_fmHttpSend.lock();
			g_lHttpSend.push_back(pTask);
			g_fmHttpSend.unlock();
		}
	}
}

bool CSendToHttpThread::doRequest(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::URI& uri, pSEND_HTTP_TASK pTask)
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
	if (pTask->nTaskType == 3 || pTask->nTaskType == 4 || pTask->nTaskType == 5)
	{
		strBody = pTask->strResult;
		request.setContentType("application/json");
		request.setContentLength(strBody.length());
		request.set("Accept", "application/json");
		request.set("Cookie", pTask->strEzs);
	}

	std::ostream& ostr = session.sendRequest(request);
	ostr << strBody;

	return true;
}

bool CSendToHttpThread::ParseResult(std::string& strInput, pSEND_HTTP_TASK pTask)
{
	bool bResult = false;
	std::string strUtf8 = CMyCodeConvert::Utf8ToGb2312(strInput);

	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strInput);
		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
		
		if (pTask->nTaskType == 3)
		{
			Poco::JSON::Object::Ptr objResult = object->getObject("status");
			bResult = objResult->get("success").convert<bool>();
			if (!bResult)
			{
				std::string strMsg;
				if (!objResult->isNull("msg"))
				{
					strMsg = CMyCodeConvert::Utf8ToGb2312(objResult->get("msg").convert<std::string>());
				}
				char szCount[5] = { 0 };
				sprintf(szCount, "%d", pTask->nSendFlag);
				std::string strLog = "提交OMR信息给后端失败(试卷袋:" + pTask->pPapers->strPapersName;
				strLog.append(")，失败原因: " + strMsg);
				strLog.append("\t发送失败次数: ");
				strLog.append(szCount);
				strLog.append("\n发送的数据: " + pTask->strResult);
				g_Log.LogOutError(strLog);
				std::string strShowInfo;
				if (strLog.length() > 1000)
				{
					strShowInfo = strLog.substr(0, 1000);
					strShowInfo.append("...");
				}
				else
					strShowInfo = strLog;
				std::cout << strShowInfo << std::endl;
			}
		}
		else if (pTask->nTaskType == 4)
		{
			Poco::JSON::Object::Ptr objResult = object->getObject("status");
			bResult = objResult->get("success").convert<bool>();
			if (!bResult)
			{
				std::string strMsg;
				if (!objResult->isNull("msg"))
				{
					strMsg = CMyCodeConvert::Utf8ToGb2312(objResult->get("msg").convert<std::string>());
				}
				char szCount[5] = { 0 };
				sprintf(szCount, "%d", pTask->nSendFlag);
				std::string strLog = "提交ZKZH信息给后端失败(试卷袋:" + pTask->pPapers->strPapersName;
				strLog.append(")，失败原因: " + strMsg);
				strLog.append("\t发送失败次数: ");
				strLog.append(szCount);
				strLog.append("\n发送的数据: " + pTask->strResult);
				g_Log.LogOutError(strLog);
				std::string strShowInfo;
				if (strLog.length() > 1000)
				{
					strShowInfo = strLog.substr(0, 1000);
					strShowInfo.append("...");
				}
				else
					strShowInfo = strLog;
				std::cout << strShowInfo << std::endl;
			}
		}
		else if (pTask->nTaskType == 5)
		{
			Poco::JSON::Object::Ptr objResult = object->getObject("status");
			bResult = objResult->get("success").convert<bool>();
			if (!bResult)
			{
				std::string strMsg;
				if (!objResult->isNull("msg"))
				{
					strMsg = CMyCodeConvert::Utf8ToGb2312(objResult->get("msg").convert<std::string>());
				}
				char szCount[5] = { 0 };
//				sprintf(szCount, "%d", pTask->nSendFlag);
				std::string strLog = "提交选做题信息给后端失败(试卷袋:" + pTask->pPapers->strPapersName;
				strLog.append(")，失败原因: " + strMsg);
				strLog.append("\t发送失败次数: ");
				strLog.append(szCount);
				strLog.append("\n发送的数据: " + pTask->strResult);
				g_Log.LogOutError(strLog);
				std::string strShowInfo;
				if (strLog.length() > 1000)
				{
					strShowInfo = strLog.substr(0, 1000);
					strShowInfo.append("...");
				}
				else
					strShowInfo = strLog;
				std::cout << strShowInfo << std::endl;
			}
		}
	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("Error when parse json: ");
		strErrInfo.append(jsone.message() + "\tData:" + strUtf8);
		g_Log.LogOutError(strErrInfo);
		std::cout << strErrInfo << std::endl;
		return false;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("Error: ");
		strErrInfo.append(exc.message() + "\tData:" + strUtf8);
		g_Log.LogOutError(strErrInfo);
		std::cout << strErrInfo << std::endl;
		return false;
	}
	catch (...)
	{
		std::string strErrInfo;
		strErrInfo.append("Unknown error.\tData:" + strUtf8);
		g_Log.LogOutError(strErrInfo);
		std::cout << strErrInfo << std::endl;
		return false;
	}

	return bResult;
}

std::string CSendToHttpThread::calcMd5(std::string& strInfo)
{
	Poco::MD5Engine md5;
	Poco::DigestOutputStream outstr(md5);
	outstr << strInfo;
	outstr.flush();
	const Poco::DigestEngine::Digest& digest = md5.digest();
	return Poco::DigestEngine::digestToHex(digest);
}

void CSendToHttpThread::checkTaskStatus(pSEND_HTTP_TASK pTask, int nState)
{
	if (pTask->pPapers->nTaskCounts == 0)
	{
		//通知外面已经提交完成
		(static_cast<CDialog*>(m_pDlg))->SendMessage(MSG_SENDRESULT_STATE, nState, (LPARAM)pTask->pPapers);

		//**************************************************
		//**************************************************
		//	后面要改
		//**************************************************
		//**************************************************
		//**************************************************
		_fmCompress_.lock();
		_nCompress_++;
		_fmCompress_.unlock();

		//删除源文件夹
		try
		{
			Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pTask->pPapers->strPapersPath));
			if (srcFileDir.exists())
				srcFileDir.remove(true);
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "删除文件夹(" + pTask->pPapers->strPapersPath + ")失败: " + exc.message();
			g_Log.LogOutError(strErr);
		}

		g_fmPapers.lock();			//释放试卷袋列表
		PAPERS_LIST::iterator itPapers = g_lPapers.begin();
		for (; itPapers != g_lPapers.end(); itPapers++)
		{
			pPAPERSINFO pPapersTask = *itPapers;
			if (pPapersTask == pTask->pPapers)
			{
				itPapers = g_lPapers.erase(itPapers);
				SAFE_RELEASE(pPapersTask);
				break;
			}
		}
		g_fmPapers.unlock();
	}
}

void CSendToHttpThread::HandleOmrTask(pSEND_HTTP_TASK pTask)
{
	std::string strLog = "开始提交OMR、ZKZH、选做题信息(" + pTask->pPapers->strPapersName + ")";
	g_Log.LogOut(strLog);
#if 1
	CRecogResultMgr resultObj(pTask->pPapers);
#else
	bool bHasElectOmr = false;
	Poco::JSON::Array snArry;
	Poco::JSON::Array omrArry;
	Poco::JSON::Array electOmrArry;
	std::stringstream jsnSnString;
	std::stringstream jsnOmrString;
	std::stringstream jsnElectOmrString;
	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itPaper = pTask->pPapers->lPaper.begin();
	for (int i = 0; itPaper != pTask->pPapers->lPaper.end(); itPaper++, i++)
	{
		if ((*itPaper)->strMd5Key.empty())
		{
			std::string strStudentInfo = pTask->pPapers->strPapersName + "_" + (*itPaper)->strStudentInfo;
			std::string strStudentKey = calcMd5(strStudentInfo);
			(*itPaper)->strMd5Key = strStudentKey;
		}

		Poco::JSON::Object jsnPaperSN;
		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itPaper)->lSnResult.begin();
		for (; itSn != (*itPaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				jsnSnPosition.set("x", itRect->rt.x);
				jsnSnPosition.set("y", itRect->rt.y);
				jsnSnPosition.set("w", itRect->rt.width);
				jsnSnPosition.set("h", itRect->rt.height);
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaperSN.set("examId", pTask->pPapers->nExamID);
		jsnPaperSN.set("subjectId", pTask->pPapers->nSubjectID);
		jsnPaperSN.set("userId", pTask->pPapers->nUserId);
		jsnPaperSN.set("teacherId", pTask->pPapers->nTeacherId);
		jsnPaperSN.set("zkzh", (*itPaper)->strSN);
		jsnPaperSN.set("papers", pTask->pPapers->strPapersName);
		if ((*itPaper)->strSN != "")
			jsnPaperSN.set("doubt", 0);
		else
			jsnPaperSN.set("doubt", 1);
		jsnPaperSN.set("studentKey", (*itPaper)->strMd5Key);
		jsnPaperSN.set("detail", jsnSnDetailArry);

		snArry.add(jsnPaperSN);

		Poco::JSON::Object jsnPaperOMR;
		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itPaper)->lOmrResult.begin();
		for (; itOmr != (*itPaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value2", itOmr->strRecogVal2);
			jsnOmr.set("doubt", itOmr->nDoubt);
			jsnOmr.set("pageId", itOmr->nPageId);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaperOMR.set("omr", jsnOmrArry);
		jsnPaperOMR.set("examId", pTask->pPapers->nExamID);
		jsnPaperOMR.set("subjectId", pTask->pPapers->nSubjectID);
		jsnPaperOMR.set("userId", pTask->pPapers->nUserId);
		jsnPaperOMR.set("teacherId", pTask->pPapers->nTeacherId);
		jsnPaperOMR.set("zkzh", (*itPaper)->strSN);
		jsnPaperOMR.set("nOmrAnswerFlag", (*itPaper)->nStandardAnswer);
		jsnPaperOMR.set("papers", pTask->pPapers->strPapersName);
		jsnPaperOMR.set("studentKey", (*itPaper)->strMd5Key);
		omrArry.add(jsnPaperOMR);
	
		if ((*itPaper)->lElectOmrResult.size() > 0)
		{
			bHasElectOmr = true;
			Poco::JSON::Object jsnPaperElectOmr;
			Poco::JSON::Array jsnElectOmrArry;
			ELECTOMR_LIST::iterator itElectOmr = (*itPaper)->lElectOmrResult.begin();
			for (; itElectOmr != (*itPaper)->lElectOmrResult.end(); itElectOmr++)
			{
				Poco::JSON::Object jsnElectOmr;
				jsnElectOmr.set("paperId", i + 1);
				jsnElectOmr.set("doubt", itElectOmr->nDoubt);
				jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
				jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
				jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
				jsnElectOmr.set("value", itElectOmr->strRecogResult);
				Poco::JSON::Array jsnPositionArry;
				RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
				for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
				{
					Poco::JSON::Object jsnItem;
					char szVal[5] = { 0 };
					sprintf_s(szVal, "%c", itRect->nAnswer + 65);
					jsnItem.set("val", szVal);
					jsnItem.set("x", itRect->rt.x);
					jsnItem.set("y", itRect->rt.y);
					jsnItem.set("w", itRect->rt.width);
					jsnItem.set("h", itRect->rt.height);
					jsnPositionArry.add(jsnItem);
				}
				jsnElectOmr.set("position", jsnPositionArry);
				jsnElectOmrArry.add(jsnElectOmr);
			}
			jsnPaperElectOmr.set("electOmr", jsnElectOmrArry);		//选做题结果
			jsnPaperElectOmr.set("examId", pTask->pPapers->nExamID);
			jsnPaperElectOmr.set("subjectId", pTask->pPapers->nSubjectID);
			jsnPaperElectOmr.set("userId", pTask->pPapers->nUserId);
			jsnPaperElectOmr.set("teacherId", pTask->pPapers->nTeacherId);
			jsnPaperElectOmr.set("zkzh", (*itPaper)->strSN);
			jsnPaperElectOmr.set("papers", pTask->pPapers->strPapersName);
			jsnPaperElectOmr.set("studentKey", (*itPaper)->strMd5Key);
			electOmrArry.add(jsnPaperElectOmr);
		}		
	}
	snArry.stringify(jsnSnString, 0);
	omrArry.stringify(jsnOmrString, 0);
	if (bHasElectOmr)
		electOmrArry.stringify(jsnElectOmrString, 0);
#endif
	if (pTask->bSendZkzh)
	{
		pTask->pPapers->fmTask.lock();
		pTask->pPapers->nTaskCounts++;			//zkzh
		pTask->pPapers->fmTask.unlock();

		pSEND_HTTP_TASK pSnTask = new SEND_HTTP_TASK;
		pSnTask->nTaskType = 4;
		pSnTask->strResult = resultObj.GetSnResult();		//jsnSnString.str();
		pSnTask->pPapers = pTask->pPapers;
		pSnTask->strEzs = pTask->pPapers->strEzs;
		pSnTask->strUri = g_strUploadUri + "/zkzh";
		g_fmHttpSend.lock();
		g_lHttpSend.push_back(pSnTask);
		g_fmHttpSend.unlock();
	}
	
	if (pTask->bSendOmr)
	{
		pTask->pPapers->fmTask.lock();
		pTask->pPapers->nTaskCounts++;			//omr
		pTask->pPapers->fmTask.unlock();

		pSEND_HTTP_TASK pOmrTask = new SEND_HTTP_TASK;
		pOmrTask->nTaskType = 3;
		pOmrTask->strResult = resultObj.GetOmrResult();		//jsnOmrString.str();
		pOmrTask->pPapers = pTask->pPapers;
		pOmrTask->strEzs = pTask->pPapers->strEzs;
		pOmrTask->strUri = g_strUploadUri + "/omr";
		g_fmHttpSend.lock();
		g_lHttpSend.push_back(pOmrTask);
		g_fmHttpSend.unlock();
	}
	

	if (pTask->bSendElectOmr)
	{
		if (resultObj._bHasElectOmr)		//bHasElectOmr
		{
			pTask->pPapers->fmTask.lock();
			pTask->pPapers->nTaskCounts++;			//electOmr
			pTask->pPapers->fmTask.unlock();

			pSEND_HTTP_TASK pElectOmrTask = new SEND_HTTP_TASK;
			pElectOmrTask->nTaskType = 5;
			pElectOmrTask->strResult = resultObj.GetElectOmrResult();		//jsnElectOmrString.str();
			pElectOmrTask->pPapers = pTask->pPapers;
			pElectOmrTask->strEzs = pTask->pPapers->strEzs;
			pElectOmrTask->strUri = g_strUploadUri + "/choosetitleinfo";
			g_fmHttpSend.lock();
			g_lHttpSend.push_back(pElectOmrTask);
			g_fmHttpSend.unlock();
		}
	}

	strLog = "ZKZH信息如下: " + resultObj.GetSnResult();	// jsnSnString.str();
	g_Log.LogOut(strLog);
	strLog = "OMR信息如下: " + resultObj.GetOmrResult();	// jsnOmrString.str();
	g_Log.LogOut(strLog);
	if (resultObj._bHasElectOmr)	/*bHasElectOmr*/
	{
		strLog = "选做题信息如下: " + resultObj.GetElectOmrResult();	 //jsnElectOmrString.str();
		g_Log.LogOut(strLog);
	}
}

void CSendToHttpThread::RecogResultRecord(pSEND_HTTP_TASK pTask)
{
	CRecogResultMgr* pRecogResult = static_cast<CRecogResultMgr*>(pTask->pRecogResult);

	//将识别结果记录到文件
	USES_CONVERSION;
	std::string strCurrPath = T2A(g_strCurrentPath);
	std::string strRecordPath = Poco::format("%sRecogResult\\%d_%d\\", strCurrPath, pRecogResult->nExamId, pRecogResult->nSubjuctId);
	try
	{
		Poco::File fileRecordDir(CMyCodeConvert::Gb2312ToUtf8(strRecordPath));
		if (!fileRecordDir.exists())
			fileRecordDir.createDirectories();
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo = "创建文件夹: " + strRecordPath + "失败, Detail: ";
		strErrInfo.append(exc.message());
		g_Log.LogOutError(strErrInfo);
	}

	std::string strFileName = strRecordPath + pRecogResult->strPkgName + "_#_omr.txt";
	std::string strOmrResult = pRecogResult->GetOmrResult();
	CFile file;
	file.Open(A2T(strFileName.c_str()), CFile::modeCreate | CFile::modeReadWrite);
	file.Write(strOmrResult.c_str(), strOmrResult.length());
	file.Close();

	SAFE_RELEASE(pRecogResult);
}


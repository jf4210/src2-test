#include "SendToHttpThread.h"


CSendToHttpThread::CSendToHttpThread()
{
	std::cout << "HttpSendThread start.\n";
}


CSendToHttpThread::~CSendToHttpThread()
{
	std::cout << "HttpSendThread exit.\n";
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
				if (!(*it)->nSendFlag || ((*it)->nSendFlag > 0 && (*it)->nSendFlag < SysSet.m_nSendTimes && (*it)->sTime.elapsed() > SysSet.m_nIntervalTime * 1000000))
				{
					pTask = *it;
					it = g_lHttpSend.erase(it);
					break;
				}
				else if ((*it)->nSendFlag >= SysSet.m_nSendTimes)		//此任务连续发送5次，每次间隔5秒，都失败了，将它删除
				{
					pTask = *it;
					std::string strEraseInfo;
					if (pTask->nTaskType == 1)
					{
						strEraseInfo = "Erase task(发送图片给zimg): ";
						strEraseInfo.append((*it)->pPic->strFileName);
						
						pTask->pPic->bUpLoadFlag = false;
						pTask->pPapers->fmNum.lock();
						pTask->pPapers->nUpLoadFail++;
						GenerateResult(pTask->pPapers, pTask);
						pTask->pPapers->fmNum.unlock();
					}
					else if (pTask->nTaskType == 2)
					{
						strEraseInfo = "Erase task(提交图片结果信息给后端),试卷袋: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);


						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 3)
					{
						strEraseInfo = "Erase task(提交 OMR 结果信息给后端),试卷袋: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 4)
					{
						strEraseInfo = "Erase task(提交 ZKZH 结果信息给后端),试卷袋: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 5)
					{
						strEraseInfo = "Erase task(提交 选做题 结果信息给后端),试卷袋: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					g_Log.LogOutError(strEraseInfo);
					std::cout << strEraseInfo << std::endl;

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
				Poco::Thread::sleep(200);
				continue;
			}
			if (pTask->nTaskType == 1)
				std::cout << "Get http upload task: " << pTask->pPic->strFileName << std::endl;
			else if (pTask->nTaskType == 3)
				std::cout << "post Omr 数据给后端服务器" << std::endl;
			else if (pTask->nTaskType == 4)
				std::cout << "post ZKZH 数据给后端服务器" << std::endl;

			Poco::URI uri(pTask->strUri);				//pTask->strUri
			Poco::Net::HTTPClientSession session;
			Poco::Net::HTTPRequest request;
			Poco::Net::HTTPResponse response;

			if (!doRequest(session, request, uri, pTask))
			{
				if (pTask->nTaskType == 1)
				{
					std::string strLog = "Open file fail: " + pTask->pPic->strFilePath;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
					
					pTask->pPic->bUpLoadFlag = false;
					pTask->pPapers->fmNum.lock();
//					pTask->pPapers->nUpLoadFail++;
					GenerateResult(pTask->pPapers, pTask);
					pTask->pPapers->fmNum.unlock();

					delete pTask;
					pTask = NULL;
					continue;
				}
				else
				{
					std::string strLog = "post图片数据给后端服务器失败";
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
				}
			}

			std::istream& iStr = session.receiveResponse(response);  // get the response from server
//			std::cout << response.getStatus() << "\t" << response.getReason() << std::endl;

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
				{		//返回的数据解析失败
					if (pTask->nTaskType == 1)
					{
						pTask->pPic->bUpLoadFlag = false;
						pTask->pPapers->fmNum.lock();
//						pTask->pPapers->nUpLoadFail++;
						GenerateResult(pTask->pPapers, pTask);
						pTask->pPapers->fmNum.unlock();
					}
					else
					{
					}
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
					if (pTask->nTaskType == 1)
					{
						pTask->pPic->bUpLoadFlag = true;
						pTask->pPapers->fmNum.lock();
						pTask->pPapers->nUpLoadSuccess++;
						GenerateResult(pTask->pPapers, pTask);
						pTask->pPapers->fmNum.unlock();
					}
					else if (pTask->nTaskType == 2)
					{
						std::string strLog = "发送试卷袋图片信息给后端成功, 试卷袋名: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;
					}
					else if (pTask->nTaskType == 3)
					{
						std::string strLog = "发送OMR信息给后端成功, 试卷袋名: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers OMR result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 4)
					{
						std::string strLog = "发送ZKZH信息给后端成功, 试卷袋名: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers ZKZH result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//zkzh
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 5)
					{
						std::string strLog = "发送选做题信息给后端成功, 试卷袋名: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers ElectOmr result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//electomr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
				}
			}
			else
			{
				std::string strLog;
				int nCode = response.getStatus();
				if (pTask->nTaskType == 1)
				{
					strLog = "send file fail: " + pTask->pPic->strFileName + "\tErrCode: " + response.getReason() + "\tPath: " + pTask->pPic->strFilePath;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;

					pTask->pPic->bUpLoadFlag = false;
					pTask->pPapers->fmNum.lock();
//					pTask->pPapers->nUpLoadFail++;
					GenerateResult(pTask->pPapers, pTask);
					pTask->pPapers->fmNum.unlock();
				}
				else if (pTask->nTaskType == 2)
				{
					strLog = "post papers result failed: " + pTask->pPapers->strPapersName + "\tErrCode: " + response.getReason() + "\tPath: " + pTask->pPapers->strPapersPath + "\nResult info: ..." /*+ pTask->strResult*/;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
				}
				else if (pTask->nTaskType == 3)
				{
					strLog = "post omr result failed: " + pTask->pPapers->strPapersName + "\tErrCode: " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
				}
				else if (pTask->nTaskType == 4)
				{
					strLog = "post zkzh result failed: " + pTask->pPapers->strPapersName + "\tErrCode: " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
				}
				else if (pTask->nTaskType == 5)
				{
					strLog = "post ElectOmr result failed: " + pTask->pPapers->strPapersName + "\tErrCode: " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
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
			if (pTask->nTaskType == 1)
			{
				strErrorInfo.append("\tFileInfo: " + pTask->pPic->strFileName);
				strErrorInfo.append("\tFilePath: " + pTask->pPic->strFilePath);
			}
			else if (pTask->nTaskType == 2)
			{
				strErrorInfo.append("\tPapersName: " + pTask->pPapers->strPapersName);
				strErrorInfo.append("\tPath: " + pTask->pPapers->strPapersPath);


				//++ test
// 				bool bHasElectOmr = false;
// 				Poco::JSON::Array electOmrArry;
// 				std::stringstream jsnElectOmrString;
// 				LIST_PAPER_INFO::iterator it = pTask->pPapers->lPaper.begin();
// 				for (; it != pTask->pPapers->lPaper.end(); it++)
// 				{
// 					pPAPER_INFO pPaper = *it;
// 					if (pPaper->nHasElectOmr)
// 					{
// 						bHasElectOmr = true;
// 						Poco::JSON::Parser parserElectOmr;
// 						Poco::Dynamic::Var resultElectOmr;
// 						try
// 						{
// 							resultElectOmr = parserElectOmr.parse(pPaper->strElectOmrDetail);
// 							Poco::JSON::Object::Ptr electOmrObj = resultElectOmr.extract<Poco::JSON::Object::Ptr>();
// 
// 							electOmrObj->set("studentKey", pPaper->strMd5Key);
// 							electOmrArry.add(electOmrObj);
// 						}
// 						catch (Poco::JSON::JSONException& jsone)
// 						{
// 							std::string strErrInfo;
// 							strErrInfo.append("Error when parse ElectOmr: ");
// 							strErrInfo.append(jsone.message() + "\tData:" + pPaper->strElectOmrDetail);
// 							g_Log.LogOutError(strErrInfo);
// 							std::cout << strErrInfo << std::endl;
// 						}
// 					}
// 				}
// 				if (bHasElectOmr)
// 					electOmrArry.stringify(jsnElectOmrString, 0);
// 				std::string strTmp = jsnElectOmrString.str();
// 				g_Log.LogOut(strTmp);
				//--
			}			

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

			if (pTask->nTaskType == 1)
			{
				strErrorInfo.append("**** Unknown error ***\tFileInfo: " + pTask->pPic->strFileName + "\t---Send fail times: ");
				strErrorInfo.append(szFailTimes);
				strErrorInfo.append("\tFilePath: " + pTask->pPic->strFilePath);
			}
			else if (pTask->nTaskType == 2)
			{
				strErrorInfo.append("**** Unknown error ***\tPapersName: " + pTask->pPapers->strPapersName);
				strErrorInfo.append("\tPath: " + pTask->pPapers->strPapersPath);
			}
			else if (pTask->nTaskType == 3)
			{
				strErrorInfo.append("**** Unknown error ***\t提交OMR异常");
			}
			else if (pTask->nTaskType == 4)
			{
				strErrorInfo.append("**** Unknown error ***\t提交ZKZH异常");
			}

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
	if (pTask->nTaskType == 1)
	{
		std::ifstream fin(pTask->pPic->strFilePath, std::ifstream::binary);
		if (!fin)	return false;
		std::stringstream buffer;
		buffer << fin.rdbuf();
		strBody = buffer.str();
		fin.close();

		request.setContentType("jpeg");
		request.setContentLength(strBody.length());
	}
	else if (pTask->nTaskType == 2)		//提交给后端
	{
		strBody = pTask->strResult;
		request.setContentType("application/json");
		request.setContentLength(strBody.length());
		request.set("Accept", "application/json");
		request.set("Cookie", pTask->strEzs);
	}
	else if (pTask->nTaskType == 3 || pTask->nTaskType == 4 || pTask->nTaskType == 5)
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
		
		if (pTask->nTaskType == 1)
		{
			bResult = object->get("ret").convert<bool>();
			if (bResult)
			{
				Poco::JSON::Object::Ptr objData = object->getObject("info");
				std::string strMd5 = objData->get("md5").convert<std::string>();
				int nSize = objData->get("size").convert<int>();
				pTask->pPic->strHashVal = strMd5;
			}
			else
			{
				Poco::JSON::Object::Ptr objData = object->getObject("error");

				std::stringstream jsnString;
				objData->stringify(jsnString, 0);

				std::string strErrInfo;
				strErrInfo.append("Error when upload file: " + pTask->pPic->strFileName + "\tDetail: " + jsnString.str());
				g_Log.LogOutError(strErrInfo);
				std::cout << strErrInfo << std::endl;
			}
		}
		else if (pTask->nTaskType == 2)
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
				std::string strLog = "发送试卷袋图片信息给后端失败，失败原因: " + strMsg;
				strLog.append("\t发送失败次数: ");
				strLog.append("\n发送数据: " + pTask->strResult);
				strLog.append(szCount);
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
			}
			else
			{
				std::string strLog = "开始提交OMR、ZKZH、选做题信息(" + pTask->pPapers->strPapersName + ")";
				g_Log.LogOut(strLog);
				bool bHasElectOmr = false;
				Poco::JSON::Array snArry;
				Poco::JSON::Array omrArry;
				Poco::JSON::Array electOmrArry;
				std::stringstream jsnSnString;
				std::stringstream jsnOmrString;
				std::stringstream jsnElectOmrString;
				LIST_PAPER_INFO::iterator it = pTask->pPapers->lPaper.begin();
				for (; it != pTask->pPapers->lPaper.end(); it++)
				{
					pPAPER_INFO pPaper = *it;

					Poco::JSON::Parser parserSN;
					Poco::Dynamic::Var resultSN;
					try
					{
						resultSN = parserSN.parse(pPaper->strSnDetail);
						Poco::JSON::Object::Ptr snObj = resultSN.extract<Poco::JSON::Object::Ptr>();

						snObj->set("studentKey", pPaper->strMd5Key);
						snArry.add(snObj);
					}
					catch (Poco::JSON::JSONException& jsone)
					{
						std::string strErrInfo;
						strErrInfo.append("Error when parse SN: ");
						strErrInfo.append(jsone.message() + "\tData:" + pPaper->strSnDetail);
						g_Log.LogOutError(strErrInfo);
						std::cout << strErrInfo << std::endl;
					}

					Poco::JSON::Parser parserOmr;
					Poco::Dynamic::Var resultOmr;
					try
					{
						resultOmr = parserOmr.parse(pPaper->strOmrDetail);
						Poco::JSON::Object::Ptr omrObj = resultOmr.extract<Poco::JSON::Object::Ptr>();

						omrObj->set("studentKey", pPaper->strMd5Key);
						omrArry.add(omrObj);
					}
					catch (Poco::JSON::JSONException& jsone)
					{
						std::string strErrInfo;
						strErrInfo.append("Error when parse Omr: ");
						strErrInfo.append(jsone.message() + "\tData:" + pPaper->strOmrDetail);
						g_Log.LogOutError(strErrInfo);
						std::cout << strErrInfo << std::endl;
					}

					if (pPaper->nHasElectOmr)
					{
						bHasElectOmr = true;
						Poco::JSON::Parser parserElectOmr;
						Poco::Dynamic::Var resultElectOmr;
						try
						{
							resultElectOmr = parserElectOmr.parse(pPaper->strElectOmrDetail);
							Poco::JSON::Object::Ptr electOmrObj = resultElectOmr.extract<Poco::JSON::Object::Ptr>();

							electOmrObj->set("studentKey", pPaper->strMd5Key);
							electOmrArry.add(electOmrObj);
						}
						catch (Poco::JSON::JSONException& jsone)
						{
							std::string strErrInfo;
							strErrInfo.append("Error when parse ElectOmr: ");
							strErrInfo.append(jsone.message() + "\tData:" + pPaper->strElectOmrDetail);
							g_Log.LogOutError(strErrInfo);
							std::cout << strErrInfo << std::endl;
						}
					}					
				}

				snArry.stringify(jsnSnString, 0);
				omrArry.stringify(jsnOmrString, 0);
				if (bHasElectOmr)
					electOmrArry.stringify(jsnElectOmrString, 0);
				
				pTask->pPapers->fmTask.lock();
				pTask->pPapers->nTaskCounts++;			//zkzh
				pTask->pPapers->fmTask.unlock();

				pSEND_HTTP_TASK pSnTask = new SEND_HTTP_TASK;
				pSnTask->nTaskType = 4;
				pSnTask->strResult = jsnSnString.str();
				pSnTask->pPapers = pTask->pPapers;
				pSnTask->strEzs = pTask->pPapers->strEzs;
				pSnTask->strUri = SysSet.m_strBackUri + "/zkzh";
				g_fmHttpSend.lock();
				g_lHttpSend.push_back(pSnTask);
				g_fmHttpSend.unlock();

				pTask->pPapers->fmTask.lock();
				pTask->pPapers->nTaskCounts++;			//omr
				pTask->pPapers->fmTask.unlock();

				pSEND_HTTP_TASK pOmrTask = new SEND_HTTP_TASK;
				pOmrTask->nTaskType = 3;
				pOmrTask->strResult = jsnOmrString.str();
				pOmrTask->pPapers = pTask->pPapers;
				pOmrTask->strEzs = pTask->pPapers->strEzs;
				pOmrTask->strUri = SysSet.m_strBackUri + "/omr";
				g_fmHttpSend.lock();
				g_lHttpSend.push_back(pOmrTask);
				g_fmHttpSend.unlock();

				//++提交选做题信息	*************	注意：这里还不行，需要和后端确认	********************

// 				pTask->pPapers->fmTask.lock();
// 				pTask->pPapers->nTaskCounts++;			//electOmr
// 				pTask->pPapers->fmTask.unlock();
// 
// 				pSEND_HTTP_TASK pElectOmrTask = new SEND_HTTP_TASK;
// 				pElectOmrTask->nTaskType = 5;
// 				pElectOmrTask->strResult = jsnElectOmrString.str();
// 				pElectOmrTask->pPapers = pTask->pPapers;
// 				pElectOmrTask->strEzs = pTask->pPapers->strEzs;
// 				pElectOmrTask->strUri = SysSet.m_strBackUri + "/electOmr";
// 				g_fmHttpSend.lock();
// 				g_lHttpSend.push_back(pElectOmrTask);
// 				g_fmHttpSend.unlock();
				//--

				strLog = "ZKZH信息如下: " + jsnSnString.str();
				g_Log.LogOut(strLog);
				strLog = "OMR信息如下: " + jsnOmrString.str();
				g_Log.LogOut(strLog); 
				if (bHasElectOmr)
				{
					strLog = "选做题信息如下: " + jsnElectOmrString.str();
					g_Log.LogOut(strLog);
				}
			}
		}
		else if (pTask->nTaskType == 3)
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
				strLog.append("\n发送数据: " + pTask->strResult);
				strLog.append(szCount);
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
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
				strLog.append("\n发送数据: " + pTask->strResult);
				strLog.append(szCount);
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
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
				sprintf(szCount, "%d", pTask->nSendFlag);
				std::string strLog = "提交选做题信息给后端失败(试卷袋:" + pTask->pPapers->strPapersName;
				strLog.append(")，失败原因: " + strMsg);
				strLog.append("\t发送失败次数: ");
				strLog.append("\n发送数据: " + pTask->strResult);
				strLog.append(szCount);
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
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

bool CSendToHttpThread::GenerateResult(pPAPERS_DETAIL pPapers, pSEND_HTTP_TASK pTask)
{
	if (pPapers->nUpLoadSuccess + pPapers->nUpLoadFail != pPapers->nTotalPics)
		return false;

//	std::cout << "上传成功 + 失败数量 = " << pPapers->nUpLoadSuccess + pPapers->nUpLoadFail << std::endl;
	std::string strLog = "试卷袋(" + pPapers->strPapersName +")图片上传服务器完成，开始提交后端。";
	g_Log.LogOut(strLog);
	std::cout << strLog << std::endl;

	Poco::JSON::Object jsnPapers;
	jsnPapers.set("papers", CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersName));
	jsnPapers.set("papersDesc", CMyCodeConvert::Gb2312ToUtf8(pPapers->strDesc));
	jsnPapers.set("totalNum", pPapers->nTotalPaper);
	jsnPapers.set("qkNum", pPapers->nQk);
	jsnPapers.set("examId", pPapers->nExamID);
	jsnPapers.set("subjectId", pPapers->nSubjectID);
	jsnPapers.set("teacherId", pPapers->nTeacherId);
	jsnPapers.set("userId", pPapers->nUserId);
	Poco::JSON::Array  paperArry;

	LIST_PAPER_INFO::iterator it2 = pPapers->lPaper.begin();
	for (; it2 != pPapers->lPaper.end(); it2++)
	{
		pPAPER_INFO pPaper = *it2;

		std::string strStudentInfo = pPapers->strPapersName + "_" + pPaper->strName;
		std::string strStudentKey = calcMd5(strStudentInfo);
		pPaper->strMd5Key = strStudentKey;

		Poco::JSON::Object jsnPaper;
		jsnPaper.set("paperName", pPaper->strName);
		jsnPaper.set("studentKey", strStudentKey);			//考试唯一MD5
		Poco::JSON::Array  picArry;

		std::cout << "---- " << pPaper->strName << " ----" << std::endl;
		LIST_PIC_DETAIL::iterator itPic = pPaper->lPic.begin();
		for (; itPic != pPaper->lPic.end(); itPic++)
		{
			pPIC_DETAIL pPic = *itPic;
			int nPos1 = pPic->strFileName.rfind("_");
			int nPos2 = pPic->strFileName.rfind(".");
			std::string strNum = pPic->strFileName.substr(nPos1 + 1, nPos2 - nPos1 - 1);
			
			char szTmp[20] = { 0 };
			sprintf(szTmp, "%s", strNum.c_str());
			char szLetter[20] = { 0 };
			sscanf(szTmp, "%[A-Za-z]", szLetter);
			int nLenLetter = strlen(szLetter);
			char szNum[15] = { 0 };
			memmove(szNum, szTmp + nLenLetter, strlen(szTmp) - nLenLetter);
			int nSN = atoi(szNum);

			Poco::JSON::Object jsnPic;
			jsnPic.set("picSN", nSN);
			jsnPic.set("picName", pPic->strFileName);
			jsnPic.set("upLoadFlag", (int)pPic->bUpLoadFlag);
			jsnPic.set("hashVal", pPic->strHashVal);
			picArry.add(jsnPic);

			std::cout << "pic: " << (*itPic)->strFileName << "\tupLoadFlag: "<< (*itPic)->bUpLoadFlag << "\tMD5: "<< (*itPic)->strHashVal << std::endl;
		}
		std::cout << "---------------" << std::endl;

		jsnPaper.set("picList", picArry);
		paperArry.add(jsnPaper);
	}
	jsnPapers.set("paperList", paperArry);
	
	std::stringstream jsnString;
	jsnPapers.stringify(jsnString, 0);

// 	std::cout << "********** 提交给后端数据: **********\n" << std::endl;
// 	std::cout << jsnString.str() << std::endl;
// 	std::cout << "****************************************" << std::endl;


	//for test 2016.10.24	图像有错乱的问题，单独处理
#if 0
	std::string strJsnModel = SysSet.m_strCurrentDir + "\\md5.txt";

	std::string strMd5Data;
	std::ifstream in(strJsnModel);
	if (!in)
		return NULL;

	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);					//不过滤空格
		strMd5Data.append(strJsnLine);
	}
	in.close();


	std::string strData;
	LIST_PAPER_INFO::iterator it3 = pPapers->lPaper.begin();
	for (; it3 != pPapers->lPaper.end(); it3++)
	{
		pPAPER_INFO pPaper = *it3;
		LIST_PIC_DETAIL::iterator itPic = pPaper->lPic.begin();
		for (; itPic != pPaper->lPic.end(); itPic++)
		{
			pPIC_DETAIL pPic = *itPic;

			if (strMd5Data.find(pPaper->strMd5Key) != std::string::npos)
			{
				char szInfo[200] = { 0 };
				sprintf_s(szInfo, "UPDATE ks_studentanswersheetimage SET address='%s' WHERE subjectId=12 AND studentKey='%s' AND picName='%s';\r\n", \
						  pPic->strHashVal.c_str(), pPaper->strMd5Key.c_str(), pPic->strFileName.c_str());
				strData.append(szInfo);				
			}	
		}
	}

	std::string strJsnFile = SysSet.m_strCurrentDir;
	strJsnFile += "\\update.txt";
	ofstream out(CMyCodeConvert::Utf8ToGb2312(strJsnFile));
	if (!out)	return false;
	out << strData.c_str();
	out.close();

	return true;
	//--
#endif

#if 1
	pSEND_HTTP_TASK pNewTask = new SEND_HTTP_TASK;
	pNewTask->nTaskType = 2;
	pNewTask->strResult = jsnString.str();
	pNewTask->pPapers	= pPapers;
	pNewTask->strEzs	= pPapers->strEzs;
	pNewTask->strUri	= SysSet.m_strBackUri + "/studentAnswerSheet";
	g_fmHttpSend.lock();
	g_lHttpSend.push_back(pNewTask);
	g_fmHttpSend.unlock();
#endif
	
	return true;
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

void CSendToHttpThread::checkTaskStatus(pPAPERS_DETAIL pPapers)
{
	if (pPapers->nTaskCounts == 0)
	{
		//删除解压的文件夹
		try
		{
			Poco::File papersDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
			if (papersDir.exists())
			{
				papersDir.remove(true);

				std::string strLog = "删除试卷袋解压文件夹(" + pPapers->strPapersPath + ")成功";
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "删除试卷袋解压文件夹(" + pPapers->strPapersPath + ")失败: " + exc.message();
			g_Log.LogOutError(strErr);
		}

		if (SysSet.m_nBackupPapers)
		{
			std::string strBackupPath = SysSet.m_strPapersBackupPath + "\\" + pPapers->strSrcPapersFileName;
			try
			{
				Poco::File filePapers(CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersPath));
				filePapers.moveTo(CMyCodeConvert::Gb2312ToUtf8(strBackupPath));
				std::string strLog = "备份试卷袋文件(" + pPapers->strSrcPapersFileName + ")完成";
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}
			catch (Poco::Exception& exc)
			{
				std::string strErrInfo = Poco::format("备份试卷袋(%s)失败,%s", pPapers->strSrcPapersPath, exc.message());
				g_Log.LogOutError(strErrInfo);
				std::cout << strErrInfo << std::endl;
			}
		}
		else
		{
			try
			{
				Poco::File filePapers(CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersPath));
				filePapers.remove(true);
				std::string strLog = "删除试卷袋文件(" + pPapers->strSrcPapersFileName + ")完成";
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}
			catch (Poco::Exception& exc)
			{
				std::string strErrInfo = Poco::format("删除试卷袋(%s)失败,%s", pPapers->strSrcPapersPath, exc.message());
				g_Log.LogOutError(strErrInfo);
				std::cout << strErrInfo << std::endl;
			}
		}

		g_fmPapers.lock();
		LIST_PAPERS_DETAIL::iterator itPapers = g_lPapers.begin();
		for (; itPapers != g_lPapers.end();)
		{
			if (*itPapers == pPapers)
			{
				SAFE_RELEASE(*itPapers);
				itPapers = g_lPapers.erase(itPapers);
				break;
			}
			else
				itPapers++;
		}
		g_fmPapers.unlock();
	}
}


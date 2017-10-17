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
				else if ((*it)->nSendFlag >= SysSet.m_nSendTimes)		//��������������5�Σ�ÿ�μ��5�룬��ʧ���ˣ�����ɾ��
				{
					pTask = *it;
					std::string strEraseInfo;
					if (pTask->nTaskType == 1)
					{
						strEraseInfo = "Erase task(����ͼƬ��zimg): ";
						strEraseInfo.append((*it)->pPic->strFileName);
						
						pTask->pPic->bUpLoadFlag = false;
						pTask->pPapers->fmNum.lock();
						pTask->pPapers->nUpLoadFail++;
						GenerateResult(pTask->pPapers, pTask);
						pTask->pPapers->fmNum.unlock();
					}
					else if (pTask->nTaskType == 2)
					{
						strEraseInfo = "Erase task(�ύͼƬ�����Ϣ�����),�Ծ��: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);


						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 3)
					{
						strEraseInfo = "Erase task(�ύ OMR �����Ϣ�����),�Ծ��: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 4)
					{
						strEraseInfo = "Erase task(�ύ ZKZH �����Ϣ�����),�Ծ��: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//zkzh
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 5)
					{
						strEraseInfo = "Erase task(�ύ ѡ���� �����Ϣ�����),�Ծ��: ";
						strEraseInfo.append(pTask->pPapers->strPapersName);

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//electomr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 6)
					{
						strEraseInfo = "Erase task(����ģ��ͼƬ��zimg): ";
						strEraseInfo.append((*it)->pPic->strFileName);

						pTask->pPic->bUpLoadFlag = false;
						pTask->pPapers->fmNum.lock();
						pTask->pPapers->nUpLoadFail++;
						GenerateResult(pTask->pPapers, pTask);
						pTask->pPapers->fmNum.unlock();
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
				std::cout << "post Omr ���ݸ���˷�����" << std::endl;
			else if (pTask->nTaskType == 4)
				std::cout << "post ZKZH ���ݸ���˷�����" << std::endl;
			else if (pTask->nTaskType == 7)
			{
				std::cout << "post Omr��ZKZH��ѡ���� ���ݸ���˷�����" << std::endl;
				HandleOmrTask(pTask);
				continue;
			}

			Poco::URI uri(pTask->strUri);				//pTask->strUri
			Poco::Net::HTTPClientSession session;
			Poco::Net::HTTPRequest request;
			Poco::Net::HTTPResponse response;

			if (!doRequest(session, request, uri, pTask))
			{
				if (pTask->nTaskType == 1 || pTask->nTaskType == 6)
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
					std::string strLog = "postͼƬ���ݸ���˷�����ʧ��";
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
				{		//���ص����ݽ���ʧ��
					if (pTask->nTaskType == 1 || pTask->nTaskType == 6)
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
					//����ʧ�ܣ��ٷ������
					g_fmHttpSend.lock();
					g_lHttpSend.push_back(pTask);
					g_fmHttpSend.unlock();
					continue;
				}
				else
				{
					if (pTask->nTaskType == 1 || pTask->nTaskType == 6)
					{
						pTask->pPic->bUpLoadFlag = true;
						pTask->pPapers->fmNum.lock();
						pTask->pPapers->nUpLoadSuccess++;
						GenerateResult(pTask->pPapers, pTask);
						pTask->pPapers->fmNum.unlock();
					}
					else if (pTask->nTaskType == 2)
					{
						//�����״̬�����1λ��1
						pTask->pPapers->fmResultState.lock();
						pTask->pPapers->nResultSendState = pTask->pPapers->nResultSendState | 1;
						pTask->pPapers->fmResultState.unlock();

						std::string strLog = "�����Ծ��ͼƬ��Ϣ����˳ɹ�, �Ծ����: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;
					}
					else if (pTask->nTaskType == 3)
					{
						//�����״̬�����2λ��1
						pTask->pPapers->fmResultState.lock();
						pTask->pPapers->nResultSendState = pTask->pPapers->nResultSendState | (1<<1);
						pTask->pPapers->fmResultState.unlock();

						std::string strLog = "����OMR��Ϣ����˳ɹ�, �Ծ����: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers OMR result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//omr
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 4)
					{
						//�����״̬�����3λ��1
						pTask->pPapers->fmResultState.lock();
						pTask->pPapers->nResultSendState = pTask->pPapers->nResultSendState | (1 << 2);
						pTask->pPapers->fmResultState.unlock();

						std::string strLog = "����ZKZH��Ϣ����˳ɹ�, �Ծ����: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
						g_Log.LogOut(strLog);
						std::cout << "post papers ZKZH result info success, papersName: " << pTask->pPapers->strPapersName << std::endl;

						pTask->pPapers->fmTask.lock();
						pTask->pPapers->nTaskCounts--;			//zkzh
						pTask->pPapers->fmTask.unlock();
						checkTaskStatus(pTask->pPapers);
					}
					else if (pTask->nTaskType == 5)
					{
						//�����״̬�����4λ��1
						pTask->pPapers->fmResultState.lock();
						pTask->pPapers->nResultSendState = pTask->pPapers->nResultSendState | (1 << 3);
						pTask->pPapers->fmResultState.unlock();

						std::string strLog = "����ѡ������Ϣ����˳ɹ�, �Ծ����: " + pTask->pPapers->strPapersName + "\tdetail: " + pTask->strResult;
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
				if (pTask->nTaskType == 1 || pTask->nTaskType == 6)
				{
					strLog = "send file fail: " + pTask->pPic->strFileName + "\tErrCode(Http������): " + response.getReason() + "\tPath: " + pTask->pPic->strFilePath;
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
					strLog = "post papers result failed: " + pTask->pPapers->strPapersName + "\tErrCode(Http������): " + response.getReason() + "\tPath: " + pTask->pPapers->strPapersPath + "\nResult info: ..." /*+ pTask->strResult*/;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
				}
				else if (pTask->nTaskType == 3)
				{
					strLog = "post omr result failed: " + pTask->pPapers->strPapersName + "\tErrCode(Http������): " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
				}
				else if (pTask->nTaskType == 4)
				{
					strLog = "post zkzh result failed: " + pTask->pPapers->strPapersName + "\tErrCode(Http������): " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
				}
				else if (pTask->nTaskType == 5)
				{
					strLog = "post ElectOmr result failed: " + pTask->pPapers->strPapersName + "\tErrCode(Http������): " + response.getReason() + "\tName: " + pTask->pPapers->strPapersName + "\nResult info: ..." /*+ pTask->strResult*/;
					g_Log.LogOutError(strLog);
					std::cout << strLog << std::endl;
				}
				pTask->nSendFlag++;
				pTask->sTime.update();
				//����ʧ�ܣ��ٷ������
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
			char szSendType[30] = { 0 };
			if (pTask->nTaskType == 1)
				sprintf_s(szSendType, "(�Ծ�ͼƬ)");
			else if (pTask->nTaskType == 2)
				sprintf_s(szSendType, "(ͼƬ��ַ)");
			else if (pTask->nTaskType == 3)
				sprintf_s(szSendType, "(OMR)");
			else if (pTask->nTaskType == 4)
				sprintf_s(szSendType, "(ZKZH)");
			else if (pTask->nTaskType == 5)
				sprintf_s(szSendType, "(ѡ����)");
			else if (pTask->nTaskType == 6)
				sprintf_s(szSendType, "(ģ��ͼƬ)");
			char szFailTimes[100] = { 0 };
			sprintf(szFailTimes, " --- %s Send fail times : %d", szSendType, pTask->nSendFlag);

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
			}			

			std::cout << "\n";
			std::cout << strErrorInfo << std::endl;
			g_Log.LogOutError(strErrorInfo);

			pTask->nSendFlag++;
			pTask->sTime.update();
			//����ʧ�ܣ��ٷ������
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
				strErrorInfo.append("**** Unknown error ***\t�ύOMR�쳣");
			}
			else if (pTask->nTaskType == 4)
			{
				strErrorInfo.append("**** Unknown error ***\t�ύZKZH�쳣");
			}
			else if (pTask->nTaskType == 5)
			{
				strErrorInfo.append("**** Unknown error ***\t�ύѡ������Ϣ�쳣");
			}

			std::cout << "\n";
			std::cout << strErrorInfo << std::endl;
			g_Log.LogOutError(strErrorInfo);

			pTask->nSendFlag++;
			pTask->sTime.update();
			//����ʧ�ܣ��ٷ������
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
	if (pTask->nTaskType == 1 || pTask->nTaskType == 6)
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
	else if (pTask->nTaskType == 2)		//�ύ�����
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
		
		if (pTask->nTaskType == 1 || pTask->nTaskType == 6)
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
				std::string strLog = "�����Ծ��ͼƬ��Ϣ�����ʧ�ܣ�ʧ��ԭ��: " + strMsg;
				strLog.append("\t����ʧ�ܴ���: ");
				strLog.append("\n��������: " + pTask->strResult);
				strLog.append(szCount);
				g_Log.LogOutError(strLog);
				std::cout << strLog << std::endl;
			}
			else
			{
				if (SysSet.m_nUpLoadOmrData || SysSet.m_nUpLoadZKZH || SysSet.m_nUpLoadElectOmr)
				{
					HandleOmrTask(pTask);
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
					if (strMsg.length() > 1000)
					{
						strMsg = strMsg.substr(0, 1000);
						strMsg.append("...");
					}
				}
				char szCount[5] = { 0 };
				sprintf(szCount, "%d", pTask->nSendFlag);
				std::string strLog = "�ύOMR��Ϣ�����ʧ��(�Ծ��:" + pTask->pPapers->strPapersName;
				strLog.append(")��ʧ��ԭ��: " + strMsg);
				strLog.append("\t����ʧ�ܴ���: ");
				strLog.append(szCount);
				strLog.append("\n���͵�����: " + pTask->strResult);
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
					if (strMsg.length() > 1000)
					{
						strMsg = strMsg.substr(0, 1000);
						strMsg.append("...");
					}
				}
				char szCount[5] = { 0 };
				sprintf(szCount, "%d", pTask->nSendFlag);
				std::string strLog = "�ύZKZH��Ϣ�����ʧ��(�Ծ��:" + pTask->pPapers->strPapersName;
				strLog.append(")��ʧ��ԭ��: " + strMsg);
				strLog.append("\t����ʧ�ܴ���: ");
				strLog.append(szCount);
				strLog.append("\n���͵�����: " + pTask->strResult);
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
					if (strMsg.length() > 1000)
					{
						strMsg = strMsg.substr(0, 1000);
						strMsg.append("...");
					}
				}
				char szCount[5] = { 0 };
				sprintf(szCount, "%d", pTask->nSendFlag);
				std::string strLog = "�ύѡ������Ϣ�����ʧ��(�Ծ��:" + pTask->pPapers->strPapersName;
				strLog.append(")��ʧ��ԭ��: " + strMsg);
				strLog.append("\t����ʧ�ܴ���: ");
				strLog.append(szCount);
				strLog.append("\n���͵�����: " + pTask->strResult);
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

bool CSendToHttpThread::GenerateResult(pPAPERS_DETAIL pPapers, pSEND_HTTP_TASK pTask)
{
	if (pPapers->nUpLoadSuccess + pPapers->nUpLoadFail != pPapers->nTotalPics)
		return false;

	if (pTask->nTaskType == 6)	//ģ��ͼƬ�ϴ����
	{
		pMODELINFO pModelInfo = NULL;
		char szIndex[50] = { 0 };
		sprintf(szIndex, "%d_%d", pPapers->nExamID, pPapers->nSubjectID);
		MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
		if (itFind != _mapModel_.end())
		{
			pModelInfo = itFind->second;
		}
		if (pModelInfo)
		{
			if (!pModelInfo->pUploadModelInfo)
			{
				std::string strLog = "ģ�岻�����ã�ģ����ϢΪ��";
				g_Log.LogOutError(strLog);
				return false;
			}
			Poco::JSON::Array arryModelPics;
			LIST_PAPER_INFO::iterator itPaper = pPapers->lPaper.begin();
			for (int i = 1; itPaper != pPapers->lPaper.end(); i++, itPaper++)
			{
				pPAPER_INFO pPaper = *itPaper;
				LIST_PIC_DETAIL::iterator itPic = pPaper->lPic.begin();
				for (; itPic != pPaper->lPic.end(); itPic++)
				{
					pPIC_DETAIL pPic = *itPic;
					Poco::JSON::Object objPic;
					objPic.set("paperId", i);
					objPic.set("w", pPic->nPicW);
					objPic.set("h", pPic->nPicH);
					objPic.set("md5", pPic->strHashVal);
					arryModelPics.add(objPic);
				}
			}
			Poco::JSON::Parser parserElectOmr;
			Poco::Dynamic::Var resultElectOmr;
			Poco::JSON::Array::Ptr electOmrArry;

			try
			{
				resultElectOmr = parserElectOmr.parse(pModelInfo->pUploadModelInfo->szElectOmr);
				electOmrArry = resultElectOmr.extract<Poco::JSON::Array::Ptr>();
			}
			catch (...)
			{
			}
			Poco::JSON::Object jsnModel;
			jsnModel.set("examId", pModelInfo->pUploadModelInfo->nExamID);
			jsnModel.set("subjectId", pModelInfo->pUploadModelInfo->nSubjectID);
			jsnModel.set("tmplateName", CMyCodeConvert::Gb2312ToUtf8(pModelInfo->pUploadModelInfo->szModelName));
			jsnModel.set("netAddr", SysSet.m_strPicWwwNetAddr);
			jsnModel.set("paper", arryModelPics);
			if (strlen(pModelInfo->pUploadModelInfo->szElectOmr) > 0)
				jsnModel.set("modelElectOmr", electOmrArry);	//pModelInfo->pUploadModelInfo->szElectOmr

			std::stringstream jsnString;
			jsnModel.stringify(jsnString, 0);

			std::string strLog = "�ύģ��ͼƬ��zimg��������ɣ�����ģ�����ƣ��ύ����˵�����: " + jsnString.str();
			g_Log.LogOut(strLog);

			std::string strEzs = pModelInfo->pUploadModelInfo->szEzs;
			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->nExamID = pModelInfo->nExamID;
			pTask->nSubjectID = pModelInfo->nSubjectID;
			pTask->strUri = SysSet.m_strBackUri + "/scanTemplate";
			pTask->pUser = pModelInfo->pUser;
			pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
			pTask->strMsg = "setScanModel";
			pTask->strRequest = jsnString.str();
			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
		}
		else
		{
			std::string strLog = Poco::format("(%d_%d)ģ��ͼƬ�ϴ�zimg��ɣ���Ӧ��ģ����ϢΪ�գ�������������ģ����Ϣ", pPapers->nExamID, pPapers->nSubjectID);
			g_Log.LogOutError(strLog);
		}

		//ɾ����ѹ���ļ���
		try
		{
			Poco::File papersDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
			if (papersDir.exists())
			{
				papersDir.remove(true);

				std::string strLog = "ɾ��ģ���ѹ�ļ���(" + pPapers->strPapersPath + ")�ɹ�";
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "ɾ��ģ���ѹ�ļ���(" + pPapers->strPapersPath + ")ʧ��: " + exc.message();
			g_Log.LogOutError(strErr);
		}

		return true;
	}

	std::string strLog = "�Ծ��(" + pPapers->strPapersName +")ͼƬ�ϴ���������ɣ���ʼ�����ظ�ͼ��У�顣";
	g_Log.LogOut(strLog);
	std::cout << strLog << std::endl;

	//++ͼ���ظ�У��
	bool bResult = true;
	LIST_PAPER_INFO::iterator it = pPapers->lPaper.begin();
	for (; it != pPapers->lPaper.end(); it++)
	{
		pPAPER_INFO pPaper = *it;

		LIST_PIC_DETAIL::iterator itPic = pPaper->lPic.begin();
		for (; itPic != pPaper->lPic.end(); itPic++)
		{
			pPIC_DETAIL pPic = *itPic;
			if (!checkPicAddr(pPic->strHashVal, pPapers, pPic))
			{
				bResult = false;
				break;
			}
		}
		if (!bResult)
			break;
	}
	if (!bResult)
	{
		//��������ƶ���ָ��Ŀ¼���ȴ��˹�����
		Poco::LocalDateTime now;
		std::string strErrorDir = Poco::format("%s\\%04d-%02d-%02d", CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strErrorPkg), now.year(), now.month(), now.day());
		std::string strErrorPath = strErrorDir + "\\" + CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersFileName);
		try
		{
			Poco::File fileErrDir(strErrorDir);
			if (!fileErrDir.exists())
				fileErrDir.createDirectories();

			Poco::File filePapers(CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersPath));
			filePapers.moveTo(strErrorPath);
			std::string strLog = "�Ծ��(" + pPapers->strSrcPapersFileName + ")�������⣬��Ҫ�˹����";
			g_Log.LogOut(strLog);
			std::cout << "\n\n*************************\n" << strLog << "\n*************************\n\n\n" << std::endl;
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "�ƶ������Ծ��(" + pPapers->strPapersPath + ")ʧ�ܣ����Ծ�������ظ�ͼ�����ͼ���ַΪ�գ���Ҫ�˹����: " + exc.message();
			g_Log.LogOutError(strErr);
		}
		return false;
	}
	//--
	strLog = "�Ծ��(" + pPapers->strPapersName + ")ͼ���ظ�У����ɣ���ʼ�ύ��ˡ�";
	g_Log.LogOut(strLog);
	std::cout << strLog << std::endl;

	std::string strShowLog;		//��Ļ����ʾ����Ϣ
	
	Poco::JSON::Object jsnPapers;
	jsnPapers.set("papers", CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersName));
	jsnPapers.set("papersDesc", CMyCodeConvert::Gb2312ToUtf8(pPapers->strDesc));
	jsnPapers.set("totalNum", pPapers->nTotalPaper);
	jsnPapers.set("qkNum", pPapers->nQk);
#if 1	//test
	std::string strExamID = Poco::format("%d", pPapers->nExamID);
	jsnPapers.set("examId", strExamID);
#else
	jsnPapers.set("examId", pPapers->nExamID);
#endif
//	jsnPapers.set("examId", pPapers->nExamID);
	jsnPapers.set("subjectId", pPapers->nSubjectID);
	jsnPapers.set("teacherId", pPapers->nTeacherId);
	jsnPapers.set("userId", pPapers->nUserId);
	jsnPapers.set("netAddr", SysSet.m_strPicWwwNetAddr);
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
		jsnPaper.set("studentKey", strStudentKey);			//����ΨһMD5
		jsnPaper.set("qkStatus", pPaper->nQkFlag);			//ȱ����Ϣ
		jsnPaper.set("wjStatus", pPaper->nWjFlag);			//Υ����Ϣ
		jsnPaper.set("nOmrAnswerFlag", pPaper->nStandardAnswer);	//��ǰ�Ծ�Ϊ��0-���������Ծ� 1-�͹�����(��ǰ�Ծ��Ϊ�͹�����), 2-��������
		jsnPaper.set("issueFlag", pPaper->nIssueFlag);		//�����Ծ��ʶ��0-�����Ծ���ȫ����ʶ�������ģ����˹���Ԥ��1-�����Ծ�ɨ��Ա�ֶ��޸Ĺ���
															//2-׼��֤��Ϊ�գ�ɨ��Աû���޸ģ�3-ɨ��Ա��ʶ����Ҫ��ɨ���Ծ� //ע�⣬���û�������ʶ����Ĭ���������Ծ�
		Poco::JSON::Array  picArry;

//		strShowLog = "---- " + pPaper->strName + "----\n";
		strShowLog.append("---- " + pPaper->strName + "----\n");
		LIST_PIC_DETAIL::iterator itPic = pPaper->lPic.begin();
		for (; itPic != pPaper->lPic.end(); itPic++)
		{
			pPIC_DETAIL pPic = *itPic;
			int nPos1 = pPic->strFileName.rfind("_");
			int nPos2 = pPic->strFileName.rfind(".");
			std::string strNum = pPic->strFileName.substr(nPos1 + 1, nPos2 - nPos1 - 1);
			
			char szTmp[60] = { 0 };
			sprintf(szTmp, "%s", strNum.c_str());
			char szLetter[60] = { 0 };
			sscanf(szTmp, "%[A-Za-z]", szLetter);
			int nLenLetter = strlen(szLetter);
			char szNum[60] = { 0 };
			memmove(szNum, szTmp + nLenLetter, strlen(szTmp) - nLenLetter);
			int nSN = atoi(szNum);

			Poco::JSON::Object jsnPic;
			jsnPic.set("picSN", nSN);
			jsnPic.set("picName", pPic->strFileName);
			jsnPic.set("upLoadFlag", (int)pPic->bUpLoadFlag);
			jsnPic.set("hashVal", pPic->strHashVal);
			picArry.add(jsnPic);

			std::string strTmp = Poco::format("pic: %s\tupLoadFlag: %d\tMD5: %s\n", (*itPic)->strFileName, (int)(*itPic)->bUpLoadFlag, (*itPic)->strHashVal);
			strShowLog.append(strTmp);
		}
		strShowLog.append("---------------\n");

		jsnPaper.set("picList", picArry);
		paperArry.add(jsnPaper);
	}
	jsnPapers.set("paperList", paperArry);
	
	std::stringstream jsnString;
	jsnPapers.stringify(jsnString, 0);

	std::cout << strShowLog << std::endl;
// 	std::cout << "********** �ύ���������: **********\n" << std::endl;
// 	std::cout << jsnString.str() << std::endl;
// 	std::cout << "****************************************" << std::endl;
	
#if 1
	pPapers->strSendPicsAddrResult = jsnString.str();

	pSEND_HTTP_TASK pNewTask = new SEND_HTTP_TASK;
	pNewTask->nTaskType = 2;
	pNewTask->strResult = jsnString.str();
	pNewTask->pPapers	= pPapers;
	pNewTask->strEzs	= pPapers->strEzs;
	if(pPapers->nStandardAnswer == 2)	//��������ʱ���ύ����ĵ�ַ
		pNewTask->strUri = SysSet.m_strBackUri + "/zganswer";
	else
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
		//ɾ����ѹ���ļ���
		try
		{
			Poco::File papersDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
			if (papersDir.exists())
			{
				papersDir.remove(true);

				std::string strLog = "ɾ���Ծ����ѹ�ļ���(" + pPapers->strPapersPath + ")�ɹ�";
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}
		}
		catch (Poco::Exception& exc)
		{
			std::string strErr = "ɾ���Ծ����ѹ�ļ���(" + pPapers->strPapersPath + ")ʧ��: " + exc.message();
			g_Log.LogOutError(strErr);
		}

		//++���ͼƬ��ַ��OMR��ZKZH��ѡ������Ϣ�Ƿ��ύ�ɹ���û���ύ�ɹ��Ľ�����Ϣ��¼���ı��ļ����´�����ʱ�Զ������ύ
		int nPushPicSucc = pPapers->nResultSendState & 1;
		int nPushOmrSucc = pPapers->nResultSendState >> 1 & 1;
		int nPushZkzhSucc = pPapers->nResultSendState >> 2 & 1;
		int nPushElectOmrSucc = pPapers->nResultSendState >> 3 & 1;
		try
		{
			Poco::File backupDir(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strReSendPkg));
			if (!backupDir.exists())
				backupDir.createDirectories();
		}
		catch (Poco::Exception& exc)
		{
			std::string strErrInfo = Poco::format("�����Ծ���ش��ļ���(%s)ʧ��,%s", SysSet.m_strReSendPkg, exc.message());
			g_Log.LogOutError(strErrInfo);
			std::cout << strErrInfo << std::endl;
		}

		bool bMoveFlag = false;		//�Ծ�����ƶ���ʶ
		if (!nPushPicSucc)
		{
			std::string strFilePath = SysSet.m_strReSendPkg + pPapers->strPapersName + "_#_pics.txt";
			ofstream out(strFilePath);
			out << pPapers->strSendPicsAddrResult.c_str();
			out.close();
			bMoveFlag = true;
		}
		else	//�������ֻ����ͼƬ��ַ��Ϣ�ύ�ɹ�ʱ�ſ��ܷ�������Ϊֻ�����ύͼƬ��ַ�ɹ��󣬲ŷ���OMR��ZKZH��ѡ����Ϣ
		{
			if (!nPushOmrSucc && !pPapers->strSendOmrResult.empty())
			{
				//***************	��Ҫ������ʱ���м�� 2017.1.19	*******************************
				std::string strFilePath = SysSet.m_strReSendPkg + pPapers->strPapersName + "_#_omr.txt";
				ofstream out(strFilePath);
				out << pPapers->strSendOmrResult.c_str();
				out.close();
				bMoveFlag = true;
			}
			if (!nPushZkzhSucc && !pPapers->strSendZkzhResult.empty())
			{
				std::string strFilePath = SysSet.m_strReSendPkg + pPapers->strPapersName + "_#_zkzh.txt";
				ofstream out(strFilePath);
				out << pPapers->strSendZkzhResult.c_str();
				out.close();
				bMoveFlag = true;
			}
			if (!nPushElectOmrSucc && !pPapers->strSendElectOmrResult.empty())
			{
				std::string strFilePath = SysSet.m_strReSendPkg + pPapers->strPapersName + "_#_electOmr.txt";
				ofstream out(strFilePath);
				out << pPapers->strSendElectOmrResult.c_str();
				out.close();
				bMoveFlag = true;
			}
		}
		//--

		if (bMoveFlag)
		{
			//�ļ��ƶ����ط���Ŀ¼�ĸ�Ŀ¼��
			std::string strMovePath = SysSet.m_strReSendPkg + pPapers->strSrcPapersFileName;
			MovePkg(pPapers, strMovePath);
		}
		else
		{
			if (SysSet.m_nBackupPapers)
			{
				Poco::LocalDateTime now;
				std::string strBackupDir = Poco::format("%s\\%04d-%02d-%02d\\%d_%d\\%s", CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strPapersBackupPath), now.year(), now.month(), now.day(), pPapers->nExamID, pPapers->nSubjectID, pPapers->strUploader);
				std::string strBackupPath = strBackupDir + "\\" + CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersFileName);
				try
				{
					Poco::File backupDir(strBackupDir);
					if (!backupDir.exists())
						backupDir.createDirectories();

					Poco::File filePapers(CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersPath));
					filePapers.moveTo(strBackupPath);
					std::string strLog = "�����Ծ���ļ�(" + pPapers->strSrcPapersFileName + ")���";
					g_Log.LogOut(strLog);
					std::cout << strLog << std::endl;
				}
				catch (Poco::Exception& exc)
				{
					std::string strErrInfo = Poco::format("�����Ծ��(%s)ʧ��,%s", pPapers->strSrcPapersPath, exc.message());
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
					std::string strLog = "ɾ���Ծ���ļ�(" + pPapers->strSrcPapersFileName + ")���";
					g_Log.LogOut(strLog);
					std::cout << strLog << std::endl;
				}
				catch (Poco::Exception& exc)
				{
					std::string strErrInfo = Poco::format("ɾ���Ծ��(%s)ʧ��,%s", pPapers->strSrcPapersPath, exc.message());
					g_Log.LogOutError(strErrInfo);
					std::cout << strErrInfo << std::endl;
				}
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

bool CSendToHttpThread::checkPicAddr(std::string& strPicAddr, pPAPERS_DETAIL pPapers, pPIC_DETAIL pPic)
{
	if (strPicAddr.empty())
	{
		std::string strLog = "��⵽��Ҫ�ύ��ͼƬ��ַΪ��(" + pPapers->strPapersName + ":" + pPic->strFileName + ")";
		g_Log.LogOutError(strLog);
		return false;
	}

	MAP_PIC_ADDR::iterator it = _mapPicAddr_.find(strPicAddr);
	if (it != _mapPicAddr_.end())
	{
		std::string strLog = "��⵽��Ҫ�ύ��ͼƬ��ַ" + strPicAddr + "(" + pPapers->strPapersName + ":" + pPic->strFileName + ")�Ѿ����Ծ��(" + it->second + ")����";
		g_Log.LogOutError(strLog);
		return false;
	}
	else
	{
		_mapPicAddrLock_.lock();
		_mapPicAddr_.insert(std::pair<std::string, std::string>(strPicAddr, pPapers->strPapersName + ":" + pPic->strFileName));
		_mapPicAddrLock_.unlock();
	}
	return true;
}

void CSendToHttpThread::HandleOmrTask(pSEND_HTTP_TASK pTask)
{
	std::string strLog = "��ʼ�ύOMR��ZKZH��ѡ������Ϣ(" + pTask->pPapers->strPapersName + ")";
	g_Log.LogOut(strLog);
	bool bHasElectOmr = false;
	Poco::JSON::Array snArry;
	Poco::JSON::Array omrArry;
	Poco::JSON::Array electOmrArry;
	std::stringstream jsnSnString;
	std::stringstream jsnOmrString;
	std::stringstream jsnElectOmrString;
	LIST_PAPER_INFO::iterator it = pTask->pPapers->lPaper.begin();
	for (int i = 1; it != pTask->pPapers->lPaper.end(); it++, i++)
	{
		pPAPER_INFO pPaper = *it;

		if (pPaper->strMd5Key.empty())
		{
			std::string strStudentInfo = pTask->pPapers->strPapersName + "_" + pPaper->strName;
			std::string strStudentKey = calcMd5(strStudentInfo);
			pPaper->strMd5Key = strStudentKey;
		}

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
			char szItem[20] = { 0 };
			sprintf_s(szItem, "%d --- ", i);
			std::string strErrInfo;
			strErrInfo.append("Error when parse SN: ");
			strErrInfo.append(szItem);
			strErrInfo.append(jsone.message() + "\tData:" + pPaper->strSnDetail);
			g_Log.LogOutError(strErrInfo);
			std::string strShowInfo;
			if (strErrInfo.length() > 1000)
			{
				strShowInfo = strErrInfo.substr(0, 1000);
				strShowInfo.append("...");
			}
			else
				strShowInfo = strErrInfo;
			std::cout << strShowInfo << std::endl;
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
			char szItem[20] = { 0 };
			sprintf_s(szItem, "%d --- ", i);
			std::string strErrInfo;
			strErrInfo.append("Error when parse Omr: ");
			strErrInfo.append(szItem);
			strErrInfo.append(jsone.message() + "\tData:" + pPaper->strOmrDetail);
			g_Log.LogOutError(strErrInfo);
			std::string strShowInfo;
			if (strErrInfo.length() > 1000)
			{
				strShowInfo = strErrInfo.substr(0, 1000);
				strShowInfo.append("...");
			}
			else
				strShowInfo = strErrInfo;
			std::cout << strShowInfo << std::endl;
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
				char szItem[20] = { 0 };
				sprintf_s(szItem, "%d --- ", i);
				std::string strErrInfo;
				strErrInfo.append("Error when parse ElectOmr: ");
				strErrInfo.append(szItem);
				strErrInfo.append(jsone.message() + "\tData:" + pPaper->strElectOmrDetail);
				g_Log.LogOutError(strErrInfo);
				std::string strShowInfo;
				if (strErrInfo.length() > 1000)
				{
					strShowInfo = strErrInfo.substr(0, 1000);
					strShowInfo.append("...");
				}
				else
					strShowInfo = strErrInfo;
				std::cout << strShowInfo << std::endl;
			}
		}
	}

	snArry.stringify(jsnSnString, 0);
	omrArry.stringify(jsnOmrString, 0);
	if (bHasElectOmr)
		electOmrArry.stringify(jsnElectOmrString, 0);

	if (SysSet.m_nUpLoadZKZH)
	{
		pTask->pPapers->fmTask.lock();
		pTask->pPapers->nTaskCounts++;			//zkzh
		pTask->pPapers->fmTask.unlock();
		pTask->pPapers->strSendZkzhResult = jsnSnString.str();	//�������Ϣ���浽�Ծ���ṹ���У��Է��쳣����ʱ��Ҫ���¼��� 2017.1.19

		pSEND_HTTP_TASK pSnTask = new SEND_HTTP_TASK;
		pSnTask->nTaskType = 4;
		pSnTask->strResult = jsnSnString.str();
		pSnTask->pPapers = pTask->pPapers;
		pSnTask->strEzs = pTask->pPapers->strEzs;
		pSnTask->strUri = SysSet.m_strBackUri + "/zkzh";
		g_fmHttpSend.lock();
		g_lHttpSend.push_back(pSnTask);
		g_fmHttpSend.unlock();
	}
	
	if (SysSet.m_nUpLoadOmrData)
	{
		pTask->pPapers->fmTask.lock();
		pTask->pPapers->nTaskCounts++;			//omr
		pTask->pPapers->fmTask.unlock();
		pTask->pPapers->strSendOmrResult = jsnOmrString.str();	//�������Ϣ���浽�Ծ���ṹ���У��Է��쳣����ʱ��Ҫ���¼��� 2017.1.19
		
		pSEND_HTTP_TASK pOmrTask = new SEND_HTTP_TASK;
		pOmrTask->nTaskType = 3;
		pOmrTask->strResult = jsnOmrString.str();
		pOmrTask->pPapers = pTask->pPapers;
		pOmrTask->strEzs = pTask->pPapers->strEzs;
		if(pOmrTask->pPapers->nStandardAnswer == 1)		//�ύ�͹���𰸵������ַ
			pOmrTask->strUri = SysSet.m_strBackUri + "/kgansweromr";
		else
			pOmrTask->strUri = SysSet.m_strBackUri + "/omr";
		g_fmHttpSend.lock();
		g_lHttpSend.push_back(pOmrTask);
		g_fmHttpSend.unlock();
	}
	
	if (SysSet.m_nUpLoadElectOmr)
	{
		//++�ύѡ������Ϣ	*************	ע�⣺���ﻹ���У���Ҫ�ͺ��ȷ��	********************
		if (bHasElectOmr)
		{
			pTask->pPapers->fmTask.lock();
			pTask->pPapers->nTaskCounts++;			//electOmr
			pTask->pPapers->fmTask.unlock();
			pTask->pPapers->strSendElectOmrResult = jsnElectOmrString.str();	//�������Ϣ���浽�Ծ���ṹ���У��Է��쳣����ʱ��Ҫ���¼��� 2017.1.19

			pSEND_HTTP_TASK pElectOmrTask = new SEND_HTTP_TASK;
			pElectOmrTask->nTaskType = 5;
			pElectOmrTask->strResult = jsnElectOmrString.str();
			pElectOmrTask->pPapers = pTask->pPapers;
			pElectOmrTask->strEzs = pTask->pPapers->strEzs;
			pElectOmrTask->strUri = SysSet.m_strBackUri + "/choosetitleinfo";
			g_fmHttpSend.lock();
			g_lHttpSend.push_back(pElectOmrTask);
			g_fmHttpSend.unlock();
		}
		//--
	}
	

	strLog = "ZKZH��Ϣ����: " + jsnSnString.str();
	g_Log.LogOut(strLog);
	strLog = "OMR��Ϣ����: " + jsnOmrString.str();
	g_Log.LogOut(strLog);
	if (bHasElectOmr)
	{
		strLog = "ѡ������Ϣ����: " + jsnElectOmrString.str();
		g_Log.LogOut(strLog);
	}
}

bool CSendToHttpThread::MovePkg(pPAPERS_DETAIL pPapers, std::string& strMovePath)
{
	bool bResult = false;
	std::string strFullPath = CMyCodeConvert::Gb2312ToUtf8(strMovePath);
	try
	{
		Poco::File reSendDir(CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strReSendPkg));
		if (!reSendDir.exists())
			reSendDir.createDirectories();

		Poco::File filePapers(CMyCodeConvert::Gb2312ToUtf8(pPapers->strSrcPapersPath));
		if (!filePapers.exists())
		{
			std::string strLog = "�Ծ���ļ�(" + pPapers->strSrcPapersFileName + ")�����ڣ������Ѿ����Ƴ�";
			g_Log.LogOutError(strLog);
			std::cout << strLog << std::endl;
			bResult = false;
			return bResult;
		}
		filePapers.moveTo(strFullPath);
		std::string strLog = "�ƶ��Ծ���ļ�(" + pPapers->strSrcPapersFileName + ")���";
		g_Log.LogOut(strLog);
		std::cout << strLog << std::endl;
		bResult = true;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo = Poco::format("�ƶ��Ծ��(%s)ʧ��,%s", pPapers->strSrcPapersPath, exc.message());
		g_Log.LogOutError(strErrInfo);
		std::cout << strErrInfo << std::endl;
	}
	return bResult;
}


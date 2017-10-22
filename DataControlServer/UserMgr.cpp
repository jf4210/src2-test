#include "UserMgr.h"
#include "DCSDef.h"

CUserMgr::CUserMgr()
{
}


CUserMgr::~CUserMgr()
{
}

int CUserMgr::HandleHeader(CMission* pMission)
{
	ST_CMD_HEADER header = *(ST_CMD_HEADER*)pMission->m_pMissionData;
	CNetUser* pUser = pMission->m_pUser;

	BOOL bFind = TRUE; //�������Ƿ񱻴���
	switch (header.usCmd)
	{
	case USER_LOGIN:
		{
			ST_LOGIN_INFO  LoginInfo = *(pStLoginInfo)(pMission->m_pMissionData + HEAD_SIZE);
			std::cout << "login info: " << LoginInfo.szUserNo << ", pwd: " << LoginInfo.szPWD << std::endl;
			
// 			_mapUserLock_.lock();
// 			MAP_USER::iterator itFind = _mapUser_.find(LoginInfo.szUserNo);
// 			if (itFind != _mapUser_.end())
// 			{
// 				//�ظ���¼���ѣ�
// 				std::string strSendData = "�˺��ѵ�¼";
// 
// 				pUser->SendResponesInfo(USER_RESPONSE_LOGIN, RESULT_LOGIN_FAIL, (char*)strSendData.c_str(), strSendData.length());
// 				_mapUserLock_.unlock();
// 				return 1;
// 			}
// 			_mapUserLock_.unlock();

			#ifdef TEST_MODE
			_mapUserLock_.lock();
			MAP_USER::iterator itFind2 = _mapUser_.find(LoginInfo.szUserNo);
			if (itFind2 == _mapUser_.end())
			{
				_mapUser_.insert(MAP_USER::value_type(LoginInfo.szUserNo, pUser));
			}
			else
			{
				itFind2->second = pUser;
			}
			_mapUserLock_.unlock();

			ST_LOGIN_RESULT stResult;
			ZeroMemory(&stResult, sizeof(stResult));
			stResult.nTeacherId = 100;
			stResult.nUserId = 200;
			std::string strEzs = "1234567890abcdefghijklmnopqrstuvwxyz";
			strncpy(stResult.szEzs, strEzs.c_str(), strEzs.length());
			strncpy(stResult.szUserInfo, LoginInfo.szUserNo, 100);

			char szLoginResult[1024] = { 0 };
			memcpy(szLoginResult, (char*)&stResult, sizeof(stResult));

			pUser->UpdateLogonInfo(LoginInfo.szUserNo, LoginInfo.szPWD);
			pUser->SendResponesInfo(USER_RESPONSE_LOGIN, RESULT_SUCCESS, szLoginResult, sizeof(stResult));
			return 1;
			#endif

//		#ifdef TO_WHTY
		#if 1
			pSCAN_REQ_TASK pTask = NULL;
			if(SysSet.m_nServerMode == 1)
			{
				std::string strAppKey = "SP0000000TEST";
				std::string strAppValue = "63d4311d46714a39-a54cf2b0537a79b6TEST";
				std::string strMsgFormat = "json";
				std::string strMethod = "login111";
				std::string strPwd = LoginInfo.szPWD;
				std::string strUser = LoginInfo.szUserNo;
				std::string strSha1Src = Poco::format("%sappKey%smessageFormat%smethod%spassword%susername%sv1.0%s", strAppValue, strAppKey, strMsgFormat, strMethod, strPwd, strUser, strAppValue);
				Poco::SHA1Engine engine;
				engine.update(strSha1Src);
				std::string strSHA1 = Poco::DigestEngine::digestToHex(engine.digest());
				std::string strSHA1_Up = Poco::toUpper(strSHA1);
				std::string strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&password=%s&sign=%s&username=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strPwd, strSHA1_Up, strUser);

				pTask = new SCAN_REQ_TASK;
				pTask->strUri		= SysSet.m_strBackUri + strUriValue;
				pTask->pUser		= pUser;
				pTask->strMsg		= "login2Ty";
				pTask->strUser		= LoginInfo.szUserNo;
				pTask->strPwd		= LoginInfo.szPWD;
				std::string strLog = Poco::format("������¼: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
				g_Log.LogOut(strLog);
			}
			else
			{
				pTask = new SCAN_REQ_TASK;
				pTask->strUri		= SysSet.m_strBackUri + "/login";
				pTask->pUser		= pUser;
				pTask->strMsg		= "login";
				pTask->strUser		= LoginInfo.szUserNo;
				pTask->strPwd		= LoginInfo.szPWD;
				char szTmp[200] = { 0 };
				sprintf(szTmp, "username=%s&password=%s", LoginInfo.szUserNo, LoginInfo.szPWD);
				pTask->strRequest = szTmp;
			}
		#else
			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri		= SysSet.m_strBackUri + "/login";
			pTask->pUser		= pUser;
			pTask->strMsg		= "login";
			pTask->strUser		= LoginInfo.szUserNo;
			pTask->strPwd		= LoginInfo.szPWD;
			char szTmp[200] = { 0 };
			sprintf(szTmp, "username=%s&password=%s", LoginInfo.szUserNo, LoginInfo.szPWD);
			pTask->strRequest = szTmp;
		#endif
			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
		}
		break;
	case USER_LOGIN_4TY:
		{
			ST_LOGIN_INFO4TY  LoginInfo = *(pStLoginInfo4TY)(pMission->m_pMissionData + HEAD_SIZE);
			std::cout << "login info: " << LoginInfo.szUserNo << ", pwd: " << LoginInfo.szPWD << ", platformcode: " << LoginInfo.szPlatformCode <<
				", encryption: " << LoginInfo.szEncryption << ", platformurl: " << LoginInfo.szPlatformUrl << std::endl;
			std::string strAppKey = "SP0000000TEST";
			std::string strAppValue = "63d4311d46714a39-a54cf2b0537a79b6TEST";
			std::string strMsgFormat = "json";
			std::string strMethod = "login111";
			std::string strPwd = LoginInfo.szPWD;
			std::string strUser = LoginInfo.szUserNo;
			std::string strEncryption = LoginInfo.szEncryption;
			std::string strPlatformCode = LoginInfo.szPlatformCode;
			std::string strPlatformUrl = LoginInfo.szPlatformUrl;
			std::string strSha1Src = Poco::format("%sappKey%sencryption%smessageFormat%smethod%spassword%splatformcode%splatformurl%susername%sv1.0%s", \
												  strAppValue, strAppKey, strEncryption, strMsgFormat, strMethod, strPwd, strPlatformCode, strPlatformUrl, strUser, strAppValue);
			Poco::SHA1Engine engine;
			engine.update(strSha1Src);
			std::string strSHA1 = Poco::DigestEngine::digestToHex(engine.digest());
			std::string strSHA1_Up = Poco::toUpper(strSHA1);
			std::string strUriValue = Poco::format("/router?appKey=%s&encryption=%s&messageFormat=%s&method=%s&password=%s&platformcode=%s&platformurl=%s&sign=%s&username=%s&v=1.0",\
												   strAppKey, strEncryption, strMsgFormat, strMethod, strPwd, strPlatformCode, strPlatformUrl, strSHA1_Up, strUser);

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri	= SysSet.m_strBackUri + strUriValue;
			pTask->pUser	= pUser;
			pTask->strMsg	= "login2Ty";
			pTask->strUser	= LoginInfo.szUserNo;
			pTask->strPwd	= LoginInfo.szPWD;
			std::string strLog = Poco::format("������¼ƽ̨: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
			g_Log.LogOut(strLog);

			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
		}
		break;
	case USER_LOGOUT:
		{
			char szData[1024] = { 0 };
			strncpy(szData, pMission->m_pMissionData + HEAD_SIZE, header.uPackSize);
			std::string strUser = szData;

			_mapUserLock_.lock();
			MAP_USER::iterator itFind = _mapUser_.find(strUser);
			if (itFind != _mapUser_.end())
			{
				_mapUser_.erase(itFind);

				std::string strLog = "�˺�" + strUser + "�˳���¼";

				pUser->SendResponesInfo(USER_RESPONSE_LOGOUT, RESULT_SUCCESS, (char*)strUser.c_str(), strUser.length());
				g_Log.LogOut(strLog);
			}
			_mapUserLock_.unlock();

		}
		break;
	case USER_GETEXAMINFO:
		{
			ST_EXAM_INFO stExamInfo = *(pStExamInfo)(pMission->m_pMissionData + HEAD_SIZE);
			std::string strEzs = stExamInfo.szEzs;
			std::cout << "��ȡ�����б�: " << pUser->m_Name <<std::endl;

			#ifdef TEST_MODE
			Poco::JSON::Object jsnSubject1;
			jsnSubject1.set("id", 200);
			jsnSubject1.set("name", CMyCodeConvert::Gb2312ToUtf8("����"));
			jsnSubject1.set("code", 1001);
			jsnSubject1.set("scanTemplateName", "100_200.mod");
			Poco::JSON::Object jsnSubject2;
			jsnSubject2.set("id", 201);
			jsnSubject2.set("name", CMyCodeConvert::Gb2312ToUtf8("��ѧ"));
			jsnSubject2.set("code", 1002);
			jsnSubject2.set("scanTemplateName", "100_201.mod");

			Poco::JSON::Array jsnSubjectArry;
			jsnSubjectArry.add(jsnSubject1);
			jsnSubjectArry.add(jsnSubject2);
			Poco::JSON::Object jsnExamObj;
			jsnExamObj.set("id", 100);
			jsnExamObj.set("name", CMyCodeConvert::Gb2312ToUtf8("������ĩ����"));
			Poco::JSON::Object jsnExamTypeObj;
			jsnExamTypeObj.set("id", 33);
			jsnExamTypeObj.set("name", CMyCodeConvert::Gb2312ToUtf8("��ĩ����"));
			jsnExamObj.set("examType", jsnExamTypeObj);
			Poco::JSON::Object jsnGrade;
			jsnGrade.set("id", 5);
			jsnGrade.set("name", CMyCodeConvert::Gb2312ToUtf8("����"));
			jsnGrade.set("aliasName", "");
			jsnExamObj.set("grade", jsnGrade);
			jsnExamObj.set("examSubjects", jsnSubjectArry);
			jsnExamObj.set("state", 1);
			Poco::JSON::Array jsnExamArry;
			jsnExamArry.add(jsnExamObj);

			Poco::JSON::Object jsnResult;
			jsnResult.set("exams", jsnExamArry);

			Poco::JSON::Object jsnStatus;
			jsnStatus.set("success", true);
			jsnResult.set("status", jsnStatus);

			std::stringstream jsnString;
			jsnResult.stringify(jsnString, 0);
			pUser->SendResponesInfo(USER_RESPONSE_EXAMINFO, RESULT_EXAMINFO_SUCCESS, (char*)jsnString.str().c_str(), jsnString.str().length());
			return 1;
			#endif

//		#ifdef TO_WHTY
		#if 1
			pSCAN_REQ_TASK pTask = NULL;
			if (SysSet.m_nServerMode == 1)
			{
				std::string strAppKey = "SP0000000TEST";
				std::string strAppValue = "63d4311d46714a39-a54cf2b0537a79b6TEST";
				std::string strMsgFormat = "json";
				std::string strMethod = "examinfo";

				std::string strPersonId;
				std::string strSchoolID;
				std::string strTmp = stExamInfo.szEzs;
				int nPos = strTmp.find("###");
				if (nPos != std::string::npos)
				{
					std::string strEzs = strTmp.substr(0, nPos);
					strSchoolID = strTmp.substr(nPos + 3);
					strPersonId = strEzs;
				}
				else
				{
					strPersonId = stExamInfo.szEzs;
				}
//				std::string strPersonId = stExamInfo.szEzs;
				std::string strSha1Src = Poco::format("%sappKey%smessageFormat%smethod%sperson_id%sv1.0%s", strAppValue, strAppKey, strMsgFormat, strMethod, strPersonId, strAppValue);
				Poco::SHA1Engine engine;
				engine.update(strSha1Src);
				std::string strSHA1 = Poco::DigestEngine::digestToHex(engine.digest());
				std::string strSHA1_Up = Poco::toUpper(strSHA1);
//				std::string strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&person_id=%s&sign=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strPersonId, strSHA1_Up);
				std::string strUriValue;
				if (nPos != std::string::npos)
					strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&person_id=%s&school_id=%s&sign=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strPersonId, strSchoolID, strSHA1_Up);	//2017.6.20���school_id
				else
					strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&person_id=%s&sign=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strPersonId, strSHA1_Up);

				pTask = new SCAN_REQ_TASK;
				pTask->strUri = SysSet.m_strBackUri + strUriValue;
				pTask->pUser = pUser;
				pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
				pTask->strMsg = "ezs";

				std::string strLog = Poco::format("����ƽ̨����ȡ�����б�: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
				g_Log.LogOut(strLog);
			}
			else
			{
				pTask = new SCAN_REQ_TASK;
				pTask->strUri = SysSet.m_strBackUri + "/examinfo";
				pTask->pUser  = pUser;
				pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
				pTask->strMsg = "ezs";
			}
			
		#else
			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = SysSet.m_strBackUri + "/examinfo";
			pTask->pUser  = pUser;
			pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
			pTask->strMsg = "ezs";
		#endif
			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
		}
		break;
	case USER_SETMODELINFO:
		{
			ST_MODELINFO stModelInfo = *(pST_MODELINFO)(pMission->m_pMissionData + HEAD_SIZE);
			bool bNeedSend = false;
			char szIndex[50] = { 0 };
			sprintf(szIndex, "%d_%d", stModelInfo.nExamID, stModelInfo.nSubjectID);

			std::cout << "���ÿ���ģ������: " << szIndex << std::endl;

			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())
			{
				bNeedSend = true;
				std::string strLog = "ģ����Ϣӳ�����δ�ҵ�";
				strLog.append(szIndex);
				strLog.append("����Ϣ�����Է��ʹ�ģ�����Ϣ");
				g_Log.LogOut(strLog);

				pST_MODELINFO pStModelInfo = new ST_MODELINFO;
				memcpy(pStModelInfo, &stModelInfo, sizeof(stModelInfo));

				pMODELINFO pModelInfo = new MODELINFO;
				pModelInfo->nExamID = stModelInfo.nExamID;
				pModelInfo->nSubjectID = stModelInfo.nSubjectID;
				pModelInfo->strName = stModelInfo.szModelName;
				pModelInfo->pUploadModelInfo = pStModelInfo;
				pModelInfo->pUser = pUser;

				_mapModelLock_.lock();
				_mapModel_.insert(MAP_MODEL::value_type(szIndex, pModelInfo));
				_mapModelLock_.unlock();

				strLog = "����µ�ģ����Ϣ���ȴ�����ģ���ļ�";
				strLog.append(stModelInfo.szModelName);
				g_Log.LogOut(strLog);
			}
			else
			{
				pMODELINFO pModelInfo = itFind->second;
				try
				{
					Poco::File modelFile(CMyCodeConvert::Gb2312ToUtf8(pModelInfo->strPath));
					if (!modelFile.exists())
						pModelInfo->strMd5 = "";
				}
				catch (Poco::Exception&exc)
				{
				}
				if (pModelInfo->strMd5 != stModelInfo.szMD5)		//�ļ����޸ģ���Ҫ���·���
				{
					bNeedSend = true;

					pST_MODELINFO pStModelInfo = new ST_MODELINFO;
					memcpy(pStModelInfo, &stModelInfo, sizeof(stModelInfo));

					SAFE_RELEASE(pModelInfo->pUploadModelInfo);
					pModelInfo->pUploadModelInfo = pStModelInfo;
					pModelInfo->pUser = pUser;


					std::string strLog = "ģ����Ϣӳ�����";
					strLog.append(szIndex);
					strLog.append("���ļ�MD5����Ҫ�ϴ����ļ�MD5��Ϣ��һ�£����Է��ʹ�ģ�����Ϣ");
					g_Log.LogOut(strLog);
				}
				else
					std::cout << "�ļ�δ�޸ģ�����Ҫ�ش�" << std::endl;
			}

			if (bNeedSend)
			{
				pUser->SendResponesInfo(USER_RESPONSE_MODELINFO, RESULT_SETMODELINFO_SEND, (char*)&stModelInfo, sizeof(stModelInfo));

				
				#ifdef TEST_MODE
				return 1;
				#endif

		#if 0				//��Ҫ�Ƚ�ѹ���ύͼƬMD5��ȥ���������Ϣ
				Poco::JSON::Object jsnModel;
				jsnModel.set("examId", stModelInfo.nExamID);
				jsnModel.set("subjectId", stModelInfo.nSubjectID);
				jsnModel.set("tmplateName", stModelInfo.szModelName);
				jsnModel.set("modelElectOmr", stModelInfo.szElectOmr);

				std::stringstream jsnString;
				jsnModel.stringify(jsnString, 0);

				std::string strEzs = stModelInfo.szEzs;
				pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
				pTask->strUri = SysSet.m_strBackUri + "/scanTemplate";
				pTask->pUser = pUser;
				pTask->strEzs = "ezs=" + strEzs;
				pTask->strMsg = "setScanModel";
				pTask->strRequest = jsnString.str();
				g_fmScanReq.lock();
				g_lScanReq.push_back(pTask);
				g_fmScanReq.unlock();
		#endif
			}
			else
				pUser->SendResult(USER_RESPONSE_MODELINFO, RESULT_SETMODELINFO_NO);
		}
		break;
	case USER_NEED_DOWN_MODEL:
		{
			ST_DOWN_MODEL stModelInfo = *(pST_DOWN_MODEL)(pMission->m_pMissionData + HEAD_SIZE);

			bool bNeedDown = true;
			char szIndex[50] = { 0 };
			sprintf(szIndex, "%d_%d", stModelInfo.nExamID, stModelInfo.nSubjectID);

			std::cout << "��������ģ������: " << szIndex << std::endl;

			pMODELINFO pModelInfo = NULL;
			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())
			{
				pUser->SendResult(USER_RESPONSE_NEEDDOWN, RESULT_DOWNMODEL_FAIL);
				break;
			}
			else
			{
				pModelInfo = itFind->second;
				if (pModelInfo->strMd5 == stModelInfo.szMD5)		//�ļ�δ�޸ģ�����Ҫ��������
					bNeedDown = false;
			}
			if (bNeedDown)
			{
				Poco::File fileModel(CMyCodeConvert::Gb2312ToUtf8(pModelInfo->strPath));
				stModelInfo.nModelSize = static_cast<int>(fileModel.getSize());
				pUser->SendResponesInfo(USER_RESPONSE_NEEDDOWN, RESULT_DOWNMODEL_OK, (char*)&stModelInfo, sizeof(stModelInfo));
			}
			else
				pUser->SendResult(USER_RESPONSE_NEEDDOWN, RESULT_DOWNMODEL_NONEED);
		}
		break;
	case USER_DOWN_MODEL:
		{
			ST_DOWN_MODEL stModelInfo = *(pST_DOWN_MODEL)(pMission->m_pMissionData + HEAD_SIZE);

			bool bNeedDown = true;
			char szIndex[50] = { 0 };
			sprintf(szIndex, "%d_%d", stModelInfo.nExamID, stModelInfo.nSubjectID);

			std::cout << "��ʼ���ؿ���ģ������: " << szIndex << std::endl;

			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())
			{
			}
			else
			{
				pMODELINFO pModelInfo = itFind->second;
				
				

 				std::string strFileData;

				std::ifstream fin(pModelInfo->strPath, std::ifstream::binary);
				if (!fin)
				{
					pUser->SendResult(USER_RESPONSE_DOWNMODEL, RESULT_ERROR_FILEIO);
					return false;
				}
				std::stringstream buffer;
				buffer << fin.rdbuf();
				strFileData = buffer.str();
				fin.close();

				int nLen = strFileData.length();
				std::cout << "ģ�峤��: " << nLen << std::endl;
				

// 				ofstream out("1.mod", std::ios::binary);
// 				std::stringstream buffer2;
// 				buffer2.write(strFileData.c_str(), strFileData.length());
// 				int n = buffer2.str().length();
// 				out << buffer2.str();
// 				out.close();

				pUser->SendResponesInfo(USER_RESPONSE_DOWNMODEL, RESULT_DOWNMODEL_RECV, (char*)strFileData.c_str(), strFileData.length());
			}
		}
		break;
	case USER_NEED_CREATE_MODEL:
		{
			ST_CREATE_MODEL stModelInfo = *(pST_CREATE_MODEL)(pMission->m_pMissionData + HEAD_SIZE);

			char szIndex[50] = { 0 };
			sprintf(szIndex, "%d_%d", stModelInfo.nExamID, stModelInfo.nSubjectID);

			std::cout << "�����Զ�����ģ������: " << szIndex << std::endl;
		#if 1
			_mapModelLock_.lock();
			pMODELINFO pModelInfo = NULL;
			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())		//��������û��ģ�壬�������ṩ��������ģ��
			{
				pModelInfo = new MODELINFO;
				pModelInfo->nExamID = stModelInfo.nExamID;
				pModelInfo->nSubjectID = stModelInfo.nSubjectID;

				_mapModel_.insert(MAP_MODEL::value_type(szIndex, pModelInfo));
			}
			_mapModelLock_.unlock();

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = Poco::format("%s/sheet/data/%d/%d", SysSet.m_strBackUri, stModelInfo.nExamID, stModelInfo.nSubjectID);
			pTask->pUser = pUser;
			pTask->strMsg = "createModel";
			pTask->nExamID = stModelInfo.nExamID;
			pTask->nSubjectID = stModelInfo.nSubjectID;
			pTask->strEzs = stModelInfo.szEzs;
			pTask->strExamName = stModelInfo.szExamName;
			pTask->strSubjectName = stModelInfo.szSubjectName;

			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
		#else
			_mapModelLock_.lock();
			pMODELINFO pModelInfo = NULL;
			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())		//��������û��ģ�壬�������ṩ��������ģ��
			{
				pModelInfo = new MODELINFO;
				pModelInfo->nExamID = stModelInfo.nExamID;
				pModelInfo->nSubjectID = stModelInfo.nSubjectID;

				_mapModel_.insert(MAP_MODEL::value_type(szIndex, pModelInfo));
				_mapModelLock_.unlock();

				pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
				pTask->strUri = Poco::format("%s/sheet/data/%d/%d", SysSet.m_strBackUri, stModelInfo.nExamID, stModelInfo.nSubjectID);
				pTask->pUser = pUser;
				pTask->strMsg = "createModel";
				pTask->nExamID = stModelInfo.nExamID;
				pTask->nSubjectID = stModelInfo.nSubjectID;
				pTask->strEzs = stModelInfo.szEzs;

				g_fmScanReq.lock();
				g_lScanReq.push_back(pTask);
				g_fmScanReq.unlock();
			}
			else	//�Ѿ����ڴ�ģ��
			{
				_mapModelLock_.unlock();
				pModelInfo = itFind->second;
				if (pModelInfo->strMd5.empty())
					pUser->SendResult(USER_RESPONSE_CREATE_MODEL, RESULT_CREATE_MODEL_DOING);
				else
					pUser->SendResult(USER_RESPONSE_CREATE_MODEL, RESULT_CREATE_MODEL_NONEED);
			}
		#endif
		}
		break;
	case GET_VERSERVER_ADDR:
		{
			std::stringstream ss;
			ss << SysSet.m_strVerServerIP << ":" << SysSet.m_nVerServerPort;
			std::string strVerAddr = ss.str();
			pUser->SendResponesInfo(RESPONSE_GET_VERSERVER_ADDR, RESULT_SUCCESS, (char*)strVerAddr.c_str(), strVerAddr.length());
			std::cout << "�ظ��汾��������ַ��Ϣ:" << strVerAddr << std::endl;
		}
		break;
	case USER_GET_BMK:
		{
			ST_GET_BMK_INFO stGetBmkInfo = *(pStGetBmkInfo)(pMission->m_pMissionData + HEAD_SIZE);

			std::cout << "������������: " << stGetBmkInfo.nExamID << "_" << stGetBmkInfo.nSubjectID << std::endl;

			std::string strEzs = stGetBmkInfo.szEzs;
			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = SysSet.m_strBackUri + "/getStudents";
			pTask->nExamID = stGetBmkInfo.nExamID;
			pTask->nSubjectID = stGetBmkInfo.nSubjectID;
// 			pTask->nExamID = 501;
// 			pTask->nSubjectID = 383;

			char szExamInfo[30] = { 0 };
			sprintf(szExamInfo, "/%d/%d", stGetBmkInfo.nExamID, stGetBmkInfo.nSubjectID);
			pTask->strUri.append(szExamInfo);

			pTask->pUser = pUser;
			pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
			pTask->strMsg = "getBmk";

			Poco::JSON::Object obj;
			obj.set("examId", stGetBmkInfo.nExamID);
			obj.set("subjectId", stGetBmkInfo.nSubjectID);
			stringstream ss;
			obj.stringify(ss, 0);
			pTask->strRequest = ss.str();

			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
		}
		break;
	case USER_GET_EXAM_BMK:
		{
			ST_GET_BMK_INFO stGetBmkInfo = *(pStGetBmkInfo)(pMission->m_pMissionData + HEAD_SIZE);

			std::cout << "�����Եı���������: " << stGetBmkInfo.nExamID << std::endl;

//			#ifdef _DEBUG	//�������ݣ�����Ҫɾ
			#if 0
			Poco::JSON::Object objBmkTestResult;
			Poco::JSON::Object objStatus;
			objStatus.set("success", true);

			Poco::JSON::Array arryStudent;
			for (int i = 0; i < 100; i++)
			{
				Poco::JSON::Object objStudent;
				std::string strZkzh = Poco::format("%d", 100 + i);
				objStudent.set("zkzh", strZkzh);
				std::string strName = Poco::format("����%d", i);
				objStudent.set("name", CMyCodeConvert::Gb2312ToUtf8(strName));
				objStudent.set("classRoom", CMyCodeConvert::Gb2312ToUtf8("һ��"));
				objStudent.set("school", CMyCodeConvert::Gb2312ToUtf8("һ��"));

				Poco::JSON::Array arryScanStatus;
				for (int j = 0; j < 4; j++)
				{
					Poco::JSON::Object objSubStatus;
					objSubStatus.set("subjectID", 590 + j);
					objSubStatus.set("scaned", (j + i) % 2);
					arryScanStatus.add(objSubStatus);
				}
				objStudent.set("scanStatus", arryScanStatus);
				arryStudent.add(objStudent);
			}

			Poco::JSON::Object objExam;
			objExam.set("examId", 402);
			objExam.set("subjectId", 590);
			objBmkTestResult.set("status", objStatus);
			objBmkTestResult.set("students", arryStudent);
			objBmkTestResult.set("examInfo", objExam); 
			std::stringstream jsnSnString;
			objBmkTestResult.stringify(jsnSnString, 0);

			std::string strSendData = jsnSnString.str();
			pUser->SendResponesInfo(USER_RESPONSE_GET_EXAM_BMK, RESULT_GET_BMK_SUCCESS, (char*)strSendData.c_str(), strSendData.length());
			break;
			#endif

			std::string strEzs = stGetBmkInfo.szEzs;
			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = SysSet.m_strBackUri + "/getStudents";
			pTask->nExamID = stGetBmkInfo.nExamID;
			pTask->nSubjectID = stGetBmkInfo.nSubjectID;

			char szExamInfo[30] = { 0 };
			sprintf(szExamInfo, "/%d/%d", stGetBmkInfo.nExamID, stGetBmkInfo.nSubjectID);
			pTask->strUri.append(szExamInfo);

			pTask->pUser = pUser;
			pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
			pTask->strMsg = "getExamBmk";

			Poco::JSON::Object obj;
			obj.set("examId", stGetBmkInfo.nExamID);
			obj.set("subjectId", stGetBmkInfo.nSubjectID);
			stringstream ss;
			obj.stringify(ss, 0);
			pTask->strRequest = ss.str();

			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
		}
		break;
	case USER_GET_FILE_UPLOAD_ADDR:
		{
			std::cout << "����ͬ��ʽ�ļ���������ַ��Ϣ����: "<< std::endl;
			int nResult = RESULT_GET_FILE_ADDR_SUCCESS;
			if (SysSet.m_strFileAddrs.empty())
				nResult = RESULT_GET_FILE_ADDR_FAIL;
			std::string strVerAddr = SysSet.m_strFileAddrs;
			pUser->SendResponesInfo(USER_RESPONSE_GET_FILE_UPLOAD_ADDR, nResult, (char*)strVerAddr.c_str(), strVerAddr.length());
		}
		break;
	case USER_NEED_UP_MODEL_PIC:
		{
			std::cout << "�����ϴ�ģ��ͼ������: " << std::endl;
			ST_MODELPIC stModelPic = *(pST_MODELPIC)(pMission->m_pMissionData + HEAD_SIZE);
			int nResult = RESULT_ERROR_UNKNOWN;

			std::string strModelPicPath = Poco::format("%s\\%d\\%d_%d_%d%s", CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strModelSavePath), stModelPic.nExamID, \
													   stModelPic.nExamID, stModelPic.nSubjectID, stModelPic.nIndex, CMyCodeConvert::Gb2312ToUtf8(stModelPic.szExtName));

// 			std::string strModelPicPath = Poco::format("%s\\%d\\%d_%d_%d_#_%s", CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strModelSavePath), stModelPic.nExamID,\
// 												   stModelPic.nExamID, stModelPic.nSubjectID, stModelPic.nIndex, CMyCodeConvert::Gb2312ToUtf8(stModelPic.szPicName));
			try
			{
				Poco::File modelPic(strModelPicPath);
				if (!modelPic.exists())
				{
					nResult = RESULT_UP_MODEL_PIC_SEND;
					std::cout << "���Է���ģ��ͼƬ��" << strModelPicPath << std::endl;
				}
				else
				{
					std::string strMd5 = calcFileMd5(strModelPicPath);
					if (strMd5 == stModelPic.szMD5)
					{
						nResult = RESULT_UP_MODEL_PIC_NONEED;
						std::cout << "����Ҫ����ģ��ͼƬ��" << strModelPicPath << std::endl;
					}
					else
					{
						nResult = RESULT_UP_MODEL_PIC_SEND;
						std::cout << "���Է���ģ��ͼƬ��" << strModelPicPath << std::endl;
					}
				}
			}
			catch (Poco::Exception& e)
			{
				std::cout << "���ģ��ͼƬ·���쳣: "<< e.displayText() << std::endl;
				nResult = RESULT_UP_MODEL_PIC_SEND;
			}
			
			pUser->SendResponesInfo(USER_RESPONSE_NEED_UP_MODEL_PIC, nResult, (char*)&stModelPic, sizeof(stModelPic));
		}
		break;
	case USER_GET_MODEL_PIC:
		{
			char szData[1024] = { 0 };
			strncpy(szData, pMission->m_pMissionData + HEAD_SIZE, header.uPackSize);
			std::string strModelPicInfo = szData;
			std::cout << "��������ģ��ͼ������: " << strModelPicInfo << std::endl;

			int nPos = strModelPicInfo.find("_");
			std::string strExamID = strModelPicInfo.substr(0, nPos);
			std::string strSubjectID = strModelPicInfo.substr(nPos + 1);

			std::string strModelPicDir = SysSet.m_strModelSavePath + "\\" + strExamID + "\\";

			std::vector<std::string> vecModelPicName;
			std::vector<std::string> vecModelPicPath;
			std::string strPaperPath = CMyCodeConvert::Gb2312ToUtf8(strModelPicDir);

			try
			{
				Poco::File modlePicDir(strPaperPath);
				if (!modlePicDir.exists())
				{
					pUser->SendResult(USER_RESPONSE_GET_MODEL_PIC, RESULT_GET_MODEL_PIC_NOPIC);
					return false;
				}
			}
			catch (Poco::Exception& e)
			{
			}

			Poco::DirectoryIterator it(strPaperPath);
			Poco::DirectoryIterator end;
			while (it != end)
			{
				Poco::Path p(it->path());
				if (it->isFile())
				{
					std::string strName = p.getFileName();
					if (strName.find(strModelPicInfo) != std::string::npos)
					{
						std::string strPicPath = p.toString();
						vecModelPicName.push_back(strName);
						vecModelPicPath.push_back(strPicPath);
					}
				}
				it++;
			}
			if (vecModelPicPath.size() == 0)
			{
				pUser->SendResult(USER_RESPONSE_GET_MODEL_PIC, RESULT_GET_MODEL_PIC_NOPIC);
				return false;
			}

			std::string strSendData;
			for (int i = 0; i < vecModelPicPath.size(); i++)
			{
				std::string strFileData;
				std::ifstream fin(CMyCodeConvert::Utf8ToGb2312(vecModelPicPath[i]), std::ifstream::binary);
				if (!fin)
				{
					pUser->SendResult(USER_RESPONSE_GET_MODEL_PIC, RESULT_ERROR_FILEIO);
					return false;
				}
				std::stringstream buffer;
				buffer << fin.rdbuf();
				strFileData = buffer.str();
				fin.close();

				strSendData.append("#_#_#_#_");
				strSendData.append(vecModelPicName[i]);
				strSendData.append("_*_");
				char szLen[20] = { 0 };
				sprintf_s(szLen, "%d", strFileData.length());
				strSendData.append(szLen);
				strSendData.append("_#####_");
				strSendData.append(strFileData);
			}

			pUser->SendResponesInfo(USER_RESPONSE_GET_MODEL_PIC, RESULT_GET_MODEL_PIC_SUCCESS, (char*)strSendData.c_str(), strSendData.length());
		}
		break;
	default:
		bFind = FALSE;
		break;
	}
	if (bFind)
	{
		return 1;
	}
	return 0;
}

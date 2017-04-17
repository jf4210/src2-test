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

	BOOL bFind = TRUE; //该命令是否被处理
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
// 				//重复登录提醒，
// 				std::string strSendData = "账号已登录";
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

		#ifdef TO_WHTY
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

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri		= SysSet.m_strBackUri + strUriValue;
			pTask->pUser		= pUser;
			pTask->strMsg		= "login2Ty";
			pTask->strUser		= LoginInfo.szUserNo;
			pTask->strPwd		= LoginInfo.szPWD;
			std::string strLog = Poco::format("天喻登录: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
			g_Log.LogOut(strLog);
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
			std::string strLog = Poco::format("天喻登录平台: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
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

				std::string strLog = "账号" + strUser + "退出登录";

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
			std::cout << "get exam info: " << pUser->m_Name <<std::endl;

			#ifdef TEST_MODE
			Poco::JSON::Object jsnSubject1;
			jsnSubject1.set("id", 200);
			jsnSubject1.set("name", CMyCodeConvert::Gb2312ToUtf8("语文"));
			jsnSubject1.set("code", 1001);
			jsnSubject1.set("scanTemplateName", "100_200.mod");
			Poco::JSON::Object jsnSubject2;
			jsnSubject2.set("id", 201);
			jsnSubject2.set("name", CMyCodeConvert::Gb2312ToUtf8("数学"));
			jsnSubject2.set("code", 1002);
			jsnSubject2.set("scanTemplateName", "100_201.mod");

			Poco::JSON::Array jsnSubjectArry;
			jsnSubjectArry.add(jsnSubject1);
			jsnSubjectArry.add(jsnSubject2);
			Poco::JSON::Object jsnExamObj;
			jsnExamObj.set("id", 100);
			jsnExamObj.set("name", CMyCodeConvert::Gb2312ToUtf8("高三期末考试"));
			Poco::JSON::Object jsnExamTypeObj;
			jsnExamTypeObj.set("id", 33);
			jsnExamTypeObj.set("name", CMyCodeConvert::Gb2312ToUtf8("期末考试"));
			jsnExamObj.set("examType", jsnExamTypeObj);
			Poco::JSON::Object jsnGrade;
			jsnGrade.set("id", 5);
			jsnGrade.set("name", CMyCodeConvert::Gb2312ToUtf8("高三"));
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

		#ifdef TO_WHTY
			std::string strAppKey = "SP0000000TEST";
			std::string strAppValue = "63d4311d46714a39-a54cf2b0537a79b6TEST";
			std::string strMsgFormat = "json";
			std::string strMethod = "examinfo";
			std::string strPersonId = stExamInfo.szEzs;
			std::string strSha1Src = Poco::format("%sappKey%smessageFormat%smethod%sperson_id%sv1.0%s", strAppValue, strAppKey, strMsgFormat, strMethod, strPersonId, strAppValue);
			Poco::SHA1Engine engine;
			engine.update(strSha1Src);
			std::string strSHA1 = Poco::DigestEngine::digestToHex(engine.digest());
			std::string strSHA1_Up = Poco::toUpper(strSHA1);
			std::string strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&person_id=%s&sign=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strPersonId, strSHA1_Up);
			
			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = SysSet.m_strBackUri + strUriValue;
			pTask->pUser  = pUser;
			pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
			pTask->strMsg = "ezs";

			std::string strLog = Poco::format("天喻平台，获取考试列表: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
			g_Log.LogOut(strLog);
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
			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())
			{
				bNeedSend = true;
				std::string strLog = "模板信息映射表中未找到";
				strLog.append(szIndex);
				strLog.append("的信息，可以发送此模板的信息");
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

				strLog = "添加新的模板信息，等待接收模板文件";
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
				if (pModelInfo->strMd5 != stModelInfo.szMD5)		//文件有修改，需要重新发送
				{
					bNeedSend = true;

					pST_MODELINFO pStModelInfo = new ST_MODELINFO;
					memcpy(pStModelInfo, &stModelInfo, sizeof(stModelInfo));

					SAFE_RELEASE(pModelInfo->pUploadModelInfo);
					pModelInfo->pUploadModelInfo = pStModelInfo;
					pModelInfo->pUser = pUser;


					std::string strLog = "模板信息映射表中";
					strLog.append(szIndex);
					strLog.append("的文件MD5与需要上传的文件MD5信息不一致，可以发送此模板的信息");
					g_Log.LogOut(strLog);
				}
				else
					std::cout << "文件未修改，不需要重传" << std::endl;
			}

			if (bNeedSend)
			{
				pUser->SendResponesInfo(USER_RESPONSE_MODELINFO, RESULT_SETMODELINFO_SEND, (char*)&stModelInfo, sizeof(stModelInfo));

				
				#ifdef TEST_MODE
				return 1;
				#endif

		#if 0				//需要先解压后提交图片MD5再去后端设置信息
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
				if (pModelInfo->strMd5 == stModelInfo.szMD5)		//文件未修改，不需要重新下载
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
				std::cout << "模板长度: " << nLen << std::endl;
				

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

		#if 1
			_mapModelLock_.lock();
			pMODELINFO pModelInfo = NULL;
			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())		//服务器上没有模板，请求后端提供数据生成模板
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
			if (itFind == _mapModel_.end())		//服务器上没有模板，请求后端提供数据生成模板
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
			else	//已经存在此模板
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
			std::cout << "回复版本服务器地址信息:" << strVerAddr << std::endl;
		}
		break;
	case USER_GET_BMK:
		{
			ST_GET_BMK_INFO stGetBmkInfo = *(pStGetBmkInfo)(pMission->m_pMissionData + HEAD_SIZE);
			std::string strEzs = stGetBmkInfo.szEzs;
			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = SysSet.m_strBackUri + "/getStudents";
// 			pTask->nExamID = stGetBmkInfo.nExamID;
// 			pTask->nSubjectID = stGetBmkInfo.nSubjectID;
			pTask->nExamID = 501;
			pTask->nSubjectID = 383;
			pTask->pUser = pUser;
			pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
			pTask->strMsg = "getBmk";

			Poco::JSON::Object obj;
// 			obj.set("examId", stGetBmkInfo.nExamID);
// 			obj.set("subjectId", stGetBmkInfo.nSubjectID);
			obj.set("examId", 501);
			obj.set("subjectId", 383);
			stringstream ss;
			obj.stringify(ss, 0);
			pTask->strRequest = ss.str();

			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
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

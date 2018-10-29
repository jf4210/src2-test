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
				std::string strLog = Poco::format("天喻登录: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
				g_Log.LogOut(strLog);
			}
			else
			{
				pTask = new SCAN_REQ_TASK;
				pTask->pUser = pUser;
				pTask->strUser = LoginInfo.szUserNo;
				pTask->strPwd = LoginInfo.szPWD;

				switch (SysSet.m_nUseThirdPlatform)
				{
					case 3:		//成都佳发，用的天喻系统，登录方法改变
					case 1:		//武汉天喻
						{
							std::string strAppKey = "SP0000000TEST";
							std::string strAppValue = "63d4311d46714a39-a54cf2b0537a79b6TEST";
							std::string strMsgFormat = "json";
							std::string strMethod = "login111";
							if (SysSet.m_nUseThirdPlatform == 3)
								strMethod = "loginjf";
							std::string strPwd = LoginInfo.szPWD;
							std::string strUser = LoginInfo.szUserNo;
							std::string strSha1Src = Poco::format("%sappKey%smessageFormat%smethod%spassword%susername%sv1.0%s", strAppValue, strAppKey, strMsgFormat, strMethod, strPwd, strUser, strAppValue);
							Poco::SHA1Engine engine;
							engine.update(strSha1Src);
							std::string strSHA1 = Poco::DigestEngine::digestToHex(engine.digest());
							std::string strSHA1_Up = Poco::toUpper(strSHA1);
							std::string strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&password=%s&sign=%s&username=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strPwd, strSHA1_Up, strUser);

							pTask->strUri = SysSet.m_str3PlatformUrl + strUriValue;
							pTask->strMsg = "login2Ty";
						}
						break;
					case 2:		//山西寰烁
						{
							pTask->strUri = SysSet.m_str3PlatformUrl + "?username=" + pTask->strUser + "&password=" + pTask->strPwd;
							pTask->strMsg = "login_SanXi_huanshuo";
						}
						break;
					default:	//默认易考乐学后端平台
						{
							pTask->strUri = SysSet.m_strBackUri + "/login";
							pTask->strMsg = "login";
							char szTmp[200] = { 0 };
							sprintf(szTmp, "username=%s&password=%s", LoginInfo.szUserNo, LoginInfo.szPWD);
							pTask->strRequest = szTmp;
						}
				}

				std::string strLog = Poco::format("登录命令: msg = %s(%s:%s)\nurl = %s", pTask->strMsg, pTask->strUser, pTask->strPwd, pTask->strUri);
				g_Log.LogOut(strLog);
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
				", encryption: " << LoginInfo.szEncryption << ", platformUrl: " << LoginInfo.szPlatformUrl << std::endl;
			std::string strAppKey = "SP0000000TEST";
			std::string strAppValue = "63d4311d46714a39-a54cf2b0537a79b6TEST";
			std::string strMsgFormat = "json";
			std::string strMethod = "login111";
			std::string strPwd = LoginInfo.szPWD;
			std::string strUser = LoginInfo.szUserNo;
			std::string strEncryption = LoginInfo.szEncryption;
			std::string strPlatformCode = LoginInfo.szPlatformCode;
			std::string strPlatformUrl = LoginInfo.szPlatformUrl;
			std::string strSha1Src = Poco::format("%sappKey%sencryption%smessageFormat%smethod%spassword%splatformUrl%splatformcode%susername%sv1.0%s", \
												  strAppValue, strAppKey, strEncryption, strMsgFormat, strMethod, strPwd, strPlatformUrl, strPlatformCode, strUser, strAppValue);
			Poco::SHA1Engine engine;
			engine.update(strSha1Src);
			std::string strSHA1 = Poco::DigestEngine::digestToHex(engine.digest());
			std::string strSHA1_Up = Poco::toUpper(strSHA1);
			std::string strUriValue = Poco::format("/router?appKey=%s&encryption=%s&messageFormat=%s&method=%s&password=%s&platformUrl=%s&platformcode=%s&sign=%s&username=%s&v=1.0",\
												   strAppKey, strEncryption, strMsgFormat, strMethod, strPwd, strPlatformUrl, strPlatformCode, strSHA1_Up, strUser);

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			if (SysSet.m_nServerMode == 0)	//非天喻服务器模式，则进入此处为登录第3方天喻平台
				pTask->strUri = SysSet.m_str3PlatformUrl + strUriValue;
			else
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
			int	nUserID = stExamInfo.nUserId;
			std::cout << "获取考试列表: " << pUser->m_Name <<std::endl;

			#ifdef TEST_MODE
			#if 1
			std::string strExamListPath = SysSet.m_strCurrentDir + "\\ServerTestData\\ServerTestData-examlist.txt";
			std::string strJsnData;
			std::ifstream in(strExamListPath);

			if (!in) return false;

			std::string strJsnLine;
			while (!in.eof())
			{
				getline(in, strJsnLine);
				strJsnData.append(strJsnLine);
			}
			in.close();
			
			pUser->SendResponesInfo(USER_RESPONSE_EXAMINFO, RESULT_EXAMINFO_SUCCESS, (char*)strJsnData.c_str(), strJsnData.length());
			return 1;
			#else
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
					strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&person_id=%s&school_id=%s&sign=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strPersonId, strSchoolID, strSHA1_Up);	//2017.6.20添加school_id
				else
					strUriValue = Poco::format("/router?appKey=%s&messageFormat=%s&method=%s&person_id=%s&sign=%s&v=1.0", strAppKey, strMsgFormat, strMethod, strPersonId, strSHA1_Up);

				pTask = new SCAN_REQ_TASK;
				pTask->strUri = SysSet.m_strBackUri + strUriValue;
				pTask->pUser = pUser;
				pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
				pTask->strMsg = "ezs";

				std::string strLog = Poco::format("天喻平台，获取考试列表: src_SHA1 = %s\nSHA1 = %s\nuri = %s", strSha1Src, strSHA1_Up, pTask->strUri);
				g_Log.LogOut(strLog);
			}
			else
			{
				pTask = new SCAN_REQ_TASK;
				std::string strUserID = Poco::format("%d", nUserID);
				if (SysSet.m_nQuYuVersion)
					pTask->strUri = SysSet.m_strBackUri + "/examinfo";
				else
					pTask->strUri = SysSet.m_strBackUri + "/examinfo/" + strUserID;
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

			std::cout << "设置考试模板命令: " << szIndex << std::endl;
			std::stringstream ssLog;
			ssLog << "设置考试模板命令: " << szIndex << "\n";

			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())
			{
				bNeedSend = true;
				ssLog << "模板信息映射表中未找到" << szIndex << "的信息，可以发送此模板的信息[Ezs = " << stModelInfo.szEzs << "]\n";
				//g_Log.LogOut(strLog);

				pST_MODELINFO pStModelInfo = new ST_MODELINFO;
				memcpy(pStModelInfo, &stModelInfo, sizeof(stModelInfo));

				pMODELINFO pModelInfo = new MODELINFO;
				pModelInfo->nExamID = stModelInfo.nExamID;
				pModelInfo->nSubjectID = stModelInfo.nSubjectID;
				pModelInfo->strName = stModelInfo.szModelName;
				pModelInfo->strEzs = stModelInfo.szEzs;
				pModelInfo->strElectOmr = stModelInfo.szElectOmr;
				pModelInfo->strMd5 = stModelInfo.szMD5;
				pModelInfo->pUploadModelInfo = pStModelInfo;
				pModelInfo->pUser = pUser;

				_mapModelLock_.lock();
				_mapModel_.insert(MAP_MODEL::value_type(szIndex, pModelInfo));
				_mapModelLock_.unlock();

				ssLog << "添加新的模板信息，等待接收模板文件: " << stModelInfo.szModelName << "\n";
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

					pModelInfo->nExamID = stModelInfo.nExamID;
					pModelInfo->nSubjectID = stModelInfo.nSubjectID;
					pModelInfo->strName = stModelInfo.szModelName;
					pModelInfo->strEzs = stModelInfo.szEzs;
					pModelInfo->strElectOmr = stModelInfo.szElectOmr;
					pModelInfo->strMd5 = stModelInfo.szMD5;

					ssLog << "模板信息映射表中" << szIndex << "的文件MD5与需要上传的文件MD5信息不一致，可以发送此模板的信息[Ezs = " << pModelInfo->strEzs <<"]";
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

			g_Log.LogOut(ssLog.str());
		}
		break;
	case USER_NEED_DOWN_MODEL:
		{
			ST_DOWN_MODEL stModelInfo = *(pST_DOWN_MODEL)(pMission->m_pMissionData + HEAD_SIZE);

			bool bNeedDown = true;
			char szIndex[50] = { 0 };
			sprintf(szIndex, "%d_%d", stModelInfo.nExamID, stModelInfo.nSubjectID);

			std::cout << "请求下载模板命令: " << szIndex << std::endl;
			std::stringstream ssLog;
			ssLog << "请求下载模板命令: " << szIndex << "\n";

			pMODELINFO pModelInfo = NULL;
			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())
			{
				pUser->SendResult(USER_RESPONSE_NEEDDOWN, RESULT_DOWNMODEL_FAIL);
				ssLog << "请求下载的模板文件不存在\n";
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
				try
				{
					Poco::File fileModel(CMyCodeConvert::Gb2312ToUtf8(pModelInfo->strPath));
					stModelInfo.nModelSize = static_cast<int>(fileModel.getSize());
					pUser->SendResponesInfo(USER_RESPONSE_NEEDDOWN, RESULT_DOWNMODEL_OK, (char*)&stModelInfo, sizeof(stModelInfo));
					ssLog << "可以正常下载";
				}
				catch (Poco::Exception& exc)
				{
					std::string strLog = "请求下载模板命令(" + std::string(szIndex) + ")-->检测模板文件路径异常: " + exc.displayText();
					std::cout << strLog << std::endl;
					ssLog << strLog;
					pUser->SendResult(USER_RESPONSE_NEEDDOWN, RESULT_DOWNMODEL_FAIL);
				}
			}
			else
			{
				pUser->SendResult(USER_RESPONSE_NEEDDOWN, RESULT_DOWNMODEL_NONEED);
				ssLog << "不需要重新下载模板\n";
			}
			g_Log.LogOut(ssLog.str());
		}
		break;
	case USER_DOWN_MODEL:
		{
			ST_DOWN_MODEL stModelInfo = *(pST_DOWN_MODEL)(pMission->m_pMissionData + HEAD_SIZE);

			std::string strCmdResponse = Poco::format("Get Cmd %d OK", (int)USER_DOWN_MODEL);
			//pUser->SendResponesInfo(USER_RESPONSE_DOWNMODEL, RESULT_SUCCESS, (char*)strCmdResponse.c_str(), strCmdResponse.length());

			bool bNeedDown = true;
			char szIndex[50] = { 0 };
			sprintf(szIndex, "%d_%d", stModelInfo.nExamID, stModelInfo.nSubjectID);

			std::cout << "开始下载考试模板命令: " << szIndex << std::endl;
			std::stringstream ssLog;
			ssLog << "开始下载考试模板命令: " << szIndex << "\n";

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
				ssLog << "模板长度: " << nLen << "\n";
// 				ofstream out("1.mod", std::ios::binary);
// 				std::stringstream buffer2;
// 				buffer2.write(strFileData.c_str(), strFileData.length());
// 				int n = buffer2.str().length();
// 				out << buffer2.str();
// 				out.close();

				pUser->SendResponesInfo(USER_RESPONSE_DOWNMODEL, RESULT_DOWNMODEL_RECV, (char*)strFileData.c_str(), strFileData.length());
				std::cout << "下载考试模板文件完成: " << szIndex << std::endl;
				ssLog << "下载考试模板文件完成: " << szIndex;
				g_Log.LogOut(ssLog.str());
			}
		}
		break;
	case USER_NEED_CREATE_MODEL:
		{
			ST_CREATE_MODEL stModelInfo = *(pST_CREATE_MODEL)(pMission->m_pMissionData + HEAD_SIZE);

			char szIndex[50] = { 0 };
			sprintf(szIndex, "%d_%d", stModelInfo.nExamID, stModelInfo.nSubjectID);

			std::cout << "请求自动创建模板命令: " << szIndex << std::endl;

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

			std::cout << "请求报名库命令: " << stGetBmkInfo.nExamID << "_" << stGetBmkInfo.nSubjectID << std::endl;
			std::string strLog = Poco::format("请求考试的报名库命令: %d_%d", stGetBmkInfo.nExamID, stGetBmkInfo.nSubjectID);
			g_Log.LogOut(strLog);

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

			std::string strCmdResponse = Poco::format("Get Cmd %d OK", (int)USER_GET_EXAM_BMK);
			//pUser->SendResponesInfo(USER_RESPONSE_GET_EXAM_BMK, RESULT_SUCCESS, (char*)strCmdResponse.c_str(), strCmdResponse.length());

			std::cout << "请求考试的报名库命令: " << stGetBmkInfo.nExamID << std::endl;
			std::string strLog = Poco::format("请求考试的报名库命令: %d", stGetBmkInfo.nExamID);
			g_Log.LogOut(strLog);

//			#ifdef _DEBUG	//测试数据，后期要删
			#ifdef TEST_MODE
			std::string strSendData;
			std::string strBmkFileName;
			if (stGetBmkInfo.nExamID == 1354 || stGetBmkInfo.nExamID == 1257 || stGetBmkInfo.nExamID == 1241)
			{
				strBmkFileName = Poco::format("ServerTestData-bmk-%d.txt", stGetBmkInfo.nExamID);
				std::string strExamListPath = SysSet.m_strCurrentDir + "\\ServerTestData\\" + strBmkFileName;
				std::string strJsnData;
				std::ifstream in(strExamListPath);

				if (!in) return false;

				std::string strJsnLine;
				while (!in.eof())
				{
					getline(in, strJsnLine);
					strJsnData.append(strJsnLine);
				}
				in.close();

				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result;
				try
				{
					result = parser.parse(strJsnData);
					Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
					//++添加考试ID和科目ID到结果信息中
					Poco::JSON::Object objExam;
					objExam.set("examId", stGetBmkInfo.nExamID);
					objExam.set("subjectId", stGetBmkInfo.nSubjectID);
					object->set("examInfo", objExam);
					std::stringstream jsnSnString;
					object->stringify(jsnSnString, 0);
					//--
					strSendData = jsnSnString.str();
				}
				catch (Poco::Exception& exc)
				{
					std::cout << "获取考试[" << stGetBmkInfo.nExamID << "]报名库数据异常\n";
					return false;
				}
			}
			else
			{
				Poco::JSON::Object objBmkTestResult;
				Poco::JSON::Object objStatus;
				objStatus.set("success", true);

				Poco::JSON::Array arryStudent;
				for (int i = 0; i < 100; i++)
				{
					Poco::JSON::Object objStudent;
					std::string strZkzh = Poco::format("%d", 100 + i);
					objStudent.set("zkzh", strZkzh);
					std::string strName = Poco::format("测试%d", i);
					objStudent.set("name", CMyCodeConvert::Gb2312ToUtf8(strName));
					objStudent.set("classRoom", CMyCodeConvert::Gb2312ToUtf8("10001"));
					objStudent.set("school", CMyCodeConvert::Gb2312ToUtf8("一中"));

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

				strSendData = jsnSnString.str();
			}
			pUser->SendResponesInfo(USER_RESPONSE_GET_EXAM_BMK, RESULT_GET_BMK_SUCCESS, (char*)strSendData.c_str(), strSendData.length());
			break;
			#endif

			std::string strEzs = stGetBmkInfo.szEzs;
			
			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = SysSet.m_strBackUri + "/getStudents";
			pTask->nExamID = stGetBmkInfo.nExamID;
			pTask->nSubjectID = stGetBmkInfo.nSubjectID;
			pTask->nTeacherID = _mapSession_[strEzs].nTeacherID;

			char szExamInfo[30] = { 0 };
			sprintf(szExamInfo, "/%d/%d", stGetBmkInfo.nExamID, stGetBmkInfo.nSubjectID);
			pTask->strUri.append(szExamInfo);

			pTask->pUser = pUser;
			pTask->strEzs = SysSet.m_strSessionName + strEzs;		//"ezs=" + strEzs;
			pTask->strMsg = "getExamBmk";

			Poco::JSON::Object obj;
			obj.set("examId", stGetBmkInfo.nExamID);
			obj.set("subjectId", stGetBmkInfo.nSubjectID);
			obj.set("teacherId", pTask->nTeacherID);
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
			std::cout << "请求不同格式文件服务器地址信息命令: "<< std::endl;
			int nResult = RESULT_GET_FILE_ADDR_SUCCESS;
			if (SysSet.m_strFileAddrs.empty())
				nResult = RESULT_GET_FILE_ADDR_FAIL;
			std::string strVerAddr = SysSet.m_strFileAddrs;
			pUser->SendResponesInfo(USER_RESPONSE_GET_FILE_UPLOAD_ADDR, nResult, (char*)strVerAddr.c_str(), strVerAddr.length());

			std::string strLog = "请求不同格式文件服务器地址信息命令: \n" + strVerAddr;
			g_Log.LogOut(strLog);
		}
		break;
	case USER_NEED_UP_MODEL_PIC:
		{
			std::cout << "请求上传模板图像命令: " << std::endl;
			ST_MODELPIC stModelPic = *(pST_MODELPIC)(pMission->m_pMissionData + HEAD_SIZE);
			int nResult = RESULT_ERROR_UNKNOWN;

			std::string strModelPicPath = Poco::format("%s\\%d\\%d_%d_%d%s", CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strModelSavePath), stModelPic.nExamID, \
													   stModelPic.nExamID, stModelPic.nSubjectID, stModelPic.nIndex, CMyCodeConvert::Gb2312ToUtf8(stModelPic.szExtName));

// 			std::string strModelPicPath = Poco::format("%s\\%d\\%d_%d_%d_#_%s", CMyCodeConvert::Gb2312ToUtf8(SysSet.m_strModelSavePath), stModelPic.nExamID,\
// 												   stModelPic.nExamID, stModelPic.nSubjectID, stModelPic.nIndex, CMyCodeConvert::Gb2312ToUtf8(stModelPic.szPicName));
			std::string strLog;
			try
			{
				Poco::File modelPic(strModelPicPath);
				if (!modelPic.exists())
				{
					nResult = RESULT_UP_MODEL_PIC_SEND;
					strLog = "请求上传模板图像命令 ==> 可以发送模板图片：" + strModelPicPath;
					std::cout << "可以发送模板图片：" << strModelPicPath << std::endl;
				}
				else
				{
					std::string strMd5 = calcFileMd5(strModelPicPath);
					if (strMd5 == stModelPic.szMD5)
					{
						nResult = RESULT_UP_MODEL_PIC_NONEED;
						strLog = "请求上传模板图像命令 ==> 不需要发送模板图片：" + strModelPicPath;
						std::cout << "不需要发送模板图片：" << strModelPicPath << std::endl;
					}
					else
					{
						nResult = RESULT_UP_MODEL_PIC_SEND;
						strLog = "请求上传模板图像命令 ==> 可以发送模板图片：" + strModelPicPath;
						std::cout << "可以发送模板图片：" << strModelPicPath << std::endl;
					}
				}
			}
			catch (Poco::Exception& e)
			{
				strLog = "请求上传模板图像命令 ==> 检测模板图片路径异常：" + e.displayText();
				std::cout << "检测模板图片路径异常: "<< e.displayText() << std::endl;
				nResult = RESULT_UP_MODEL_PIC_SEND;
			}
			
			pUser->SendResponesInfo(USER_RESPONSE_NEED_UP_MODEL_PIC, nResult, (char*)&stModelPic, sizeof(stModelPic));
			g_Log.LogOut(strLog);
		}
		break;
	case USER_GET_MODEL_PIC:
		{
			char szData[1024] = { 0 };
			strncpy(szData, pMission->m_pMissionData + HEAD_SIZE, header.uPackSize);
			std::string strModelPicInfo = szData;
			std::cout << "请求下载模板图像命令: " << strModelPicInfo << std::endl;
			std::stringstream ssLog;
			ssLog<< "请求下载模板图像命令: " << strModelPicInfo << std::endl;

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
					ssLog << "模板图像不存在";
					g_Log.LogOut(ssLog.str());
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
				ssLog << "模板图像不存在";
				g_Log.LogOut(ssLog.str());
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
					ssLog << "读取模板文件失败";
					g_Log.LogOut(ssLog.str());
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
			ssLog << "模板文件发送完成";
			g_Log.LogOut(ssLog.str());
		}
		break;
	case USER_CHK_NEW_GUARDEXE:
		{
			char szData[1024] = { 0 };
			strncpy(szData, pMission->m_pMissionData + HEAD_SIZE, header.uPackSize);
			std::string strClientGuardExeMd5 = szData;
			if (_strNewGuardExeMd5_.empty())
			{
				std::cout << "服务器上新守护进程不存在" << std::endl;
				pUser->SendResult(USER_RESPONSE_CHK_NEW_GUARDEXE, RESULT_GET_NEW_GUARDEXE_NOFILE);
			}
			else if (_strNewGuardExeMd5_ == strClientGuardExeMd5)
			{
				std::cout << "客户端上的守护进程和服务器一致，不需要下载" << std::endl;
				pUser->SendResult(USER_RESPONSE_CHK_NEW_GUARDEXE, RESULT_GET_NEW_GUARDEXE_NONEED);
			}
			else
			{
				std::string strGuardExePath = SysSet.m_strNewGuardProcessPath + "EasyTntGuardProcess.exe";
				std::string strFileData;
				std::ifstream fin(strGuardExePath, std::ifstream::binary);
				if (!fin)
				{
					pUser->SendResult(USER_RESPONSE_CHK_NEW_GUARDEXE, RESULT_ERROR_FILEIO);
					g_Log.LogOut("读取新守护进程文件时打开文件失败: " + strGuardExePath);
					return false;
				}
				std::cout << "正在发送新守护进程文件..." << std::endl;
				std::stringstream buffer;
				buffer << fin.rdbuf();
				strFileData = buffer.str();
				fin.close();
				pUser->SendResponesInfo(USER_RESPONSE_CHK_NEW_GUARDEXE, RESULT_GET_NEW_GUARDEXE_SUCCESS, (char*)strFileData.c_str(), strFileData.length());
				g_Log.LogOut("新守护进程文件发送完成");
				std::cout << "新守护进程文件发送完成" << std::endl;
			}
		}
		break;
	case KEEPALIVE_PKG:
		{
			char szData[1024] = { 0 };
			strncpy(szData, pMission->m_pMissionData + HEAD_SIZE, header.uPackSize);
			std::string strEzs = szData;

			MAP_SESSION::iterator itSession = _mapSession_.begin();
			for (; itSession != _mapSession_.end(); itSession++)
			{
				if (itSession->first == strEzs)
				{
					itSession->second.tmStamp.update();
					itSession->second.nChkHeartPkgFailTimes = 0;
					break;
				}
			}
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

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

			#ifdef TEST_MODE
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

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri	  = SysSet.m_strScanReqUri + "/login";
			pTask->pUser	  = pUser;
			pTask->strMsg	  = "login";
			pTask->strUser		= LoginInfo.szUserNo;
			pTask->strPwd		= LoginInfo.szPWD;
			char szTmp[200] = { 0 };
			sprintf(szTmp, "username=%s&password=%s", LoginInfo.szUserNo, LoginInfo.szPWD);
			pTask->strRequest = szTmp;
			g_fmScanReq.lock();
			g_lScanReq.push_back(pTask);
			g_fmScanReq.unlock();
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

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = SysSet.m_strScanReqUri + "/examinfo";
			pTask->pUser  = pUser;
			pTask->strEzs = "ezs=" + strEzs;
			pTask->strMsg = "ezs";
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
			}
			else
			{
				pMODELINFO pModelInfo = itFind->second;
				if (pModelInfo->strMd5 != stModelInfo.szMD5)		//文件未修改，不需要重新发送
				{
					bNeedSend = true;

					std::string strLog = "模板信息映射表中";
					strLog.append(szIndex);
					strLog.append("的文件MD5与需要上传的文件MD5信息不一致，可以发送此模板的信息");
					g_Log.LogOut(strLog);
				}
			}

			if (bNeedSend)
			{
				pUser->SendResponesInfo(USER_RESPONSE_MODELINFO, RESULT_SETMODELINFO_SEND, (char*)&stModelInfo, sizeof(stModelInfo));

				#ifdef TEST_MODE
				return 1;
				#endif

				Poco::JSON::Object jsnModel;
				jsnModel.set("examId", stModelInfo.nExamID);
				jsnModel.set("subjectId", stModelInfo.nSubjectID);
				jsnModel.set("tmplateName", stModelInfo.szModelName);

				std::stringstream jsnString;
				jsnModel.stringify(jsnString, 0);

				std::string strEzs = stModelInfo.szEzs;
				pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
				pTask->strUri = SysSet.m_strScanReqUri + "/scanTemplate";
				pTask->pUser = pUser;
				pTask->strEzs = "ezs=" + strEzs;
				pTask->strMsg = "setScanModel";
				pTask->strRequest = jsnString.str();
				g_fmScanReq.lock();
				g_lScanReq.push_back(pTask);
				g_fmScanReq.unlock();
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
				Poco::File fileModel(pModelInfo->strPath);
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
				if (!fin)	return false;
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

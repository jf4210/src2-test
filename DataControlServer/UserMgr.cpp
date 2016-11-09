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

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri		= SysSet.m_strBackUri + "/login";
			pTask->pUser		= pUser;
			pTask->strMsg		= "login";
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
			std::cout << "get exam info: " << pUser->m_Name <<std::endl;

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

			pSCAN_REQ_TASK pTask = new SCAN_REQ_TASK;
			pTask->strUri = SysSet.m_strBackUri + "/examinfo";
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

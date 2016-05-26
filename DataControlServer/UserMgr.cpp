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
			}
			else
			{
				pMODELINFO pModelInfo = itFind->second;
				if (pModelInfo->strMd5 != stModelInfo.szMD5)		//文件未修改，不需要重新发送
					bNeedSend = true;
			}

			if (bNeedSend)
			{
				pUser->SendResponesInfo(USER_RESPONSE_MODELINFO, RESULT_SETMODELINFO_SEND, (char*)&stModelInfo, sizeof(stModelInfo));

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
			MAP_MODEL::iterator itFind = _mapModel_.find(szIndex);
			if (itFind == _mapModel_.end())
			{
				pUser->SendResult(USER_RESPONSE_NEEDDOWN, RESULT_DOWNMODEL_FAIL);
				break;
			}
			else
			{
				pMODELINFO pModelInfo = itFind->second;
				if (pModelInfo->strMd5 == stModelInfo.szMD5)		//文件未修改，不需要重新下载
					bNeedDown = false;
			}
			if (bNeedDown)
			{
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
// 				std::ifstream in(pModelInfo->strPath, std::ios::in | std::ios::binary);
// 				std::string strLine;
// 				while (!in.eof())
// 				{
// 					getline(in, strLine);
// 					strFileData.append(strLine);
// 				}
// 				in.close();

				std::ifstream fin(pModelInfo->strPath, std::ifstream::binary);
				if (!fin)	return false;
				std::stringstream buffer;
				buffer << fin.rdbuf();
				strFileData = buffer.str();
				fin.close();

				int nLen = strFileData.length();
				std::cout << "模板长度: " << nLen << std::endl;
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

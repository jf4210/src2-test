#include "UserMgr.h"
#include "VCSDef.h"

CUserMgr::CUserMgr()
{
}


CUserMgr::~CUserMgr()
{
}

int CUserMgr::HandleHeader(CMission* pMission)
{
	ST_CMD_HEADERTOVER header = *(ST_CMD_HEADERTOVER*)pMission->m_pMissionData;
	CNetUser* pUser = pMission->m_pUser;

	BOOL bFind = TRUE; //�������Ƿ񱻴���
	switch (header.usCmd)
	{
		case GET_FILELIST:
		{
			std::string strData;
			LIST_FILEINFO::iterator it = g_lFileInfo.begin();
			for (; it != g_lFileInfo.end(); it++)
			{
				std::string strFileInfo = (*it)->strFileName + ":" + (*it)->strMd5 + "__";
				strData.append(strFileInfo);
			}
			std::stringstream ss;
			ss << "�յ��ͻ���(" << pUser->m_pIPAddress << ":" << pUser->m_wPort << ")�����ļ��б�����,�������ݣ�" << strData;
			std::string strLog = ss.str();
			ss.str("");
			g_Log.LogOut(strLog);
			std::cout << strLog << std::endl;
			pUser->SendResponesInfo(RESPONSE_GET_FILELIST, RESULT_SUCCESS, (char*)strData.c_str(), strData.length());
		}
		break;
		case GET_FILE:
		{
			char szFileName[200] = { 0 };
			memcpy(szFileName, pMission->m_pMissionData + HEAD_SIZE, header.uPackSize);
			std::stringstream ss;
			ss << "�յ��ͻ���(" << pUser->m_pIPAddress << ":" << pUser->m_wPort << ")���������ļ���" << szFileName;
			std::string strLog = ss.str();
			ss.str("");
			g_Log.LogOut(strLog);
			std::cout << strLog << std::endl;

			std::string strFileName = szFileName;
			pST_FILEINFO pFileInfo = NULL;
			LIST_FILEINFO::iterator it = g_lFileInfo.begin();
			for (; it != g_lFileInfo.end(); it++)
			{
				if ((*it)->strFileName == strFileName)
				{
					pFileInfo = *it;
					break;
				}
			}

			std::string strFileData;

			std::ifstream fin(pFileInfo->strFilePath, std::ifstream::binary);
			if (!fin)
			{
				pUser->SendResult(RESPONSE_GET_FILE, RESULT_ERROR_FILEIO);
				return false;
			}
			std::stringstream buffer;
			buffer << fin.rdbuf();
			strFileData = buffer.str();
			fin.close();

			int nLen = strFileData.length();
			ss << "���ͻ���(" << pUser->m_pIPAddress << ":" << pUser->m_wPort << ")�����ļ�: " << strFileName << "   ����: " << nLen;
			strLog = ss.str();
			ss.str("");
			g_Log.LogOut(strLog);
			std::cout << strLog << std::endl;

			if (!pUser->SendResponesInfo(RESPONSE_GET_FILE, RESULT_SUCCESS, (char*)strFileData.c_str(), strFileData.length()))
			{
				pUser->SendResult(RESPONSE_GET_FILE, RESULT_ERROR_SEND);
				ss << "�����ļ����ͻ���(" << pUser->m_pIPAddress << ":" << pUser->m_wPort << ")ʱ�������ʹ���";
				strLog = ss.str();
				ss.str("");
				g_Log.LogOut(strLog);
				std::cout << strLog << std::endl;
			}
		}
		break;
		case RESULT_UPDATA:
		{
			char szResult[500] = { 0 };
			memcpy(szResult, pMission->m_pMissionData + HEAD_SIZE, header.uPackSize);
			std::stringstream ss;
			ss << "�ͻ���(" << pUser->m_pIPAddress << ":" << pUser->m_wPort << ")�������: " << szResult;
			std::string strLog = ss.str();
			ss.str("");
			g_Log.LogOut(strLog);
			std::cout << strLog << std::endl;
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

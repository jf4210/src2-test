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

	BOOL bFind = TRUE; //该命令是否被处理
	switch (header.usCmd)
	{
		case GET_FILELIST:
		{
			std::string strData;
			LIST_FILEINFO::iterator it = g_lFileInfo.begin();
			for (; it != g_lFileInfo.end(); it++)
			{
				std::string strFileInfo = (*it)->strFileName + ":" + (*it)->strMd5 + "_";
				strData.append(strFileInfo);
			}

			std::cout << "收到请求文件列表命令,发送数据："<< strData << std::endl;
			pUser->SendResponesInfo(RESPONSE_GET_FILELIST, RESULT_SUCCESS, (char*)strData.c_str(), strData.length());
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

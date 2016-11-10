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
		case GET_FILE:
		{
			char szFileName[200] = { 0 };
			memcpy(szFileName, pMission->m_pMissionData + HEAD_SIZE, header.uPackSize);
			std::cout << "收到请求下载文件:" << szFileName << std::endl;

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
			std::cout << "发送文件: "<< strFileName <<"   长度: " << nLen << std::endl;
			pUser->SendResponesInfo(RESPONSE_GET_FILE, RESULT_SUCCESS, (char*)strFileData.c_str(), strFileData.length());
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

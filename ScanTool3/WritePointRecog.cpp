#include "stdafx.h"
#include "WritePointRecog.h"


bool CQKPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("识别缺考\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lQK_CP.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lQK_CP.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
		bool bResult_Recog = Recog(rc, matCompPic, strLog);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				((pST_PaperInfo)pPic->pPaper)->nQKFlag = 1;			//设置学生缺考
				pPic->nQKFlag = 1;
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			//g_pLogger->information(szLog);
			strLog.append(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (nRecogMode == 2)
		{
			bResult = false;						//找到问题点
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别缺考失败, 图片名: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CWJPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("识别违纪\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lWJ_CP.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lWJ_CP.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
		bool bResult_Recog = Recog(rc, matCompPic, strLog);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				((pST_PaperInfo)pPic->pPaper)->nWJFlag = 1;			//设置学生违纪
				pPic->nWJFlag = 1;
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			//g_pLogger->information(szLog);
			strLog.append(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (nRecogMode == 2)
		{
			bResult = false;						//找到问题点
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别违纪失败, 图片名: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

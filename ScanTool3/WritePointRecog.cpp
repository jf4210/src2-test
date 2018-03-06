#include "stdafx.h"
#include "WritePointRecog.h"


bool CQKPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("ʶ��ȱ��\n");
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
				((pST_PaperInfo)pPic->pPaper)->nQKFlag = 1;			//����ѧ��ȱ��
				pPic->nQKFlag = 1;
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "У��ʧ��, �쳣����, �����: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			//g_pLogger->information(szLog);
			strLog.append(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (nRecogMode == 2)
		{
			bResult = false;						//�ҵ������
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��ȱ��ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CWJPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("ʶ��Υ��\n");
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
				((pST_PaperInfo)pPic->pPaper)->nWJFlag = 1;			//����ѧ��Υ��
				pPic->nWJFlag = 1;
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "У��ʧ��, �쳣����, �����: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			//g_pLogger->information(szLog);
			strLog.append(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (nRecogMode == 2)
		{
			bResult = false;						//�ҵ������
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��Υ��ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

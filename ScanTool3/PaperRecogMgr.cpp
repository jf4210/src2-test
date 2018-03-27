#include "stdafx.h"
#include "PaperRecogMgr.h"
#include "SNPoint.h"
#include "OmrPoint.h"


CPaperRecogMgr::CPaperRecogMgr(int nRecogModel): _nRecogModel(nRecogModel), pPrintPointRecogObj(NULL), pWritePointRecogObj(NULL)
#ifdef USE_TESSERACT
,m_pTess(NULL)
#endif
{
}


CPaperRecogMgr::~CPaperRecogMgr()
{
	SAFE_RELEASE(pPrintPointRecogObj);
	SAFE_RELEASE(pWritePointRecogObj);
}

bool CPaperRecogMgr::RecogPaper(pST_PaperInfo pPaper, pMODEL pModel, bool bMustRecog /*= false*/)
{
	bool bResult = false;
	ClearPaperRecogData(pPaper);

	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		clock_t start_pic, end_pic;
		start_pic = clock();

		if (!bMustRecog && (*itPic)->nRecoged)		//已经识别过，不再识别
			continue;

	#ifdef TEST_PAGINATION
		int nPic = (*itPic)->nPicModelIndex;
		if (!bMustRecog && pModel->nUsePagination && pPaper->nPaginationStatus == 0)
		{
			pPaper->bIssuePaper = true;
			(*itPic)->nRecoged = 2;			//先临时设置识别完成标识，后面在人工确认完成后需要重新识别
			continue;
		}
	#else
		int nPic = i;
	#endif
		
		RecogPic(nPic, *itPic, pModel);
	}

	return bResult;
}

bool CPaperRecogMgr::RecogPic(int nPic, pST_PicInfo pPic, pMODEL pModel)
{
	clock_t start_pic, end_pic;
	start_pic = clock();
	
	bool bResult = true;
	if (!pPic) return bResult;

	pPic->nRecoged = 1;
	if (nPic >= pModel->vecPaperModel.size())
	{
		pPic->nRecoged = 2;
		return bResult;
	}

	int nCount = pModel->vecPaperModel[nPic]->lH_Head.size() + pModel->vecPaperModel[nPic]->lV_Head.size()
		+ pModel->vecPaperModel[nPic]->lPagination.size() + pModel->vecPaperModel[nPic]->lABModel.size() + pModel->vecPaperModel[nPic]->lCourse.size()
		+ pModel->vecPaperModel[nPic]->lQK_CP.size() + pModel->vecPaperModel[nPic]->lWJ_CP.size() + pModel->vecPaperModel[nPic]->lGray.size()
		+ pModel->vecPaperModel[nPic]->lWhite.size() + pModel->vecPaperModel[nPic]->lSNInfo.size() + pModel->vecPaperModel[nPic]->lOMR2.size()
		+ pModel->vecPaperModel[nPic]->lElectOmr.size();
	if (!nCount)	//如果当前模板试卷没有校验点就不需要进行试卷打开操作，直接下一张试卷
	{
		pPic->nRecoged = 2;
		return bResult;
	}

	std::string strPicFileName = pPic->strPicName;
	cv::Mat matCompSrcPic;
	if (!InitPic(pPic, matCompSrcPic))
	{
		bResult = false;
		std::string strLog = "几次打开文件都失败2: " + pPic->strPicPath;
		g_pLogger->information(strLog);

		pPic->nRecoged = 2;
		return bResult;
	}

#ifdef PIC_RECTIFY_TEST	//图像旋转纠正测试
	Mat matDst;
	Mat matCompPic;
	Mat rotMat;
	PicRectify(matCompSrcPic, matDst, rotMat);
	if (matDst.channels() == 1)
		cvtColor(matDst, matCompPic, CV_GRAY2BGR);
	else
		matCompPic = matDst;
#else
	cv::Mat matCompPic = matCompSrcPic;
#endif

	clock_t end1_pic = clock();

	ClearPicRecogData(pPic);

	CPaperRecogMgr paperRecogMgrObj(_nRecogModel);
	if (g_nOperatingMode == 1)
	{
		if (pModel->nUseWordAnchorPoint)
			bResult = paperRecogMgrObj.RecogCharacter(nPic, matCompPic, pPic, pModel);
		else
			bResult = paperRecogMgrObj.RecogFixCP(nPic, matCompPic, pPic, pModel);

	#ifdef WarpAffine_TEST
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		bResult = GetFixPicTransfer(nPic, matCompPic, pPic, pModel, inverseMat);
	#endif
		bResult = paperRecogMgrObj.RecogABModel(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogCourse(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogQKCP(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogWJCP(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogGrayCP(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogWhiteCP(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogSN(nPic, matCompPic, pPic, pModel);
		// 			{
		// 				(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
		// 				if ((static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.empty())
		// 				{
		// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
		// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
		// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
		// 				}
		// 			}
		bResult = paperRecogMgrObj.RecogOMR(nPic, matCompPic, pPic, pModel);
		bResult = paperRecogMgrObj.RecogElectOmr(nPic, matCompPic, pPic, pModel);
	}
	else
	{
		if (pModel->nUseWordAnchorPoint)
			bResult = paperRecogMgrObj.RecogCharacter(nPic, matCompPic, pPic, pModel);
		else
			bResult = paperRecogMgrObj.RecogFixCP(nPic, matCompPic, pPic, pModel);

	#ifdef WarpAffine_TEST
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		if (bResult) bResult = GetFixPicTransfer(nPic, matCompPic, pPic, pModel, inverseMat);
	#endif
		if (bResult) bResult = paperRecogMgrObj.RecogABModel(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogCourse(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogQKCP(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogWJCP(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogGrayCP(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogWhiteCP(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogSN(nPic, matCompPic, pPic, pModel);
		// 			if (bResult)
		// 			{
		// 				(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
		// 				if ((static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.empty())
		// 				{
		// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
		// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
		// 					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
		// 				}
		// 			}
		if (bResult) bResult = paperRecogMgrObj.RecogOMR(nPic, matCompPic, pPic, pModel);
		if (bResult) bResult = paperRecogMgrObj.RecogElectOmr(nPic, matCompPic, pPic, pModel);
	}
	pPic->nRecoged = 2;

	return bResult;
}

bool CPaperRecogMgr::InitPic(pST_PicInfo pPic, cv::Mat& matCompSrcPic)
{
	std::string strPicFileName = pPic->strPicName;

	bool bOpenSucc = false;
	for (int j = 0; j < 3; j++)
	{
		if (!bOpenSucc)
		{
			try
			{
				Poco::File fPic(CMyCodeConvert::Gb2312ToUtf8(pPic->strPicPath));
				if (!fPic.exists())
					continue;
				matCompSrcPic = cv::imread(pPic->strPicPath);			//imread((*itPic)->strPicPath);
				bOpenSucc = true;
				break;
			}
			catch (cv::Exception& exc)
			{
				_strLog.append("read pic fail: " + std::string(exc.what()));
				Sleep(500);
			}
		}
	}
	return bOpenSucc;
}

bool CPaperRecogMgr::RecogCharacter(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
#ifdef USE_TESSERACT
	pPrintPointRecogObj = new CCharacterPoint(m_pTess);
#endif
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CFixPoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel,_strLog);
}

bool CPaperRecogMgr::RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CABPoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CCoursePoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new CQKPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new CWJPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CGrayPoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pPrintPointRecogObj);
	pPrintPointRecogObj = new CWhitePoint();
	if (!pPrintPointRecogObj) return false;

	return pPrintPointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new CSNPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new COmrPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

bool CPaperRecogMgr::RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	SAFE_RELEASE(pWritePointRecogObj);
	pWritePointRecogObj = new CElectOmrPoint();
	if (!pWritePointRecogObj) return false;

	return pWritePointRecogObj->RecogPrintPoint(nPic, matCompPic, pPic, pModel, _nRecogModel, _strLog);
}

std::string CPaperRecogMgr::GetLog()
{
	return _strLog;
}
#ifdef USE_TESSERACT
void CPaperRecogMgr::SetTesseractObj(tesseract::TessBaseAPI* pTess)
{
	m_pTess = pTess;
}
#endif
void CPaperRecogMgr::ClearPicRecogData(pST_PicInfo pPic)
{
	pPic->strPicZKZH = "";

	pPic->lFix.clear();
	pPic->lNormalRect.clear();
	pPic->lIssueRect.clear();
	pPic->lOmrResult.clear();
	pPic->lElectOmrResult.clear();
	pPic->lCalcRect.clear();
	pPic->lModelWordFix.clear();
	CHARACTER_ANCHOR_AREA_LIST::iterator itCharAnchorArea = pPic->lCharacterAnchorArea.begin();
	for (; itCharAnchorArea != pPic->lCharacterAnchorArea.end();)
	{
		pST_CHARACTER_ANCHOR_AREA pCharAnchorArea = *itCharAnchorArea;
		itCharAnchorArea = pPic->lCharacterAnchorArea.erase(itCharAnchorArea);
		SAFE_RELEASE(pCharAnchorArea);
	}
	pPic->lCharacterAnchorArea.clear();

// 	(static_cast<pST_PaperInfo>(pPic->pPaper))->strSN = "";
// 	(static_cast<pST_PaperInfo>(pPic->pPaper))->strRecogSN4Search = "";
}

void CPaperRecogMgr::ClearPaperRecogData(pST_PaperInfo pPaper)
{
	pPaper->strSN = "";
	pPaper->strRecogSN4Search = "";

	for (auto itSn : pPaper->lSnResult)
	{
		pSN_ITEM pSNItem = itSn;
		SAFE_RELEASE(pSNItem);
	}
	pPaper->lSnResult.clear();
	pPaper->lOmrResult.clear();
	pPaper->lElectOmrResult.clear();
}

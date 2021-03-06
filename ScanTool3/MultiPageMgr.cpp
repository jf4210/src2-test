#include "stdafx.h"
#include "MultiPageMgr.h"
#include "PaperRecogMgr.h"


CMultiPageMgr::CMultiPageMgr(pMODEL pModel)
:_pModel(pModel)
{
}


CMultiPageMgr::~CMultiPageMgr()
{
}

std::string CMultiPageMgr::GetLog()
{
	return _strLog;
}

void CMultiPageMgr::MergePaper(pST_PaperInfo pSrcPaper, pST_PaperInfo pDstPaper)
{
	//试卷合并
	if (!pDstPaper->bIssuePaper)	pDstPaper->bIssuePaper = pSrcPaper->bIssuePaper;
	if (!pDstPaper->bModifyZKZH)	pDstPaper->bModifyZKZH = pSrcPaper->bModifyZKZH;
	if (!pDstPaper->bModifyPagination)	pDstPaper->bModifyPagination = true;
	if (!pDstPaper->bReScan)		pDstPaper->bReScan = pSrcPaper->bReScan;
	if (!pDstPaper->bRecogCourse)	pDstPaper->bRecogCourse = pSrcPaper->bRecogCourse;
	pDstPaper->nPicsExchange += pSrcPaper->nPicsExchange;
	pDstPaper->nPaginationStatus = 2;
	if (pDstPaper->nQKFlag == 0)	pDstPaper->nQKFlag = pSrcPaper->nQKFlag;
	if (pDstPaper->nWJFlag == 0)	pDstPaper->nWJFlag = pSrcPaper->nWJFlag;
	pDstPaper->nZkzhInBmkStatus = pSrcPaper->nZkzhInBmkStatus;
	//pPaper->strRecogSN4Search
	//pPaper->lSnResult

	SCAN_PAPER_LIST::iterator itSrcScanPaper = pSrcPaper->lSrcScanPaper.begin();
	for (; itSrcScanPaper != pSrcPaper->lSrcScanPaper.end();)
	{
		pST_SCAN_PAPER pScanPaper = *itSrcScanPaper;
		pDstPaper->lSrcScanPaper.push_back(pScanPaper);
		itSrcScanPaper = pSrcPaper->lSrcScanPaper.erase(itSrcScanPaper);
	}
	//先找当前图片所在纸张的所有扫描图片
	std::vector<pST_PicInfo> vecScanPaper;
	for (auto pScanPic : pSrcPaper->lPic)
	{
		vecScanPaper.push_back(pScanPic);
	}
	for (auto pMergePic : vecScanPaper)
	{
		MergePic(pSrcPaper, pMergePic, pDstPaper);
	}

// 	for (auto omrObj : pSrcPaper->lOmrResult)			//************	这里有问题：如果试卷被多次合并，则试卷的识别结果会变多	**********************
// 		pDstPaper->lOmrResult.push_back(omrObj);
// 	for (auto electOmrObj : pSrcPaper->lElectOmrResult)	//同上
// 		pDstPaper->lElectOmrResult.push_back(electOmrObj);

// 	PIC_LIST::iterator itPic = pSrcPaper->lPic.begin();
// 	for (; itPic != pSrcPaper->lPic.end(); )
// 	{
// 		pST_PicInfo pPic = *itPic;
// 		itPic = pSrcPaper->lPic.erase(itPic);
// 
// 		pPic->pPaper = pDstPaper;
// 		bool bInsert = false;
// 		PIC_LIST::iterator itNewPic = pDstPaper->lPic.begin();
// 		for (; itNewPic != pDstPaper->lPic.end(); itNewPic++)
// 		{
// 			if ((*itNewPic)->nPicModelIndex > pPic->nPicModelIndex)
// 			{
// 				bInsert = true;
// 				pDstPaper->lPic.insert(itNewPic, pPic);
// 				break;
// 			}
// 		}
// 		if (!bInsert)
// 			pDstPaper->lPic.push_back(pPic);
// 
// 
// 		for (auto omrResult : pSrcPaper->lOmrResult)
// 			pDstPaper->lOmrResult.push_back(omrResult);
// 		for (auto electOmr : pSrcPaper->lElectOmrResult)
// 			pDstPaper->lElectOmrResult.push_back(electOmr);
// 	}
// 
// 	//移动Omr和选做题信息到新试卷，同时移除原试卷对应信息
// 	UpdateOmrInfo(pSrcPaper);
}

void CMultiPageMgr::MergePic(pST_PaperInfo pSrcPaper, pST_PicInfo pSrcPic, pST_PaperInfo pDstPaper)
{
	PIC_LIST::iterator itPic = pSrcPaper->lPic.begin();
	for (; itPic != pSrcPaper->lPic.end(); )
	{
		pST_PicInfo pPic = *itPic;
		if (pPic == pSrcPic)
		{
			itPic = pSrcPaper->lPic.erase(itPic);
			pPic->pPaper = pDstPaper;
			pPic->strPicZKZH = pDstPaper->strSN;
			bool bInsert = false;
			PIC_LIST::iterator itNewPic = pDstPaper->lPic.begin();
			for (; itNewPic != pDstPaper->lPic.end(); itNewPic++)
			{
				if ((*itNewPic)->nPicModelIndex > pPic->nPicModelIndex)
				{
					bInsert = true;
					pDstPaper->lPic.insert(itNewPic, pPic);
					break;
				}
			}
			if (!bInsert)
				pDstPaper->lPic.push_back(pPic);
			break;
		}
		else
			itPic++;
	}
	//设置试卷修改标识
	pDstPaper->bModifyPagination = true;
	pSrcPaper->bModifyPagination = true;
	//移动Omr和选做题信息到新试卷，同时移除原试卷对应信息
	UpdateOmrInfo(pSrcPaper);
	for (auto omrResult : pSrcPic->lOmrResult)
		pDstPaper->lOmrResult.push_back(omrResult);
	for (auto electOmr : pSrcPic->lElectOmrResult)
		pDstPaper->lElectOmrResult.push_back(electOmr);
}

bool CMultiPageMgr::MergePapers(pPAPERSINFO pPapers)
{
	bool bMergedPaper = false;
	pST_PaperInfo pSrcPaper = NULL;
	PAPER_LIST::iterator itCurrentPaper = pPapers->lPaper.begin();
	for (; itCurrentPaper != pPapers->lPaper.end(); )
	{
		pSrcPaper = *itCurrentPaper;
		if (!pSrcPaper->strSN.empty())
		{
			bool bFindZKZH = false;
			pST_PaperInfo pNewPaper = NULL;
			PAPER_LIST::iterator itNewPaper = pPapers->lPaper.begin();
			for (; itNewPaper != pPapers->lPaper.end(); )
			{
				pNewPaper = *itNewPaper;
				if (pNewPaper->strSN == pSrcPaper->strSN && pSrcPaper != pNewPaper)
				{
					bFindZKZH = true;
					bMergedPaper = true;
					MergePaper(pNewPaper, pSrcPaper);

					itNewPaper = pPapers->lPaper.erase(itNewPaper);
					SAFE_RELEASE(pNewPaper);
				}
				else
					itNewPaper++;
			}
			if(bFindZKZH)
				ChkPaperValid(pSrcPaper, _pModel);
		}
		itCurrentPaper++;
	}	
	if(bMergedPaper) ReNamePicInPapers(pPapers);
	return bMergedPaper;
}

bool CMultiPageMgr::ModifyPicPagination(pST_PicInfo pPic, int nNewPage)
{
	if (!ChkModifyPagination(pPic, nNewPage)) return false;

	pST_PicInfo pFirstPic = pPic;		//一张试卷的正反两面的图片指针
	pST_PicInfo pSecondPic = NULL;

	pPic->nPicOldModelIndex = pPic->nPicModelIndex;
	pPic->nPicModelIndex = nNewPage - 1;
	pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	pCurrentPaper->bModifyPagination = true;
	if (static_cast<pST_SCAN_PAPER>(pPic->pSrcScanPic->pParentScanPaper)->bDoubleScan)
	{
		//双面扫描时，修改这页试卷页码后，修改对应的另一页的试卷页码，如果不修改，后面可能出现不知道属于那张试卷的情况
		for (auto pScanPic : pCurrentPaper->lPic)
		{
			if (pScanPic != pPic && pScanPic->pSrcScanPic->pParentScanPaper == pPic->pSrcScanPic->pParentScanPaper)
			{
				pScanPic->nPicOldModelIndex = pScanPic->nPicModelIndex;
				if (pPic->nPicModelIndex % 2 == 0)
					pScanPic->nPicModelIndex = pPic->nPicModelIndex + 1;
				else
					pScanPic->nPicModelIndex = pPic->nPicModelIndex - 1;
				pSecondPic = pScanPic;
				break;
			}
		}
	}
	//************************************************************************
	//************	修改页码后，需要根据新页码来重新识别	******************
	//************************************************************************
#if 1
	CPaperRecogMgr paperRecogMgrObj(g_nOperatingMode);
	if (pCurrentPaper->lSrcScanPaper.size() == 1)	//当前考生试卷只有1张
	{
		paperRecogMgrObj.RecogPaper(pCurrentPaper, _pModel, true);
	}
	else
	{
		//当前考生试卷不止一张，即多页试卷，只需要重识别当前扫描页的试卷
		paperRecogMgrObj.RecogPic(pFirstPic->nPicModelIndex, pFirstPic, _pModel);
		paperRecogMgrObj.RecogPic(pSecondPic->nPicModelIndex, pSecondPic, _pModel);
	}
	
	pCurrentPaper->lPic.sort([](pST_PicInfo pPic1, pST_PicInfo pPic2)
	{return pPic1->nPicModelIndex < pPic2->nPicModelIndex; });

	ChkPaperValid(pCurrentPaper, _pModel);
#else
	//下面的修改omr所属页面也没有意义，在调用修改页码之前，必须先根据新页码来重识别

	//修改Omr和选做题的页码信息
	for (auto pScanPic : pCurrentPaper->lPic)
	{
		if (pScanPic->pSrcScanPic->pParentScanPaper == pPic->pSrcScanPic->pParentScanPaper)
		{
			OMRRESULTLIST::iterator itOmr = pScanPic->lOmrResult.begin();
			for (; itOmr != pScanPic->lOmrResult.end(); itOmr++)
			{
				itOmr->nPageId = pScanPic->nPicModelIndex + 1;
			}
			ELECTOMR_LIST::iterator itElectOmr = pScanPic->lElectOmrResult.begin();
			for (; itElectOmr != pScanPic->lElectOmrResult.end(); itElectOmr++)
			{
				itElectOmr->nPageId = pScanPic->nPicModelIndex + 1;
			}
		}
	}
#endif
	return true;
}

bool CMultiPageMgr::ModifyPicZkzh(pST_PicInfo pPic, pPAPERSINFO pPapers, std::string strZKZH)
{
	bool bResult = true;
	if (strZKZH.empty() || !pPic || !pPapers)
		return false;

	clock_t sT, eT;
	sT = clock();
	std::stringstream ssLog;

	//检查准考证号修改
	pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	if (pCurrentPaper->strSN == strZKZH)	//准考证号没有修改，不需要进行试卷合并
	{
		pCurrentPaper->lPic.sort([](pST_PicInfo pPic1, pST_PicInfo pPic2)
		{return pPic1->nPicModelIndex < pPic2->nPicModelIndex; });
	}
	else
	{
		//准考证号有修改，需要检查试卷袋是否存在此新准考证号，存在则合并，
		pST_SCAN_PAPER pScanPaperTask = static_cast<pST_SCAN_PAPER>(pPic->pSrcScanPic->pParentScanPaper);
		//如果图片只有2张，又是双面扫描，或者，单面扫描，试卷只有1张，则当存在此新准考证号时，不需要重新构建试卷，将当前试卷整张合并到新试卷
		if (pScanPaperTask->bDoubleScan && pCurrentPaper->lPic.size() <= 2 || \
			!pScanPaperTask->bDoubleScan && pCurrentPaper->lPic.size() <= 1)
		{
			bool bFindZKZH = false;
			pST_PaperInfo pDstPaper = NULL;
			PAPER_LIST::iterator itCurrentPaper;
			PAPER_LIST::iterator itNewPaper = pPapers->lPaper.begin();
			for (; itNewPaper != pPapers->lPaper.end(); )
			{
				pST_PaperInfo pNewPaper = *itNewPaper;
				if (pNewPaper->strSN == strZKZH && pCurrentPaper != pNewPaper)
				{
					bFindZKZH = true;
					pDstPaper = pNewPaper;
					MergePaper(pCurrentPaper, pNewPaper);
				}
				if (pCurrentPaper == pNewPaper)
					itCurrentPaper = itNewPaper;
				itNewPaper++;
			}
			if (bFindZKZH)
			{
				pPapers->lPaper.erase(itCurrentPaper);
				SAFE_RELEASE(pCurrentPaper);
				ChkPaperValid(pDstPaper, _pModel);
			}
			else
			{
				//如果没发现相同的准考证号，则不需要重新添加到试卷袋，修改当前试卷的准考证号
				pCurrentPaper->strSN = strZKZH;
				for (auto picItem : pCurrentPaper->lPic)
					picItem->strPicZKZH = strZKZH;
				ChkPaperValid(pCurrentPaper, _pModel);
			}
		}
		else	//图片超过2张，则当前试卷不需要从试卷袋移除
		{
			bool bFindZKZH = false;
			PAPER_LIST::iterator itNewPaper = pPapers->lPaper.begin();
			for (; itNewPaper != pPapers->lPaper.end(); )
			{
				pST_PaperInfo pNewPaper = *itNewPaper;
				if (pNewPaper->strSN == strZKZH && pCurrentPaper != pNewPaper)
				{
					bFindZKZH = true;
					MergeScanPaperToDstPaper(pScanPaperTask, pCurrentPaper, pNewPaper);
					break;
				}
				itNewPaper++;
			}
			if (!bFindZKZH)		//如果没发现相同的准考证号，则重新构建试卷并加入到试卷袋
			{
				pST_PaperInfo pNewPaper = GetBaseNewPaperFromScanPaper(pScanPaperTask, pPapers, pCurrentPaper, strZKZH);
				MergeScanPaperToDstPaper(pScanPaperTask, pCurrentPaper, pNewPaper);
			}
		}
	}
	eT = clock();
	ssLog << "设置图片(" << pPic->strPicName << ")所属的扫描试卷完成. " << (int)(eT - sT) << "ms\n";
	std::string strTmpLog = ssLog.str();
	TRACE(strTmpLog.c_str());

	return bResult;
}

bool CMultiPageMgr::ChkModifyPagination(pST_PicInfo pPic, int nNewPage)
{
	bool bResult = true;
	if (nNewPage > _pModel->vecPaperModel.size() || nNewPage < 1)
		bResult = false;
	return bResult;
}

void CMultiPageMgr::UpdateOmrInfo(pST_PaperInfo pPaper)
{
	pPaper->bRecogCourse = true;
	pPaper->nQKFlag = 0;
	pPaper->nWJFlag = 0;
	pPaper->lOmrResult.clear();
	pPaper->lElectOmrResult.clear();
	for (auto pPic : pPaper->lPic)
	{
		if (pPaper->bRecogCourse) pPaper->bRecogCourse = pPic->bRecogCourse;
		if (pPaper->nQKFlag == 0) pPaper->nQKFlag = pPic->nQKFlag;
		if (pPaper->nWJFlag == 0) pPaper->nWJFlag = pPic->nWJFlag;

		OMRRESULTLIST::iterator itOmr = pPic->lOmrResult.begin();
		for (; itOmr != pPic->lOmrResult.end(); itOmr++)
		{
			OMR_RESULT omrResult = *itOmr;
			pPaper->lOmrResult.push_back(omrResult);
		}
		ELECTOMR_LIST::iterator itElectOmr = pPic->lElectOmrResult.begin();
		for (; itElectOmr != pPic->lElectOmrResult.end(); itElectOmr++)
		{
			ELECTOMR_QUESTION omrResult = *itElectOmr;
			pPaper->lElectOmrResult.push_back(omrResult);
		}
	}
}

pST_PaperInfo CMultiPageMgr::GetBaseNewPaperFromScanPaper(pST_SCAN_PAPER pScanPaper, pPAPERSINFO pPapers, pST_PaperInfo pCurrentPaper, std::string strSN)
{
	pST_PaperInfo pNewPaper = NULL;
#if 1
	char szStudentName[30] = { 0 };
	sprintf_s(szStudentName, "S%d", pScanPaper->vecScanPic[0]->nStudentID);
	pNewPaper = new ST_PaperInfo;
	pNewPaper->nIndex = pScanPaper->vecScanPic[0]->nStudentID;
	pNewPaper->strStudentInfo = szStudentName;
	pNewPaper->strSN = strSN;
	pNewPaper->pModel = _pModel_;
	pNewPaper->pPapers = pScanPaper->pPapersInfo;
	pNewPaper->pSrcDlg = pCurrentPaper->pSrcDlg;		//m_pDlg;
	//pNewPaper->lSrcScanPaper.push_back(pScanPaper);

	SCAN_PAPER_LIST::iterator itSrcScanPaper = pCurrentPaper->lSrcScanPaper.begin();
	for (; itSrcScanPaper != pCurrentPaper->lSrcScanPaper.end();)
	{
		pST_SCAN_PAPER pScanPaper = *itSrcScanPaper;
		if (pScanPaper == pScanPaper)		//当前图片所属的扫描试卷移动到新考生试卷，并从原考生扫描试卷列表中删除
		{
			pNewPaper->lSrcScanPaper.push_back(pScanPaper);
			itSrcScanPaper = pCurrentPaper->lSrcScanPaper.erase(itSrcScanPaper);
		}
		else
			itSrcScanPaper++;
	}

	if (!pScanPaper->bCanRecog)
		pNewPaper->nPaginationStatus = 0;	//没有识别到页码，不能参与识别，设置问题卷，人工确认后再识别
	else
		pNewPaper->nPaginationStatus = 1;	//识别完页码，可以识别，不能确定具体属于哪个学生(默认)

	pPapers->fmlPaper.lock();
	pPapers->lPaper.push_back(pNewPaper);
	pPapers->fmlPaper.unlock();
#else
	for (int i = 0; i < pScanPaper->vecScanPic.size(); i++)
	{
		pST_SCAN_PIC pScanPic = pScanPaper->vecScanPic[i];
		//CScanMgrDlg* pDlg = static_cast<CScanMgrDlg*>(pScanPic->pNotifyDlg);

		pST_PicInfo pPic = new ST_PicInfo;
		pPic->strPicName = pScanPic->strPicName;
		pPic->strPicPath = pScanPic->strPicPath;
		pPic->pSrcScanPic = pScanPic;
		if (pScanPaper->bDoubleScan)
		{
			pPic->nPicModelIndex = pScanPaper->nModelPaperID * 2 + i;	//设置图片是属于模板的第几页
			pPic->nPicOldModelIndex = pPic->nPicModelIndex;
		}
		else
		{
			pPic->nPicModelIndex = i;
			pPic->nPicOldModelIndex = pPic->nPicModelIndex;
		}
		if (pScanPic->nOrder == 1)	//(pScanPic->nOrder + 1) % 2 == 0	第1、3、5...页的时候创建新的试卷信息，如果是多页模式时，每一张试卷创建一个考生信息，最后根据准考证号合并考生
		{
			char szStudentName[30] = { 0 };
			sprintf_s(szStudentName, "S%d", pScanPic->nStudentID);
			pNewPaper = new ST_PaperInfo;
			pNewPaper->nIndex = pScanPic->nStudentID;
			pNewPaper->strStudentInfo = szStudentName;
			pNewPaper->strSN = strSN;
			pNewPaper->pModel = _pModel_;
			pNewPaper->pPapers = pScanPaper->pPapersInfo;
			pNewPaper->pSrcDlg = pCurrentPaper->pSrcDlg;		//m_pDlg;
			pNewPaper->lPic.push_back(pPic);
			pNewPaper->lSrcScanPaper.push_back(pScanPaper);

			if (!pScanPaper->bCanRecog)
				pNewPaper->nPaginationStatus = 0;	//没有识别到页码，不能参与识别，设置问题卷，人工确认后再识别
			else
				pNewPaper->nPaginationStatus = 1;	//识别完页码，可以识别，不能确定具体属于哪个学生(默认)

			pPapers->fmlPaper.lock();
			pPapers->lPaper.push_back(pNewPaper);
			pPapers->fmlPaper.unlock();
		}
		else
		{
			pNewPaper->lPic.push_back(pPic);
		}
		pPic->pPaper = pNewPaper;
	}
#endif
	return pNewPaper;
}

void CMultiPageMgr::MergeScanPaperToDstPaper(pST_SCAN_PAPER pScanPaper, pST_PaperInfo pCurrentPaper, pST_PaperInfo pDstPaper)
{
	//先找当前图片所在纸张的所有扫描图片
	std::vector<pST_PicInfo> vecScanPaper;
	for (auto pScanPic : pCurrentPaper->lPic)
	{
		if (pScanPic->pSrcScanPic->pParentScanPaper == pScanPaper)
			vecScanPaper.push_back(pScanPic);
	}
	SCAN_PAPER_LIST::iterator itSrcScanPaper = pCurrentPaper->lSrcScanPaper.begin();
	for (; itSrcScanPaper != pCurrentPaper->lSrcScanPaper.end();)
	{
		pST_SCAN_PAPER pTmpScanPaper = *itSrcScanPaper;
		if (pTmpScanPaper == pScanPaper)		//当前图片所属的扫描试卷移动到新考生试卷，并从原考生扫描试卷列表中删除
		{
			pDstPaper->lSrcScanPaper.push_back(pTmpScanPaper);
			itSrcScanPaper = pCurrentPaper->lSrcScanPaper.erase(itSrcScanPaper);
		}
		else
			itSrcScanPaper++;
	}
	for (auto pMergePic : vecScanPaper)
	{
		MergePic(pCurrentPaper, pMergePic, pDstPaper);
	}
	ChkPaperValid(pCurrentPaper, _pModel);
	ChkPaperValid(pDstPaper, _pModel);
}

void CMultiPageMgr::ChkPaperValid(pST_PaperInfo pPaper, pMODEL pModel)
{
	if (!pModel) return;

	if (pPaper->lPic.size() != pModel->vecPaperModel.size())
		pPaper->nPaginationStatus = 3;
	else
	{
		//图片的页码是否存在重复的
		bool bFind = false;
		int nLastPicIndex = -1;
		PIC_LIST::iterator itPic = pPaper->lPic.begin();		//图片在插入的时候已经初步排序
		for (; itPic != pPaper->lPic.end(); itPic++)
		{
			pST_PicInfo pPic = *itPic;
			if (nLastPicIndex == pPic->nPicModelIndex)
			{
				bFind = true;
				pPaper->nPaginationStatus = 4;
				break;
			}
			nLastPicIndex = pPic->nPicModelIndex;
		}
		if (!bFind)
		{
			pPaper->nPaginationStatus = 2;
			pPaper->bRecogComplete = true;
		}
	}
}

void CMultiPageMgr::ReNamePicInPapers(pPAPERSINFO pPapers)
{
	std::string strLog;
	PAPER_LIST::iterator itReNamePaper = pPapers->lPaper.begin();
	for (; itReNamePaper != pPapers->lPaper.end(); itReNamePaper++)
	{
		pST_PaperInfo pCurrentPaper = *itReNamePaper;

		PIC_LIST::iterator itPic = pCurrentPaper->lPic.begin();
		for (; itPic != pCurrentPaper->lPic.end(); itPic++)
		{
			pST_PicInfo pPic = *itPic;
			std::string strNewPicName = Poco::format("%s_%d.jpg", pCurrentPaper->strStudentInfo, pPic->nPicModelIndex + 1);
			int nPos = pPic->strPicPath.rfind('\\') + 1;
			std::string strBasePath = pPic->strPicPath.substr(0, nPos);
			std::string strNewPath = strBasePath + strNewPicName;
			if (strNewPicName != pPic->strPicName)
			{
				try
				{
					Poco::File fNewPic(CMyCodeConvert::Gb2312ToUtf8(pPic->strPicPath));
					fNewPic.renameTo(CMyCodeConvert::Gb2312ToUtf8(strNewPath));
					pPic->strPicPath = strNewPath;
					pPic->strPicName = strNewPicName;
				}
				catch (Poco::Exception& exc)
				{
					std::string strTmpLog;
					strTmpLog = Poco::format("原始图片(%s)重命名为(%s)失败, 原因: %s\n", pPic->strPicName, strNewPicName, std::string(exc.what()));
					strLog.append(strTmpLog);
				}
			}
		}
	}
	_strLog.append(strLog);
}

void CMultiPageMgr::ReNamePaper(pST_PaperInfo pPaper)
{
	std::string strLog;
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (; itPic != pPaper->lPic.end(); itPic++)
	{
		pST_PicInfo pPic = *itPic;
		std::string strNewPicName = Poco::format("%s_%d.jpg", pPaper->strStudentInfo, pPic->nPicModelIndex + 1);
		int nPos = pPic->strPicPath.rfind('\\') + 1;
		std::string strBasePath = pPic->strPicPath.substr(0, nPos);
		std::string strNewPath = strBasePath + strNewPicName;
		if (strNewPicName != pPic->strPicName)
		{
			try
			{
				Poco::File fNewPic(CMyCodeConvert::Gb2312ToUtf8(pPic->strPicPath));
				fNewPic.renameTo(CMyCodeConvert::Gb2312ToUtf8(strNewPath));
				pPic->strPicPath = strNewPath;
				pPic->strPicName = strNewPicName;
			}
			catch (Poco::Exception& exc)
			{
				std::string strTmpLog;
				strTmpLog = Poco::format("原始图片(%s)重命名为(%s)失败, 原因: %s\n", pPic->strPicName, strNewPicName, std::string(exc.what()));
				strLog.append(strTmpLog);
			}
		}
	}
	_strLog.append(strLog);
}


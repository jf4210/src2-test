#include "stdafx.h"
#include "MultiPageMgr.h"


CMultiPageMgr::CMultiPageMgr()
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
	for (auto omrObj : pSrcPaper->lOmrResult)			//************	这里有问题：如果试卷被多次合并，则试卷的识别结果会变多	**********************
		pDstPaper->lOmrResult.push_back(omrObj);
	for (auto electOmrObj : pSrcPaper->lElectOmrResult)	//同上
		pDstPaper->lElectOmrResult.push_back(electOmrObj);

	PIC_LIST::iterator itPic = pSrcPaper->lPic.begin();
	for (; itPic != pSrcPaper->lPic.end(); )
	{
		pST_PicInfo pPic = *itPic;
		itPic = pSrcPaper->lPic.erase(itPic);

		pPic->pPaper = pDstPaper;
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
	}
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
}

void CMultiPageMgr::ModifyPicPagination(pST_PicInfo pPic, int nNewPage)
{
	pPic->nPicModelIndex = nNewPage - 1;
	if (static_cast<pST_SCAN_PAPER>(pPic->pSrcScanPic->pParentScanPaper)->bDoubleScan)
	{
		pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
		//双面扫描时，修改这页试卷页码后，修改对应的另一页的试卷页码，如果不修改，后面可能出现不知道属于那张试卷的情况
		for (auto pScanPic : pCurrentPaper->lPic)
			if (pScanPic != pPic && pScanPic->pSrcScanPic->pParentScanPaper == pPic->pSrcScanPic->pParentScanPaper)
			{
				if (pPic->nPicModelIndex % 2 == 0)
					pScanPic->nPicModelIndex = pPic->nPicModelIndex + 1;
				else
					pScanPic->nPicModelIndex = pPic->nPicModelIndex - 1;
				break;
			}
	}
}

void CMultiPageMgr::ModifyPic(pST_PicInfo pPic, pPAPERSINFO pPapers, int nNewPage, std::string strZKZH)
{
	//检查页码修改
	if (pPic->nPicModelIndex != nNewPage - 1)
	{
		ModifyPicPagination(pPic, nNewPage);
	}

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
		//不存在时，分两种情况，1、当前试卷的图片<=2张，不需要新构建试卷，2、当前试卷的图片>2张，需要从原试卷移除，并新构建试卷
		if (static_cast<pST_SCAN_PAPER>(pPic->pSrcScanPic->pParentScanPaper)->bDoubleScan)
		{
			if (pCurrentPaper->lPic.size() <= 2)		//如果图片只有2张，又是双面扫描，则不需要重新构建试卷
			{
				bool bFindZKZH = false;
				for (auto pNewPaper : pPapers->lPaper)
				{
					if (pNewPaper->strSN == strZKZH)
					{
						bFindZKZH = true;
						MergePaper(pCurrentPaper, pNewPaper);
						break;
					}
				}
				if (!bFindZKZH)
					pPapers->lPaper.push_back(pCurrentPaper);		//逻辑有问题
			}
			else
			{

			}
		}
	}
}


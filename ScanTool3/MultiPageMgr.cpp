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
	//�Ծ�ϲ�
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
	for (auto omrObj : pSrcPaper->lOmrResult)			//************	���������⣺����Ծ���κϲ������Ծ��ʶ��������	**********************
		pDstPaper->lOmrResult.push_back(omrObj);
	for (auto electOmrObj : pSrcPaper->lElectOmrResult)	//ͬ��
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
		//˫��ɨ��ʱ���޸���ҳ�Ծ�ҳ����޸Ķ�Ӧ����һҳ���Ծ�ҳ�룬������޸ģ�������ܳ��ֲ�֪�����������Ծ�����
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
	//���ҳ���޸�
	if (pPic->nPicModelIndex != nNewPage - 1)
	{
		ModifyPicPagination(pPic, nNewPage);
	}

	//���׼��֤���޸�
	pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	if (pCurrentPaper->strSN == strZKZH)	//׼��֤��û���޸ģ�����Ҫ�����Ծ�ϲ�
	{
		pCurrentPaper->lPic.sort([](pST_PicInfo pPic1, pST_PicInfo pPic2)
		{return pPic1->nPicModelIndex < pPic2->nPicModelIndex; });
	}
	else
	{
		//׼��֤�����޸ģ���Ҫ����Ծ���Ƿ���ڴ���׼��֤�ţ�������ϲ���
		//������ʱ�������������1����ǰ�Ծ��ͼƬ<=2�ţ�����Ҫ�¹����Ծ�2����ǰ�Ծ��ͼƬ>2�ţ���Ҫ��ԭ�Ծ��Ƴ������¹����Ծ�
		if (static_cast<pST_SCAN_PAPER>(pPic->pSrcScanPic->pParentScanPaper)->bDoubleScan)
		{
			if (pCurrentPaper->lPic.size() <= 2)		//���ͼƬֻ��2�ţ�����˫��ɨ�裬����Ҫ���¹����Ծ�
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
					pPapers->lPaper.push_back(pCurrentPaper);		//�߼�������
			}
			else
			{

			}
		}
	}
}


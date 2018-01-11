#include "stdafx.h"
#include "MultiPageMgr.h"


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
	//���ҵ�ǰͼƬ����ֽ�ŵ�����ɨ��ͼƬ
	std::vector<pST_PicInfo> vecScanPaper;
	for (auto pScanPic : pSrcPaper->lPic)
	{
		vecScanPaper.push_back(pScanPic);
	}
	for (auto pMergePic : vecScanPaper)
	{
		MergePic(pSrcPaper, pMergePic, pDstPaper);
	}

// 	for (auto omrObj : pSrcPaper->lOmrResult)			//************	���������⣺����Ծ���κϲ������Ծ��ʶ��������	**********************
// 		pDstPaper->lOmrResult.push_back(omrObj);
// 	for (auto electOmrObj : pSrcPaper->lElectOmrResult)	//ͬ��
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
// 	//�ƶ�Omr��ѡ������Ϣ�����Ծ�ͬʱ�Ƴ�ԭ�Ծ��Ӧ��Ϣ
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
	//�ƶ�Omr��ѡ������Ϣ�����Ծ�ͬʱ�Ƴ�ԭ�Ծ��Ӧ��Ϣ
	UpdateOmrInfo(pSrcPaper);
	for (auto omrResult : pSrcPic->lOmrResult)
		pDstPaper->lOmrResult.push_back(omrResult);
	for (auto electOmr : pSrcPic->lElectOmrResult)
		pDstPaper->lElectOmrResult.push_back(electOmr);
}

bool CMultiPageMgr::ModifyPicPagination(pST_PicInfo pPic, int nNewPage)
{
	bool bResult = true; 
// 	bResult = ChkModifyPagination(pPic, nNewPage);
// 	if (!bResult) return bResult;

	pPic->nPicOldModelIndex = pPic->nPicModelIndex;
	pPic->nPicModelIndex = nNewPage - 1;
	pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	if (static_cast<pST_SCAN_PAPER>(pPic->pSrcScanPic->pParentScanPaper)->bDoubleScan)
	{
		//˫��ɨ��ʱ���޸���ҳ�Ծ�ҳ����޸Ķ�Ӧ����һҳ���Ծ�ҳ�룬������޸ģ�������ܳ��ֲ�֪�����������Ծ�����
		for (auto pScanPic : pCurrentPaper->lPic)
		{
			if (pScanPic != pPic && pScanPic->pSrcScanPic->pParentScanPaper == pPic->pSrcScanPic->pParentScanPaper)
			{
				pScanPic->nPicOldModelIndex = pScanPic->nPicModelIndex;
				if (pPic->nPicModelIndex % 2 == 0)
					pScanPic->nPicModelIndex = pPic->nPicModelIndex + 1;
				else
					pScanPic->nPicModelIndex = pPic->nPicModelIndex - 1;
				break;
			}
		}
	}
	//�޸�Omr��ѡ�����ҳ����Ϣ
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
	return bResult;
}

bool CMultiPageMgr::ModifyPic(pST_PicInfo pPic, pPAPERSINFO pPapers, int nNewPage, std::string strZKZH)
{
	bool bResult = true;
	if (nNewPage < 1 || strZKZH.empty() || !pPic || !pPapers)
		return false;

	//���ҳ���޸�
	if (pPic->nPicModelIndex != nNewPage - 1)
	{
		bResult = ModifyPicPagination(pPic, nNewPage);
		if (!bResult)
			return bResult;
	}

	//���׼��֤���޸�
	pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	if (pCurrentPaper->strSN == strZKZH)	//׼��֤��û���޸ģ�����Ҫ�����Ծ�ϲ�
	{
		pCurrentPaper->lPic.sort([](pST_PicInfo pPic1, pST_PicInfo pPic2)
		{return pPic1->nPicModelIndex < pPic2->nPicModelIndex;});
	}
	else
	{
		//׼��֤�����޸ģ���Ҫ����Ծ���Ƿ���ڴ���׼��֤�ţ�������ϲ���

		pST_SCAN_PAPER pScanPaperTask = static_cast<pST_SCAN_PAPER>(pPic->pSrcScanPic->pParentScanPaper);
		//���ͼƬֻ��2�ţ�����˫��ɨ�裬���ߣ�����ɨ�裬�Ծ�ֻ��1�ţ��򵱲����ڴ���׼��֤��ʱ������Ҫ���¹����Ծ�����ǰ�Ծ����źϲ������Ծ�
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
			//���û������ͬ��׼��֤�ţ�����Ҫ������ӵ��Ծ��
		}
		else	//ͼƬ����2�ţ���ǰ�Ծ���Ҫ���Ծ���Ƴ�
		{
			bool bFindZKZH = false;
			PAPER_LIST::iterator itNewPaper = pPapers->lPaper.begin();
			for (; itNewPaper != pPapers->lPaper.end(); )
			{
				pST_PaperInfo pNewPaper = *itNewPaper;
				if (pNewPaper->strSN == strZKZH && pCurrentPaper != pNewPaper)
				{
					bFindZKZH = true;
					//���ҵ�ǰͼƬ����ֽ�ŵ�����ɨ��ͼƬ
					std::vector<pST_PicInfo> vecScanPaper;
					for (auto pScanPic : pCurrentPaper->lPic)
					{
						if (pScanPic->pSrcScanPic->pParentScanPaper == pPic->pSrcScanPic->pParentScanPaper)
							vecScanPaper.push_back(pScanPic);
					}
					SCAN_PAPER_LIST::iterator itSrcScanPaper = pCurrentPaper->lSrcScanPaper.begin();
					for (; itSrcScanPaper != pCurrentPaper->lSrcScanPaper.end();)
					{
						pST_SCAN_PAPER pScanPaper = *itSrcScanPaper;
						if (pScanPaper == pPic->pSrcScanPic->pParentScanPaper)		//��ǰͼƬ������ɨ���Ծ��ƶ����¿����Ծ�����ԭ����ɨ���Ծ��б���ɾ��
						{
							pNewPaper->lSrcScanPaper.push_back(pScanPaper);
							itSrcScanPaper = pCurrentPaper->lSrcScanPaper.erase(itSrcScanPaper);
						}
						else
							itSrcScanPaper++;
					}
					for (auto pMergePic : vecScanPaper)
					{
						MergePic(pCurrentPaper, pMergePic, pNewPaper);
					}
					ChkPaperValid(pCurrentPaper, _pModel);
					ChkPaperValid(pNewPaper, _pModel);
					break;
				}
				itNewPaper++;
			}
			if (!bFindZKZH)		//���û������ͬ��׼��֤�ţ������¹����Ծ����뵽�Ծ��
			{
				pST_PaperInfo pNewPaper = GetNewPaperFromScanPaper(pScanPaperTask, pPapers, pCurrentPaper->pSrcDlg, strZKZH);
				UpdateOmrInfo(pNewPaper);
				ChkPaperValid(pNewPaper, _pModel);

				PIC_LIST::iterator itPic = pCurrentPaper->lPic.begin();
				for (; itPic != pCurrentPaper->lPic.end(); )
				{
					pST_PicInfo pCurrentPic = *itPic;
					if (pCurrentPic->pSrcScanPic->pParentScanPaper == pPic->pSrcScanPic->pParentScanPaper)
						itPic = pCurrentPaper->lPic.erase(itPic);
					else
						itPic++;
				}
				//��ԭ�Ծ�Ŀ�����Ϣ�����޸�
				UpdateOmrInfo(pCurrentPaper);
				ChkPaperValid(pCurrentPaper, _pModel);
			}
		}
	}
	return bResult;
}

bool CMultiPageMgr::ChkModifyPagination(pST_PicInfo pPic, int nNewPage)
{
	bool bResult = true;
	pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	for (auto pTmpPic : pCurrentPaper->lPic)
	{
		if (pTmpPic->nPicModelIndex == nNewPage - 1)
		{
			bResult = false;
			break;
		}
	}
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

pST_PaperInfo CMultiPageMgr::GetNewPaperFromScanPaper(pST_SCAN_PAPER pScanPaper, pPAPERSINFO pPapers, void* pNotifyDlg, std::string strSN)
{
	pST_PaperInfo pNewPaper = NULL;
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
			pPic->nPicModelIndex = pScanPaper->nModelPaperID * 2 + i;	//����ͼƬ������ģ��ĵڼ�ҳ
			pPic->nPicOldModelIndex = pPic->nPicModelIndex;
		}
		else
		{
			pPic->nPicModelIndex = i;
			pPic->nPicOldModelIndex = pPic->nPicModelIndex;
		}
		if (pScanPic->nOrder == 1)	//(pScanPic->nOrder + 1) % 2 == 0	��1��3��5...ҳ��ʱ�򴴽��µ��Ծ���Ϣ������Ƕ�ҳģʽʱ��ÿһ���Ծ���һ��������Ϣ��������׼��֤�źϲ�����
		{
			char szStudentName[30] = { 0 };
			sprintf_s(szStudentName, "S%d", pScanPic->nStudentID);
			pNewPaper = new ST_PaperInfo;
			pNewPaper->nIndex = pScanPic->nStudentID;
			pNewPaper->strStudentInfo = szStudentName;
			pNewPaper->strSN = strSN;
			pNewPaper->pModel = _pModel_;
			pNewPaper->pPapers = pScanPaper->pPapersInfo;
			pNewPaper->pSrcDlg = pNotifyDlg;		//m_pDlg;
			pNewPaper->lPic.push_back(pPic);
			pNewPaper->lSrcScanPaper.push_back(pScanPaper);

			if (!pScanPaper->bCanRecog)
				pNewPaper->nPaginationStatus = 0;	//û��ʶ��ҳ�룬���ܲ���ʶ������������˹�ȷ�Ϻ���ʶ��
			else
				pNewPaper->nPaginationStatus = 1;	//ʶ����ҳ�룬����ʶ�𣬲���ȷ�����������ĸ�ѧ��(Ĭ��)

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
	return pNewPaper;
}

void CMultiPageMgr::ChkPapersValid(pPAPERSINFO pPapers)
{

}

void CMultiPageMgr::ChkPaperValid(pST_PaperInfo pPaper, pMODEL pModel)
{
	if (!pModel) return;

	if (pPaper->lPic.size() != pModel->vecPaperModel.size())
		pPaper->nPaginationStatus = 3;
	else
	{
		//ͼƬ��ҳ���Ƿ�����ظ���
		int nLastPicIndex = -1;
		PIC_LIST::iterator itPic = pPaper->lPic.begin();		//ͼƬ�ڲ����ʱ���Ѿ���������
		for (; itPic != pPaper->lPic.end(); itPic++)
		{
			pST_PicInfo pPic = *itPic;
			if (nLastPicIndex == pPic->nPicModelIndex)
			{
				pPaper->nPaginationStatus = 4;
				break;
			}
			nLastPicIndex = pPic->nPicModelIndex;
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
					strTmpLog = Poco::format("ԭʼͼƬ(%s)������Ϊ(%s)ʧ��, ԭ��: %s\n", pPic->strPicName, strNewPicName, std::string(exc.what()));
					strLog.append(strTmpLog);
				}
			}
		}
	}
	_strLog.append(strLog);
}


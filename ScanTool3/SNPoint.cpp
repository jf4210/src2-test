#include "stdafx.h"
#include "SNPoint.h"


using namespace cv;
bool CSNPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;
	if (pModel->nZkzhType == 2)
		bResult = RecogSn_code(nPic, matCompPic, pPic, pModel, strLog);
	else
		bResult = RecogSn_omr(nPic, matCompPic, pPic, pModel, strLog);
	return bResult;
}

bool CSNPoint::RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog)
{
	clock_t start, end;
	start = clock();
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	bool bResult = true;
	SNLIST::iterator itSN = pModel->vecPaperModel[nPic]->lSNInfo.begin();
	for (; itSN != pModel->vecPaperModel[nPic]->lSNInfo.end(); itSN++)
	{
		pSN_ITEM pSnItem = *itSN;

		pSN_ITEM pSn = new SN_ITEM;
		pSn->nItem = 0;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lSnResult.push_back(pSn);

		RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
		for (; itSnItem != pSnItem->lSN.end(); itSnItem++)
		{
			RECTINFO rc = *itSnItem;
			pSn->lSN.push_back(rc);

			try
			{
				if (rc.rt.x < 0) rc.rt.x = 0;
				if (rc.rt.y < 0) rc.rt.y = 0;
				if (rc.rt.br().x > matCompPic.cols)
					rc.rt.width = matCompPic.cols - rc.rt.x;
				if (rc.rt.br().y > matCompPic.rows)
					rc.rt.height = matCompPic.rows - rc.rt.y;

				Mat matCompRoi;
				matCompRoi = matCompPic(rc.rt);

				if (matCompRoi.channels() == 3)
					cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

				string strTypeName;
				string strResult = GetQR(matCompRoi, strTypeName);

				if (strResult.empty() && _nUseOcrRecogSn_)
				{
					//RecogSN_code_Character(matCompPic, rc, pPic, pModel->vecPaperModel[nPic]->nPicSaveRotation, strResult);
				}

				std::string strTmpLog;
				if (strResult != "")
				{
					strTmpLog = "ʶ��׼��֤�����(" + strResult + "), ͼƬ��: " + pPic->strPicName;
					//(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
				}
				else
				{
					strTmpLog = "ʶ��׼��֤��ʧ��, ͼƬ��:" + pPic->strPicName;
					//					bResult = false;
				}
				(static_cast<pST_PaperInfo>(pPic->pPaper))->strSN = strResult;
				pPic->strPicZKZH = strResult;
				strLog.append(strTmpLog);
			}
			catch (cv::Exception& exc)
			{
				std::string strTmpLog = "ʶ���ά�������ʧ��(" + pPic->strPicName + "): " + exc.msg;
				strLog.append(strTmpLog);
				break;
			}
			catch (...)
			{
				std::string strTmpLog = "ʶ���ά�������ʧ��(" + pPic->strPicName + "), δ֪�쳣.\n";
				strLog.append(strTmpLog);
				break;
			}
		}
	}

	pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	for (auto pTmpPic : pCurrentPaper->lPic)	//�޸ĵ�ǰ�Ծ�ҳ������ɨ���Ծ����һҳ��׼��֤�ź͵�ǰҳһ��
	{
		if (pTmpPic != pPic && pTmpPic->pSrcScanPic != NULL && pPic->pSrcScanPic != NULL && pTmpPic->pSrcScanPic->pParentScanPaper == pPic->pSrcScanPic->pParentScanPaper)
		{
			pTmpPic->strPicZKZH = pPic->strPicZKZH;
			break;
		}
	}

	// 	if ((static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.empty())
	// 	{
	// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
	// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
	// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
	// 	}
	end = clock();
	std::string strTime = Poco::format("ʶ�𿼺�ʱ��: %dms\n", (int)(end - start));
	strLog.append(strTime);
	//g_pLogger->information(strLog);
	return bResult;
}

bool CSNPoint::RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog)
{
	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecSN;
	std::stringstream ssLog;

	clock_t start, end;
	start = clock();
	ssLog << "��ʼʶ��׼��֤(omr)[" << pPic->strPicName << "]:\n";

	if (pModel->vecPaperModel[nPic]->lSNInfo.size() == 0)
	{
		ssLog << "\tû�п�����Ҫʶ��.\n";
		strLog.append(ssLog.str());
		return true;
	}

	for (auto itSn : (static_cast<pST_PaperInfo>(pPic->pPaper))->lSnResult)
	{
		pSN_ITEM pSNItem = itSn;
		SAFE_RELEASE(pSNItem);
	}
	(static_cast<pST_PaperInfo>(pPic->pPaper))->lSnResult.clear();

	SNLIST::iterator itSN = pModel->vecPaperModel[nPic]->lSNInfo.begin();
	for (; itSN != pModel->vecPaperModel[nPic]->lSNInfo.end(); itSN++)
	{
		pSN_ITEM pSnItem = *itSN;

		pSN_ITEM pSn = new SN_ITEM;
		pSn->nItem = pSnItem->nItem;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lSnResult.push_back(pSn);

		std::vector<int> vecItemVal;
		RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
		for (; itSnItem != pSnItem->lSN.end(); itSnItem++)
		{
			RECTINFO rc = *itSnItem;

			AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
#if 1
			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModel, strLog);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > rc.fStandardValuePercent)
					vecItemVal.push_back(rc.nSnVal);
			}
			char szTmp[300] = { 0 };
			sprintf_s(szTmp, "ͼƬ��: %s, SN: ��%dλ, ѡ��=%d, ʶ��ʵ�ʱ���=%.3f, val=%.2f, ʶ���׼=%.3f, val=%.2f, �Ƿ�ɹ�:%d\n", pPic->strPicName.c_str(), \
					  pSnItem->nItem, rc.nSnVal, rc.fRealValuePercent, rc.fRealValue, rc.fStandardValuePercent, rc.fStandardValue, rc.fRealValuePercent > rc.fStandardValuePercent);
			//			TRACE(szTmp);
#else
			bool bResult_Recog = RecogVal(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog)
			{
				vecItemVal.push_back(rc.nSnVal);
			}
#endif
			pSn->lSN.push_back(rc);

#ifdef PrintWordRecogPoint
			if (pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() > 0 && pPic->lModelWordFix.size() > 0)
			{
				VEC_NEWRTBY2FIX vecNewRt;
				RECTLIST::iterator itFix = pPic->lFix.begin();
				RECTLIST::iterator itModelFix = pPic->lModelWordFix.begin();
				itFix++;
				itModelFix++;
				for (int i = 1; itFix != pPic->lFix.end(); itFix++, itModelFix++, i++)
				{
					RECTLIST lTmpFix, lTmpModelFix;
					lTmpFix.push_back(pPic->lFix.front());
					lTmpModelFix.push_back(pPic->lModelWordFix.front());

					lTmpFix.push_back(*itFix);
					lTmpModelFix.push_back(*itModelFix);

					ST_NEWRTBY2FIX stNewRt;
					stNewRt.nFirstFix = 0;
					stNewRt.nSecondFix = i;
					stNewRt.rt = itSnItem->rt;
					GetPosition(lTmpFix, lTmpModelFix, stNewRt.rt);
					vecNewRt.push_back(stNewRt);
				}
				for (auto newRt : vecNewRt)
				{
					RECTINFO rcTmp;
					rcTmp = rc;
					rcTmp.rt = newRt.rt;

					pPic->lCalcRect.push_back(rcTmp);
				}
			}
#endif
#ifdef PaintOmrSnRect	//��ӡOMR��SNλ��
			pPic->lNormalRect.push_back(rc);
#endif
		}
		if (!bResult)
			break;

#if 1	//����ѡ���ֵ�ж�ѡ��
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecSnItemDiff;
		calcDensityDiffVal(pSn->lSN, vecItemsDesc, vecSnItemDiff);

		float fCompThread = 0.0;		//�Ҷȼ���ﵽҪ��ʱ����һ��ѡ��ĻҶȱ���ﵽ��Ҫ��
		float fDiffThread = 0.0;		//ѡ�������Ϳ�Ŀ��ܻҶ��ݶȷ�ֵ
		float fDiffExit = 0;			//�Ҷȵ��ݶȵݼ�̫��ʱ��������Ϊ����ѡ��û����Ϳ����ʱ�ĻҶ��ݶȷ�ֵ
		if (pModel->nHasHead)
		{
			fCompThread = _dCompThread_Head_;
			fDiffThread = _dDiffThread_Head_;
			fDiffExit = _dDiffExit_Head_;
		}
		else
		{
			fCompThread = _dCompThread_Fix_;
			fDiffThread = _dDiffThread_Fix_;
			fDiffExit = _dDiffExit_Fix_;
		}

		int nFlag = -1;
		float fThreld = 0.0;

		vecItemVal.clear();
		if (vecItemsDesc[0]->fRealValuePercent > fCompThread)
		{
			vecItemVal.push_back(vecItemsDesc[0]->nSnVal);
		}

		if (vecItemVal.size() != 1)
		{
			ssLog << "\t��" << pSn->nItem << "λSN[";
			for (int i = 0; i < vecSnItemDiff.size(); i++)
			{
				char szTmp[15] = { 0 };
				sprintf_s(szTmp, "%s:%.5f ", vecSnItemDiff[i].szVal, vecSnItemDiff[i].fDiff);
				ssLog << szTmp;
			}
			ssLog << "]\n";
		}
#endif

		if (vecItemVal.size() == 1)
		{
			pSn->nRecogVal = vecItemVal[0];
			vecSN.push_back(vecItemVal[0]);
			for (auto itSn : pSn->lSN)
				if (itSn.nSnVal == pSn->nRecogVal)
				{
					pSn->rt = itSn.rt;
					break;
				}
		}
		else
		{
#if 1	//�ڶ���ZKZHʶ�𷽷� test
			std::vector<int> vecItemVal2;
			RecogVal_Sn2(nPic, matCompPic, pPic, pModel, pSn, vecItemVal2);
#endif

			if (vecItemVal.size() == 0 && vecItemVal2.size() == 1)
			{
				pSn->nRecogVal = vecItemVal2[0];
				vecSN.push_back(vecItemVal2[0]);
				for (auto itSn : pSn->lSN)
					if (itSn.nSnVal == pSn->nRecogVal)
					{
						pSn->rt = itSn.rt;
						break;
					}
			}
			else
			{
				std::vector<int> vecItemVal3;
				RecogVal_Sn3(pSn, vecItemVal3);
				if (vecItemVal3.size())
				{
					pSn->nRecogVal = vecItemVal3[0];
					vecSN.push_back(vecItemVal3[0]);
					for (auto itSn : pSn->lSN)
						if (itSn.nSnVal == pSn->nRecogVal)
						{
							pSn->rt = itSn.rt;
							break;
						}
				}
				else
				{
					bRecogAll = false;
					char szVal[21] = { 0 };
					for (int i = 0; i < vecItemVal.size(); i++)
					{
						char szTmp[3] = { 0 };
						sprintf_s(szTmp, "%d ", vecItemVal[i]);
						strcat(szVal, szTmp);
					}
					char szLog[MAX_PATH] = { 0 };
					sprintf_s(szLog, "\tʶ��׼��֤�ŵ�%dλʧ��,ʶ������%dλ(%s), ͼƬ��: %s\n", pSnItem->nItem, vecItemVal.size(), szVal, pPic->strPicName.c_str());
					ssLog << szLog;
					TRACE(szLog);

					vecSN.push_back(-1);
				}
			}
		}
	}
	if (bRecogAll && vecSN.size() > 0)
	{
		for (int i = 0; i < vecSN.size(); i++)
		{
			char szTmp[5] = { 0 };
			itoa(vecSN[i], szTmp, 10);
			(static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.append(szTmp);
			pPic->strPicZKZH.append(szTmp);
		}
		pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
		for (auto pTmpPic : pCurrentPaper->lPic)	//�޸ĵ�ǰ�Ծ�ҳ������ɨ���Ծ����һҳ��׼��֤�ź͵�ǰҳһ��
		{
			if (pTmpPic != pPic && pTmpPic->pSrcScanPic != NULL && pPic->pSrcScanPic != NULL && pTmpPic->pSrcScanPic->pParentScanPaper == pPic->pSrcScanPic->pParentScanPaper)
			{
				pTmpPic->strPicZKZH = pPic->strPicZKZH;
				break;
			}
		}

		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��׼��֤�����(%s), ͼƬ��: %s\n", (static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.c_str(), pPic->strPicName.c_str());
		ssLog << "\tʶ��׼��֤�����(" << (static_cast<pST_PaperInfo>(pPic->pPaper))->strSN << "), ͼƬ[" << pPic->strPicName << "]\n";
		TRACE(szLog);
		//(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
	}
	else
	{
		bool bAllEmpty = true;
		for (int i = 0; i < vecSN.size(); i++)
		{
			char szTmp[5] = { 0 };
			if (vecSN[i] >= 0)
			{
				itoa(vecSN[i], szTmp, 10);
				bAllEmpty = false;
			}
			else
				szTmp[0] = '#';		//δʶ������Ļ���ʶ�𵽶������#���棬������ģ������
			if (!bAllEmpty)			//ֻҪʶ��һ���֣��ͽ���ʶ�𵽵Ľ������ģ�������ֶ���
				(static_cast<pST_PaperInfo>(pPic->pPaper))->strRecogSN4Search.append(szTmp);
		}
		//(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
	}
// 	if (!bRecogAll)
// 	{
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
// 	}
	if (!bResult)
	{
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();

		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��׼��֤��ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		ssLog << "ʶ��׼��֤��ʧ��, ͼƬ��: " << pPic->strPicName << "\n";
		TRACE(szLog);
	}
	end = clock();
	ssLog << "\tʶ�𿼺�ʱ��: " << end - start << "ms\n";
	strLog.append(ssLog.str());
	return bResult;
}

bool CSNPoint::RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, pSN_ITEM pSn, std::vector<int>& vecItemVal)
{
	std::string strResult;
	RecogVal2(nPic, matCompPic, pPic, pModel, pSn->lSN, strResult);
	char* p = NULL;
	for (int i = 0; i < strResult.length(); i++)
	{
		p = const_cast<char*>(strResult.c_str() + i);
		vecItemVal.push_back(atoi(p));
	}
	return true;
}

bool CSNPoint::RecogVal_Sn3(pSN_ITEM pSn, std::vector<int>& vecItemVal)
{
	std::vector<int> vecVal_AnswerSuer;
	RECTLIST::iterator itItem = pSn->lSN.begin();
	for (; itItem != pSn->lSN.end(); itItem++)
	{
		if (itItem->fRealMeanGray < _dAnswerSure_)
		{
			vecVal_AnswerSuer.push_back(itItem->nSnVal);
		}
	}

	std::string strRecogAnswer;
	std::vector<pRECTINFO> vecItemsGrayAsc;
	std::vector<ST_ITEM_DIFF> vecOmrItemGrayDiff;
	calcGrayDiffVal(pSn->lSN, vecItemsGrayAsc, vecOmrItemGrayDiff);

	float fCompThread = 0.0;		//�Ҷȼ���ﵽҪ��ʱ����һ��ѡ��ĻҶȱ���ﵽ��Ҫ��
	float fDiffThread = 0.0;		//ѡ�������Ϳ�Ŀ��ܻҶ��ݶȷ�ֵ
	float fDiffExit = 0;			//�Ҷȵ��ݶȵݼ�̫��ʱ��������Ϊ����ѡ��û����Ϳ����ʱ�ĻҶ��ݶȷ�ֵ

	fCompThread = _dCompThread_3_;
	fDiffThread = _dDiffThread_3_;
	fDiffExit = _dDiffExit_3_;
#if 1
	float fMeanGrayDiff = 0.0;
	for (int i = 0; i < vecItemsGrayAsc.size(); i++)
	{
		fMeanGrayDiff += (vecItemsGrayAsc[i]->fRealMeanGray - vecItemsGrayAsc[i]->fStandardMeanGray);
	}
	fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayAsc.size();

	int nFlag = -1;
	float fThreld = 0.0;
	float fGrayDiffLast = 0.0;		//����һ���ж�ѡ�е�ѡ�����һ��ѡ��ѡ���жϵ�����
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		float fGrayThresholdGray = vecItemsGrayAsc[i]->fRealMeanGray - vecItemsGrayAsc[i]->fStandardMeanGray - fMeanGrayDiff;
		if ((vecOmrItemGrayDiff[i].fDiff >= fDiffThread + fGrayThresholdGray + fGrayDiffLast))
		{
			nFlag = i;
			fThreld = vecOmrItemGrayDiff[i].fFirst;
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//�Ҷ�ֵ�仯�ϴ�ֱ���˳��������ֱֵ���жϳ����ĸ���������ǰ�жϵ��������Ͳ���������
				break;
		}
		fGrayDiffLast += abs(fGrayThresholdGray) / 2;
	}
	if (nFlag >= 0)
	{
		if (nFlag == vecOmrItemGrayDiff.size() - 1)
		{
			if (vecItemsGrayAsc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)
			{
				fThreld = vecItemsGrayAsc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
			}
		}
		RECTLIST::iterator itItem = pSn->lSN.begin();
		for (; itItem != pSn->lSN.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				vecItemVal.push_back(itItem->nSnVal);
			}
		}
	}
	else if (vecItemsGrayAsc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)		//++�ж�ȫ��ѡ�е����
	{
		fThreld = vecItemsGrayAsc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
		RECTLIST::iterator itItem = pSn->lSN.begin();
		for (; itItem != pSn->lSN.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				vecItemVal.push_back(itItem->nSnVal);
			}
		}
	}
	else
	{
		vecItemVal = vecVal_AnswerSuer;
	}
#else
	int nFlag = -1;
	float fThreld = 0.0;
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		if ((vecOmrItemGrayDiff[i].fDiff >= fDiffThread && vecOmrItemGrayDiff[i].fFirst < fCompThread))
		{
			nFlag = i;
			fThreld = vecOmrItemGrayDiff[i].fFirst;
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//�Ҷ�ֵ�仯�ϴ�ֱ���˳��������ֱֵ���жϳ����ĸ���������ǰ�жϵ��������Ͳ���������
				break;
		}
	}
	if (nFlag >= 0)
	{
		RECTLIST::iterator itItem = pSn->lSN.begin();
		for (; itItem != pSn->lSN.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				vecItemVal.push_back(itItem->nSnVal);
			}
		}
	}
	else
	{
		vecItemVal = vecVal_AnswerSuer;
	}
#endif
	return true;
}

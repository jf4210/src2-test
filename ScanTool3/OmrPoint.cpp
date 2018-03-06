#include "stdafx.h"
#include "OmrPoint.h"


bool COmrPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	int nNullCount = 0;
	int nDoubtCount = 0;
	int nEqualCount = 0;
	int nNullCount_1 = 0;	//第一种方法识别出的空值
	int nNullCount_2 = 0;	//第二种方法识别出的空值
	int nNullCount_3 = 0;	//第三种方法识别出的空值

	clock_t start, end;
	start = clock();
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecOmr;
	OMRLIST::iterator itOmr = pModel->vecPaperModel[nPic]->lOMR2.begin();
	for (; itOmr != pModel->vecPaperModel[nPic]->lOMR2.end(); itOmr++)
	{
		pOMR_QUESTION pOmrQuestion = &(*itOmr);

		OMR_RESULT omrResult;
		omrResult.nTH = pOmrQuestion->nTH;
		omrResult.nSingle = pOmrQuestion->nSingle;
		omrResult.nPageId = nPic + 1;

		std::vector<int> vecVal_calcHist;		//直方图灰度计算的识别结果
		std::vector<int> vecVal_threshold;		//二值化计算的识别结果
		RECTLIST::iterator itOmrItem = pOmrQuestion->lSelAnswer.begin();
		for (; itOmrItem != pOmrQuestion->lSelAnswer.end(); itOmrItem++)
		{
			RECTINFO rc = *itOmrItem;

			AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);

			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModel, strLog);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > _dAnswerSure_DensityFix_)	//rc.fStandardValuePercent
				{
					vecVal_calcHist.push_back(rc.nAnswer);
				}
			}

			// 			bool bResult_Recog2 = RecogVal(nPic, rc, matCompPic, pPic, pModelInfo);
			// 			if (bResult_Recog2)
			// 			{
			// 				vecVal_threshold.push_back(rc.nAnswer);
			// 			}
			omrResult.lSelAnswer.push_back(rc);

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
					stNewRt.rt = itOmrItem->rt;
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

#ifdef PaintOmrSnRect	//打印OMR、SN位置
			pPic->lNormalRect.push_back(rc);
#endif
		}

#if 1
		std::string strRecogAnswer;
		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDiff;
		calcOmrDensityDiffVal(omrResult.lSelAnswer, vecItemsDesc, vecOmrItemDiff);

		float fCompThread = 0.0;		//密度间隔达到要求时，第一个选项的密度必须达到的要求
		float fDiffThread = 0.0;		//选项可能填涂的可能密度梯度阀值
		float fDiffExit = 0;			//密度的梯度递减太快时，可以认为后面选项没有填涂，此时的密度梯度阀值
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
		float fDensityThreshold = 0.0;
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
			//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
			float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.5;
			if ((vecOmrItemDiff[i].fDiff >= fDiffThread + fDensityThreshold * 0.5))
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
				fDensityThreshold += vecOmrItemDiff[i].fDiff;
			}
		}
		if (nFlag >= 0)
		{
			//++判断全选的情况
			if (nFlag == vecOmrItemDiff.size() - 1)
			{
				if (vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent >= fCompThread + fDiffExit)	//如果密度最低的选项，它的密度大于“最低比较密度 + 最大退出密度差”，则认为全选
					fThreld = vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent;
			}
			//--
			RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
			for (; itItem != omrResult.lSelAnswer.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					char szVal[10] = { 0 };
					sprintf_s(szVal, "%c", itItem->nAnswer + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else if (vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent >= fCompThread + fDiffExit)	//如果密度最低的选项，它的密度大于“最低比较密度 + 最大退出密度差”，则认为全选
		{
			fThreld = vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent;

			RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
			for (; itItem != omrResult.lSelAnswer.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					char szVal[10] = { 0 };
					sprintf_s(szVal, "%c", itItem->nAnswer + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else if (vecItemsDesc[0]->fRealValuePercent >= fCompThread && vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent < fCompThread + fDiffExit)
		{
			int nFlag = -1;
			float fThreld = 0.0;
			for (int i = 0; i < vecOmrItemDiff.size(); i++)
			{
				//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
				//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
				float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.1;
				if ((vecOmrItemDiff[i].fDiff >= fDiffThread && vecOmrItemDiff[i].fFirst > fCompThread) ||
					(vecOmrItemDiff[i].fDiff >= fDiffThread + fDiff && vecOmrItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
				{
					nFlag = i;
					fThreld = vecOmrItemDiff[i].fFirst;
					if (vecOmrItemDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_calcHist.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
						break;
				}
			}
			if (nFlag >= 0)
			{
				RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
				for (; itItem != omrResult.lSelAnswer.end(); itItem++)
				{
					if (itItem->fRealValuePercent >= fThreld)
					{
						char szVal[2] = { 0 };
						sprintf_s(szVal, "%c", itItem->nAnswer + 65);
						strRecogAnswer1.append(szVal);
					}
				}
			}
			else if (vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent > fCompThread && vecItemsDesc[vecOmrItemDiff.size()]->fRealMeanGray < (_dCompThread_3_ + _dDiffExit_3_ + _dAnswerSure_) / 2)	//全选，最低密度>比较密度，同时此选项的灰度<(比较灰度 + 灰度答案确认值 + 灰度退出密度值)/2
			{
				fThreld = vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent;

				RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
				for (; itItem != omrResult.lSelAnswer.end(); itItem++)
				{
					if (itItem->fRealValuePercent >= fThreld)
					{
						char szVal[10] = { 0 };
						sprintf_s(szVal, "%c", itItem->nAnswer + 65);
						strRecogAnswer1.append(szVal);
					}
				}
			}
			else
			{
				for (int i = 0; i < vecVal_calcHist.size(); i++)
				{
					char szVal[5] = { 0 };
					sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else
		{
			for (int i = 0; i < vecVal_calcHist.size(); i++)
			{
				char szVal[10] = { 0 };
				sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
				strRecogAnswer1.append(szVal);
			}
		}
#else
		std::string strRecogAnswer;
		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDiff;
		calcOmrDensityDiffVal(omrResult.lSelAnswer, vecItemsDesc, vecOmrItemDiff);

		float fCompThread = 0.0;		//灰度间隔达到要求时，第一个选项的灰度必须达到的要求
		float fDiffThread = 0.0;		//选项可能填涂的可能灰度梯度阀值
		float fDiffExit = 0;			//灰度的梯度递减太快时，可以认为后面选项没有填涂，此时的灰度梯度阀值
		if (pModelInfo->pModel->nHasHead)
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
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
			//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
			float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.1;
			if ((vecOmrItemDiff[i].fDiff >= fDiffThread && vecOmrItemDiff[i].fFirst > fCompThread) ||
				(vecOmrItemDiff[i].fDiff >= fDiffThread + fDiff && vecOmrItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
				if (vecOmrItemDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_calcHist.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
					break;
			}
		}
		if (nFlag >= 0)
		{
			RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
			for (; itItem != omrResult.lSelAnswer.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					char szVal[2] = { 0 };
					sprintf_s(szVal, "%c", itItem->nAnswer + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else
		{
			for (int i = 0; i < vecVal_calcHist.size(); i++)
			{
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
				strRecogAnswer1.append(szVal);
			}
		}
#endif

#ifdef Test_RecogOmr3
		RecogVal_Omr2(nPic, matCompPic, pPic, pModel, omrResult);
		RecogVal_Omr3(nPic, matCompPic, pPic, pModel, omrResult);

		std::string strRecogAnswer2 = omrResult.strRecogVal2;
		std::string strRecogAnswer3 = omrResult.strRecogVal3;

		if (strRecogAnswer1 == "") nNullCount_1++;
		if (strRecogAnswer2 == "") nNullCount_2++;
		if (strRecogAnswer3 == "") nNullCount_3++;

		int nDoubt = 0;
		if (strRecogAnswer1 == strRecogAnswer2 && strRecogAnswer1 == strRecogAnswer3)	//为空判断时，方法1与方法3准确度更高
		{
			if (strRecogAnswer1 == "")
			{
				strRecogAnswer = strRecogAnswer1;
				nDoubt = 2;
				nNullCount++;

				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrNull++;
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
			}
			else
			{
				strRecogAnswer = strRecogAnswer1;
				nDoubt = 0;
				nEqualCount++;
			}
		}
		else
		{
			if (strRecogAnswer1 == "")
			{
				if (strRecogAnswer2 == strRecogAnswer3)
				{
					strRecogAnswer = strRecogAnswer3;
					nDoubt = 0;
					nEqualCount++;
				}
				else
				{
					//方法2、方法3两种方法有一种方法判断非空
					if (strRecogAnswer2 == "" && strRecogAnswer3 != "")
					{
						strRecogAnswer = strRecogAnswer3;
						nDoubt = 1;
						nDoubtCount++;

						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrDoubt++;
						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
					}
					else if (strRecogAnswer3 == "" && strRecogAnswer2 != "")
					{
						strRecogAnswer = strRecogAnswer3;
						nDoubt = 2;
						nEqualCount++;
					}
					else if (strRecogAnswer2 != "" && strRecogAnswer3 != "")
					{
						strRecogAnswer = strRecogAnswer3;
						nDoubt = 1;
						nDoubtCount++;

						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrDoubt++;
						(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
					}
				}
			}
			else
			{
				if (strRecogAnswer1 == strRecogAnswer2 || strRecogAnswer1 == strRecogAnswer3)
				{
					strRecogAnswer = strRecogAnswer1;
					nDoubt = 0;
					nEqualCount++;
				}
				else if ((strRecogAnswer1 != strRecogAnswer2) && (strRecogAnswer2 == strRecogAnswer3 && strRecogAnswer3 != ""))		//2017.10.20
				{
					strRecogAnswer = strRecogAnswer3;
					nDoubt = 0;
					nEqualCount++;
				}
				else
				{
					strRecogAnswer = strRecogAnswer1;
					nDoubt = 1;
					nDoubtCount++;

					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrDoubt++;
					(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
				}
			}
		}
#else
		//++ test	测试整题选项进行二值化识别
		RecogVal_Omr2(nPic, matCompPic, pPic, pModelInfo, omrResult);
		//--
		std::string strRecogAnswer2 = omrResult.strRecogVal2;

		if (strRecogAnswer1 == "") nNullCount_1++;
		if (strRecogAnswer2 == "") nNullCount_2++;

		int nDoubt = 0;
		if (strRecogAnswer1 == "" && strRecogAnswer2 == "")
		{
			nDoubt = 2;
			nNullCount++;

			(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
			(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrNull++;
			(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
		}
		else
		{
			if (strRecogAnswer1 == strRecogAnswer2)
			{
				nDoubt = 0;
				nEqualCount++;
			}
			else
			{
				nDoubt = 1;
				nDoubtCount++;

				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.lock();
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nOmrDoubt++;
				(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmOmrStatistics.unlock();
			}
		}
#endif
		omrResult.nDoubt = nDoubt;
		omrResult.strRecogVal = strRecogAnswer;
		omrResult.strRecogVal1 = strRecogAnswer1;
		omrResult.strRecogVal2 = strRecogAnswer2;
		omrResult.strRecogVal3 = strRecogAnswer3;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lOmrResult.push_back(omrResult);
		pPic->lOmrResult.push_back(omrResult);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别OMR失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}

	int nCount = pModel->vecPaperModel[nPic]->lOMR2.size();

	// 	std::stringstream ss;
	// 	ss << "总数" << nCount << ",空值" << nNullCount << "(" << (double)nNullCount / nCount * 100 << "%)[No.1=" << nNullCount_1 << "(" << (double)nNullCount_1 / nCount * 100 << "%),No.2=" << nNullCount_2
	// 		<< "(" << (double)nNullCount_2 / nCount * 100 << "%)],怀疑" << nDoubtCount << "(" << (double)nDoubtCount / nCount * 100 << "%),无怀疑" << nEqualCount << "(" << (double)nEqualCount / nCount * 100
	// 		<< "%)";
	// 	std::string strTmp = ss.str();

	if (bResult && nCount)
	{
		char szStatistics[150] = { 0 };
		sprintf_s(szStatistics, "图片(%s)选项总数[%d],空值%d(%.2f%%)[No.1=%d(%.2f%%),No.2=%d(%.2f%%), No.3=%d(%.2f%%)],怀疑%d(%.2f%%),无怀疑%d(%.2f%%)", pPic->strPicName.c_str(), nCount, nNullCount, (float)nNullCount / nCount * 100, \
				  nNullCount_1, (float)nNullCount_1 / nCount * 100, nNullCount_2, (float)nNullCount_2 / nCount * 100, nNullCount_3, (float)nNullCount_3 / nCount * 100, \
				  nDoubtCount, (float)nDoubtCount / nCount * 100, nEqualCount, (float)nEqualCount / nCount * 100);

		strLog.append(szStatistics);
	}
	end = clock();
	std::string strTime = Poco::format("识别Omr时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	//g_pLogger->information(strLog);
	return bResult;
}

bool COmrPoint::RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, OMR_RESULT& omrResult)
{
	return RecogVal2(nPic, matCompPic, pPic, pModel, omrResult.lSelAnswer, omrResult.strRecogVal2);
}

bool COmrPoint::RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, OMR_RESULT& omrResult)
{
	std::vector<int> vecVal_AnswerSuer;
	RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
	for (; itItem != omrResult.lSelAnswer.end(); itItem++)
	{
		if (itItem->fRealMeanGray < _dAnswerSure_)
		{
			vecVal_AnswerSuer.push_back(itItem->nAnswer);
		}
	}

	std::string strRecogAnswer;
	std::vector<pRECTINFO> vecItemsGrayDesc;
	std::vector<ST_ITEM_DIFF> vecOmrItemGrayDiff;
	calcOmrGrayDiffVal(omrResult.lSelAnswer, vecItemsGrayDesc, vecOmrItemGrayDiff);

	float fCompThread = 0.0;		//灰度间隔达到要求时，第一个选项的灰度必须达到的要求
	float fDiffThreshold = 0.0;		//选项可能填涂的可能灰度梯度阀值
	float fDiffExit = 0;			//灰度的梯度递减太快时，可以认为后面选项没有填涂，此时的灰度梯度阀值

	fCompThread = _dCompThread_3_;
	fDiffThreshold = _dDiffThread_3_;
	fDiffExit = _dDiffExit_3_;


#if 1		//灰度差值满足后，加上单项Omr的灰度需要比标准的小
	float fMeanGrayDiff = 0.0;
	for (int i = 0; i < vecItemsGrayDesc.size(); i++)
	{
		fMeanGrayDiff += (vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray);
	}
	fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayDesc.size();

	int nFlag = -1;
	float fThreld = 0.0;
	float fGrayDiffLast = 0.0;		//对上一次判断选中的选项对下一个选项选中判断的增益
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		float fGrayThresholdGray = vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray - fMeanGrayDiff;
		if (vecOmrItemGrayDiff[i].fDiff >= fDiffThreshold + fGrayThresholdGray + fGrayDiffLast)		//vecOmrItemGrayDiff[i].fDiff >= fDiffThreshold + fGrayThresholdGray
		{
			nFlag = i;
			fThreld = vecOmrItemGrayDiff[i].fFirst;
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
				break;
		}
		fGrayDiffLast += abs(fGrayThresholdGray) / 2;
	}
	if (nFlag >= 0)
	{
		//++判断全都选中的情况
		if (nFlag == vecOmrItemGrayDiff.size() - 1)
		{
			if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)
			{
				fThreld = vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
			}
		}
		//--
		RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
		for (; itItem != omrResult.lSelAnswer.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				char szVal[2] = { 0 };
				sprintf_s(szVal, "%c", itItem->nAnswer + 65);
				strRecogAnswer.append(szVal);
			}
		}
	}
	else if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)		//++判断全都选中的情况
	{
		fThreld = vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
		RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
		for (; itItem != omrResult.lSelAnswer.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				char szVal[2] = { 0 };
				sprintf_s(szVal, "%c", itItem->nAnswer + 65);
				strRecogAnswer.append(szVal);
			}
		}
	}
	else
	{
		for (int i = 0; i < vecVal_AnswerSuer.size(); i++)
		{
			char szVal[5] = { 0 };
			sprintf_s(szVal, "%c", vecVal_AnswerSuer[i] + 65);
			strRecogAnswer.append(szVal);
		}
	}
#else
	int nFlag = -1;
	float fThreld = 0.0;
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		if ((vecOmrItemGrayDiff[i].fDiff >= fDiffThreshold && vecOmrItemGrayDiff[i].fFirst < fCompThread))
		{
			nFlag = i;
			fThreld = vecOmrItemGrayDiff[i].fFirst;
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
				break;
		}
	}
	if (nFlag >= 0)
	{
		RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
		for (; itItem != omrResult.lSelAnswer.end(); itItem++)
		{
			if (itItem->fRealMeanGray <= fThreld)
			{
				char szVal[2] = { 0 };
				sprintf_s(szVal, "%c", itItem->nAnswer + 65);
				strRecogAnswer.append(szVal);
			}
		}
	}
	else
	{
		for (int i = 0; i < vecVal_AnswerSuer.size(); i++)
		{
			char szVal[5] = { 0 };
			sprintf_s(szVal, "%c", vecVal_AnswerSuer[i] + 65);
			strRecogAnswer.append(szVal);
		}
	}
#endif
	omrResult.strRecogVal3 = strRecogAnswer;
	return true;
}

int COmrPoint::calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff)
{
#if 1	//下面将所有选项识别灰度值降序排列并相邻比较
	RECTLIST::iterator itItem = rectList.begin();
	for (; itItem != rectList.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), [](pRECTINFO item1, pRECTINFO item2)
	{
		return item1->fRealValuePercent > item2->fRealValuePercent ? true : false;
	});

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsDesc[i]->nAnswer + 65), (char)(vecItemsDesc[j]->nAnswer + 65));
			stDiff.fDiff = vecItemsDesc[i]->fRealValuePercent - vecItemsDesc[j]->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
#else	//下面是整题所有选项的两两识别灰度值的比较并按降序排列
	RECTLIST::iterator itFirst = rectList.begin();
	for (; itFirst != rectList.end(); itFirst++)
	{
		RECTLIST::iterator itSecond = itFirst;
		itSecond++;
		for (; itSecond != rectList.end(); itSecond++)
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(itFirst->nAnswer + 65), (char)(itSecond->nAnswer + 65));
			stDiff.fDiff = itFirst->fRealValuePercent - itSecond->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
	std::sort(vecOmrItemDiff.begin(), vecOmrItemDiff.end(), SortByItemDiff);
#endif
	return 1;
}

int COmrPoint::calcOmrGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemGrayDiff)
{
	RECTLIST::iterator itItem = rectList.begin();
	for (; itItem != rectList.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), [](pRECTINFO item1, pRECTINFO item2)
	{
		return item1->fRealMeanGray < item2->fRealMeanGray ? true : false;
	});

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsDesc[i]->nAnswer + 65), (char)(vecItemsDesc[j]->nAnswer + 65));
			stDiff.fDiff = abs(vecItemsDesc[i]->fRealMeanGray - vecItemsDesc[j]->fRealMeanGray);
			stDiff.fFirst = vecItemsDesc[i]->fRealMeanGray;
			stDiff.fSecond = vecItemsDesc[j]->fRealMeanGray;
			vecOmrItemGrayDiff.push_back(stDiff);
		}
	}
	return 1;
}

bool CElectOmrPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;

	std::vector<int> vecOmr;
	ELECTOMR_LIST::iterator itElectOmr = pModel->vecPaperModel[nPic]->lElectOmr.begin();
	for (; itElectOmr != pModel->vecPaperModel[nPic]->lElectOmr.end(); itElectOmr++)
	{
		pELECTOMR_QUESTION pOmrQuestion = &(*itElectOmr);

		ELECTOMR_QUESTION omrResult;
		omrResult.nPageId = nPic + 1;
		omrResult.sElectOmrGroupInfo = pOmrQuestion->sElectOmrGroupInfo;

		std::vector<int> vecVal_calcHist;		//直方图灰度计算的识别结果
		std::vector<int> vecVal_threshold;		//二值化计算的识别结果
		RECTLIST::iterator itOmrItem = pOmrQuestion->lItemInfo.begin();
		for (; itOmrItem != pOmrQuestion->lItemInfo.end(); itOmrItem++)
		{
			RECTINFO rc = *itOmrItem;

			AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);

			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModel, strLog);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > rc.fStandardValuePercent)
				{
					vecVal_calcHist.push_back(rc.nAnswer);
				}
			}

			// 			bool bResult_Recog2 = RecogVal(nPic, rc, matCompPic, pPic, pModelInfo);
			// 			if (bResult_Recog2)
			// 			{
			// 				vecVal_threshold.push_back(rc.nAnswer);
			// 			}
			omrResult.lItemInfo.push_back(rc);

#ifdef PaintOmrSnRect	//打印OMR、SN位置
			pPic->lNormalRect.push_back(rc);
#endif
		}

		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDiff;
		calcOmrDensityDiffVal(omrResult.lItemInfo, vecItemsDesc, vecOmrItemDiff);

		float fCompThread = 0.0;		//灰度间隔达到要求时，第一个选项的灰度必须达到的要求
		float fDiffThread = 0.0;		//选项可能填涂的可能灰度梯度阀值
		float fDiffExit = 0;			//灰度的梯度递减太快时，可以认为后面选项没有填涂，此时的灰度梯度阀值
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
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
			//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
			float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.1;
			if ((vecOmrItemDiff[i].fDiff >= fDiffThread && vecOmrItemDiff[i].fFirst > fCompThread) ||
				(vecOmrItemDiff[i].fDiff >= fDiffThread + fDiff && vecOmrItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
				if (vecOmrItemDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_calcHist.size())	//灰度值变化较大，直接退出，如果阀值直接判断出来的个数超过当前判断的数量，就不能马上退
					break;
			}
		}
		if (nFlag >= 0)
		{
			RECTLIST::iterator itItem = omrResult.lItemInfo.begin();
			for (; itItem != omrResult.lItemInfo.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					char szVal[2] = { 0 };
					sprintf_s(szVal, "%c", itItem->nAnswer + 65);
					strRecogAnswer1.append(szVal);
				}
			}
		}
		else
		{
			for (int i = 0; i < vecVal_calcHist.size(); i++)
			{
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
				strRecogAnswer1.append(szVal);
			}
		}

		std::string strRecogAnswer2 = strRecogAnswer1;		//目前第二种方法不可用
		int nDoubt = 0;
		if (strRecogAnswer1 == "" && strRecogAnswer2 == "")
			nDoubt = 2;
		else
		{
			if (strRecogAnswer1 == strRecogAnswer2)
				nDoubt = 0;
			else
				nDoubt = 1;
		}

		omrResult.nDoubt = nDoubt;
		omrResult.strRecogResult = strRecogAnswer1;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lElectOmrResult.push_back(omrResult);
		pPic->lElectOmrResult.push_back(omrResult);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别选做题OMR失败, 图片名: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}

	return bResult;
}

int CElectOmrPoint::calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff)
{
#if 1	//下面将所有选项识别灰度值降序排列并相邻比较
	RECTLIST::iterator itItem = rectList.begin();
	for (; itItem != rectList.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), [](pRECTINFO item1, pRECTINFO item2)
	{
		return item1->fRealValuePercent > item2->fRealValuePercent ? true : false;
	});

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsDesc[i]->nAnswer + 65), (char)(vecItemsDesc[j]->nAnswer + 65));
			stDiff.fDiff = vecItemsDesc[i]->fRealValuePercent - vecItemsDesc[j]->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
#else	//下面是整题所有选项的两两识别灰度值的比较并按降序排列
	RECTLIST::iterator itFirst = rectList.begin();
	for (; itFirst != rectList.end(); itFirst++)
	{
		RECTLIST::iterator itSecond = itFirst;
		itSecond++;
		for (; itSecond != rectList.end(); itSecond++)
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(itFirst->nAnswer + 65), (char)(itSecond->nAnswer + 65));
			stDiff.fDiff = itFirst->fRealValuePercent - itSecond->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
	std::sort(vecOmrItemDiff.begin(), vecOmrItemDiff.end(), SortByItemDiff);
#endif
	return 1;
}

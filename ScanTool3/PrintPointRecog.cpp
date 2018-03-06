#include "stdafx.h"
#include "PrintPointRecog.h"
#include "CoordinationConvert.h"

using namespace cv;

bool CCharacterPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;

	clock_t start, end;
	start = clock();
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);
	if (pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() == 0)
	{
		strLog = Poco::format("ͼƬ%sû�����ֶ�λ����Ҫʶ��", pPic->strPicName);
		//g_pLogger->information(strLog);
		return true;
	}

#ifdef USE_TESSERACT
	if (NULL == m_pTess && !InitCharacterRecog())
	{
		strLog.append("tesseract����Ϊʵ����\n");
		return false;
	}
#endif

	CHARACTER_ANCHOR_AREA_LIST::iterator itBigRecogCharRt = pModel->vecPaperModel[nPic]->lCharacterAnchorArea.begin();
	for (int i = 0; itBigRecogCharRt != pModel->vecPaperModel[nPic]->lCharacterAnchorArea.end(); i++, itBigRecogCharRt++)
	{
		pST_CHARACTER_ANCHOR_AREA pstBigRecogCharRt = *itBigRecogCharRt;

		std::vector<Rect>RectCompList;
		try
		{
			if (pstBigRecogCharRt->rt.x < 0) pstBigRecogCharRt->rt.x = 0;
			if (pstBigRecogCharRt->rt.y < 0) pstBigRecogCharRt->rt.y = 0;
			if (pstBigRecogCharRt->rt.br().x > matCompPic.cols)
			{
				pstBigRecogCharRt->rt.width = matCompPic.cols - pstBigRecogCharRt->rt.x;
			}
			if (pstBigRecogCharRt->rt.br().y > matCompPic.rows)
			{
				pstBigRecogCharRt->rt.height = matCompPic.rows - pstBigRecogCharRt->rt.y;
			}

			Mat matCompRoi;
			matCompRoi = matCompPic(pstBigRecogCharRt->rt);		//������

			Mat matCompRoi2 = matCompRoi.clone();
			if (matCompRoi2.channels() == 3)
				cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);

			switch (pModel->vecPaperModel[nPic]->nPicSaveRotation)
			{
				case 1:	break;
				case 3:			//ͼ����Ҫ�������������Ӿ�����(������)����ģ��������90����ʵ��ͼƬ������90
					{
						cv::Mat dst;
						transpose(matCompRoi2, dst);	//����90������ 
						flip(dst, matCompRoi2, 0);	//����90��ģ��ͼ����Ҫ����90��ԭͼ����Ҫ����90
					}
					break;
				case 2:			//ͼ����Ҫ�������������Ӿ�����(������)����ģ��������90����ʵ��ͼƬ������90
					{
						cv::Mat dst;
						transpose(matCompRoi2, dst);	//����90������ 
						flip(dst, matCompRoi2, 1);	//����90��ģ��ͼ����Ҫ����90��ԭͼ����Ҫ����90
					}
					break;
				case 4:
					{
						cv::Mat dst;
						transpose(matCompRoi2, dst);	//����90������ 
						cv::Mat dst2;
						flip(dst, dst2, 1);
						cv::Mat dst5;
						transpose(dst2, dst5);
						flip(dst5, matCompRoi2, 1);	//����180
					}
					break;
				default:
					break;
			}

			GaussianBlur(matCompRoi2, matCompRoi2, cv::Size(pstBigRecogCharRt->nGaussKernel, pstBigRecogCharRt->nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi2, matCompRoi2, pstBigRecogCharRt->nSharpKernel);

			//double dThread = threshold(matCompRoi, matCompRoi, pstBigRecogCharRt->nThresholdValue, 255, THRESH_OTSU | THRESH_BINARY);
			cv::adaptiveThreshold(matCompRoi2, matCompRoi2, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 19, 20);	//blockSize, constValue

			CCoordinationConvert convertObj(matCompPic);	//����ת������
			cv::Rect rtShowRect = convertObj.GetShowFakePosRect(pstBigRecogCharRt->rt, pModel->vecPaperModel[nPic]->nPicSaveRotation);
#ifdef USE_TESSERACT
			m_pTess->SetImage((uchar*)matCompRoi2.data, matCompRoi2.cols, matCompRoi2.rows, 1, matCompRoi2.cols);

			std::string strWhiteList;
			for (auto itChar : pstBigRecogCharRt->vecCharacterRt)
				strWhiteList.append(itChar->strVal);
			m_pTess->SetVariable("tessedit_char_whitelist", CMyCodeConvert::Gb2312ToUtf8(strWhiteList).c_str());

			m_pTess->Recognize(0);

			tesseract::ResultIterator* ri = m_pTess->GetIterator();
			tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;	//RIL_WORD
			if (ri != 0)
			{
				int nIndex = 1;

				pST_CHARACTER_ANCHOR_AREA pstRecogCharacterRt = new ST_CHARACTER_ANCHOR_AREA();
				pstRecogCharacterRt->nIndex = pstBigRecogCharRt->nIndex;
				pstRecogCharacterRt->nThresholdValue = pstBigRecogCharRt->nThresholdValue;
				pstRecogCharacterRt->nGaussKernel = pstBigRecogCharRt->nGaussKernel;
				pstRecogCharacterRt->nSharpKernel = pstBigRecogCharRt->nSharpKernel;
				pstRecogCharacterRt->nCannyKernel = pstBigRecogCharRt->nCannyKernel;
				pstRecogCharacterRt->nDilateKernel = pstBigRecogCharRt->nDilateKernel;
				pstRecogCharacterRt->nCharacterConfidence = pstBigRecogCharRt->nCharacterConfidence;
				pstRecogCharacterRt->rt = pstBigRecogCharRt->rt;	//������

				convertObj.SetPicRect(matCompPic.rows, matCompPic.cols);	//�ı����ͼ��Ĵ�С���������Ӽ����굽������ʱ����׼ͼ��Ĵ�С�Ѿ��仯

																			//�ظ�����ʱ�Ǽ��б�����ɾ�������ظ�����
				std::vector<std::string> vecRepeatWord;
				do
				{
					const char* word = ri->GetUTF8Text(level);
					float conf = ri->Confidence(level);
					if (word && strcmp(word, " ") != 0 && conf >= pstRecogCharacterRt->nCharacterConfidence)
					{
						int x1, y1, x2, y2;
						ri->BoundingBox(level, &x1, &y1, &x2, &y2);
						Point start, end;
						start.x = rtShowRect.x + x1;
						start.y = rtShowRect.y + y1;
						end.x = rtShowRect.x + x2;
						end.y = rtShowRect.y + y2;
						Rect rtSrc(start, end);		//������

						pST_CHARACTER_ANCHOR_POINT pstCharRt = new ST_CHARACTER_ANCHOR_POINT();
						pstCharRt->nIndex = nIndex;
						pstCharRt->fConfidence = conf;
						pstCharRt->rc.eCPType = CHARACTER_AREA;
						pstCharRt->rc.rt = convertObj.GetSrcSaveRect(rtSrc, pModel->vecPaperModel[nPic]->nPicSaveRotation);
						pstCharRt->rc.nTH = pstRecogCharacterRt->nIndex;	//��¼�µ�ǰ�������ڵڼ���������ʶ����
						pstCharRt->rc.nAnswer = nIndex;						//��¼�µ�ǰ�������ڵ�ǰ����ʶ�����еĵڼ���ʶ�������

						pstCharRt->rc.nThresholdValue = pstBigRecogCharRt->nThresholdValue;
						pstCharRt->rc.nGaussKernel = pstBigRecogCharRt->nGaussKernel;
						pstCharRt->rc.nSharpKernel = pstBigRecogCharRt->nSharpKernel;
						pstCharRt->rc.nCannyKernel = pstBigRecogCharRt->nCannyKernel;
						pstCharRt->rc.nDilateKernel = pstBigRecogCharRt->nDilateKernel;

						pstCharRt->strVal = CMyCodeConvert::Utf8ToGb2312(word);

						//**********	��Ҫɾ�������ظ����֣���֤ʶ�������û���ظ���
						for (auto item : pstRecogCharacterRt->vecCharacterRt)
							if (item->strVal == pstCharRt->strVal)
							{
								vecRepeatWord.push_back(item->strVal);
								break;
							}

						pstRecogCharacterRt->vecCharacterRt.push_back(pstCharRt);
						nIndex++;
					}
				} while (ri->Next(level));

				//Ҫ���ֵ��ظ���飬ȥ���ظ����֣���ͳһʶ�����У����ܴ����ظ����֣��������Ӱ��ȡ��
				for (int i = 0; i < vecRepeatWord.size(); i++)
				{
					std::vector<pST_CHARACTER_ANCHOR_POINT>::iterator itCharAnchorPoint = pstRecogCharacterRt->vecCharacterRt.begin();
					for (; itCharAnchorPoint != pstRecogCharacterRt->vecCharacterRt.end(); )
						if ((*itCharAnchorPoint)->strVal == vecRepeatWord[i])
						{
							pST_CHARACTER_ANCHOR_POINT pstCharRt = *itCharAnchorPoint;
							itCharAnchorPoint = pstRecogCharacterRt->vecCharacterRt.erase(itCharAnchorPoint);
							SAFE_RELEASE(pstCharRt);
						}
						else
							itCharAnchorPoint++;
				}

				if (pstRecogCharacterRt->vecCharacterRt.size() > 0)
				{
					pPic->lCharacterAnchorArea.push_back(pstRecogCharacterRt);

					int nSize = pstRecogCharacterRt->vecCharacterRt.size();
					if (nSize > 2)
					{
						//��鵱ǰʶ�����о�����Զ��������
						int nX_min = 0, nX_Max = 0;				//X�����Сֵ�����ֵ
						int nX_minIndex = 0, nX_maxIndex = 0;	//X�����Сֵ�����ֵ��Ӧ�ı�ǩ�����ڼ���
						int nY_min = 0, nY_Max = 0;				//Y�����Сֵ�����ֵ
						int nY_minIndex = 0, nY_maxIndex = 0;	//Y�����Сֵ�����ֵ��Ӧ�ı�ǩ�����ڼ���
						for (int i = 0; i < nSize; i++)
						{
							if (nX_min > pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.x)
							{
								nX_min = pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.x;
								nX_minIndex = i;
							}
							if (nX_Max < pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.x)
							{
								nX_Max = pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.x;
								nX_maxIndex = i;
							}
							if (nY_min > pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.y)
							{
								nY_min = pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.y;
								nY_minIndex = i;
							}
							if (nY_Max < pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.y)
							{
								nY_Max = pstRecogCharacterRt->vecCharacterRt[i]->rc.rt.y;
								nY_maxIndex = i;
							}
						}
						int nXDist = nX_Max - nX_min;
						int nYDist = nY_Max - nY_min;
						if (nXDist > nYDist)
						{
							pstRecogCharacterRt->arryMaxDist[0] = nX_minIndex;
							pstRecogCharacterRt->arryMaxDist[1] = nX_maxIndex;
						}
						else
						{
							pstRecogCharacterRt->arryMaxDist[0] = nY_minIndex;
							pstRecogCharacterRt->arryMaxDist[1] = nY_maxIndex;
						}

						// 						std::vector<pST_CHARACTER_ANCHOR_POINT> vecTmpCharacterRt(pstRecogCharacterRt->vecCharacterRt);
						// 						std::sort(vecTmpCharacterRt.begin(), vecTmpCharacterRt.end(), [](pST_CHARACTER_ANCHOR_POINT st1, pST_CHARACTER_ANCHOR_POINT st2)
						// 						{
						// 							return st1->rc.rt.x < st2->rc.rt.x;
						// 						});
						// 						int nXStart, nXEnd;
						// 						nXStart = vecTmpCharacterRt[0]->nIndex;
						// 						nXEnd = vecTmpCharacterRt[nSize - 1]->nIndex;
						// 						int nXDist = abs(vecTmpCharacterRt[nSize - 1]->rc.rt.x - vecTmpCharacterRt[0]->rc.rt.x); //X����Զ����
						// 
						// 						std::sort(vecTmpCharacterRt.begin(), vecTmpCharacterRt.end(), [](pST_CHARACTER_ANCHOR_POINT st1, pST_CHARACTER_ANCHOR_POINT st2)
						// 						{
						// 							return st1->rc.rt.y > st2->rc.rt.y;
						// 						});
						// 						int nYStart, nYEnd;
						// 						nYStart = vecTmpCharacterRt[0]->nIndex;
						// 						nYEnd = vecTmpCharacterRt[nSize - 1]->nIndex;
						// 						int nYDist = abs(vecTmpCharacterRt[nSize - 1]->rc.rt.x - vecTmpCharacterRt[0]->rc.rt.x); //X����Զ����
						// 
						// 						if (nXDist > nYDist)
						// 						{
						// 							pstBigRecogCharRt->arryMaxDist[0] = nXStart;
						// 							pstBigRecogCharRt->arryMaxDist[1] = nXEnd;
						// 						}
						// 						else
						// 						{
						// 							pstBigRecogCharRt->arryMaxDist[0] = nYStart;
						// 							pstBigRecogCharRt->arryMaxDist[1] = nYEnd;
						// 						}
					}
					else if (nSize == 2)
					{
						pstRecogCharacterRt->arryMaxDist[0] = pstRecogCharacterRt->vecCharacterRt[0]->nIndex;
						pstRecogCharacterRt->arryMaxDist[1] = pstRecogCharacterRt->vecCharacterRt[1]->nIndex;
					}
					else
					{
						pstRecogCharacterRt->arryMaxDist[0] = pstRecogCharacterRt->vecCharacterRt[0]->nIndex;
						pstRecogCharacterRt->arryMaxDist[1] = -1;
					}
				}
				else
					SAFE_RELEASE(pstRecogCharacterRt);
			}
#endif
		}
		catch (...)
		{
			std::string strLog2 = Poco::format("ʶ�����ֶ�λ����(%d)�쳣\n", i);
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

			// 			pPic->bFindIssue = true;
			// 			pPic->lIssueRect.push_back(stBigRecogCharRt);
			if (g_nOperatingMode == 2)
			{
				bResult = false;						//�ҵ������
				break;
			}
		}

	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ�����ֶ�λ����ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();

	std::string strRecogCharacter;
	for (auto item : pPic->lCharacterAnchorArea)
	{
		for (auto item2 : item->vecCharacterRt)
			strRecogCharacter.append(item2->strVal);
		strRecogCharacter.append("\t");
	}
	std::string strTime = Poco::format("ʶ�����ֶ�λ����ʱ��: %dms, ʶ������: %s\n", (int)(end - start), strRecogCharacter);
	strLog.append(strTime);

#if 1	 //���ö��㣬��ʶ�������������ѡ2����Ϊ����
	if (!GetPicFix(nPic, pPic, pModel))
	{
		std::string strGetAnchorPoint = "\n��ȡͼƬ�����ֶ�λ��ʧ��\n";
		strLog.append(strGetAnchorPoint);
	}
	cv::Mat matTmp = matCompPic.clone();
	for (auto item : pPic->lFix)
		cv::rectangle(matTmp, item.rt, CV_RGB(255, 0, 0), 2);
#endif
	//g_pLogger->information(strLog);
	return bResult;
}

bool CCharacterPoint::InitCharacterRecog()
{
	TRACE("*********************  �����ڴ�����ʵ����Tesseract���� ***********************\n");
#ifdef USE_TESSERACT
// 	m_pTess = new tesseract::TessBaseAPI();
// 	m_pTess->Init(NULL, "chi_sim", tesseract::OEM_DEFAULT);
#endif
	return !m_pTess ? true : false;
}

bool CCharacterPoint::GetPicFix(int nPic, pST_PicInfo pPic, pMODEL pModel)
{
	int nModelCharArea = pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size();	//ģ���ϵ����ֶ�λ��
	int nRealRecogCharArea = pPic->lCharacterAnchorArea.size();	//ʵ��ʶ��������ֶ�λ��

	if (nModelCharArea <= 0 || nRealRecogCharArea <= 0) return false;

	pPic->lFix.clear();
	int nNeedCount = pModel->nCharacterAnchorPoint;	//��Ҫȡ�����ֶ������

	//2������ʱ��ѡ������Զ����������θ��ݸ��ɾ������ϵ�����µ�ֱ����2�����β������غ϶ȣ������غ϶���ߵľ��ε����ĵ���Ϊ������ε����ĵ�

	//	nNeedCount = nNeedCount > 4 ? 4 : nNeedCount;	//���㲻����4��
#if 1
	//����һ�����㣬����Y���X��С
	pST_CHARACTER_ANCHOR_POINT ptPeak = pPic->lCharacterAnchorArea.front()->vecCharacterRt[0];
	for (auto itArea : pPic->lCharacterAnchorArea)
		for (auto itPoint : itArea->vecCharacterRt)
			if ((itPoint->rc.rt.y > ptPeak->rc.rt.y) || (itPoint->rc.rt.y == ptPeak->rc.rt.y && itPoint->rc.rt.x < ptPeak->rc.rt.x))
				ptPeak = itPoint;

	pPic->lFix.push_back(ptPeak->rc);
	for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
		if (itModelCharAnchorArea->nIndex == ptPeak->rc.nTH)
		{
			for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
			{
				if (itModelCharAnchorPoint->strVal == ptPeak->strVal)
				{
					pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
					break;
				}
			}
			break;
		}

	//�������е㣬������붥��ľ��룬���������ĵ�������Ϊ���������λ��
	VEC_POINTDIST2PEAK vecPeakDist;
	for (auto itArea : pPic->lCharacterAnchorArea)
		for (auto itPoint : itArea->vecCharacterRt)
		{
			if (itPoint == ptPeak) continue;
			ST_POINTDIST2PEAK stPtDist;
			stPtDist.pAnchorPoint = itPoint;
			stPtDist.nDist = sqrt((itPoint->rc.rt.x - ptPeak->rc.rt.x) * (itPoint->rc.rt.x - ptPeak->rc.rt.x) + (itPoint->rc.rt.y - ptPeak->rc.rt.y) * (itPoint->rc.rt.y - ptPeak->rc.rt.y));
			vecPeakDist.push_back(stPtDist);
		}
	std::sort(vecPeakDist.begin(), vecPeakDist.end(), [](ST_POINTDIST2PEAK& s1, ST_POINTDIST2PEAK& s2)
	{
		return s1.nDist > s2.nDist;
	});
	for (int i = 0; i < nNeedCount; i++)
	{
		if (i >= vecPeakDist.size())
			break;
		pPic->lFix.push_back(vecPeakDist[i].pAnchorPoint->rc);
		for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
			if (itModelCharAnchorArea->nIndex == vecPeakDist[i].pAnchorPoint->rc.nTH)
			{
				for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
				{
					if (itModelCharAnchorPoint->strVal == vecPeakDist[i].pAnchorPoint->strVal)
					{
						pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
						break;
					}
				}
				break;
			}
	}
#else
	if (nRealRecogCharArea == 1)	//ֻʶ���һ�����ֶ�λ���������ʶ����ȡ nNeedCount ��׼ȷ����ߵ�����Ϊ����
	{
		CHARACTER_ANCHOR_AREA_LIST::iterator it = pPic->lCharacterAnchorArea.begin();

		nNeedCount = (*it)->vecCharacterRt.size() > nNeedCount ? nNeedCount : (*it)->vecCharacterRt.size();

#if 1
		if ((*it)->vecCharacterRt.size() <= 2)	//ֻʶ��1����2������ʱ����ѡ��������Ϊ���ֶ�λ��
		{
			for (int i = 0; i < nNeedCount; i++)
			{
				pPic->lFix.push_back((*it)->vecCharacterRt[i]->rc);

				for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
					if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
					{
						for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
						{
							if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[i]->strVal)
							{
								pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
								break;
							}
						}
						break;
					}
			}
		}
		else	//ʶ��2����������ʱ��ȡ���е�ĳ����
		{
			int* pTmpArry = new int[nNeedCount];

			for (int i = 0; i < nNeedCount; i++)
			{
				if (i < 2)
				{
					pPic->lFix.push_back((*it)->vecCharacterRt[(*it)->arryMaxDist[i]]->rc);
					pTmpArry[i] = (*it)->arryMaxDist[i];
				}
				else
				{
					//���ȡ��
					Poco::Random rnd;
					rnd.seed();
					int n = rnd.next(pPic->lFix.size());

					for (int j = 0; j < i; j++)
					{
						if (pTmpArry[j] == n)
						{
							n++;
							j = 0;
						}
					}
					pTmpArry[i] = n;		//�洢�������б��еĵ�����
					pPic->lFix.push_back((*it)->vecCharacterRt[n]->rc);
				}

				for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
					if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
					{
						for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
						{
							if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[pTmpArry[i]]->strVal)	//����Ҫ��
							{
								pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
								break;
							}
						}
						break;
					}
			}
			SAFE_RELEASE_ARRY(pTmpArry);
		}

		//-------------------------------------------------

		// 		//ֻ��2���������ȥ��β����������
		// 		pPic->lFix.push_back((*it)->vecCharacterRt[0]->rc);
		// 		pPic->lFix.push_back((*it)->vecCharacterRt[(*it)->vecCharacterRt.size() - 1]->rc);
		// 
		// 		//��ȡģ���ϵĶ�Ӧ�ֵĶ���λ��
		// 		for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
		// 			if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
		// 			{
		// 				for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
		// 				{
		// 					if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[0]->strVal)
		// 					{
		// 						pPic->lModelFix.push_back(itModelCharAnchorPoint->rc);
		// 						break;
		// 					}
		// 				}
		// 				for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
		// 				{
		// 					if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[(*it)->vecCharacterRt.size() - 1]->strVal)
		// 					{
		// 						pPic->lModelFix.push_back(itModelCharAnchorPoint->rc);
		// 						break;
		// 					}
		// 				}
		// 				break;
		// 			}
#else
		//ȡǰ����׼ȷ����ߵ���������, �����붨���б�
		std::sort((*it)->vecCharacterRt.begin(), (*it)->vecCharacterRt.end(), [](pST_CHARACTER_ANCHOR_POINT& st1, pST_CHARACTER_ANCHOR_POINT& st2)
		{
			return st1->fConfidence > st2->fConfidence;
		});
		for (int i = 0; i < nNeedCount; i++)
			pPic->lFix.push_back((*it)->vecCharacterRt[i]->rc);

		//��ȡģ���ϵĶ�Ӧ�ֵĶ���λ��
		for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
			if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
			{
				for (int i = 0; i < nNeedCount; i++)
				{
					for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
					{
						if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[i]->strVal)
						{
							pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
							break;
						}
					}
				}
				break;
			}
#endif
	}
	else if (nRealRecogCharArea < nNeedCount)	//������ʶ��������������ʶ��׼ȷ������������ÿ��ʶ����ȡһ���ֲ�ѭ��ȡ��ֱ���ﵽҪ���������
	{
		//ÿ��ʶ����������ʶ��׼ȷ������
		CHARACTER_ANCHOR_AREA_LIST::iterator itAnchorArea = pPic->lCharacterAnchorArea.begin();
		for (; itAnchorArea != pPic->lCharacterAnchorArea.end(); itAnchorArea++)
			std::sort((*itAnchorArea)->vecCharacterRt.begin(), (*itAnchorArea)->vecCharacterRt.end(), SortByCharacterConfidence);

		for (int i = 0; i < nNeedCount; i++)
		{
			int nItem = i / nRealRecogCharArea;

			CHARACTER_ANCHOR_AREA_LIST::iterator it = pPic->lCharacterAnchorArea.begin();
			for (int k = 0; k < i - nItem * nRealRecogCharArea; k++)
			{
				it++;
				while ((*it)->vecCharacterRt.size() < nItem + 1)			//*********************		�㷨������	****************************************
					it++;
			}

			pPic->lFix.push_back((*it)->vecCharacterRt[nItem]->rc);

			//��ȡģ���ϵĶ�Ӧ�ֵĶ���λ��
			for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
				if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
				{
					for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
					{
						if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[nItem]->strVal)
						{
							pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
							break;
						}
					}
					break;
				}
		}

	}
	else		//ʶ���������ֶ�λ����ֻ��ǰ nNeedCount ��ʶ����ȡ1��׼ȷ����ߵ�����Ϊ����
	{
		CHARACTER_ANCHOR_AREA_LIST::iterator it = pPic->lCharacterAnchorArea.begin();
		for (int i = 0; it != pPic->lCharacterAnchorArea.end(); it++, i++)
		{
			if (i > nNeedCount - 1) break;

			if ((*it)->vecCharacterRt.size() >= 2)
				std::sort((*it)->vecCharacterRt.begin(), (*it)->vecCharacterRt.end(), SortByCharacterConfidence);
			//ȡ׼ȷ����ߵĵ�������, �����붨���б�
			pPic->lFix.push_back((*it)->vecCharacterRt[0]->rc);

			//��ȡģ���ϵĶ�Ӧ�ֵĶ���λ��
			for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
				if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
				{
					for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
					{
						if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[0]->strVal)
						{
							pPic->lModelWordFix.push_back(itModelCharAnchorPoint->rc);
							break;
						}
					}
					break;
				}
		}
	}
#endif
	return true;
}

//----------------------------------------------------------
bool CFixPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;

	clock_t start, end;
	start = clock();
	strLog = Poco::format("ͼƬ%s��ʶ�𶨵�\n", pPic->strPicName);

	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lSelFixRoi.begin();
	for (int i = 0; itCP != pModel->vecPaperModel[nPic]->lSelFixRoi.end(); i++, itCP++)
	{
		RECTINFO rc = *itCP;

		std::vector<cv::Rect>RectCompList;
		try
		{
			float fModelW = pModel->vecPaperModel[nPic]->nPicW;
			float fModelH = pModel->vecPaperModel[nPic]->nPicH;
			int nRealW = matCompPic.cols;
			int nRealH = matCompPic.rows;

			cv::Rect rtTmp;
			rtTmp.x = rc.rt.x * nRealW / fModelW;
			rtTmp.y = rc.rt.y * nRealH / fModelH;
			rtTmp.width = rc.rt.width * nRealW / fModelW;
			rtTmp.height = rc.rt.height * nRealH / fModelH;

			if (rtTmp.x < 0) rtTmp.x = 0;
			if (rtTmp.y < 0) rtTmp.y = 0;
			if (rtTmp.br().x > matCompPic.cols)
			{
				rtTmp.width = matCompPic.cols - rtTmp.x;
			}
			if (rtTmp.br().y > matCompPic.rows)
			{
				rtTmp.height = matCompPic.rows - rtTmp.y;
			}
			cv::Mat matCompRoi;
			matCompRoi = matCompPic(rtTmp);

			cv::Mat matCompRoi2 = matCompRoi.clone();
			if (matCompRoi2.channels() == 3)
				cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);

			GaussianBlur(matCompRoi2, matCompRoi2, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi2, matCompRoi2, rc.nSharpKernel);

			int nRealThreshold = 150;
			RECTLIST::iterator itFix = pModel->vecPaperModel[nPic]->lFix.begin();
			for (int j = 0; itFix != pModel->vecPaperModel[nPic]->lFix.end(); j++, itFix++)
			{
				if (j == i)
				{
					nRealThreshold = itFix->nThresholdValue;
					break;
				}
			}

#ifdef USES_GETTHRESHOLD_ZTFB
			const int channels[1] = { 0 };
			const int histSize[1] = { nRealThreshold };	//150
			float hranges[2] = { 0, nRealThreshold };	//150
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matCompRoi2, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = nRealThreshold;		//150
			if (nCount > 0)
			{
				float fMean = (float)nSum / nCount;		//��ֵ

				for (int h = 0; h < hist.rows; h++)	//histSize
				{
					float binVal = hist.at<float>(h);

					nDevSum += pow(h - fMean, 2)*binVal;
				}
				float fStdev = sqrt(nDevSum / nCount);
				nThreshold = fMean + 2 * fStdev;
				if (fStdev > fMean)
					nThreshold = fMean + fStdev;
			}

			if (nThreshold > nRealThreshold) nThreshold = nRealThreshold;	//150
			threshold(matCompRoi2, matCompRoi2, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi2, matCompRoi2, 60, 255, THRESH_BINARY);
#endif
			//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
			dilate(matCompRoi2, matCompRoi2, element_Anticlutter);
			erode(matCompRoi2, matCompRoi2, element_Anticlutter);

			cv::Canny(matCompRoi2, matCompRoi2, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	��ͨ�հ׿��ʶ��	Size(_nDilateKernel_, _nDilateKernel_)
			dilate(matCompRoi2, matCompRoi2, element);

			IplImage ipl_img(matCompRoi2);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//��ȡ����  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm = rm + rtTmp.tl();	// rc.rt.tl();
				RectCompList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);
		}
		catch (cv::Exception& exc)
		{
			std::string strLog2 = Poco::format("ʶ�𶨵�(%d)�쳣: %s\n", i, exc.msg.c_str());
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
			if (nRecogMode == 2)
			{
				bResult = false;						//�ҵ������
				break;
			}
		}

		std::string strLog2;	//��ʱ��־����¼���ξ���ʶ����
		bool bFindRect = false;
		if (RectCompList.size() == 0)
			bFindRect = true;
		else
		{
			std::sort(RectCompList.begin(), RectCompList.end(), SortByArea);
			Rect& rtFix = RectCompList[0];

			RECTINFO rcFix;
			RECTLIST::iterator itFix = pModel->vecPaperModel[nPic]->lFix.begin();
			for (int j = 0; itFix != pModel->vecPaperModel[nPic]->lFix.end(); j++, itFix++)
			{
				if (j == i)
				{
					rcFix = *itFix;
					break;
				}
			}

			bool bOnlyOne = false;		//ֻ��һ�����Σ���Ҫ�ж�����ͻҶȣ����Ǳ������Խ���
			bool bFind = false;

			//ͨ���Ҷ�ֵ���ж�
			for (int k = 0; k < RectCompList.size(); k++)
			{
				RECTINFO rcTmp = rcFix;
				rcTmp.rt = RectCompList[k];

				//���ݶ������ϵ������µ�λ���ж��Ƿ����Ծ�ı����ϣ�����ڣ���������۽ǻ��߱�������
				cv::Point pt1 = RectCompList[k].tl();
				cv::Point pt2 = RectCompList[k].br();
				int nDiff = 4;	//��ͼ��߽�ľ����������ֵ֮�ڣ���Ϊ���ڱ߽�����
				if (pt1.x < nDiff || pt1.y < nDiff || matCompPic.cols - pt2.x < nDiff || matCompPic.rows - pt2.y < nDiff)
				{
					TRACE("����(%d,%d,%d,%d)λ�þ������̫�����������۽ǻ���\n", RectCompList[k].x, RectCompList[k].y, RectCompList[k].width, RectCompList[k].height);
					continue;
				}

				Recog(rcTmp, matCompPic, strLog);
				float fArea = rcTmp.fRealArea / rcTmp.fStandardArea;
				float fDensity = rcTmp.fRealDensity / rcTmp.fStandardDensity;
				float fWper = (float)rcTmp.rt.width / rcFix.rt.width;			//���ҵľ��εĿ����ģ���Ӧ����Ŀ��֮��
				float fHper = (float)rcTmp.rt.height / rcFix.rt.height;			//���ҵľ��εĿ����ģ���Ӧ����ĸ߶�֮��
				std::string strTmpLog = Poco::format("��%d������:area=%f, Density=%f\t", k, (double)fArea, (double)fDensity);
				strLog2.append(strTmpLog);
				if ((bOnlyOne && fArea > 0.4 && fArea < 2.5 && fDensity > rcTmp.fStandardValuePercent * 0.9 && fWper < 2.0 && fWper > 0.4 && fHper < 2.0 && fHper > 0.4) || \
					(fArea > 0.5 && fArea < 2.0 && fDensity > rcTmp.fStandardValuePercent && fWper < 2.0 && fWper > 0.4 && fHper < 2.0 && fHper > 0.4))	//fArea > 0.7 && fArea < 1.5 && fDensity > 0.6
				{
					bFind = true;
					rtFix = RectCompList[k];
					break;
				}
			}

			if (!bFind)
				bFindRect = true;
			else
			{
				RECTINFO rcFixInfo = rc;
				rcFixInfo.nTH = i;			//��������ģ���϶����б�ĵڼ���
				rcFixInfo.rt = rtFix;
				pPic->lFix.push_back(rcFixInfo);
				TRACE("�������: (%d,%d,%d,%d)\n", rtFix.x, rtFix.y, rtFix.width, rtFix.height);
			}
		}
		if (bFindRect)
		{
			std::string strLog3 = Poco::format("ʶ�𶨵�(%d)ʧ�� -- %s\n", i, strLog2);
			strLog.append(strLog3);
			bResult = false;						//�ҵ������
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
			if (nRecogMode == 2)
				break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ�𶨵�ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("ʶ�𶨵�ʱ��: %dms\n", (int)(end - start));
	strLog.append(strTime);
	//g_pLogger->information(strLog);
	return bResult;
}

//----------------------------------------------------------
bool CABPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("ʶ��AB����\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lABModel.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lABModel.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
		bool bFindRect = Recog(rc, matCompPic, strLog);
		if (bFindRect)
		{
			bResult = false;						//�ҵ������
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
		else
			pPic->lNormalRect.push_back(rc);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��AB����ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

//----------------------------------------------------------
bool CCoursePoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("ʶ���Ŀ\n");
	clock_t start, end;
	start = clock();
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lCourse.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lCourse.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
		bool bResult_Recog = Recog(rc, matCompPic, strLog);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)				//**************	�����Ŀ���ɶ��У�����Ϲ��ɣ���Ҫ���⿼��	*******************
			{
				pPic->lNormalRect.push_back(rc);
				bResult = true;
				continue;
			}
			else
			{
				char szLog[MAX_PATH] = { 0 };
				sprintf_s(szLog, "У��ʧ��, �ҶȰٷֱ�: %f, �����: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				strLog.append(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "У��ʧ��, �쳣����, �����: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			strLog.append(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (nRecogMode == 2)
		{
			bResult = false;						//�ҵ������
			break;		//���������ʱ�������жϺ���ĵ㣬��ֹͣɨ��
		}
	}
	if (!bResult)
	{
		(static_cast<pST_PaperInfo>(pPic->pPaper))->bRecogCourse = false;
		pPic->bRecogCourse = false;
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ���Ŀʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("ʶ���ĿУ���ʱ��: %dms\n", (int)(end - start));
	strLog.append(strTime);
	return bResult;
}

//----------------------------------------------------------
bool CGrayPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("ʶ��Ҷȵ�\n");

	clock_t start, end;
	start = clock();
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lGray.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lGray.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
		bool bResult_Recog = Recog(rc, matCompPic, strLog);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				pPic->lNormalRect.push_back(rc);
				bResult = true;
				continue;
			}
			else
			{
				char szLog[MAX_PATH] = { 0 };
				sprintf_s(szLog, "У��ʧ��, �ҶȰٷֱ�: %f, �����: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				strLog.append(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "У��ʧ��, �쳣����, �����: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
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
		sprintf_s(szLog, "ʶ��Ҷ�У���ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("ʶ��Ҷ�У���ʱ��: %dms\n", (int)(end - start));
	strLog.append(strTime);
	//g_pLogger->information(strLog);
	return bResult;
}

bool CWhitePoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lWhite.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lWhite.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
		bool bResult_Recog = Recog(rc, matCompPic, strLog);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				pPic->lNormalRect.push_back(rc);
				bResult = true;
				continue;
			}
			else
			{
				char szLog[MAX_PATH] = { 0 };
				sprintf_s(szLog, "У��ʧ��, �ҶȰٷֱ�: %f, �����: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				//g_pLogger->information(szLog);
				strLog.append(szLog);
				TRACE(szLog);
			}
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
		sprintf_s(szLog, "ʶ��հ�У���ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

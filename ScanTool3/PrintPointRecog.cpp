#include "stdafx.h"
#include "PrintPointRecog.h"
#include "CoordinationConvert.h"

using namespace cv;

bool CCharacterPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;

	clock_t start, end;
	start = clock();
	strLog = Poco::format("图片%s\n", pPic->strPicName);
	if (pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() == 0)
	{
		strLog = Poco::format("图片%s没有文字定位点需要识别", pPic->strPicName);
		//g_pLogger->information(strLog);
		return true;
	}

#ifdef USE_TESSERACT
	if (NULL == m_pTess && !InitCharacterRecog())
	{
		strLog.append("tesseract对象为实例化\n");
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
			matCompRoi = matCompPic(pstBigRecogCharRt->rt);		//真坐标

			Mat matCompRoi2 = matCompRoi.clone();
			if (matCompRoi2.channels() == 3)
				cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);

			switch (pModel->vecPaperModel[nPic]->nPicSaveRotation)
			{
				case 1:	break;
				case 3:			//图像需要调整到正常的视觉方向(作答方向)，则模板是右旋90，则实际图片需左旋90
					{
						cv::Mat dst;
						transpose(matCompRoi2, dst);	//左旋90，镜像 
						flip(dst, matCompRoi2, 0);	//左旋90，模板图像需要右旋90，原图即需要左旋90
					}
					break;
				case 2:			//图像需要调整到正常的视觉方向(作答方向)，则模板是左旋90，则实际图片需右旋90
					{
						cv::Mat dst;
						transpose(matCompRoi2, dst);	//左旋90，镜像 
						flip(dst, matCompRoi2, 1);	//右旋90，模板图像需要左旋90，原图即需要右旋90
					}
					break;
				case 4:
					{
						cv::Mat dst;
						transpose(matCompRoi2, dst);	//左旋90，镜像 
						cv::Mat dst2;
						flip(dst, dst2, 1);
						cv::Mat dst5;
						transpose(dst2, dst5);
						flip(dst5, matCompRoi2, 1);	//右旋180
					}
					break;
				default:
					break;
			}

			GaussianBlur(matCompRoi2, matCompRoi2, cv::Size(pstBigRecogCharRt->nGaussKernel, pstBigRecogCharRt->nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi2, matCompRoi2, pstBigRecogCharRt->nSharpKernel);

			//double dThread = threshold(matCompRoi, matCompRoi, pstBigRecogCharRt->nThresholdValue, 255, THRESH_OTSU | THRESH_BINARY);
			cv::adaptiveThreshold(matCompRoi2, matCompRoi2, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 19, 20);	//blockSize, constValue

			CCoordinationConvert convertObj(matCompPic);	//坐标转换对象
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
				pstRecogCharacterRt->rt = pstBigRecogCharRt->rt;	//真坐标

				convertObj.SetPicRect(matCompPic.rows, matCompPic.cols);	//改变参照图像的大小，后面计算从假坐标到真坐标时，基准图像的大小已经变化

																			//重复字临时登记列表，后面删除所有重复的字
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
						Rect rtSrc(start, end);		//假坐标

						pST_CHARACTER_ANCHOR_POINT pstCharRt = new ST_CHARACTER_ANCHOR_POINT();
						pstCharRt->nIndex = nIndex;
						pstCharRt->fConfidence = conf;
						pstCharRt->rc.eCPType = CHARACTER_AREA;
						pstCharRt->rc.rt = convertObj.GetSrcSaveRect(rtSrc, pModel->vecPaperModel[nPic]->nPicSaveRotation);
						pstCharRt->rc.nTH = pstRecogCharacterRt->nIndex;	//记录下当前文字属于第几个大文字识别区
						pstCharRt->rc.nAnswer = nIndex;						//记录下当前文字属于当前文字识别区中的第几个识别的文字

						pstCharRt->rc.nThresholdValue = pstBigRecogCharRt->nThresholdValue;
						pstCharRt->rc.nGaussKernel = pstBigRecogCharRt->nGaussKernel;
						pstCharRt->rc.nSharpKernel = pstBigRecogCharRt->nSharpKernel;
						pstCharRt->rc.nCannyKernel = pstBigRecogCharRt->nCannyKernel;
						pstCharRt->rc.nDilateKernel = pstBigRecogCharRt->nDilateKernel;

						pstCharRt->strVal = CMyCodeConvert::Utf8ToGb2312(word);

						//**********	需要删除所有重复的字，保证识别的文字没有重复字
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

				//要做字的重复检查，去除重复的字，即统一识别区中，不能存在重复的字，否则可能影响取字
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
						//检查当前识别区中距离最远的两个点
						int nX_min = 0, nX_Max = 0;				//X轴的最小值、最大值
						int nX_minIndex = 0, nX_maxIndex = 0;	//X轴的最小值、最大值对应的标签，即第几个
						int nY_min = 0, nY_Max = 0;				//Y轴的最小值、最大值
						int nY_minIndex = 0, nY_maxIndex = 0;	//Y轴的最小值、最大值对应的标签，即第几个
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
						// 						int nXDist = abs(vecTmpCharacterRt[nSize - 1]->rc.rt.x - vecTmpCharacterRt[0]->rc.rt.x); //X轴最远距离
						// 
						// 						std::sort(vecTmpCharacterRt.begin(), vecTmpCharacterRt.end(), [](pST_CHARACTER_ANCHOR_POINT st1, pST_CHARACTER_ANCHOR_POINT st2)
						// 						{
						// 							return st1->rc.rt.y > st2->rc.rt.y;
						// 						});
						// 						int nYStart, nYEnd;
						// 						nYStart = vecTmpCharacterRt[0]->nIndex;
						// 						nYEnd = vecTmpCharacterRt[nSize - 1]->nIndex;
						// 						int nYDist = abs(vecTmpCharacterRt[nSize - 1]->rc.rt.x - vecTmpCharacterRt[0]->rc.rt.x); //X轴最远距离
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
			std::string strLog2 = Poco::format("识别文字定位区域(%d)异常\n", i);
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

			// 			pPic->bFindIssue = true;
			// 			pPic->lIssueRect.push_back(stBigRecogCharRt);
			if (g_nOperatingMode == 2)
			{
				bResult = false;						//找到问题点
				break;
			}
		}

	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别文字定位区域失败, 图片名: %s\n", pPic->strPicName.c_str());
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
	std::string strTime = Poco::format("识别文字定位区域时间: %dms, 识别文字: %s\n", (int)(end - start), strRecogCharacter);
	strLog.append(strTime);

#if 1	 //重置定点，在识别出来的文字中选2个作为定点
	if (!GetPicFix(nPic, pPic, pModel))
	{
		std::string strGetAnchorPoint = "\n获取图片的文字定位点失败\n";
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
	TRACE("*********************  不能在此类中实例化Tesseract对象 ***********************\n");
#ifdef USE_TESSERACT
// 	m_pTess = new tesseract::TessBaseAPI();
// 	m_pTess->Init(NULL, "chi_sim", tesseract::OEM_DEFAULT);
#endif
	return !m_pTess ? true : false;
}

bool CCharacterPoint::GetPicFix(int nPic, pST_PicInfo pPic, pMODEL pModel)
{
	int nModelCharArea = pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size();	//模板上的文字定位区
	int nRealRecogCharArea = pPic->lCharacterAnchorArea.size();	//实际识别出的文字定位区

	if (nModelCharArea <= 0 || nRealRecogCharArea <= 0) return false;

	pPic->lFix.clear();
	int nNeedCount = pModel->nCharacterAnchorPoint;	//需要取的文字定点个数

	//2个定点时，选距离最远的两个，如何根据给成矩形左上点和右下点分别计算2个矩形并计算重合度，根据重合度最高的矩形的中心点作为结果矩形的中心点

	//	nNeedCount = nNeedCount > 4 ? 4 : nNeedCount;	//定点不超过4个
#if 1
	//查找一个顶点，它的Y最大，X最小
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

	//遍历所有点，计算距离顶点的距离，将距离最大的点依次作为定点求矩形位置
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
	if (nRealRecogCharArea == 1)	//只识别出一个文字定位区，在这个识别区取 nNeedCount 个准确度最高的字作为定点
	{
		CHARACTER_ANCHOR_AREA_LIST::iterator it = pPic->lCharacterAnchorArea.begin();

		nNeedCount = (*it)->vecCharacterRt.size() > nNeedCount ? nNeedCount : (*it)->vecCharacterRt.size();

#if 1
		if ((*it)->vecCharacterRt.size() <= 2)	//只识别到1个或2个文字时，就选这两个做为文字定位点
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
		else	//识别到2个以上文字时，取其中的某几个
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
					//随机取点
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
					pTmpArry[i] = n;		//存储定点在列表中的的索引
					pPic->lFix.push_back((*it)->vecCharacterRt[n]->rc);
				}

				for (auto itModelCharAnchorArea : pModel->vecPaperModel[nPic]->lCharacterAnchorArea)
					if (itModelCharAnchorArea->nIndex == (*it)->nIndex)
					{
						for (auto itModelCharAnchorPoint : itModelCharAnchorArea->vecCharacterRt)
						{
							if (itModelCharAnchorPoint->strVal == (*it)->vecCharacterRt[pTmpArry[i]]->strVal)	//这里要改
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

		// 		//只对2个的情况，去首尾两个做定点
		// 		pPic->lFix.push_back((*it)->vecCharacterRt[0]->rc);
		// 		pPic->lFix.push_back((*it)->vecCharacterRt[(*it)->vecCharacterRt.size() - 1]->rc);
		// 
		// 		//获取模板上的对应字的定点位置
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
		//取前两个准确度最高的字做定点, 并放入定点列表
		std::sort((*it)->vecCharacterRt.begin(), (*it)->vecCharacterRt.end(), [](pST_CHARACTER_ANCHOR_POINT& st1, pST_CHARACTER_ANCHOR_POINT& st2)
		{
			return st1->fConfidence > st2->fConfidence;
		});
		for (int i = 0; i < nNeedCount; i++)
			pPic->lFix.push_back((*it)->vecCharacterRt[i]->rc);

		//获取模板上的对应字的定点位置
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
	else if (nRealRecogCharArea < nNeedCount)	//在所有识别区中所有文字识别准确度排序，依次在每个识别区取一个字并循环取，直到达到要求的文字数
	{
		//每个识别区的文字识别准确度排序
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
				while ((*it)->vecCharacterRt.size() < nItem + 1)			//*********************		算法有问题	****************************************
					it++;
			}

			pPic->lFix.push_back((*it)->vecCharacterRt[nItem]->rc);

			//获取模板上的对应字的定点位置
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
	else		//识别出多个文字定位区，只在前 nNeedCount 个识别区取1个准确度最高的字作为定点
	{
		CHARACTER_ANCHOR_AREA_LIST::iterator it = pPic->lCharacterAnchorArea.begin();
		for (int i = 0; it != pPic->lCharacterAnchorArea.end(); it++, i++)
		{
			if (i > nNeedCount - 1) break;

			if ((*it)->vecCharacterRt.size() >= 2)
				std::sort((*it)->vecCharacterRt.begin(), (*it)->vecCharacterRt.end(), SortByCharacterConfidence);
			//取准确度最高的点做定点, 并放入定点列表
			pPic->lFix.push_back((*it)->vecCharacterRt[0]->rc);

			//获取模板上的对应字的定点位置
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
	strLog = Poco::format("图片%s：识别定点\n", pPic->strPicName);

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
				float fMean = (float)nSum / nCount;		//均值

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
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi2, matCompRoi2, element_Anticlutter);
			erode(matCompRoi2, matCompRoi2, element_Anticlutter);

			cv::Canny(matCompRoi2, matCompRoi2, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别	Size(_nDilateKernel_, _nDilateKernel_)
			dilate(matCompRoi2, matCompRoi2, element);

			IplImage ipl_img(matCompRoi2);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
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
			std::string strLog2 = Poco::format("识别定点(%d)异常: %s\n", i, exc.msg.c_str());
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
			if (nRecogMode == 2)
			{
				bResult = false;						//找到问题点
				break;
			}
		}

		std::string strLog2;	//临时日志，记录矩形具体识别结果
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

			bool bOnlyOne = false;		//只有一个矩形，需要判断面积和灰度，但是比例可以降低
			bool bFind = false;

			//通过灰度值来判断
			for (int k = 0; k < RectCompList.size(); k++)
			{
				RECTINFO rcTmp = rcFix;
				rcTmp.rt = RectCompList[k];

				//根据定点左上点与右下点位置判断是否在试卷的边线上，如果在，则可能是折角或者边上有损坏
				cv::Point pt1 = RectCompList[k].tl();
				cv::Point pt2 = RectCompList[k].br();
				int nDiff = 4;	//与图像边界的距离间隔在这个值之内，认为属于边界线上
				if (pt1.x < nDiff || pt1.y < nDiff || matCompPic.cols - pt2.x < nDiff || matCompPic.rows - pt2.y < nDiff)
				{
					TRACE("矩形(%d,%d,%d,%d)位置距离边线太近，可能是折角或损坏\n", RectCompList[k].x, RectCompList[k].y, RectCompList[k].width, RectCompList[k].height);
					continue;
				}

				Recog(rcTmp, matCompPic, strLog);
				float fArea = rcTmp.fRealArea / rcTmp.fStandardArea;
				float fDensity = rcTmp.fRealDensity / rcTmp.fStandardDensity;
				float fWper = (float)rcTmp.rt.width / rcFix.rt.width;			//查找的矩形的宽度与模板对应定点的宽度之比
				float fHper = (float)rcTmp.rt.height / rcFix.rt.height;			//查找的矩形的宽度与模板对应定点的高度之比
				std::string strTmpLog = Poco::format("第%d个矩形:area=%f, Density=%f\t", k, (double)fArea, (double)fDensity);
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
				rcFixInfo.nTH = i;			//这是属于模板上定点列表的第几个
				rcFixInfo.rt = rtFix;
				pPic->lFix.push_back(rcFixInfo);
				TRACE("定点矩形: (%d,%d,%d,%d)\n", rtFix.x, rtFix.y, rtFix.width, rtFix.height);
			}
		}
		if (bFindRect)
		{
			std::string strLog3 = Poco::format("识别定点(%d)失败 -- %s\n", i, strLog2);
			strLog.append(strLog3);
			bResult = false;						//找到问题点
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
			if (nRecogMode == 2)
				break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别定点失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别定点时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	//g_pLogger->information(strLog);
	return bResult;
}

//----------------------------------------------------------
bool CABPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("识别AB卷型\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lABModel.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lABModel.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
		bool bFindRect = Recog(rc, matCompPic, strLog);
		if (bFindRect)
		{
			bResult = false;						//找到问题点
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
		else
			pPic->lNormalRect.push_back(rc);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别AB卷型失败, 图片名: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

//----------------------------------------------------------
bool CCoursePoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("识别科目\n");
	clock_t start, end;
	start = clock();
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lCourse.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lCourse.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		AssistObj.GetRecogPosition(nPic, pPic, pModel, rc.rt);
		bool bResult_Recog = Recog(rc, matCompPic, strLog);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)				//**************	如果科目是由多个校验点组合构成，需要另外考虑	*******************
			{
				pPic->lNormalRect.push_back(rc);
				bResult = true;
				continue;
			}
			else
			{
				char szLog[MAX_PATH] = { 0 };
				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				strLog.append(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			strLog.append(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (nRecogMode == 2)
		{
			bResult = false;						//找到问题点
			break;		//发现问题点时，继续判断后面的点，不停止扫描
		}
	}
	if (!bResult)
	{
		(static_cast<pST_PaperInfo>(pPic->pPaper))->bRecogCourse = false;
		pPic->bRecogCourse = false;
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别科目失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别科目校验点时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	return bResult;
}

//----------------------------------------------------------
bool CGrayPoint::RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("识别灰度点\n");

	clock_t start, end;
	start = clock();
	strLog = Poco::format("图片%s\n", pPic->strPicName);

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
				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				strLog.append(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
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
		sprintf_s(szLog, "识别灰度校验点失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别灰度校验点时间: %dms\n", (int)(end - start));
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
				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				//g_pLogger->information(szLog);
				strLog.append(szLog);
				TRACE(szLog);
			}
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
		sprintf_s(szLog, "识别空白校验点失败, 图片名: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

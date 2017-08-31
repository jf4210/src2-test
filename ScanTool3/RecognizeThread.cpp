#include "stdafx.h"
#include "RecognizeThread.h"
#include "ScanTool3.h"


using namespace cv;
CRecognizeThread::CRecognizeThread()
{
}

CRecognizeThread::~CRecognizeThread()
{
	if (m_vecContours.size())
		m_vecContours.clear();
	g_pLogger->information("RecognizeThread exit.");
	TRACE("RecognizeThread exit1.\n");
}

void CRecognizeThread::run()
{
	g_pLogger->information("RecognizeThread start...");
	TRACE("RecognizeThread start...\n");
	eExit.reset();

	while (!g_nExitFlag)
	{
		pRECOGTASK pTask = NULL;
		g_fmRecog.lock();
		RECOGTASKLIST::iterator it = g_lRecogTask.begin();
		for (; it != g_lRecogTask.end();)
		{
			pTask = *it;
			it = g_lRecogTask.erase(it);
			break;
		}
		g_fmRecog.unlock();
		if (NULL == pTask)
		{
			Poco::Thread::sleep(100);
			continue;
		}

		HandleTask(pTask);

		delete pTask;
		pTask = NULL;
	}

	std::map<pMODEL, pMODELINFO>::iterator it = _mapModel.begin();
	for (; it != _mapModel.end();)
	{
		pMODELINFO pModelInfo = NULL;
		pModelInfo = it->second;
		if (pModelInfo)
		{
			delete pModelInfo;
			pModelInfo = NULL;
		}
		it = _mapModel.erase(it);
	}
	eExit.set();
	TRACE("RecognizeThread exit 0\n");
}

bool CRecognizeThread::HandleTask(pRECOGTASK pTask)
{
	//��������ʶ��
	pMODELINFO pModelInfo = NULL;
	std::map<pMODEL, pMODELINFO>::iterator it = _mapModel.find(pTask->pPaper->pModel);		//pTask->pModel
	if (it == _mapModel.end())
	{
		pModelInfo = new MODELINFO;
		_mapModel.insert(std::map<pMODEL, pMODELINFO>::value_type(pTask->pPaper->pModel, pModelInfo));	//pTask->pModel

		std::string strLog = "����ģ��";
		g_pLogger->information(strLog);

		pModelInfo->pModel = pTask->pPaper->pModel;		//pTask->pModel;
	#if 0	//���Բ���Ҫ
		bool bResult = LoadModel(pModelInfo);
		if (!bResult)
			return bResult;
	#endif
	}
	else
		pModelInfo = it->second;

	PaperRecognise(pTask->pPaper, pModelInfo);
	bool bRecogAllPic = true;
	for (auto objPic : pTask->pPaper->lPic)
	{
		if (objPic->nRecoged < 2)
		{
			bRecogAllPic = false;
			break;
		}
	}
	if (bRecogAllPic)
	{
		TRACE("----->�����Ծ�(%s)ʶ�����\n", pTask->pPaper->strStudentInfo.c_str());
		pTask->pPaper->bRecogComplete = true;
	}

	return true;
}
void CRecognizeThread::sharpenImage1(const cv::Mat &image, cv::Mat &result)
{
	//��������ʼ���˲�ģ��
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = _nSharpKernel_;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//��ͼ������˲�
	cv::filter2D(image, result, image.depth(), kernel);
}

bool CRecognizeThread::LoadModel(pMODELINFO pModelInfo)
{
	USES_CONVERSION;
	for (int i = 0; i < pModelInfo->pModel->nPicNum; i++)
	{
		std::string strModelPicPath = g_strModelSavePath + "\\" + pModelInfo->pModel->strModelName + "\\" + pModelInfo->pModel->vecPaperModel[i]->strModelPicName;

		cv::Mat matSrc;
		try
		{
			matSrc = cv::imread(strModelPicPath);
		}
		catch (cv::Exception& exc)
		{
			std::string strLog = "���ļ�ʧ��: " + exc.msg;
			g_pLogger->information(strLog);		
			return false;
		}
#ifdef PIC_RECTIFY_TEST
		Mat dst;
		Mat rotMat;
		PicRectify(matSrc, dst, rotMat);
		Mat matImg;
		if (dst.channels() == 1)
			cvtColor(dst, matImg, CV_GRAY2BGR);
		else
			matImg = dst;
#else
		Mat matImg = matSrc;
#endif
		pModelInfo->vecMatSrc.push_back(matImg);
	}
	return true;
}

void CRecognizeThread::PaperRecognise(pST_PaperInfo pPaper, pMODELINFO pModelInfo)
{
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		clock_t start_pic, end_pic;
		start_pic = clock();

		if ((*itPic)->nRecoged)		//�Ѿ�ʶ���������ʶ��
			continue;

		(*itPic)->nRecoged = 1;

		if (i >= pModelInfo->pModel->vecPaperModel.size())
		{
			(*itPic)->nRecoged = 2;
			continue;
		}

		int nCount = pModelInfo->pModel->vecPaperModel[i]->lH_Head.size() + pModelInfo->pModel->vecPaperModel[i]->lV_Head.size() + pModelInfo->pModel->vecPaperModel[i]->lABModel.size()
			+ pModelInfo->pModel->vecPaperModel[i]->lCourse.size() + pModelInfo->pModel->vecPaperModel[i]->lQK_CP.size() + pModelInfo->pModel->vecPaperModel[i]->lWJ_CP.size() + pModelInfo->pModel->vecPaperModel[i]->lGray.size()
			+ pModelInfo->pModel->vecPaperModel[i]->lWhite.size() + pModelInfo->pModel->vecPaperModel[i]->lSNInfo.size() + pModelInfo->pModel->vecPaperModel[i]->lOMR2.size()
			+ pModelInfo->pModel->vecPaperModel[i]->lElectOmr.size();
		if (!nCount)	//�����ǰģ���Ծ�û��У���Ͳ���Ҫ�����Ծ�򿪲�����ֱ����һ���Ծ�
		{
			(*itPic)->nRecoged = 2;
			continue;
		}

		std::string strPicFileName = (*itPic)->strPicName;
		Mat matCompSrcPic;
		bool bOpenSucc = false;
		for (int i = 0; i < 3; i++)
		{
			if (!bOpenSucc)
			{
				try
				{
					Poco::File fPic(CMyCodeConvert::Gb2312ToUtf8((*itPic)->strPicPath));
					if (!fPic.exists())
						continue;
					matCompSrcPic = imread((*itPic)->strPicPath);			//imread((*itPic)->strPicPath);
					bOpenSucc = true;
					break;
				}
				catch (cv::Exception& exc)
				{
					Sleep(500);
				}
			}			
		}
		if (!bOpenSucc)
		{
			HandleWithErrPaper(pPaper);
			std::string strLog = "���δ��ļ���ʧ��2: " + (*itPic)->strPicPath;
			g_pLogger->information(strLog);

			(*itPic)->nRecoged = 2;
			continue;
		}
		
#ifdef PIC_RECTIFY_TEST	//ͼ����ת��������
		Mat matDst;
		Mat matCompPic;
		Mat rotMat;
		PicRectify(matCompSrcPic, matDst, rotMat);
		if (matDst.channels() == 1)
			cvtColor(matDst, matCompPic, CV_GRAY2BGR);
		else
			matCompPic = matDst;
#else
		Mat matCompPic = matCompSrcPic;
#endif

		clock_t end1_pic = clock();

		bool bFind = false;
		int nPic = i;

		m_ptFixCP = Point(0, 0);

		if (g_nOperatingMode == 1)
		{
			bool bResult = RecogFixCP(nPic, matCompPic, *itPic, pModelInfo);
		#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			bResult = PicTransfer(nPic, matCompPic, (*itPic)->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, inverseMat);
		#endif
			bResult = RecogHHead(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogVHead(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogABModel(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogCourse(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogQKCP(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogWJCP(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogGrayCP(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogWhiteCP(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogSN(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogOMR(nPic, matCompPic, *itPic, pModelInfo);
			bResult = RecogElectOmr(nPic, matCompPic, *itPic, pModelInfo);
		}
		else
		{
			bool bResult = RecogFixCP(nPic, matCompPic, *itPic, pModelInfo);
		#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			if (bResult) bResult = PicTransfer(nPic, matCompPic, (*itPic)->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, inverseMat);
		#endif
			if(bResult) bResult = RecogHHead(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogVHead(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogABModel(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogCourse(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogQKCP(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogWJCP(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogGrayCP(nPic, matCompPic, *itPic, pModelInfo);
			if(bResult) bResult = RecogWhiteCP(nPic, matCompPic, *itPic, pModelInfo);
			if (bResult) bResult = RecogSN(nPic, matCompPic, *itPic, pModelInfo);
			if (bResult) bResult = RecogOMR(nPic, matCompPic, *itPic, pModelInfo);
			if (bResult) bResult = RecogElectOmr(nPic, matCompPic, *itPic, pModelInfo);
			if(!bResult) bFind = true;
			if (bFind)
			{
				HandleWithErrPaper(pPaper);
				break;									//�ҵ������Ծ�������㣬��������һ���Ծ�ļ��
			}
		}

		(*itPic)->nRecoged = 2;

		end_pic = clock();
		TRACE("�Ծ� %s ��ʱ��: %d, ʶ����ʱ��: %d\n", strPicFileName.c_str(), end1_pic - start_pic, end_pic - start_pic);
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "�Ծ� %s ��ʱ��: %d, ʶ����ʱ��: %d\n", strPicFileName.c_str(), end1_pic - start_pic, end_pic - start_pic);
		g_pLogger->information(szLog);
	}

#ifdef PrintRecogLog	//test log
	std::string strPaperLog = "�Ծ�(";
	strPaperLog.append(pPaper->strStudentInfo);
	strPaperLog.append(")ʶ����: ");

	int nNullCount = 0;
	int nDoubtCount = 0;
	int nEqualCount = 0;

	char szSN[100] = { 0 };
	sprintf_s(szSN, "SN(%s), ", pPaper->strSN.c_str());
	strPaperLog.append(szSN);

// 	int nOmrCount = 0;
// 	for (int k = 0; k < pModelInfo->pModel->vecPaperModel.size(); k++)
// 	{
// 		nOmrCount += pModelInfo->pModel->vecPaperModel[k]->lOMR2.size();
// 	}
// 	int nPapersCount = (static_cast<pPAPERSINFO>(pPaper->pPapers))->lPaper.size() + (static_cast<pPAPERSINFO>(pPaper->pPapers))->lIssue.size();
// 	char szStatisticsInfo[300] = { 0 };
// 	sprintf_s(szStatisticsInfo, "\nͳ����Ϣ: omrDoubt = %.2f(%d/%d), omrNull = %.2f(%d/%d), zkzhNull = %.2f(%d/%d)\n", (float)(static_cast<pPAPERSINFO>(pPaper->pPapers))->nOmrDoubt / nOmrCount, (static_cast<pPAPERSINFO>(pPaper->pPapers))->nOmrDoubt, nOmrCount, \
// 			  (float)(static_cast<pPAPERSINFO>(pPaper->pPapers))->nOmrNull / nOmrCount, (static_cast<pPAPERSINFO>(pPaper->pPapers))->nOmrNull, nOmrCount, \
// 			  (float)(static_cast<pPAPERSINFO>(pPaper->pPapers))->nSnNull / nPapersCount, (static_cast<pPAPERSINFO>(pPaper->pPapers))->nSnNull, nPapersCount);
// 	strPaperLog.append(szStatisticsInfo);

	OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
	for (; itOmr != pPaper->lOmrResult.end(); itOmr++)
	{
		//����ͳ��
		if (itOmr->nDoubt == 0)
			nEqualCount++;
		else if (itOmr->nDoubt == 1)
			nDoubtCount++;
		else
			nNullCount++;

		char szSingle[10] = { 0 };
		if (itOmr->nSingle == 0)
			strcpy_s(szSingle, "��");
		else
			strcpy_s(szSingle, "��");
		
		std::string strItemLog;
		if (itOmr->nDoubt)	//itOmr->nDoubt
		{
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				char szTmp[100] = { 0 }; 
				sprintf_s(szTmp, "%c,ʶ��=%.3f(R/S=%.1f/%.1f),Succ:%d. ", itRect->nAnswer + 65, \
					itRect->fRealValuePercent, itRect->fRealValue, itRect->fStandardValue, itRect->fRealValuePercent > itRect->fStandardValuePercent);
				strItemLog.append(szTmp);
			}
			strItemLog.append("\n");
			RECTLIST::iterator itRect2 = itOmr->lSelAnswer.begin();
			for (; itRect2 != itOmr->lSelAnswer.end(); itRect2++)
			{
				char szTmp[200] = { 0 };
				sprintf_s(szTmp, "%c,�ܶ�=%.3f/%.3f,�Ҷ�=%.3f(%.3f-%.3f) ", itRect2->nAnswer + 65, \
						  itRect2->fRealDensity, itRect2->fStandardDensity, itRect2->fRealMeanGray - itRect2->fStandardMeanGray, itRect2->fRealMeanGray, itRect2->fStandardMeanGray);
				strItemLog.append(szTmp);
			}
			strItemLog.append("\n");
			RECTLIST::iterator itRect3 = itOmr->lSelAnswer.begin();
			for (; itRect3 != itOmr->lSelAnswer.end(); itRect3++)
			{
				char szTmp[200] = { 0 };
				sprintf_s(szTmp, "%c,�Ҷ�=%.3f(%.3f-%.3f), ", itRect3->nAnswer + 65, \
						  itRect3->fRealMeanGray - itRect3->fStandardMeanGray, itRect3->fRealMeanGray, itRect3->fStandardMeanGray);
				strItemLog.append(szTmp);
			}
		}

		//++++++++	test	++++++++
		std::vector<pRECTINFO> vecItemsDensityDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDensityDiff;
		calcOmrDensityDiffVal(itOmr->lSelAnswer, vecItemsDensityDesc, vecOmrItemDensityDiff);
		strItemLog.append("\n[");
		for (int i = 0; i < vecOmrItemDensityDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.5f ", vecOmrItemDensityDiff[i].szVal, vecOmrItemDensityDiff[i].fDiff);
			strItemLog.append(szTmp);
		}
		strItemLog.append("]");

		float fDensityThreshold = 0.0;
		strItemLog.append("�ܶ�ѡ�з�ֵ:[");
		for (int i = 0; i < vecOmrItemDensityDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.5f ", vecOmrItemDensityDiff[i].szVal, _dDiffThread_Fix_ + fDensityThreshold * 0.5);
			strItemLog.append(szTmp);
			if ((vecOmrItemDensityDiff[i].fDiff >= _dDiffThread_Fix_ + fDensityThreshold * 0.5))
				fDensityThreshold += vecOmrItemDensityDiff[i].fDiff;
		}
		strItemLog.append("]");

		//test��־
		float fDensityMeanPer = 0.0;
		for (int i = 0; i < vecItemsDensityDesc.size(); i++)
			fDensityMeanPer += vecItemsDensityDesc[i]->fRealValuePercent;
		fDensityMeanPer = fDensityMeanPer / vecItemsDensityDesc.size();

		char szTmp2[40] = { 0 };
		sprintf_s(szTmp2, "�ܶ�ƽ��ֵ:%.3f, ", fDensityMeanPer);
		strItemLog.append(szTmp2);

		strItemLog.append("���ܶ�ƽ��ֵ��ֵ:[");
		for (int i = 0; i < vecItemsDensityDesc.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%c:%.5f ", vecItemsDensityDesc[i]->nAnswer + 65, vecItemsDensityDesc[i]->fRealValuePercent - fDensityMeanPer);
			strItemLog.append(szTmp);
		}
		strItemLog.append("]");

		std::vector<pRECTINFO> vecItemsGrayDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemGrayDiff;
		calcOmrGrayDiffVal(itOmr->lSelAnswer, vecItemsGrayDesc, vecOmrItemGrayDiff);
		strItemLog.append("\n[");
		for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.3f ", vecOmrItemGrayDiff[i].szVal, vecOmrItemGrayDiff[i].fDiff);
			strItemLog.append(szTmp);
		}
		strItemLog.append("]");

		float fMeanGrayDiff = 0.0;
		for (int i = 0; i < vecItemsGrayDesc.size(); i++)
		{
			fMeanGrayDiff += (vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray);
		}
		fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayDesc.size();
		char szTmp1[40] = { 0 };
		sprintf_s(szTmp1, "ƽ���ҶȲ�:%.3f, ", fMeanGrayDiff);
		strItemLog.append(szTmp1);
		strItemLog.append("�Ҷ�ѡ�еķ�ֵ[");
		float fThreld = 0.0;
		float fGrayDiffLast = 0.0;		//����һ���ж�ѡ�е�ѡ�����һ��ѡ��ѡ���жϵ�����
		for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
		{
			float fGrayThresholdGray = vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray - fMeanGrayDiff;
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.3f ", vecOmrItemGrayDiff[i].szVal, _dDiffThread_3_ + fGrayThresholdGray + fGrayDiffLast);
			strItemLog.append(szTmp);
			fGrayDiffLast += abs(fGrayThresholdGray) / 2;
		}
		strItemLog.append("]");
		//--------------------------
		
		char szOmrItem[3060] = { 0 };
		if (itOmr->nDoubt)	//itOmr->nDoubt
			sprintf_s(szOmrItem, "%d(%s):%s[%s -- %s -- %s] Doubt(%d)\t==>%s\n", itOmr->nTH, szSingle, itOmr->strRecogVal.c_str(), itOmr->strRecogVal1.c_str(), itOmr->strRecogVal2.c_str(), itOmr->strRecogVal3.c_str(), itOmr->nDoubt, strItemLog.c_str());	//szItemInfo
		else
			sprintf_s(szOmrItem, "%d(%s):%s[%s -- %s -- %s] Doubt(%d)\n", itOmr->nTH, szSingle, itOmr->strRecogVal.c_str(), itOmr->strRecogVal1.c_str(), itOmr->strRecogVal2.c_str(), itOmr->strRecogVal3.c_str(), itOmr->nDoubt);

		strPaperLog.append(szOmrItem);
	}
	int nCount = pPaper->lOmrResult.size();
	if (nCount)
	{
		std::string strCorrectPerInfo = Poco::format("%s����[%u],��ֵ%d(%.2f%%),����%d(%.2f%%),�޻���%d(%.2f%%)", pPaper->strStudentInfo, pPaper->lOmrResult.size(), nNullCount, (double)nNullCount / nCount * 100, \
													 nDoubtCount, (double)nDoubtCount / nCount * 100, nEqualCount, (double)nEqualCount / nCount * 100);

		g_pLogger->information(strCorrectPerInfo);
	}
	g_pLogger->information(strPaperLog);
#endif
}

inline bool CRecognizeThread::Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	Mat matCompRoi;
	Rect rt = rc.rt;
	bool bResult = false;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matCompPic.cols)
		{
			rt.width = matCompPic.cols - rt.x;
		}
		if (rt.br().y > matCompPic.rows)
		{
			rt.height = matCompPic.rows - rt.y;
		}

 		matCompRoi = matCompPic(rt);

		Mat imag_src, img_comp;
		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);
		cv::GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//_nGauseKernel_
		SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

		const int channels[1] = { 0 };
		const float* ranges[1];
		const int histSize[1] = { 1 };
		float hranges[2];
#if 1	//for test

		if (rc.eCPType != WHITE_CP)
		{
			hranges[0] = g_nRecogGrayMin;
			hranges[1] = static_cast<float>(rc.nThresholdValue);
			ranges[0] = hranges;
		}
		else
		{
			hranges[0] = static_cast<float>(rc.nThresholdValue);
			hranges[1] = g_nRecogGrayMax_White;	//255		//256ʱ��ͳ����ȫ�հ׵ĵ㣬��RGBֵΪ255����ȫ�հ׵�;255ʱֻ��ͳ�Ƶ�RGBΪ254��ֵ��255��ֵͳ�Ʋ���
			ranges[0] = hranges;
		}
#else
		if (rc.eCPType != WHITE_CP && rc.eCPType != OMR)
		{
			hranges[0] = g_nRecogGrayMin;
			hranges[1] = static_cast<float>(rc.nThresholdValue);
			ranges[0] = hranges;
		}
		else if (rc.eCPType == OMR)
		{
			hranges[0] = g_nRecogGrayMin_OMR;
			hranges[1] = g_RecogGrayMax_OMR;
			ranges[0] = hranges;
		}
		else
		{
			hranges[0] = static_cast<float>(rc.nThresholdValue);
			hranges[1] = g_nRecogGrayMax_White;	//255		//256ʱ��ͳ����ȫ�հ׵ĵ㣬��RGBֵΪ255����ȫ�հ׵�;255ʱֻ��ͳ�Ƶ�RGBΪ254��ֵ��255��ֵͳ�Ʋ���
			ranges[0] = hranges;
		}
#endif
		MatND src_hist, comp_hist;
		cv::calcHist(&matCompRoi, 1, channels, Mat(), comp_hist, 1, histSize, ranges, false);

		double maxValComp = 0;
		double minValComp = 0;

		//�ҵ�ֱ��ͼ�е����ֵ����Сֵ
		cv::minMaxLoc(comp_hist, &minValComp, &maxValComp, 0, 0);
		int nSize = comp_hist.rows;

		// ��������ֵΪͼ��߶ȵ�90%
		int hpt = static_cast<int>(0.9 * 256);		//histSize
		for (int h = 0; h < nSize; h++)	//histSize
		{
// 			float binValSrc = src_hist.at<float>(h);
// 			int intensitySrc = static_cast<int>(binValSrc*hpt / maxValSrc);
//			TRACE("h = %d. binValSrc = %f. intensitySrc = %d\n", h, binValSrc, intensitySrc);

			float binValComp = comp_hist.at<float>(h);
			int intensityComp = static_cast<int>(binValComp*hpt / maxValComp);

			rc.fRealValue = binValComp;
			rc.fRealArea = rc.rt.area();
			rc.fRealDensity = rc.fRealValue / rc.fRealArea;
			

			float fStandardCompare = (rc.fRealArea / rc.fStandardArea) * rc.fStandardValue;
			if (binValComp == 0 && rc.fStandardValue == 0)
				rc.fRealValuePercent = 1.0;
			else if (rc.fStandardValue == 0)
				rc.fRealValuePercent = 1.0;
			else
				rc.fRealValuePercent = binValComp / rc.fStandardValue;
			bResult = true;
			break;
		}

#if 1
		MatND mean;
		MatND stddev;
		meanStdDev(matCompRoi, mean, stddev);

		IplImage *src;
		src = &IplImage(mean);
		rc.fRealMeanGray = cvGetReal2D(src, 0, 0);

		IplImage *src2;
		src2 = &IplImage(stddev);
		rc.fRealStddev = cvGetReal2D(src2, 0, 0);
#else
		MatND src_hist2;
		const int histSize2[1] = { 256 };	//rc.nThresholdValue - g_nRecogGrayMin
		const float* ranges2[1];
		float hranges2[2];
		hranges2[0] = 0;
		hranges2[1] = 255;
		ranges2[0] = hranges2;
		cv::calcHist(&matCompRoi, 1, channels, Mat(), src_hist2, 1, histSize2, ranges2, true, false);
		int nCount = 0;
		for (int i = 0; i < 256; i++)
		{
			nCount += i * src_hist2.at<float>(i);
		}
		rc.fRealMeanGray = nCount / rc.fRealArea;
#endif
	}
	catch (cv::Exception &exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "CRecognizeThread::Recog error. detail: %s\n", exc.msg);
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	
	return bResult;
}

bool CRecognizeThread::RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
// 	if (pModelInfo->pModel->nHasHead != 0)	//��ͬ��ͷ�ģ�����Ҫ���ж���ʶ��
// 		return bResult;

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.begin();
	for (int i = 0; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.end(); i++, itCP++)
	{
		RECTINFO rc = *itCP;

		std::vector<Rect>RectCompList;
		try
		{
			if (rc.rt.x < 0) rc.rt.x = 0;
			if (rc.rt.y < 0) rc.rt.y = 0;
			if (rc.rt.br().x > matCompPic.cols)
			{
				rc.rt.width = matCompPic.cols - rc.rt.x;
			}
			if (rc.rt.br().y > matCompPic.rows)
			{
				rc.rt.height = matCompPic.rows - rc.rt.y;
			}

			Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

			int nRealThreshold = 150;
			RECTLIST::iterator itFix = pModelInfo->pModel->vecPaperModel[nPic]->lFix.begin();
			for (int j = 0; itFix != pModelInfo->pModel->vecPaperModel[nPic]->lFix.end(); j++, itFix++)
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
			calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

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
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	��ͨ�հ׿��ʶ��	Size(_nDilateKernel_, _nDilateKernel_)
			dilate(matCompRoi, matCompRoi, element);

#if 1
			//		std::vector<std::vector<cv::Point> > vecContours;		//������Ϣ�洢
			//		m_vecContours.clear();
			//		std::vector<cv::Mat> vecContours;
			//		cv::findContours(matCompRoi.clone(), vecContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);	//hsvRe.clone()		//CV_RETR_EXTERNAL	//CV_CHAIN_APPROX_SIMPLE

			// 		vector<Mat> vecContours;
			// 		Mat hierarchy;
			// 		findContours(matCompRoi, vecContours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
			// 
			// 		std::vector<Rect>RectCompList;
			// 		for (int i = 0; i < vecContours.size(); i++)
			// 		{
			// 			cv::Mat matTmp = cv::Mat(vecContours[i]);
			// 			Rect rm = cv::boundingRect(matTmp);
			// 
			// 			// 			Rect rm = cv::boundingRect(vecContours[i]);
			// 			// 			vecContours[i].release();
			// 
			// 			RectCompList.push_back(rm);
			// 			matTmp.release();
			// 		}

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//��ȡ����  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm = rm + rc.rt.tl();
				RectCompList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);
#else
			//		std::vector<std::vector<cv::Point> > vecContours;		//������Ϣ�洢
			m_vecContours.clear();
			//		std::vector<cv::Mat> vecContours;
			cv::findContours(matCompRoi.clone(), m_vecContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);	//hsvRe.clone()		//CV_RETR_EXTERNAL	//CV_CHAIN_APPROX_SIMPLE

			//		bool bResult = false;
			std::vector<Rect>RectCompList;
			for (int i = 0; i < m_vecContours.size(); i++)
			{
				Rect rm = cv::boundingRect(cv::Mat(m_vecContours[i]));
				//			Rect rm = cv::boundingRect(cv::Mat(vecContours[i]));

				// 			std::vector<cv::Point> v;
				// 			m_vecContours[i].swap(v);
				// 			if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50 || rm.area() < 70)
				// 			{
				// 				TRACE("���������������ľ��ο�,(%d, %d, %d, %d)\n", rm.x, rm.y, rm.width, rm.height);
				// 				continue;
				// 			}
				RectCompList.push_back(rm);

				//			m_vecContours[i].clear();
				//			bResult = true;
			}
#endif
		}
		catch (cv::Exception& exc)
		{
			std::string strLog2 = Poco::format("ʶ�𶨵�(%d)�쳣: %s\n", i, exc.msg.c_str());
			strLog.append(strLog2);
			TRACE(strLog2.c_str());

			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc); 
			if (g_nOperatingMode == 2)
			{
				bResult = false;						//�ҵ������
				break;
			}
		}

		std::string strLog2;	//��ʱ��־����¼���ξ���ʶ����
		bool bFindRect = false;
		if(RectCompList.size() == 0)
			bFindRect = true;
		else
		{
			std::sort(RectCompList.begin(), RectCompList.end(), SortByArea);
			Rect& rtFix = RectCompList[0];

			RECTINFO rcFix;
			RECTLIST::iterator itFix = pModelInfo->pModel->vecPaperModel[nPic]->lFix.begin();
			for (int j = 0; itFix != pModelInfo->pModel->vecPaperModel[nPic]->lFix.end(); j++, itFix++)
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
				Recog(nPic, rcTmp, matCompPic, pPic, pModelInfo);
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
				m_ptFixCP.x = static_cast<int>(rtFix.x + rtFix.width / 2 + 0.5 + rc.rt.x);
				m_ptFixCP.y = static_cast<int>(rtFix.y + rtFix.height / 2 + 0.5 + rc.rt.y);

				// 			rtFix.x = rtFix.x + rc.rt.x;
				// 			rtFix.y = rtFix.y + rc.rt.y;

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
			if (g_nOperatingMode == 2)
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
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	if (pModelInfo->pModel->nHasHead == 0)
		return true;

	std::string strErrDesc;
	m_vecH_Head.clear();
	RECTLIST::iterator itRoi = pModelInfo->pModel->vecPaperModel[nPic]->lSelHTracker.begin();
	for (; itRoi != pModelInfo->pModel->vecPaperModel[nPic]->lSelHTracker.end(); itRoi++)
	{
		RECTINFO rc = *itRoi;
#if 1
		std::vector<Rect>RectCompList;
		try
		{
			if (rc.rt.x < 0) rc.rt.x = 0;
			if (rc.rt.y < 0) rc.rt.y = 0;
			if (rc.rt.br().x > matCompPic.cols)
			{
				rc.rt.width = matCompPic.cols - rc.rt.x;
			}
			if (rc.rt.br().y > matCompPic.rows)
			{
				rc.rt.height = matCompPic.rows - rc.rt.y;
			}

			Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

#ifdef USES_GETTHRESHOLD_ZTFB
			const int channels[1] = { 0 };
			const int histSize[1] = { 150 };
			float hranges[2] = { 0, 150 };
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = 150;
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
			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//��ȡ����  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			//ģ��ͼ���ˮƽͬ��ͷƽ������
			RECTLIST::iterator itBegin = pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.begin();
			RECTINFO rcFist = *itBegin;
			RECTINFO rcSecond = *(++itBegin);
			
			int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
			int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;
#if 0	//test
			float fOffset = 0.2;
			nMid_minW = rcSecond.rt.width * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_maxW = rcSecond.rt.width * (1 + fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_minH = rcSecond.rt.height * (1 - fOffset);		//ͬ��
			nMid_maxH = rcSecond.rt.height * (1 + fOffset);		//ͬ��

			nHead_minW = rcFist.rt.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nHead_maxW = rcFist.rt.width * (1 + fOffset);		//ͬ��
			nHead_minH = rcFist.rt.height * (1 - fOffset);		//ͬ��
			nHead_maxH = rcFist.rt.height * (1 + fOffset);		//ͬ��
#else
			if (pModelInfo->pModel->nType == 1)
			{
				int nMid_modelW = rcSecond.rt.width;
				int nMid_modelH = rcSecond.rt.height;
				int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
				nMidInterW = 3;
				nMidInterH = 3;
				nHeadInterW = 4;
				nHeadInterH = 4;
				nMid_minW = nMid_modelW - nMidInterW;
				nMid_maxW = nMid_modelW + nMidInterW;
				nMid_minH = nMid_modelH - nMidInterH;
				nMid_maxH = nMid_modelH + nMidInterH;

				nHead_minW = rcFist.rt.width - nHeadInterW;
				nHead_maxW = rcFist.rt.width + nHeadInterW;
				nHead_minH = rcFist.rt.height - nHeadInterH;
				nHead_maxH = rcFist.rt.height + nHeadInterH;

// 				float fOffset = 0.1;
// 				float fPer_W, fPer_H;	//ģ��ڶ��������һ����Ŀ��ߵı�����������Сֵ����
// 				fPer_W = 0.5;
// 				fPer_H = 0.25;
// 				int nMid_modelW = rcSecond.rt.width + 2;		//��2����Ϊ�ƾ�ģ����û�о�����߿����㣬������߿����������������Σ���Ҫ�������¸�1����λ���߿�
// 				int nMid_modelH = rcSecond.rt.height + 2;
// 				if (nMid_modelW < rcFist.rt.width * fPer_W + 0.5)	nMid_modelW = rcFist.rt.width * fPer_W + 0.5;
// 				if (nMid_modelH < rcFist.rt.height * fPer_H + 0.5)	nMid_modelH = rcFist.rt.height * fPer_H + 0.5;
// 				nMid_minW = nMid_modelW * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
// 				nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
// 				nMid_minH = nMid_modelH * (1 - fOffset);				//ͬ��
// 				nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//ͬ��
// 
// 				nHead_minW = rcFist.rt.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
// 				nHead_maxW = rcFist.rt.width * (1 + fOffset * 4) + 0.5;		//ͬ��
// 				nHead_minH = rcFist.rt.height * (1 - fOffset);				//ͬ��
// 				nHead_maxH = rcFist.rt.height * (1 + fOffset * 4) + 0.5;	//ͬ��
			}
			else
			{
				float fOffset_mid = 0.4;
				nMid_minW = rcSecond.rt.width * (1 - fOffset_mid);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nMid_maxW = rcSecond.rt.width * (1 + fOffset_mid);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nMid_minH = rcSecond.rt.height * (1 - fOffset_mid);		//ͬ��
				nMid_maxH = rcSecond.rt.height * (1 + fOffset_mid);		//ͬ��

				float fOffset_Head = 0.3;
				nHead_minW = rcFist.rt.width * (1 - fOffset_Head);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nHead_maxW = rcFist.rt.width * (1 + fOffset_Head);		//ͬ��
				nHead_minH = rcFist.rt.height * (1 - fOffset_Head);		//ͬ��
				nHead_maxH = rcFist.rt.height * (1 + fOffset_Head);		//ͬ��
			}
#endif

			int nYSum = 0;
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm.x = rm.x + rc.rt.x;
				rm.y = rm.y + rc.rt.y;
				
				if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
				{
					if (!(rm.width > nHead_minW && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//�ų���һ�������һ�����ͬ��ͷ
					{
						TRACE("����ˮƽͬ��ͷ(%d,%d,%d,%d), Ҫ��ΧW:[%d,%d], H[%d,%d], �ο���С(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
						continue;
					}
					else
					{
						TRACE("��βˮƽͬ��ͷ(����λ��ͬ��ͷ)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
					}
				}
				RectCompList.push_back(rm);
				nYSum += rm.y;
			}
			cvReleaseMemStorage(&storage);
//			int nYMean = nYSum / RectCompList.size();
		}
		catch (cv::Exception& exc)
		{
			std::string strLog = "ʶ��ˮƽͬ��ͷ�쳣: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;
			break;
		}
		if (RectCompList.size() == 0)
		{
			bResult = false;
			strErrDesc = "ˮƽͬ��ͷ����Ϊ0.";
		}
		else
		{
			//���ι���
			std::sort(RectCompList.begin(), RectCompList.end(), SortByPositionX2);
			std::vector<Rect>::iterator itHead = RectCompList.begin();
			for (int i = 0; itHead != RectCompList.end(); i++)
			{
				RECTINFO rcHead1;
				RECTLIST::iterator itModelHead = pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.begin();
				for (int j = 0; itModelHead != pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.end(); j++, itModelHead++)
				{
					if (j == i)
					{
						rcHead1 = *itModelHead;
						break;
					}
				}
				RECTINFO rcTmp = rcHead1;
				rcTmp.rt = RectCompList[i];
				Recog(nPic, rcTmp, matCompPic, pPic, pModelInfo);
				if (rcTmp.fRealArea / rcTmp.fStandardArea < 0.75 || rcTmp.fRealDensity / rcTmp.fStandardDensity < 0.8)
				{
					itHead = RectCompList.erase(itHead);
					i = i - 1;
				}
				else
					itHead++;
			}
			for (int i = 0; i < RectCompList.size(); i++)
			{
				RECTINFO rcHead;
				rcHead.rt = RectCompList[i];
				
				rcHead.nGaussKernel = rc.nGaussKernel;
				rcHead.nSharpKernel = rc.nSharpKernel;
				rcHead.nCannyKernel = rc.nCannyKernel;
				rcHead.nDilateKernel = rc.nDilateKernel;

				rcHead.eCPType = H_HEAD;
				m_vecH_Head.push_back(rcHead);
				//++ for test
				pPic->lNormalRect.push_back(rcHead);
				//--
			}
			std::sort(m_vecH_Head.begin(), m_vecH_Head.end(), SortByPositionX);
		}
		if(m_vecH_Head.size() != pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.size())
		{
			bResult = false;
			pPic->bFindIssue = true;
			for(int i = 0; i < m_vecH_Head.size(); i++)
				pPic->lIssueRect.push_back(m_vecH_Head[i]);

			strErrDesc = Poco::format("ˮƽͬ��ͷ����Ϊ%u, ��ģ��ˮƽͬ��ͷ����(%u)��һ��", m_vecH_Head.size(), pModelInfo->pModel->vecPaperModel[nPic]->lH_Head.size());
		}
#else
		GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic].lFix, rc.rt);
		bool bFindRect = Recog(nPic, rc, matCompPic, pPic, pModelInfo);

		if (bFindRect)
		{
			bResult = false;						//�ҵ������
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
		else
			pPic->lNormalRect.push_back(rc);
#endif
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��ˮƽͬ��ͷʧ��, ԭ��: %s, ͼƬ��: %s\n", strErrDesc.c_str(), pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	if (pModelInfo->pModel->nHasHead == 0)
		return true;

	std::string strErrDesc;
	m_vecV_Head.clear();
	RECTLIST::iterator itRoi = pModelInfo->pModel->vecPaperModel[nPic]->lSelVTracker.begin();
	for (; itRoi != pModelInfo->pModel->vecPaperModel[nPic]->lSelVTracker.end(); itRoi++)
	{
		RECTINFO rc = *itRoi;
#if 1
		std::vector<Rect>RectCompList;
		try
		{
			if (rc.rt.x < 0) rc.rt.x = 0;
			if (rc.rt.y < 0) rc.rt.y = 0;
			if (rc.rt.br().x > matCompPic.cols)
			{
				rc.rt.width = matCompPic.cols - rc.rt.x;
			}
			if (rc.rt.br().y > matCompPic.rows)
			{
				rc.rt.height = matCompPic.rows - rc.rt.y;
			}

			Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//_nGauseKernel_
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

#ifdef USES_GETTHRESHOLD_ZTFB
			const int channels[1] = { 0 };
			const int histSize[1] = { 150 };
			float hranges[2] = { 0, 150 };
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = 150;
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
			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//��ȡ����  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

#if 1
			//ģ��ͼ���ˮƽͬ��ͷƽ������
			RECTLIST::iterator itBegin = pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.begin();
			RECTINFO rcFist = *itBegin;
			RECTINFO rcSecond = *(++itBegin);

			int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
			int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;
#if 0	//test
			float fOffset = 0.2;
			nMid_minW = rcSecond.rt.width * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_maxW = rcSecond.rt.width * (1 + fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_minH = rcSecond.rt.height * (1 - fOffset);		//ͬ��
			nMid_maxH = rcSecond.rt.height * (1 + fOffset);		//ͬ��

			nHead_minW = rcFist.rt.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nHead_maxW = rcFist.rt.width * (1 + fOffset);		//ͬ��
			nHead_minH = rcFist.rt.height * (1 - fOffset);		//ͬ��
			nHead_maxH = rcFist.rt.height * (1 + fOffset);		//ͬ��
#else
			if (pModelInfo->pModel->nType == 1)
			{
				int nMid_modelW = rcSecond.rt.width;
				int nMid_modelH = rcSecond.rt.height;
				int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
				nMidInterW = 3;
				nMidInterH = 3;
				nHeadInterW = 4;
				nHeadInterH = 4;
				nMid_minW = nMid_modelW - nMidInterW;
				nMid_maxW = nMid_modelW + nMidInterW;
				nMid_minH = nMid_modelH - nMidInterH;
				nMid_maxH = nMid_modelH + nMidInterH;

				nHead_minW = rcFist.rt.width - nHeadInterW;
				nHead_maxW = rcFist.rt.width + nHeadInterW;
				nHead_minH = rcFist.rt.height - nHeadInterH;
				nHead_maxH = rcFist.rt.height + nHeadInterH;
// 				float fOffset = 0.1;
// 				float fPer_W, fPer_H;	//ģ��ڶ��������һ����Ŀ��ߵı�����������Сֵ����
// 				fPer_W = 0.5;
// 				fPer_H = 0.25;
// 				int nMid_modelW = rcSecond.rt.width + 2;		//��2����Ϊ�ƾ�ģ����û�о�����߿����㣬������߿����������������Σ���Ҫ�������¸�1����λ���߿�
// 				int nMid_modelH = rcSecond.rt.height + 2;
// 				if (nMid_modelW < rcFist.rt.width * fPer_W + 0.5)	nMid_modelW = rcFist.rt.width * fPer_W + 0.5;
// 				if (nMid_modelH < rcFist.rt.height * fPer_H + 0.5)	nMid_modelH = rcFist.rt.height * fPer_H + 0.5;
// 				nMid_minW = nMid_modelW * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
// 				nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
// 				nMid_minH = nMid_modelH * (1 - fOffset);				//ͬ��
// 				nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//ͬ��
// 
// 				nHead_minW = rcFist.rt.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
// 				nHead_maxW = rcFist.rt.width * (1 + fOffset * 4) + 0.5;		//ͬ��
// 				nHead_minH = rcFist.rt.height * (1 - fOffset);				//ͬ��
// 				nHead_maxH = rcFist.rt.height * (1 + fOffset * 4) + 0.5;	//ͬ��
			}
			else
			{
				float fOffset_mid = 0.4;
				nMid_minW = rcSecond.rt.width * (1 - fOffset_mid);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nMid_maxW = rcSecond.rt.width * (1 + fOffset_mid);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nMid_minH = rcSecond.rt.height * (1 - fOffset_mid);		//ͬ��
				nMid_maxH = rcSecond.rt.height * (1 + fOffset_mid);		//ͬ��

				float fOffset_Head = 0.3;
				nHead_minW = rcFist.rt.width * (1 - fOffset_Head);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nHead_maxW = rcFist.rt.width * (1 + fOffset_Head);		//ͬ��
				nHead_minH = rcFist.rt.height * (1 - fOffset_Head);		//ͬ��
				nHead_maxH = rcFist.rt.height * (1 + fOffset_Head);		//ͬ��
			}
#endif
			int nXMidSum = 0;	//x�����������ܺ�
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm.x = rm.x + rc.rt.x;
				rm.y = rm.y + rc.rt.y;

				if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
				{
					if (!(rm.width > nHead_minW && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//�ų���һ�������һ�����ͬ��ͷ
					{
						TRACE("���˴�ֱͬ��ͷ(%d,%d,%d,%d), Ҫ��ΧW:[%d,%d], H[%d,%d], �ο���С(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
						continue;
					}
					else
					{
						TRACE("��β��ֱͬ��ͷ(����λ��ͬ��ͷ)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
					}
				}
				RectCompList.push_back(rm);
				nXMidSum += (rm.x + rm.width / 2);
			}
			cvReleaseMemStorage(&storage);
			int nXMean = nXMidSum / RectCompList.size();

			//��������λ�ù���
// 			int nDeviation = 10;
// 			int xMin = nXMean - (rcFist.rt.width / 2 + nDeviation);
// 			int xMax = nXMean + (rcFist.rt.width / 2 + nDeviation);
// 			TRACE("��ֱͬ��ͷ����X = %d, ƫ��[%d,%d]\n", nXMean, xMin, xMax);
// 			std::vector<Rect>::iterator itHead = RectCompList.begin();
// 			for (int i = 0; itHead != RectCompList.end(); i++)
// 			{
// 				if(itHead->x < xMin || itHead->x > xMax)
// 					itHead = RectCompList.erase(itHead);
// 				else if(itHead->x > nXMean && itHead->br().x > xMax)
// 					itHead = RectCompList.erase(itHead);
// 				else
// 					itHead++;
// 			}



#else
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm.x = rm.x + rc.rt.x;
				rm.y = rm.y + rc.rt.y;
				if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50 || rm.area() < 70)
				{
					continue;
				}
				RectCompList.push_back(rm);
			}
#endif
		}
		catch (cv::Exception& exc)
		{
			std::string strLog = "ʶ��ֱͬ��ͷ�쳣: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;
			break;
		}
		if (RectCompList.size() == 0)
		{
			bResult = false;
			strErrDesc = "��ֱͬ��ͷ����Ϊ0.";
		}
		else
		{
			//���ι���
			std::sort(RectCompList.begin(), RectCompList.end(), SortByPositionY2);
			std::vector<Rect>::iterator itHead = RectCompList.begin();
			for (int i = 0; itHead != RectCompList.end(); i++)
			{
				RECTINFO rcHead1;
				RECTLIST::iterator itModelHead = pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.begin();
				for (int j = 0; itModelHead != pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.end(); j++, itModelHead++)
				{
					if (j == i)
					{
						rcHead1 = *itModelHead;
						break;
					}
				}
				RECTINFO rcTmp = rcHead1;
				rcTmp.rt = RectCompList[i];
				Recog(nPic, rcTmp, matCompPic, pPic, pModelInfo);
				if (rcTmp.fRealArea / rcTmp.fStandardArea < 0.7 || rcTmp.fRealDensity / rcTmp.fStandardDensity < 0.8)
				{
					itHead = RectCompList.erase(itHead);
					i = i - 1;
				}
				else
					itHead++;
			}
			for (int i = 0; i < RectCompList.size(); i++)
			{
				RECTINFO rcHead;
				rcHead.rt = RectCompList[i];

				rcHead.nGaussKernel = rc.nGaussKernel;
				rcHead.nSharpKernel = rc.nSharpKernel;
				rcHead.nCannyKernel = rc.nCannyKernel;
				rcHead.nDilateKernel = rc.nDilateKernel;

				rcHead.eCPType = V_HEAD;
				m_vecV_Head.push_back(rcHead);
				//++ for test
				pPic->lNormalRect.push_back(rcHead);
				//--
			}
			std::sort(m_vecV_Head.begin(), m_vecV_Head.end(), SortByPositionY);
		}
		if(m_vecV_Head.size() != pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.size())
		{
			bResult = false;
			pPic->bFindIssue = true;
			for(int i = 0; i < m_vecV_Head.size(); i++)
				pPic->lIssueRect.push_back(m_vecV_Head[i]);

			strErrDesc = Poco::format("��ֱͬ��ͷ����Ϊ%u, ��ģ�崹ֱͬ��ͷ����(%u)��һ��", m_vecV_Head.size(), pModelInfo->pModel->vecPaperModel[nPic]->lV_Head.size());
		}
#else
		GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic].lFix, rc.rt);
		bool bFindRect = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bFindRect)
		{
			bResult = false;						//�ҵ������
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
		else
			pPic->lNormalRect.push_back(rc);
#endif
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��ֱͬ��ͷʧ��, ԭ��: %s, ͼƬ��: %s\n", strErrDesc.c_str(), pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("ʶ��AB����\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lABModel.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lABModel.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bFindRect = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
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
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("ʶ���Ŀ\n");
	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lCourse.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lCourse.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
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
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//�ҵ������
			break;		//���������ʱ�������жϺ���ĵ㣬��ֹͣɨ��
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ���Ŀʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
//		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("ʶ���ĿУ���ʱ��: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("ʶ��ȱ��\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lQK_CP.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lQK_CP.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				((pST_PaperInfo)pPic->pPaper)->nQKFlag = 1;			//����ѧ��ȱ��
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "У��ʧ��, �쳣����, �����: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}
		
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//�ҵ������
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��ȱ��ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("ʶ��Υ��\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lWJ_CP.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lWJ_CP.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				((pST_PaperInfo)pPic->pPaper)->nWJFlag = 1;			//����ѧ��Υ��
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "У��ʧ��, �쳣����, �����: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//�ҵ������
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��Υ��ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("ʶ��Ҷȵ�\n");

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lGray.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lGray.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
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
		if (g_nOperatingMode == 2)
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
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lWhite.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lWhite.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModelInfo->pModel->nHasHead)
		{
			if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
			{
				bResult = false;
				pPic->bFindIssue = true;
				pPic->lIssueRect.push_back(rc);
				break;
			}
			rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
			rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
			rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
			rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
		}
		else
			GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
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
				g_pLogger->information(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "У��ʧ��, �쳣����, �����: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}

		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		if (g_nOperatingMode == 2)
		{
			bResult = false;						//�ҵ������
			break;
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��հ�У���ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	if (pModelInfo->pModel->nZkzhType == 2)
		bResult = RecogSn_code(nPic, matCompPic, pPic, pModelInfo);
	else
		bResult = RecogSn_omr(nPic, matCompPic, pPic, pModelInfo);
	return bResult;
}

bool CRecognizeThread::RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	int nNullCount = 0;
	int nDoubtCount = 0;
	int nEqualCount = 0;
	int nNullCount_1 = 0;	//��һ�ַ���ʶ����Ŀ�ֵ
	int nNullCount_2 = 0;	//�ڶ��ַ���ʶ����Ŀ�ֵ
	int nNullCount_3 = 0;	//�����ַ���ʶ����Ŀ�ֵ

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecOmr;
	OMRLIST::iterator itOmr = pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.begin();
	for (; itOmr != pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.end(); itOmr++)
	{
		pOMR_QUESTION pOmrQuestion = &(*itOmr);
		
		OMR_RESULT omrResult;
		omrResult.nTH = pOmrQuestion->nTH;
		omrResult.nSingle = pOmrQuestion->nSingle;

		std::vector<int> vecVal_calcHist;		//ֱ��ͼ�Ҷȼ����ʶ����
		std::vector<int> vecVal_threshold;		//��ֵ�������ʶ����
		RECTLIST::iterator itOmrItem = pOmrQuestion->lSelAnswer.begin();
		for (; itOmrItem != pOmrQuestion->lSelAnswer.end(); itOmrItem++)
		{
			RECTINFO rc = *itOmrItem;

			if (pModelInfo->pModel->nHasHead)
			{
				if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
				{
					bResult = false;
					pPic->bFindIssue = true;
					pPic->lIssueRect.push_back(rc);
					break;
				}
				rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
				rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
				rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
				rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
			}
			else
				GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);

			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModelInfo);
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
			omrResult.lSelAnswer.push_back(rc);

			#ifdef PaintOmrSnRect	//��ӡOMR��SNλ��
			pPic->lNormalRect.push_back(rc);
			#endif
		}

	#if 1
		std::string strRecogAnswer;
		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDiff;
		calcOmrDensityDiffVal(omrResult.lSelAnswer, vecItemsDesc, vecOmrItemDiff);

		float fCompThread = 0.0;		//�ܶȼ���ﵽҪ��ʱ����һ��ѡ����ܶȱ���ﵽ��Ҫ��
		float fDiffThread = 0.0;		//ѡ�������Ϳ�Ŀ����ܶ��ݶȷ�ֵ
		float fDiffExit = 0;			//�ܶȵ��ݶȵݼ�̫��ʱ��������Ϊ����ѡ��û����Ϳ����ʱ���ܶ��ݶȷ�ֵ
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
		float fDensityThreshold = 0.0;
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			//��������ѡ��Ҷ�ֵ�������ڻҶ�ֵ��ֵ������ֵ��ͬʱ���е�һ�����ĻҶ�ֵ����1.0������Ϊ�������Ϊѡ�еķ�ֵ����
			//(����1.0�Ƿ�ֹ��С�ĻҶ�ֵ��С��ʱ��Ӱ�췧ֵ�ж�)
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
			//++�ж�ȫѡ�����
			if (nFlag == vecOmrItemDiff.size() - 1)
			{
				if (vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent >= fCompThread + fDiffExit)	//����ܶ���͵�ѡ������ܶȴ��ڡ���ͱȽ��ܶ� + ����˳��ܶȲ������Ϊȫѡ
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
		else if (vecItemsDesc[vecOmrItemDiff.size()]->fRealValuePercent >= fCompThread + fDiffExit)	//����ܶ���͵�ѡ������ܶȴ��ڡ���ͱȽ��ܶ� + ����˳��ܶȲ������Ϊȫѡ
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

		float fCompThread = 0.0;		//�Ҷȼ���ﵽҪ��ʱ����һ��ѡ��ĻҶȱ���ﵽ��Ҫ��
		float fDiffThread = 0.0;		//ѡ�������Ϳ�Ŀ��ܻҶ��ݶȷ�ֵ
		float fDiffExit = 0;			//�Ҷȵ��ݶȵݼ�̫��ʱ��������Ϊ����ѡ��û����Ϳ����ʱ�ĻҶ��ݶȷ�ֵ
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
			//��������ѡ��Ҷ�ֵ�������ڻҶ�ֵ��ֵ������ֵ��ͬʱ���е�һ�����ĻҶ�ֵ����1.0������Ϊ�������Ϊѡ�еķ�ֵ����
			//(����1.0�Ƿ�ֹ��С�ĻҶ�ֵ��С��ʱ��Ӱ�췧ֵ�ж�)
			float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.1;
			if ((vecOmrItemDiff[i].fDiff >= fDiffThread && vecOmrItemDiff[i].fFirst > fCompThread) ||
				(vecOmrItemDiff[i].fDiff >= fDiffThread + fDiff && vecOmrItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
				if (vecOmrItemDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_calcHist.size())	//�Ҷ�ֵ�仯�ϴ�ֱ���˳��������ֱֵ���жϳ����ĸ���������ǰ�жϵ��������Ͳ���������
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
		RecogVal_Omr2(nPic, matCompPic, pPic, pModelInfo, omrResult);
		RecogVal_Omr3(nPic, matCompPic, pPic, pModelInfo, omrResult);

		std::string strRecogAnswer2 = omrResult.strRecogVal2;
		std::string strRecogAnswer3 = omrResult.strRecogVal3;

		if (strRecogAnswer1 == "") nNullCount_1++;
		if (strRecogAnswer2 == "") nNullCount_2++;
		if (strRecogAnswer3 == "") nNullCount_3++;

		int nDoubt = 0;
		if (strRecogAnswer1 == strRecogAnswer2 && strRecogAnswer1 == strRecogAnswer3)	//Ϊ���ж�ʱ������1�뷽��3׼ȷ�ȸ���
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
					//����2������3���ַ�����һ�ַ����жϷǿ�
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
						nDoubt = 0;
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
		//++ test	��������ѡ����ж�ֵ��ʶ��
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
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��OMRʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}

	int nCount = pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.size();

// 	std::stringstream ss;
// 	ss << "����" << nCount << ",��ֵ" << nNullCount << "(" << (double)nNullCount / nCount * 100 << "%)[No.1=" << nNullCount_1 << "(" << (double)nNullCount_1 / nCount * 100 << "%),No.2=" << nNullCount_2
// 		<< "(" << (double)nNullCount_2 / nCount * 100 << "%)],����" << nDoubtCount << "(" << (double)nDoubtCount / nCount * 100 << "%),�޻���" << nEqualCount << "(" << (double)nEqualCount / nCount * 100
// 		<< "%)";
// 	std::string strTmp = ss.str();

	if (bResult && nCount)
	{
		char szStatistics[150] = { 0 };
		sprintf_s(szStatistics, "ͼƬ(%s)ѡ������[%d],��ֵ%d(%.2f%%)[No.1=%d(%.2f%%),No.2=%d(%.2f%%), No.3=%d(%.2f%%)],����%d(%.2f%%),�޻���%d(%.2f%%)", pPic->strPicName.c_str(), nCount, nNullCount, (float)nNullCount / nCount * 100, \
				  nNullCount_1, (float)nNullCount_1 / nCount * 100, nNullCount_2, (float)nNullCount_2 / nCount * 100, nNullCount_3, (float)nNullCount_3 / nCount * 100, \
				  nDoubtCount, (float)nDoubtCount / nCount * 100, nEqualCount, (float)nEqualCount / nCount * 100);
		
		strLog.append(szStatistics);
	}	
	end = clock();
	std::string strTime = Poco::format("ʶ��Omrʱ��: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;

	std::vector<int> vecOmr;
	ELECTOMR_LIST::iterator itElectOmr = pModelInfo->pModel->vecPaperModel[nPic]->lElectOmr.begin();
	for (; itElectOmr != pModelInfo->pModel->vecPaperModel[nPic]->lElectOmr.end(); itElectOmr++)
	{
		pELECTOMR_QUESTION pOmrQuestion = &(*itElectOmr);

		ELECTOMR_QUESTION omrResult;
		omrResult.sElectOmrGroupInfo = pOmrQuestion->sElectOmrGroupInfo;

		std::vector<int> vecVal_calcHist;		//ֱ��ͼ�Ҷȼ����ʶ����
		std::vector<int> vecVal_threshold;		//��ֵ�������ʶ����
		RECTLIST::iterator itOmrItem = pOmrQuestion->lItemInfo.begin();
		for (; itOmrItem != pOmrQuestion->lItemInfo.end(); itOmrItem++)
		{
			RECTINFO rc = *itOmrItem;

			if (pModelInfo->pModel->nHasHead)
			{
				if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
				{
					bResult = false;
					pPic->bFindIssue = true;
					pPic->lIssueRect.push_back(rc);
					break;
				}
				rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
				rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
				rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
				rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
			}
			else
				GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);

			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModelInfo);
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

#ifdef PaintOmrSnRect	//��ӡOMR��SNλ��
			pPic->lNormalRect.push_back(rc);
#endif
		}

#if 1
		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDiff;
		calcOmrDensityDiffVal(omrResult.lItemInfo, vecItemsDesc, vecOmrItemDiff);

		float fCompThread = 0.0;		//�Ҷȼ���ﵽҪ��ʱ����һ��ѡ��ĻҶȱ���ﵽ��Ҫ��
		float fDiffThread = 0.0;		//ѡ�������Ϳ�Ŀ��ܻҶ��ݶȷ�ֵ
		float fDiffExit = 0;			//�Ҷȵ��ݶȵݼ�̫��ʱ��������Ϊ����ѡ��û����Ϳ����ʱ�ĻҶ��ݶȷ�ֵ
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
			//��������ѡ��Ҷ�ֵ�������ڻҶ�ֵ��ֵ������ֵ��ͬʱ���е�һ�����ĻҶ�ֵ����1.0������Ϊ�������Ϊѡ�еķ�ֵ����
			//(����1.0�Ƿ�ֹ��С�ĻҶ�ֵ��С��ʱ��Ӱ�췧ֵ�ж�)
			float fDiff = (fCompThread - vecOmrItemDiff[i].fFirst) * 0.1;
			if ((vecOmrItemDiff[i].fDiff >= fDiffThread && vecOmrItemDiff[i].fFirst > fCompThread) ||
				(vecOmrItemDiff[i].fDiff >= fDiffThread + fDiff && vecOmrItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
				if (vecOmrItemDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_calcHist.size())	//�Ҷ�ֵ�仯�ϴ�ֱ���˳��������ֱֵ���жϳ����ĸ���������ǰ�жϵ��������Ͳ���������
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
#else	//һ����ֱ��ͨ����ֵ�ж��Ƿ�ѡ�У����ã�����Ϳ���淶���Ҳ������������������
		std::string strRecogAnswer1;
		for (int i = 0; i < vecVal_calcHist.size(); i++)
		{
			char szVal[5] = { 0 };
			sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
			strRecogAnswer1.append(szVal);
		}
#endif
		
		std::string strRecogAnswer2 = strRecogAnswer1;		//Ŀǰ�ڶ��ַ���������
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

	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��ѡ����OMRʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}

	return bResult;
}

bool CRecognizeThread::RecogVal(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	Mat matSrcRoi, matCompRoi;
	Rect rt = rc.rt;
	bool bResult = false;
	try
	{
		matCompRoi = matCompPic(rt);

		Mat imag_src, img_comp;
		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		//ͼƬ��ֵ��
		threshold(matCompRoi, matCompRoi, 240, 255, THRESH_BINARY_INV);				//200, 255
#if 0
		//ȷ����ʴ�����ͺ˵Ĵ�С
		Mat element = getStructuringElement(MORPH_RECT, Size(4, 4));	//Size(4, 4)
		//���Ͳ���
		dilate(matCompRoi, matCompRoi, element);
		
		Mat element2 = getStructuringElement(MORPH_RECT, Size(15, 15));	//Size(4, 4)
		//��ʴ����1
		erode(matCompRoi, matCompRoi, element2);
		
		Mat element3 = getStructuringElement(MORPH_RECT, Size(4, 4));	//Size(4, 4)
		//��ʴ����2
		erode(matCompRoi, matCompRoi, element3);
		
		Mat element4 = getStructuringElement(MORPH_RECT, Size(5, 5));	//Size(4, 4)
		dilate(matCompRoi, matCompRoi, element4);
		
#else
	#if 0
		//ȷ����ʴ�����ͺ˵Ĵ�С
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
		//��ʴ����1
		erode(matCompRoi, matCompRoi, element);
	#else
		//ȷ����ʴ�����ͺ˵Ĵ�С
		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));	//Size(4, 4)
		//��ʴ����1
		erode(matCompRoi, matCompRoi, element);

		//ȷ����ʴ�����ͺ˵Ĵ�С
		Mat element2 = getStructuringElement(MORPH_RECT, Size(3, 3));	//Size(4, 4)
		//��ʴ����2
		erode(matCompRoi, matCompRoi, element2);

		//���Ͳ���
		dilate(matCompRoi, matCompRoi, element2);
		//���Ͳ���
		dilate(matCompRoi, matCompRoi, element);
	#endif
#endif
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//��ȡ����  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		std::vector<Rect>RectCompList;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			RectCompList.push_back(rm);

// 			cv::Rect rtTmp = rm;
// 			rtTmp.x += rt.x;
// 			rtTmp.y += rt.y;
// 			rectangle(matCompPic, rtTmp, CV_RGB(255, 0, 0));
		}
		cvReleaseMemStorage(&storage);
		if (RectCompList.size() == 0)
			bResult = false;
		else
			bResult = true;
	}
	catch (cv::Exception &exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "CRecognizeThread::RecogVal error. detail: %s\n", exc.msg);
		g_pLogger->information(szLog);
		TRACE(szLog);
	}

	return bResult;
}

inline bool CRecognizeThread::RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, RECTLIST& lSelInfo, std::string& strResult)
{
	Mat matCompRoi;
	
	bool bResult = false;

	//omr��Ĵ�С�߶�
	RECTLIST::iterator itItem = lSelInfo.begin();
	RECTLIST::reverse_iterator itEndItem = lSelInfo.rbegin();

 	cv::Point pt1, pt2;
	if (itItem->eCPType == SN)
	{
		switch (itItem->nRecogFlag)
		{
			case 10:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 9:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
			case 6:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 5:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
		}
	}
	else if (itItem->eCPType == OMR)
	{
		switch (itItem->nRecogFlag)
		{
			case 42:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 41:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 38:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
			case 37:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
			case 26:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 25:
				pt1 = itItem->rt.tl() - cv::Point(3, 3);
				pt2 = itEndItem->rt.br() + cv::Point(3, 3);
				break;
			case 22:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
			case 21:
				pt1 = itEndItem->rt.tl() - cv::Point(3, 3);
				pt2 = itItem->rt.br() + cv::Point(3, 3);
				break;
		}
	}
// 	pt1 = itItem->rt.tl() - cv::Point(3, 3);
// 	pt2 = itEndItem->rt.br() + cv::Point(3, 3);
	Rect rt = cv::Rect(pt1, pt2);	//ABCD������Ŀ��ѡ����

	RECTLIST::iterator itFirst = itItem++;
#if 0
	int nSpace = 0;		//ѡ���Ŀո����(�����ǿ������Ǹ�)
	switch (itItem->nRecogFlag)	//��ȡѡ���Ŀհ������Ȼ�߶�
	{
		case 42:	//101010
			nSpace = itItem->rt.tl().y - itFirst->rt.br().y;
			break;
		case 41:	//101001
			nSpace = itItem->rt.tl().y - itFirst->rt.br().y;
			break;
		case 38:	//100110
			nSpace = itFirst->rt.tl().y - itItem->rt.br().y;
			break;
		case 37:	//100101
			nSpace = itFirst->rt.tl().y - itItem->rt.br().y;
			break;
		case 26:
			nSpace = itItem->rt.tl().x - itFirst->rt.br().x;
			break;
		case 25:
			nSpace = itItem->rt.tl().x - itFirst->rt.br().x;
			break;
		case 22:
			nSpace = itFirst->rt.tl().x - itItem->rt.br().x;
			break;
		case 21:
			nSpace = itFirst->rt.tl().x - itItem->rt.br().x;
			break;
		default:
			nSpace = max(abs(itFirst->rt.br().x - itItem->rt.tl().x), abs(itFirst->rt.br().y - itItem->rt.tl().y));
			break;
	}
#endif
	int nOmrMinW, nOmrMinH, nAreaMin;
	nOmrMinW = itFirst->rt.width * 0.4;
	nOmrMinH = itFirst->rt.height * 0.4;
	nAreaMin = itFirst->rt.area() * 0.3;
	//���ݴ�С������ȹ���һ�¿��ܿ�ѡ����ŵ����

	try
	{
		cv::Point ptNew1, ptNew2;
		if (!pModelInfo->pModel->nHasHead)	//��ͬ��ͷʱ�����������ض�λ
		{
			if (rt.x < 0) rt.x = 0;
			if (rt.y < 0) rt.y = 0;
			if (rt.br().x > matCompPic.cols)
			{
				rt.width = matCompPic.cols - rt.x;
			}
			if (rt.br().y > matCompPic.rows)
			{
				rt.height = matCompPic.rows - rt.y;
			}
			matCompRoi = matCompPic(rt);

			cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			//ͼƬ��ֵ��
			threshold(matCompRoi, matCompRoi, _nThreshold_Recog2_, 255, THRESH_BINARY_INV);				//200, 255

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//��ȡ����  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			bool bFirst = true;
			std::vector<Rect> RectBaseList;
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				cv::Rect rm = aRect;

				if (rm.width < nOmrMinW || rm.height < nOmrMinH || rm.area() < nAreaMin)		//************	��������ŵ�ѡ����������	************
					continue;

				//��Ҫȥ�����ܰ��������


				if (bFirst)
				{
					bFirst = false;
					ptNew1 = rm.tl() + pt1;
					ptNew2 = rm.br() + pt1;
				}
				if (itItem->nRecogFlag >= 37)	//ѡ��Ϊ�����
				{
					if (rm.tl().y + pt1.y < ptNew1.y)	ptNew1 = rm.tl() + pt1;
					if (rm.br().y + pt1.y > ptNew2.y)	ptNew2 = rm.br() + pt1;
				}
				else  //ѡ��Ϊ�����
				{
					if (rm.tl().x + pt1.x < ptNew1.x)	ptNew1 = rm.tl() + pt1;
					if (rm.br().x + pt1.x > ptNew2.x)	ptNew2 = rm.br() + pt1;
				}
				RectBaseList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);

			if (RectBaseList.size() <= lSelInfo.size())	//ʶ����ľ�����������ѡ��һ�£�������ɾ�������������ȥ�µ������ԭ�����м�ֵ
			{
				ptNew1 = (pt1 + ptNew1) / 2;
				ptNew2 = (pt2 + ptNew2) / 2;
			}

// 			if (itItem->nRecogFlag >= 37)
// 				std::sort(RectBaseList.begin(), RectBaseList.end(), SortByPositionY2);
// 			else
// 				std::sort(RectBaseList.begin(), RectBaseList.end(), SortByPositionX2);
		}
		else
		{
			ptNew1 = pt1;
			ptNew2 = pt2;
		}
		//�����µ�����������ѡ�������ε���Ϳ���
//		matCompRoi.deallocate();
		matCompRoi = matCompPic(cv::Rect(ptNew1, ptNew2));
		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

// 		GaussianBlur(matCompRoi, matCompRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
// 		sharpenImage1(matCompRoi, matCompRoi);

		//++�Ȼ�ȡ��ֵ�ͱ�׼��ټ����µĶ�ֵ����ֵ	2017.4.27
		const int channels[1] = { 0 };
		const int histSize[1] = { _nThreshold_Recog2_ };
		float hranges[2] = { 0, _nThreshold_Recog2_ };
		const float* ranges[1];
		ranges[0] = hranges;
		MatND hist;
		calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		int nThreshold = _nThreshold_Recog2_;
		if (nCount > 0)
		{
			float fMean = (float)nSum / nCount;		//��ֵ

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);
			nThreshold = fMean + fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;
		}

		if (nThreshold > _nThreshold_Recog2_) nThreshold = _nThreshold_Recog2_;
		//--

		//ͼƬ��ֵ��
		threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY_INV);				//200, 255

		//������п�������
		//ȷ����ʴ�����ͺ˵Ĵ�С
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
		//��ʴ����
		erode(matCompRoi, matCompRoi, element);

		IplImage ipl_img2(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage2 = cvCreateMemStorage(0);
		CvSeq* contour2 = 0;

		//��ȡ����  
		cvFindContours(&ipl_img2, storage2, &contour2, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		int nMaxArea = 0;
		std::vector<Rect>RectCompList;
		for (int iteratorIdx = 0; contour2 != 0; contour2 = contour2->h_next, iteratorIdx++)
		{
			CvRect aRect = cvBoundingRect(contour2, 0);
			cv::Rect rm = aRect;
			rm.x += ptNew1.x;
			rm.y += ptNew1.y;
			if (rm.area() > nMaxArea)
				nMaxArea = rm.area();

			RectCompList.push_back(rm);
		}
		cvReleaseMemStorage(&storage2);

		if (RectCompList.size() > 1)
		{
			int nMinArea = nMaxArea * 0.3;				//�����ж�ֵ��ʶ������ľ������ٹ���һ�飬�������ʶ����ε����
			std::vector<Rect>::iterator itRect = RectCompList.begin();
			for (; itRect != RectCompList.end();)
			{
				if (itRect->area() < nMinArea)
					itRect = RectCompList.erase(itRect);
				else
					itRect++;
			}
		}		

		//����������λ����Ϣ�ж�abcd
		float fThreod;
		if (pModelInfo->pModel->nHasHead)
			fThreod = _dCompThread_Head_;
		else
			fThreod = _dCompThread_Fix_;

		std::string strRecogAnswer;
		if (RectCompList.size())
		{
			if (itItem->eCPType == OMR && itItem->nRecogFlag >= 37)	//ѡ����ֱ����
			{
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().y > itItem->rt.tl().y && itItem->fRealValuePercent > fThreod)	//�Ҷ�ֵ�������1.0����Ϊ����Ϳ
						{
							char szVal[5] = { 0 };
							sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 							if (strTmpVal.find(szVal) == std::string::npos)
// 								strTmpVal.append(szVal);

							if (strRecogAnswer.find(szVal) == std::string::npos)
								strRecogAnswer.append(szVal);
						}
					}
				}

				//�����߼����������ظ����Ȳ�������ģ�û����Ϳ���ɾ������
// 				if (strTmpVal.length())
// 				{
// 					RECTLIST::iterator itItem = lSelInfo.begin();
// 					for (; itItem != lSelInfo.end(); itItem++)
// 					{
// 						for (int i = 0; i < RectCompList.size(); i++)
// 						{
// 							if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().x > itItem->rt.tl().x)
// 							{
// 								if (itItem->fRealValuePercent > fThreod)	//�Ҷ�ֵ�������1.0����Ϊ����Ϳ
// 								{
// 									char szVal[2] = { 0 };
// 									sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 									if (strRecogAnswer.find(szVal) == std::string::npos)
// 										strRecogAnswer.append(szVal);
// 								}
// 							}
// 						}
// 					}
// 				}

				//���¿���
// 				RECTLIST::iterator itItem = lSelInfo.begin();
// 				for (; itItem != lSelInfo.end(); itItem++)
// 				{
// 					for (int i = 0; i < RectCompList.size(); i++)
// 					{
// 						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().x > itItem->rt.tl().x)
// 						{
// 							float fThreod = 1.0;
// 							if (RectCompList.size() == 1)		//��ֻʶ���ֻ��һ��ѡ��ʱ�����ͻҶ�ֵ��׼
// 								fThreod = 0.95;
// 
// 							if (itItem->fRealValuePercent > fThreod)	//�Ҷ�ֵ�������1.0����Ϊ����Ϳ
// 							{
// 								char szVal[2] = { 0 };
// 								sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 								if (strRecogAnswer.find(szVal) == std::string::npos)
// 									strRecogAnswer.append(szVal);
// 							}
// 						}
// 					}
// 				}
			}
			else if(itItem->eCPType == OMR && itItem->nRecogFlag < 37)	//ѡ���������
			{
#if 1
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x && itItem->fRealValuePercent > fThreod)	//�Ҷ�ֵ�������1.0����Ϊ����Ϳ
						{
							char szVal[5] = { 0 };
							sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 							if (strTmpVal.find(szVal) == std::string::npos)
// 								strTmpVal.append(szVal);

							if (strRecogAnswer.find(szVal) == std::string::npos)
								strRecogAnswer.append(szVal);
						}
					}
				}

// 				if (strTmpVal.length())
// 				{
// 					RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
// 					for (; itItem != omrResult.lSelAnswer.end(); itItem++)
// 					{
// 						for (int i = 0; i < RectCompList.size(); i++)
// 						{
// 							if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x)
// 							{
// 								if (itItem->fRealValuePercent > fThreod)	//�Ҷ�ֵ�������1.0����Ϊ����Ϳ
// 								{
// 									char szVal[2] = { 0 };
// 									sprintf_s(szVal, "%c", itItem->nAnswer + 65);
// 									if (strRecogAnswer.find(szVal) == std::string::npos)
// 										strRecogAnswer.append(szVal);
// 								}
// 							}
// 						}
// 					}
// 				}
#else
				RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
				for (; itItem != omrResult.lSelAnswer.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x)
						{
							float fThreod = 1.0;
							if (!bFind)								//������ѡ��ʶ������Сʶ���ʱ
							{
								bFind = true;
								if (RectCompList.size() == 1)		//��ֻʶ���ֻ��һ��ѡ��ʱ�����ͻҶ�ֵ��׼
									fThreod = 0.95;
							}
							else
							{
								fThreod = 0.95;
							}
							
							if (itItem->fRealValuePercent > fThreod)	//�Ҷ�ֵ�������1.0����Ϊ����Ϳ
							{
								char szVal[5] = { 0 };
								sprintf_s(szVal, "%c", itItem->nAnswer + 65);
								if (strRecogAnswer.find(szVal) == std::string::npos)
									strRecogAnswer.append(szVal);
							}
						}
					}
				}
#endif
			}
			else if (itItem->eCPType == SN && itItem->nRecogFlag >= 9)	//��ֱ����
			{
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().y > itItem->rt.tl().y && itItem->fRealValuePercent > fThreod)	//�Ҷ�ֵ�������1.0����Ϊ����Ϳ
						{
							char szVal[5] = { 0 };
							sprintf_s(szVal, "%d", itItem->nSnVal);

							if (strRecogAnswer.find(szVal) == std::string::npos)
								strRecogAnswer.append(szVal);
						}
					}
				}
			}
			else if (itItem->eCPType == SN && itItem->nRecogFlag < 9)
			{
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x && itItem->fRealValuePercent > fThreod)	//�Ҷ�ֵ�������1.0����Ϊ����Ϳ
						{
							char szVal[5] = { 0 };
							sprintf_s(szVal, "%d", itItem->nSnVal);

							if (strRecogAnswer.find(szVal) == std::string::npos)
								strRecogAnswer.append(szVal);
						}
					}
				}
			}
		}
		strResult = strRecogAnswer;
	}
	catch (cv::Exception &exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "CRecognizeThread::RecogVal2 error. detail: %s\n", exc.msg);
		g_pLogger->information(szLog);
		TRACE(szLog);
	}

	return bResult;
}

int CRecognizeThread::calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff)
{
#if 1	//���潫����ѡ��ʶ��Ҷ�ֵ�������в����ڱȽ�
	RECTLIST::iterator itItem = rectList.begin();
	for (; itItem != rectList.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), SortByItemDensity);

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
#else	//��������������ѡ�������ʶ��Ҷ�ֵ�ıȽϲ�����������
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

int CRecognizeThread::calcSnDensityDiffVal(pSN_ITEM pSn, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff)
{
	RECTLIST::iterator itItem = pSn->lSN.begin();
	for (; itItem != pSn->lSN.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), SortByItemDensity);

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%d_%d", vecItemsDesc[i]->nSnVal, vecItemsDesc[j]->nSnVal);
			stDiff.fDiff = vecItemsDesc[i]->fRealValuePercent - vecItemsDesc[j]->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
	return 1;
}

int CRecognizeThread::calcOmrGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemGrayDiff)
{
	RECTLIST::iterator itItem = rectList.begin();
	for (; itItem != rectList.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), SortByItemGray);

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

bool CRecognizeThread::Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult_Recog = false;

	if (!pModelInfo->pModel->nHasHead)	//ͬ��ͷģʽ����Ҫ�ж��Ƿ��������ѡ������������
	{
		try
		{
			Mat matCompRoi;

			if (rc.rt.x < 0) rc.rt.x = 0;
			if (rc.rt.y < 0) rc.rt.y = 0;
			if (rc.rt.br().x > matCompPic.cols)
			{
				rc.rt.width = matCompPic.cols - rc.rt.x;
			}
			if (rc.rt.br().y > matCompPic.rows)
			{
				rc.rt.height = matCompPic.rows - rc.rt.y;
			}
			matCompRoi = matCompPic(rc.rt);

			cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			//ͼƬ��ֵ��
			// �ֲ�����Ӧ��ֵ��ͼ���ֵ��
			int blockSize = 25;		//25
			int constValue = 10;
			cv::Mat local;
//			cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
			cv::threshold(matCompRoi, matCompRoi, rc.nThresholdValue, 255, THRESH_BINARY);
			cv::Canny(matCompRoi, matCompRoi, 0, _nCannyKernel_, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));	//Size(6, 6)	��ͨ�հ׿��ʶ��
			dilate(matCompRoi, matCompRoi, element);

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//��ȡ����  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			int nMaxArea = 0;
			cv::Rect rt;
			int i = 0;
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				cv::Rect rm = aRect;

				if (rm.area() > nMaxArea)
				{
					nMaxArea = rm.area();
					rt = rm;
					rt.x += rc.rt.x;
					rt.y += rc.rt.y;
				}
				i++;
			}
			cvReleaseMemStorage(&storage);

#if 1

#else		//�����Ȳ�����ѡ���Ĳü�(�ü�Ŀ���Ƿ���ѡ�����Ŀ������ѡ��ľ�����)������������������ģ�ѡ������Ϳ��ʱ�����������ȫ����ѡ������ʶ���������Σ������Ҫ���
			if (i > 1)
				rc.rt = rt;
#endif
		}
		catch (cv::Exception &exc)
		{
			bResult_Recog = false;
		}
	}
	bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
	return bResult_Recog;
}

bool CRecognizeThread::RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult)
{
	return RecogVal2(nPic, matCompPic, pPic, pModelInfo, omrResult.lSelAnswer, omrResult.strRecogVal2);
}

bool CRecognizeThread::RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal)
{
	std::string strResult;
	RecogVal2(nPic, matCompPic, pPic, pModelInfo, pSn->lSN, strResult);
	char* p = NULL;
	for (int i = 0; i < strResult.length(); i++)
	{
		p = const_cast<char*>(strResult.c_str() + i);
		vecItemVal.push_back(atoi(p));
	}
	return true;
}

bool CRecognizeThread::RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecSN;

	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	SNLIST::iterator itSN = pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.begin();
	for (; itSN != pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.end(); itSN++)
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

			if (pModelInfo->pModel->nHasHead)
			{
				if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
				{
					bResult = false;
					pPic->bFindIssue = true;
					pPic->lIssueRect.push_back(rc);
					break;
				}
				rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
				rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
				rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
				rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
			}
			else
				GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
#if 1
			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModelInfo);
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

#ifdef PaintOmrSnRect	//��ӡOMR��SNλ��
			pPic->lNormalRect.push_back(rc);
#endif
		}
		if (!bResult)
			break;

#if 1	//����ѡ���ֵ�ж�ѡ��
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecSnItemDiff;
		calcSnDensityDiffVal(pSn, vecItemsDesc, vecSnItemDiff);

		float fCompThread = 0.0;		//�Ҷȼ���ﵽҪ��ʱ����һ��ѡ��ĻҶȱ���ﵽ��Ҫ��
		float fDiffThread = 0.0;		//ѡ�������Ϳ�Ŀ��ܻҶ��ݶȷ�ֵ
		float fDiffExit = 0;			//�Ҷȵ��ݶȵݼ�̫��ʱ��������Ϊ����ѡ��û����Ϳ����ʱ�ĻҶ��ݶȷ�ֵ
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
#if 1
		vecItemVal.clear();
		if(vecItemsDesc[0]->fRealValuePercent > fCompThread)
		{
			vecItemVal.push_back(vecItemsDesc[0]->nSnVal);
		}
#else
		for (int i = 0; i < vecSnItemDiff.size(); i++)
		{
			//��������ѡ��Ҷ�ֵ�������ڻҶ�ֵ��ֵ������ֵ��ͬʱ���е�һ�����ĻҶ�ֵ����1.0������Ϊ�������Ϊѡ�еķ�ֵ����
			//(����1.0�Ƿ�ֹ��С�ĻҶ�ֵ��С��ʱ��Ӱ�췧ֵ�ж�)
			float fDiff = (fCompThread - vecSnItemDiff[i].fFirst) * 0.1;
			if ((vecSnItemDiff[i].fDiff >= fDiffThread && vecSnItemDiff[i].fFirst > fCompThread) ||
				(vecSnItemDiff[i].fDiff >= fDiffThread + fDiff && vecSnItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecSnItemDiff[i].fFirst;
				if (vecSnItemDiff[i].fDiff > fDiffExit)	//�Ҷ�ֵ�仯�ϴ�ֱ���˳�
					break;
			}
		}

		if (nFlag >= 0)
		{
			vecItemVal.clear();
			RECTLIST::iterator itItem = pSn->lSN.begin();
			for (; itItem != pSn->lSN.end(); itItem++)
			{
				if (itItem->fRealValuePercent >= fThreld)
				{
					vecItemVal.push_back(itItem->nSnVal);
				}
			}
		}
#endif
		if (vecItemVal.size() != 1)
		{
			char szTmpLog[300] = { 0 };
			sprintf_s(szTmpLog, "��%dλSN[", pSn->nItem);
			for (int i = 0; i < vecSnItemDiff.size(); i++)
			{
				char szTmp[15] = { 0 };
				sprintf_s(szTmp, "%s:%.5f ", vecSnItemDiff[i].szVal, vecSnItemDiff[i].fDiff);
				strcat_s(szTmpLog, szTmp);
			}
			strcat_s(szTmpLog, "]\n");
			strLog.append(szTmpLog);
		}
#endif

		if (vecItemVal.size() == 1)
		{
			pSn->nRecogVal = vecItemVal[0];
			vecSN.push_back(vecItemVal[0]);
		}
		else
		{
		#if 1	//�ڶ���ZKZHʶ�𷽷� test
			std::vector<int> vecItemVal2;
			RecogVal_Sn2(nPic, matCompPic, pPic, pModelInfo, pSn, vecItemVal2);
		#endif

			if (vecItemVal.size() == 0 && vecItemVal2.size() == 1)
			{
				vecSN.push_back(vecItemVal2[0]);
			}
			else
			{
				std::vector<int> vecItemVal3;
				RecogVal_Sn3(nPic, matCompPic, pPic, pModelInfo, pSn, vecItemVal3);
				if (vecItemVal3.size())
				{
					vecSN.push_back(vecItemVal3[0]);
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
					sprintf_s(szLog, "ʶ��׼��֤�ŵ�%dλʧ��,ʶ������%dλ(%s), ͼƬ��: %s\n", pSnItem->nItem, vecItemVal.size(), szVal, pPic->strPicName.c_str());
					strLog.append(szLog);
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
		}
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��׼��֤�����(%s), ͼƬ��: %s\n", (static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.c_str(), pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
		(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
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
		(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
	}
	if (!bRecogAll)
	{
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
	}
	if (!bResult)
	{
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();

		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��׼��֤��ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("ʶ�𿼺�ʱ��: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	clock_t start, end;
	start = clock();
	std::string strLog;
	strLog = Poco::format("ͼƬ%s\n", pPic->strPicName);

	bool bResult = true;
	SNLIST::iterator itSN = pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.begin();
	for (; itSN != pModelInfo->pModel->vecPaperModel[nPic]->lSNInfo.end(); itSN++)
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

				cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

				string strTypeName;
				string strResult = GetQR(matCompRoi, strTypeName);

				std::string strTmpLog;
				if (strResult != "")
				{
					strTmpLog = "ʶ��׼��֤�����(" + strResult + "), ͼƬ��: " + pPic->strPicName;
					(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
				}
				else
				{
					strTmpLog = "ʶ��׼��֤��ʧ��, ͼƬ��:" + pPic->strPicName;
//					bResult = false;
				}
				(static_cast<pST_PaperInfo>(pPic->pPaper))->strSN = strResult;
				strLog.append(strTmpLog);
			}
			catch (cv::Exception& exc)
			{
				std::string strTmpLog = "ʶ���ά�������ʧ��(" + pPic->strPicName + "): " + exc.msg;
				strLog.append(strTmpLog);
				break;
			}
		}
	}
	end = clock();
	std::string strTime = Poco::format("ʶ�𿼺�ʱ��: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CRecognizeThread::RecogVal_ElectOmr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pELECTOMR_QUESTION pOmrQuestion, OMR_RESULT& omrResult)
{
	try
	{
		RECTLIST::iterator itOmrItem = pOmrQuestion->lItemInfo.begin();
		for (; itOmrItem != pOmrQuestion->lItemInfo.end(); itOmrItem++)
		{
			RECTINFO rc = *itOmrItem;

			cv::Point pt1, pt2;
			pt1 = rc.rt.tl() - cv::Point(3, 3);
			pt2 = rc.rt.br() + cv::Point(3, 3);
			Mat matCompRoi;
			matCompRoi = matCompPic(cv::Rect(pt1, pt2));
			cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			//ͼƬ��ֵ��
			threshold(matCompRoi, matCompRoi, _nThreshold_Recog2_, 255, THRESH_BINARY_INV);				//200, 255

			//������п�������
			//ȷ����ʴ�����ͺ˵Ĵ�С
			Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
			//��ʴ����
			erode(matCompRoi, matCompRoi, element);

			IplImage ipl_img2(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage2 = cvCreateMemStorage(0);
			CvSeq* contour2 = 0;

			//��ȡ����  
			cvFindContours(&ipl_img2, storage2, &contour2, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			std::vector<Rect>RectCompList;
			for (int iteratorIdx = 0; contour2 != 0; contour2 = contour2->h_next, iteratorIdx++)
			{
				CvRect aRect = cvBoundingRect(contour2, 0);
				cv::Rect rm = aRect;
				rm.x += pt1.x;
				rm.y += pt1.y;

				RectCompList.push_back(rm);
			}
			cvReleaseMemStorage(&storage2);
		}
	}
	catch (cv::Exception& exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "CRecognizeThread::RecogVal_ElectOmr2 error. detail: %s\n", exc.msg);
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return true;
}

bool CRecognizeThread::RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult)
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

	float fCompThread = 0.0;		//�Ҷȼ���ﵽҪ��ʱ����һ��ѡ��ĻҶȱ���ﵽ��Ҫ��
	float fDiffThreshold = 0.0;		//ѡ�������Ϳ�Ŀ��ܻҶ��ݶȷ�ֵ
	float fDiffExit = 0;			//�Ҷȵ��ݶȵݼ�̫��ʱ��������Ϊ����ѡ��û����Ϳ����ʱ�ĻҶ��ݶȷ�ֵ

	fCompThread = _dCompThread_3_;
	fDiffThreshold = _dDiffThread_3_;
	fDiffExit = _dDiffExit_3_;


#if 1		//�ҶȲ�ֵ����󣬼��ϵ���Omr�ĻҶ���Ҫ�ȱ�׼��С
	float fMeanGrayDiff = 0.0;
	for (int i = 0; i < vecItemsGrayDesc.size(); i++)
	{
		fMeanGrayDiff += (vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray);
	}
	fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayDesc.size();

	int nFlag = -1;
	float fThreld = 0.0;
	float fGrayDiffLast = 0.0;		//����һ���ж�ѡ�е�ѡ�����һ��ѡ��ѡ���жϵ�����
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		float fGrayThresholdGray = vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray - fMeanGrayDiff;
		if (vecOmrItemGrayDiff[i].fDiff >= fDiffThreshold + fGrayThresholdGray + fGrayDiffLast)		//vecOmrItemGrayDiff[i].fDiff >= fDiffThreshold + fGrayThresholdGray
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
		//++�ж�ȫ��ѡ�е����
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
	else if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)		//++�ж�ȫ��ѡ�е����
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
			if (vecOmrItemGrayDiff[i].fDiff > fDiffExit && i + 1 >= vecVal_AnswerSuer.size())	//�Ҷ�ֵ�仯�ϴ�ֱ���˳��������ֱֵ���жϳ����ĸ���������ǰ�жϵ��������Ͳ���������
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

bool CRecognizeThread::RecogVal_Sn3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal)
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
	std::vector<pRECTINFO> vecItemsGrayDesc;
	std::vector<ST_ITEM_DIFF> vecOmrItemGrayDiff;
	calcOmrGrayDiffVal(pSn->lSN, vecItemsGrayDesc, vecOmrItemGrayDiff);

	float fCompThread = 0.0;		//�Ҷȼ���ﵽҪ��ʱ����һ��ѡ��ĻҶȱ���ﵽ��Ҫ��
	float fDiffThread = 0.0;		//ѡ�������Ϳ�Ŀ��ܻҶ��ݶȷ�ֵ
	float fDiffExit = 0;			//�Ҷȵ��ݶȵݼ�̫��ʱ��������Ϊ����ѡ��û����Ϳ����ʱ�ĻҶ��ݶȷ�ֵ

	fCompThread = _dCompThread_3_;
	fDiffThread = _dDiffThread_3_;
	fDiffExit = _dDiffExit_3_;
#if 1
	float fMeanGrayDiff = 0.0;
	for (int i = 0; i < vecItemsGrayDesc.size(); i++)
	{
		fMeanGrayDiff += (vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray);
	}
	fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayDesc.size();

	int nFlag = -1;
	float fThreld = 0.0;
	float fGrayDiffLast = 0.0;		//����һ���ж�ѡ�е�ѡ�����һ��ѡ��ѡ���жϵ�����
	for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
	{
		float fGrayThresholdGray = vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray - fMeanGrayDiff;
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
			if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)
			{
				fThreld = vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
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
	else if (vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray <= fCompThread)		//++�ж�ȫ��ѡ�е����
	{
		fThreld = vecItemsGrayDesc[vecOmrItemGrayDiff.size()]->fRealMeanGray;
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

void CRecognizeThread::HandleWithErrPaper(pST_PaperInfo pPaper)
{
	pPaper->bIssuePaper = true;				//��ʶ��ѧ���Ծ����������Ծ�
	pPAPERSINFO pPapers = static_cast<pPAPERSINFO>(pPaper->pPapers);

	pPapers->fmlPaper.lock();
	PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
	for (; itPaper != pPapers->lPaper.end();)
	{
		if (*itPaper == pPaper)
		{
			itPaper = pPapers->lPaper.erase(itPaper);
			break;
		}
		else
			itPaper++;
	}
	pPapers->fmlPaper.unlock();

	pPapers->fmlIssue.lock();
	bool bFind = false;
	PAPER_LIST::iterator itIssuePaper = pPapers->lIssue.begin();
	for (; itIssuePaper != pPapers->lIssue.end();)
	{
		if (*itIssuePaper == pPaper)
		{
			bFind = true;
			break;
		}
		else
			itIssuePaper++;
	}
	if (!bFind)
	{
		pPapers->lIssue.push_back(pPaper);
		pPapers->nRecogErrCount++;
	}
	pPapers->fmlIssue.unlock();

	(static_cast<CDialog*>(pPaper->pSrcDlg))->SendMessage(MSG_ERR_RECOG, (WPARAM)pPaper, (LPARAM)pPapers);		//PostMessageW
}

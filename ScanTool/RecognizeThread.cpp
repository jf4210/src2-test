#include "stdafx.h"
#include "RecognizeThread.h"
#include "ScanTool.h"


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

		pModelInfo->pModel = pTask->pPaper->pModel;		//pTask->pModel;
		LoadModel(pModelInfo);
	}
	else
		pModelInfo = it->second;

	PaperRecognise(pTask->pPaper, pModelInfo);

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

		cv::Mat matSrc = cv::imread(strModelPicPath);
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

		if ((*itPic)->bRecoged)		//�Ѿ�ʶ���������ʶ��
			continue;

		(*itPic)->bRecoged = true;

		int nCount = pModelInfo->pModel->vecPaperModel[i]->lH_Head.size() + pModelInfo->pModel->vecPaperModel[i]->lV_Head.size() + pModelInfo->pModel->vecPaperModel[i]->lABModel.size()
			+ pModelInfo->pModel->vecPaperModel[i]->lCourse.size() + pModelInfo->pModel->vecPaperModel[i]->lQK_CP.size() + pModelInfo->pModel->vecPaperModel[i]->lGray.size()
			+ pModelInfo->pModel->vecPaperModel[i]->lWhite.size() + pModelInfo->pModel->vecPaperModel[i]->lSNInfo.size();
		if (!nCount)	//�����ǰģ���Ծ�û��У���Ͳ���Ҫ�����Ծ�򿪲�����ֱ����һ���Ծ�
			continue;

		std::string strPicFileName = (*itPic)->strPicName;
		Mat matCompSrcPic = imread((*itPic)->strPicPath);			//imread((*itPic)->strPicPath);
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
		if(bResult) bResult = RecogGrayCP(nPic, matCompPic, *itPic, pModelInfo);
		if(bResult) bResult = RecogWhiteCP(nPic, matCompPic, *itPic, pModelInfo);
		if (bResult) bResult = RecogSN(nPic, matCompPic, *itPic, pModelInfo);
		if (bResult) bResult = RecogOMR(nPic, matCompPic, *itPic, pModelInfo);
		if(!bResult) bFind = true;
		if (bFind)
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
			break;									//�ҵ������Ծ�������㣬��������һ���Ծ�ļ��
		}	

		end_pic = clock();
		TRACE("�Ծ� %s ��ʱ��: %d, ʶ����ʱ��: %d\n", strPicFileName.c_str(), end1_pic - start_pic, end_pic - start_pic);
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "�Ծ� %s ��ʱ��: %d, ʶ����ʱ��: %d\n", strPicFileName.c_str(), end1_pic - start_pic, end_pic - start_pic);
		g_pLogger->information(szLog);
	}

#if 1	//test log
// 	char szPaperLog[1000] = { 0 };
// 	sprintf_s(szPaperLog, "�Ծ�(%s)ʶ����: ", pPaper->strStudentInfo.c_str());
	std::string strPaperLog = "�Ծ�(";
	strPaperLog.append(pPaper->strStudentInfo);
	strPaperLog.append(")ʶ����: ");

	char szSN[30] = { 0 };
	sprintf_s(szSN, "SN(%s), ", pPaper->strSN.c_str());
//	strcat_s(szPaperLog, szSN);
	strPaperLog.append(szSN);
	OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
	for (; itOmr != pPaper->lOmrResult.end(); itOmr++)
	{
		char szSingle[10] = { 0 };
		if (itOmr->nSingle == 0)
			strcpy_s(szSingle, "��");
		else
			strcpy_s(szSingle, "��");
		
		char szItemInfo[600] = { 0 };
		if (itOmr->nDoubt)
		{
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				// 			if (itOmr->strRecogVal.find((char)(itRect->nAnswer + 65)) != std::string::npos)
				// 			{
				char szTmp[100] = { 0 }; 
				sprintf_s(szTmp, "%c, ʶ��=%.3f(R/S= %.1f/%.1f), Succ:%d.  ", itRect->nAnswer + 65, \
					itRect->fRealValuePercent, itRect->fRealValue, itRect->fStandardValue, itRect->fRealValuePercent > itRect->fStandardValuePercent);
// 				sprintf_s(szTmp, "ѡ��=%c, ʶ��ʵ�ʱ���=%.3f, val=%.2f, ʶ���׼val=%.2f, �Ƿ�ɹ�:%d\t", itRect->nAnswer + 65, \
// 					itRect->fRealValuePercent, itRect->fRealValue, itRect->fStandardValue, itRect->fRealValuePercent > itRect->fStandardValuePercent);
				strcat_s(szItemInfo, szTmp);
				//			}
			}
		}

		//++++++++	test	++++++++
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_OMR_ITEM_DIFF> vecOmrItemDiff;
		calcDiffVal2(*itOmr, vecItemsDesc, vecOmrItemDiff);
		strcat_s(szItemInfo, "\n[");
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.3f ", vecOmrItemDiff[i].szVal, vecOmrItemDiff[i].fDiff);
			strcat_s(szItemInfo, szTmp);
		}
		strcat_s(szItemInfo, "]");
		//--------------------------
		
		char szOmrItem[660] = { 0 };
		if (itOmr->nDoubt)
			sprintf_s(szOmrItem, "%d(%s):%s ---%s Doubt(%d)\t==>%s\n", itOmr->nTH, szSingle, itOmr->strRecogVal.c_str(), itOmr->strRecogVal2.c_str(), itOmr->nDoubt, szItemInfo);
		else
			sprintf_s(szOmrItem, "%d(%s):%s ---%s Doubt(%d)\n", itOmr->nTH, szSingle, itOmr->strRecogVal.c_str(), itOmr->strRecogVal2.c_str(), itOmr->nDoubt);
		//		strcat_s(szPaperLog, szOmrItem);
		strPaperLog.append(szOmrItem);
	}
	g_pLogger->information(strPaperLog);
#endif
}

inline bool CRecognizeThread::Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	Mat matSrcRoi, matCompRoi;
	Rect rt = rc.rt;
	bool bResult = false;
	try
	{
 		matCompRoi = matCompPic(rt);

		Mat imag_src, img_comp;
		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);
		cv::GaussianBlur(matSrcRoi, matSrcRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
		SharpenImage(matSrcRoi, matSrcRoi);

		const int channels[1] = { 0 };
		const float* ranges[1];
		const int histSize[1] = { 1 };
		float hranges[2];
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
			if (binValComp == 0 && rc.fStandardValue == 0)
				rc.fRealValuePercent = 1.0;
			else if (rc.fStandardValue == 0)
				rc.fRealValuePercent = 1.0;
			else
				rc.fRealValuePercent = binValComp / rc.fStandardValue;
			bResult = true;
// 			if (rc.fRealValuePercent < rc.fStandardValuePercent)		//g_fSamePercent
// 			{
// 				bFindRect = true;
// 				TRACE("У��ʧ��, �ҶȰٷֱ�: %f, �����: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
// 				char szLog[MAX_PATH] = { 0 };
// 				sprintf_s(szLog, "У��ʧ��, �ҶȰٷֱ�: %f, �����: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
// 				g_pLogger->information(szLog);
// 			}
			break;
		}
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
	if (pModelInfo->pModel->nHasHead != 0)	//��ͬ��ͷ�ģ�����Ҫ���ж���ʶ��
		return bResult;

	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.end(); itCP++)
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

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
			SharpenImage(matCompRoi, matCompRoi);


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
			float fMean = (float)nSum / nCount;		//��ֵ

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);
			int nThreshold = fMean + 2 * fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;

			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			cv::Canny(matCompRoi, matCompRoi, 0, _nCannyKernel_, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(_nDilateKernel_, _nDilateKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��
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
				RectCompList.push_back(rm);
			}
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
			std::string strLog = "ʶ�𶨵��쳣: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;						//�ҵ������
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
			break;
		}
		bool bFindRect = false;
		if(RectCompList.size() == 0)
			bFindRect = true;
		else
		{
			std::sort(RectCompList.begin(), RectCompList.end(), SortByArea);
			Rect& rtFix = RectCompList[0];
			m_ptFixCP.x = static_cast<int>(rtFix.x + rtFix.width / 2 + 0.5 + rc.rt.x);
			m_ptFixCP.y = static_cast<int>(rtFix.y + rtFix.height / 2 + 0.5 + rc.rt.y);

			rtFix.x = rtFix.x + rc.rt.x;
			rtFix.y = rtFix.y + rc.rt.y;

			RECTINFO rcFixInfo;
			rcFixInfo.rt = rtFix;
			pPic->lFix.push_back(rcFixInfo);
			TRACE("�������: (%d,%d,%d,%d)\n", rtFix.x, rtFix.y, rtFix.width, rtFix.height);
		}
		if (bFindRect)
		{
			bResult = false;						//�ҵ������
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ�𶨵�ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
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

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
			SharpenImage(matCompRoi, matCompRoi);

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
			float fMean = (float)nSum / nCount;		//��ֵ

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);
			int nThreshold = fMean + 2 * fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;

			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			cv::Canny(matCompRoi, matCompRoi, 0, _nCannyKernel_, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));	//Size(6, 6)	��ͨ�հ׿��ʶ��
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//��ȡ����  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

#if 1
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
				float fOffset = 0.2;
				nMid_minW = rcSecond.rt.width * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nMid_maxW = rcSecond.rt.width * (1 + fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nMid_minH = rcSecond.rt.height * (1 - fOffset);		//ͬ��
				nMid_maxH = rcSecond.rt.height * (1 + fOffset);		//ͬ��

				nHead_minW = rcFist.rt.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nHead_maxW = rcFist.rt.width * (1 + fOffset);		//ͬ��
				nHead_minH = rcFist.rt.height * (1 - fOffset);		//ͬ��
				nHead_maxH = rcFist.rt.height * (1 + fOffset);		//ͬ��
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
					if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//�ų���һ�������һ�����ͬ��ͷ
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
//			int nYMean = nYSum / RectCompList.size();
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
			for (int i = 0; i < RectCompList.size(); i++)
			{
				RECTINFO rc;
				rc.rt = RectCompList[i];
				rc.eCPType = H_HEAD;
				m_vecH_Head.push_back(rc);
				//++ for test
				pPic->lNormalRect.push_back(rc);
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

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
			SharpenImage(matCompRoi, matCompRoi);

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
			float fMean = (float)nSum / nCount;		//��ֵ

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);
			int nThreshold = fMean + 2 * fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;

			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			cv::Canny(matCompRoi, matCompRoi, 0, _nCannyKernel_, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));	//Size(6, 6)	��ͨ�հ׿��ʶ��
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
				float fOffset = 0.2;
				nMid_minW = rcSecond.rt.width * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nMid_maxW = rcSecond.rt.width * (1 + fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nMid_minH = rcSecond.rt.height * (1 - fOffset);		//ͬ��
				nMid_maxH = rcSecond.rt.height * (1 + fOffset);		//ͬ��

				nHead_minW = rcFist.rt.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
				nHead_maxW = rcFist.rt.width * (1 + fOffset);		//ͬ��
				nHead_minH = rcFist.rt.height * (1 - fOffset);		//ͬ��
				nHead_maxH = rcFist.rt.height * (1 + fOffset);		//ͬ��
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
					if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//�ų���һ�������һ�����ͬ��ͷ
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
				nYSum += rm.y;
			}
//			int nYMean = nYSum / RectCompList.size();
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
			for (int i = 0; i < RectCompList.size(); i++)
			{
				RECTINFO rc;
				rc.rt = RectCompList[i];
				rc.eCPType = V_HEAD;
				m_vecV_Head.push_back(rc);
				//++ for test
				pPic->lNormalRect.push_back(rc);
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
		
		bResult = false;						//�ҵ������
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		break;		
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ���Ŀʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
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

		bResult = false;						//�ҵ������
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		break;
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

bool CRecognizeThread::RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	TRACE("ʶ��Ҷȵ�\n");
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

		bResult = false;						//�ҵ������
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		break;
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��Ҷ�У���ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
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

		bResult = false;						//�ҵ������
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		break;
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
	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecSN;
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
#if 0
			bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > rc.fStandardValuePercent)
					vecItemVal.push_back(rc.nSnVal);
			}
			char szTmp[300] = {0};
			sprintf_s(szTmp, "ͼƬ��: %s, SN: ��%dλ, ѡ��=%d, ʶ��ʵ�ʱ���=%.3f, val=%.2f, ʶ���׼=%.3f, val=%.2f, �Ƿ�ɹ�:%d\n", pPic->strPicName.c_str(),\
				pSnItem->nItem, rc.nSnVal, rc.fRealValuePercent, rc.fRealValue, rc.fStandardValuePercent, rc.fStandardValue, rc.fRealValuePercent > rc.fStandardValuePercent);
			TRACE(szTmp);
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
		if (vecItemVal.size() == 1)
		{
			pSn->nRecogVal = vecItemVal[0];
			vecSN.push_back(vecItemVal[0]);
		}
		else
		{
			bRecogAll = false;
			char szVal[40] = { 0 };
			for (int i = 0; i < vecItemVal.size(); i++)
			{
				char szTmp[5] = { 0 };
				sprintf_s(szTmp, "%d ", vecItemVal[i]);
				strcat(szVal, szTmp);
			}
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "ʶ��׼��֤�ŵ�%dλʧ��,ʶ������%dλ(%s), ͼƬ��: %s\n", pSnItem->nItem, vecItemVal.size(), szVal, pPic->strPicName.c_str());
			g_pLogger->information(szLog);
			TRACE(szLog);
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
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��׼��֤��ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecOmr;
	OMRLIST::iterator itOmr = pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.begin();
	for (; itOmr != pModelInfo->pModel->vecPaperModel[nPic]->lOMR2.end(); itOmr++)
	{
		pOMR_QUESTION pOmrQuestion = &(*itOmr);
		
		OMR_RESULT omrResult;
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

			bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > rc.fStandardValuePercent)
				{
					vecVal_calcHist.push_back(rc.nAnswer);
				}
			}

			bool bResult_Recog2 = RecogVal(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog2)
			{
				vecVal_threshold.push_back(rc.nAnswer);
			}
			omrResult.lSelAnswer.push_back(rc);

			#ifdef PaintOmrSnRect	//��ӡOMR��SNλ��
			pPic->lNormalRect.push_back(rc);
			#endif
		}

	#if 1
		std::string strRecogAnswer1;
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_OMR_ITEM_DIFF> vecOmrItemDiff;
		calcDiffVal2(omrResult, vecItemsDesc, vecOmrItemDiff);
		int nFlag = -1;
		float fThreld = 0.0;
		for (int i = 0; i < vecOmrItemDiff.size(); i++)
		{
			//��������ѡ��Ҷ�ֵ�������ڻҶ�ֵ��ֵ������ֵ��ͬʱ���е�һ�����ĻҶ�ֵ����1.0������Ϊ�������Ϊѡ�еķ�ֵ����
			//(����1.0�Ƿ�ֹ��С�ĻҶ�ֵ��С��ʱ��Ӱ�췧ֵ�ж�)
			if (vecOmrItemDiff[i].fDiff >= 0.07 && vecOmrItemDiff[i].fFirst > 1.0)
			{
				nFlag = i;
				fThreld = vecOmrItemDiff[i].fFirst;
			}
		}
		if (nFlag >= 0)
		{
			for (int i = 0; i < vecItemsDesc.size(); i++)
			{
				if (vecItemsDesc[i]->fRealValuePercent >= fThreld)
				{
					char szVal[2] = { 0 };
					sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
					strRecogAnswer1.append(szVal);
				}
				else
					break;
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
		std::string strRecogAnswer2;
		for (int i = 0; i < vecVal_threshold.size(); i++)
		{
			char szVal[5] = { 0 };
			sprintf_s(szVal, "%c", vecVal_threshold[i] + 65);
			strRecogAnswer2.append(szVal);
		}

		int nDoubt = 0;
		if (strRecogAnswer1 == "")
		{
			nDoubt = 2;
		}
		else
		{
			if (strRecogAnswer1 == strRecogAnswer2)
				nDoubt = 0;
			else
				nDoubt = 1;
		}		

//		OMR_RESULT omrResult;
		omrResult.nTH			= pOmrQuestion->nTH;
		omrResult.nSingle		= pOmrQuestion->nSingle;
		omrResult.nDoubt		= nDoubt;
		omrResult.strRecogVal	= strRecogAnswer1;
		omrResult.strRecogVal2	= strRecogAnswer2;
		(static_cast<pST_PaperInfo>(pPic->pPaper))->lOmrResult.push_back(omrResult);

// 		char szSingle[10] = { 0 };
// 		if (pOmrQuestion->nSingle == 0)
// 			strcpy_s(szSingle, "��ѡ��");
// 		else
// 			strcpy_s(szSingle, "��ѡ��");
// 		char szLog[MAX_PATH] = { 0 };
// 		sprintf_s(szLog, "ʶ��OMR��%d��(%s),ʶ����(%s), ͼƬ��: %s\n", pOmrQuestion->nTH, szSingle, strRecogAnswer.c_str(), pPic->strPicName.c_str());
// 		g_pLogger->information(szLog);
// 		TRACE(szLog);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "ʶ��OMRʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
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
	#if 1
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

bool CRecognizeThread::RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pOMR_QUESTION pOmrQuestion)
{
	Mat matSrcRoi, matCompRoi;
	
	bool bResult = false;



	//omr��Ĵ�С�߶�
	RECTLIST::iterator itItem = pOmrQuestion->lSelAnswer.begin();
	RECTLIST::reverse_iterator itEndItem = pOmrQuestion->lSelAnswer.rbegin();
	cv::Point pt1, pt2;
	pt1 = itItem->rt.tl();
	pt2 = itEndItem->rt.br();
	Rect rt = cv::Rect(pt1, pt2);	//ABCD������Ŀ��ѡ����

	RECTLIST::iterator itFirst = itItem++;
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
			nSpace = max(abs(itFirst->rt.tl().x - itItem->rt.br().x), abs(itFirst->rt.tl().y - itItem->rt.br().y));
			break;
	}

	int nOmrMinW, nOmrMinH, nAreaMin;
	nOmrMinW = itFirst->rt.width * 0.4;
	nOmrMinH = itFirst->rt.height * 0.4;
	nAreaMin = itFirst->rt.area() * 0.3;
	//���ݴ�С������ȹ���һ�¿��ܿ�ѡ����ŵ����

	try
	{
		matCompRoi = matCompPic(rt);
		matSrcRoi = matCompRoi.clone();

		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		//ͼƬ��ֵ��
		threshold(matCompRoi, matCompRoi, 240, 255, THRESH_BINARY_INV);				//200, 255

		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//��ȡ����  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	#if 1
		std::vector<Rect> RectBaseList;
		cv::Point ptNew1, ptNew2;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;

			if (rm.width < nOmrMinW || rm.height < nOmrMinH || rm.area() < nAreaMin)		//************	��������ŵ�ѡ����������	************
				continue;

			//��Ҫȥ�����ܰ��������


			if(iteratorIdx == 0)
			{
				ptNew1 = rm.tl();
				ptNew2 = rm.br();
			}
			if(itItem->nRecogFlag >= 37)	//ѡ��Ϊ�����
			{
				if (rm.tl().y < ptNew1.y)	ptNew1 = rm.tl();
				if (rm.br().y > ptNew2.y)	ptNew2 = rm.br();
			}
			else  //ѡ��Ϊ�����
			{
				if(rm.tl().x < ptNew1.x)	ptNew1 = rm.tl();
				if(rm.br().x > ptNew2.x)	ptNew2 = rm.br();
			}
			RectBaseList.push_back(rm);
		}
		if (itItem->nRecogFlag >= 37)
			std::sort(RectBaseList.begin(), RectBaseList.end(), SortByPositionY2);
		else
			std::sort(RectBaseList.begin(), RectBaseList.end(), SortByPositionX2);

		//�����µ�����������ѡ�������ε���Ϳ���
		matCompRoi.deallocate();
		matCompRoi = matSrcRoi(cv::Rect(ptNew1, ptNew2));
		cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		//ͼƬ��ֵ��
		threshold(matCompRoi, matCompRoi, 240, 255, THRESH_BINARY_INV);				//200, 255

		//������п�������
		//ȷ����ʴ�����ͺ˵Ĵ�С
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
		erode(matCompRoi, matCompRoi, element);


		IplImage ipl_img2(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage2 = cvCreateMemStorage(0);
		CvSeq* contour2 = 0;

		//��ȡ����  
		cvFindContours(&ipl_img2, storage2, &contour2, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
				
		std::vector<Rect>RectCompList;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;

			RectCompList.push_back(rm);
		}

		//����������λ����Ϣ�ж�abcd
		if (RectCompList.size())
		{

		}

	#else
		std::vector<Rect>RectCompList;
		if (contour->total > pOmrQuestion->lSelAnswer.size())	//ʶ�����ѡ��������ģ��ѡ��������, ���ܿ�ѡ�����
		{			
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				cv::Rect rm = aRect;

				if (rm.width < nOmrMinW || rm.height < nOmrMinH || rm.area() < nAreaMin)		//************	��������ŵ�ѡ����������	************
					continue;

				RectCompList.push_back(rm);
			}
		}
		else if (contour->total < pOmrQuestion->lSelAnswer.size())	//ʶ�����ѡ��������ģ��ѡ��������, ����ѡ��������һ������
		{

		}
			
		//ʶ�����ѡ��������ģ��ѡ������һ�£�ֻ���ж�ABCDֵ
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			
			RectCompList.push_back(rm);
		}
		if (RectCompList.size() == 0)
			bResult = false;
		else
			bResult = true;
	#endif
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

int CRecognizeThread::calcDiffVal(pOMR_QUESTION pOmrQuestion, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_OMR_ITEM_DIFF>& vecOmrItemDiff)
{
#if 1	//���潫����ѡ��ʶ��Ҷ�ֵ�������в����ڱȽ�
	RECTLIST::iterator itItem = pOmrQuestion->lSelAnswer.begin();
	for (; itItem != pOmrQuestion->lSelAnswer.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), SortByOmrGray);

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_OMR_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsDesc[i]->nAnswer + 65), (char)(vecItemsDesc[j]->nAnswer + 65));
			stDiff.fDiff = vecItemsDesc[i]->fRealValuePercent - vecItemsDesc[j]->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
#else	//��������������ѡ�������ʶ��Ҷ�ֵ�ıȽϲ�����������
	RECTLIST::iterator itFirst = pOmrQuestion->lSelAnswer.begin();
	for (; itFirst != pOmrQuestion->lSelAnswer.end(); itFirst++)
	{
		RECTLIST::iterator itSecond = itFirst;
		itSecond++;
		for (; itSecond != pOmrQuestion->lSelAnswer.end(); itSecond++)
		{
			ST_OMR_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(itFirst->nAnswer + 65), (char)(itSecond->nAnswer + 65));
			stDiff.fDiff = itFirst->fRealValuePercent - itSecond->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
	std::sort(vecOmrItemDiff.begin(), vecOmrItemDiff.end(), SortByOmrDiff);
#endif
	return 1;
}

int CRecognizeThread::calcDiffVal2(OMR_RESULT& omrResult, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_OMR_ITEM_DIFF>& vecOmrItemDiff)
{
#if 1	//���潫����ѡ��ʶ��Ҷ�ֵ�������в����ڱȽ�
	RECTLIST::iterator itItem = omrResult.lSelAnswer.begin();
	for (; itItem != omrResult.lSelAnswer.end(); itItem++)
	{
		vecItemsDesc.push_back(&(*itItem));
	}
	std::sort(vecItemsDesc.begin(), vecItemsDesc.end(), SortByOmrGray);

	for (int i = 0; i < vecItemsDesc.size(); i++)
	{
		int j = i + 1;
		if (j < vecItemsDesc.size())
		{
			ST_OMR_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsDesc[i]->nAnswer + 65), (char)(vecItemsDesc[j]->nAnswer + 65));
			stDiff.fDiff = vecItemsDesc[i]->fRealValuePercent - vecItemsDesc[j]->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
#else	//��������������ѡ�������ʶ��Ҷ�ֵ�ıȽϲ�����������
	RECTLIST::iterator itFirst = omrResult.lSelAnswer.begin();
	for (; itFirst != omrResult.lSelAnswer.end(); itFirst++)
	{
		RECTLIST::iterator itSecond = itFirst;
		itSecond++;
		for (; itSecond != omrResult.lSelAnswer.end(); itSecond++)
		{
			ST_OMR_ITEM_DIFF stDiff;
			sprintf_s(stDiff.szVal, "%c%c", (char)(itFirst->nAnswer + 65), (char)(itSecond->nAnswer + 65));
			stDiff.fDiff = itFirst->fRealValuePercent - itSecond->fRealValuePercent;
			stDiff.fFirst = vecItemsDesc[i]->fRealValuePercent;
			stDiff.fSecond = vecItemsDesc[j]->fRealValuePercent;
			vecOmrItemDiff.push_back(stDiff);
		}
	}
	std::sort(vecOmrItemDiff.begin(), vecOmrItemDiff.end(), SortByOmrDiff);
#endif
	return 1;
}



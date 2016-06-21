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
	//进行任务识别
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
	//创建并初始化滤波模板
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = 5;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//对图像进行滤波
	cv::filter2D(image, result, image.depth(), kernel);
}

bool CRecognizeThread::LoadModel(pMODELINFO pModelInfo)
{
	USES_CONVERSION;
	for (int i = 0; i < pModelInfo->pModel->nPicNum; i++)
	{
		std::string strModelPicPath = g_strModelSavePath + "\\" + T2A(pModelInfo->pModel->strModelName + _T("\\") + pModelInfo->pModel->vecPaperModel[i]->strModelPicName);

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

		if ((*itPic)->bRecoged)		//已经识别过，不再识别
			continue;

		(*itPic)->bRecoged = true;

		int nCount = pModelInfo->pModel->vecPaperModel[i]->lH_Head.size() + pModelInfo->pModel->vecPaperModel[i]->lV_Head.size() + pModelInfo->pModel->vecPaperModel[i]->lABModel.size()
			+ pModelInfo->pModel->vecPaperModel[i]->lCourse.size() + pModelInfo->pModel->vecPaperModel[i]->lQK_CP.size() + pModelInfo->pModel->vecPaperModel[i]->lGray.size()
			+ pModelInfo->pModel->vecPaperModel[i]->lWhite.size() + pModelInfo->pModel->vecPaperModel[i]->lSNInfo.size();
		if (!nCount)	//如果当前模板试卷没有校验点就不需要进行试卷打开操作，直接下一张试卷
			continue;

		std::string strPicFileName = (*itPic)->strPicName;
		Mat matCompSrcPic = imread((*itPic)->strPicPath);			//imread((*itPic)->strPicPath);
#ifdef PIC_RECTIFY_TEST	//图像旋转纠正测试
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
		if (bResult) bResult = PicTransfer(nPic, matCompPic, (*itPic)->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix);
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
			pPaper->bIssuePaper = true;				//标识此学生试卷属于问题试卷
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
			pPapers->lIssue.push_back(pPaper);
			pPapers->nRecogErrCount++;
			pPapers->fmlIssue.unlock();

			(static_cast<CDialog*>(pPaper->pSrcDlg))->PostMessageW(MSG_ERR_RECOG, (WPARAM)pPaper, (LPARAM)pPapers);
			break;									//找到这张试卷有问题点，不进行下一张试卷的检测
		}	

		end_pic = clock();
		TRACE("试卷 %s 打开时间: %d, 识别总时间: %d\n", strPicFileName.c_str(), end1_pic - start_pic, end_pic - start_pic);
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "试卷 %s 打开时间: %d, 识别总时间: %d\n", strPicFileName.c_str(), end1_pic - start_pic, end_pic - start_pic);
		g_pLogger->information(szLog);
	}

#if 1	//test log
// 	char szPaperLog[1000] = { 0 };
// 	sprintf_s(szPaperLog, "试卷(%s)识别结果: ", pPaper->strStudentInfo.c_str());
	std::string strPaperLog = "试卷(";
	strPaperLog.append(pPaper->strStudentInfo);
	strPaperLog.append(")识别结果: ");

	char szSN[30] = { 0 };
	sprintf_s(szSN, "SN(%s), ", pPaper->strSN.c_str());
//	strcat_s(szPaperLog, szSN);
	strPaperLog.append(szSN);
	OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
	for (; itOmr != pPaper->lOmrResult.end(); itOmr++)
	{
		char szSingle[10] = { 0 };
		if (itOmr->nSingle == 0)
			strcpy_s(szSingle, "单");
		else
			strcpy_s(szSingle, "多");
		
		char szItemInfo[600] = { 0 };
		if (itOmr->nDoubt)
		{
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				// 			if (itOmr->strRecogVal.find((char)(itRect->nAnswer + 65)) != std::string::npos)
				// 			{
				char szTmp[100] = { 0 };
				sprintf_s(szTmp, "选项=%c, 识别实际比例=%.3f, val=%.2f, 识别标准val=%.2f, 是否成功:%d\t", itRect->nAnswer + 65, \
					itRect->fRealValuePercent, itRect->fRealValue, itRect->fStandardValue, itRect->fRealValuePercent > itRect->fStandardValuePercent);
				strcat_s(szItemInfo, szTmp);
				//			}
			}
		}
		
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

		const int channels[1] = { 0 };
		const float* ranges[1];
		const int histSize[1] = { 1 };
		float hranges[2];
		if (rc.eCPType != WHITE_CP && rc.eCPType != OMR)
		{
			hranges[0] = 0;
			hranges[1] = static_cast<float>(rc.nThresholdValue);
			ranges[0] = hranges;
		}
		else if (rc.eCPType == OMR)
		{
			hranges[0] = 0;
			hranges[1] = 235;
			ranges[0] = hranges;
		}
		else
		{
			hranges[0] = static_cast<float>(rc.nThresholdValue);
			hranges[1] = 255;
			ranges[0] = hranges;
		}
		MatND src_hist, comp_hist;
		cv::calcHist(&matCompRoi, 1, channels, Mat(), comp_hist, 1, histSize, ranges, false);

		double maxValComp = 0;
		double minValComp = 0;

		//找到直方图中的最大值和最小值
		cv::minMaxLoc(comp_hist, &minValComp, &maxValComp, 0, 0);
		int nSize = comp_hist.rows;

		// 设置最大峰值为图像高度的90%
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
// 				TRACE("校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
// 				char szLog[MAX_PATH] = { 0 };
// 				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
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
#if 0
int CRecognizeThread::FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix)
{
	if (lFix.size() < 3)
		return 1;

	std::vector<cv::Point2f> vecFixPt;
	RECTLIST::iterator itCP = lModelFix.begin();
	for (; itCP != lModelFix.end(); itCP++)
	{
		cv::Point2f pt;
		pt.x = itCP->rt.x + itCP->rt.width / 2;
		pt.y = itCP->rt.y + itCP->rt.height / 2;
		vecFixPt.push_back(pt);
	}
	std::vector<cv::Point2f> vecFixNewPt;
	RECTLIST::iterator itCP2 = lFix.begin();
	for (; itCP2 != lFix.end(); itCP2++)
	{
		cv::Point2f pt;
		pt.x = itCP2->rt.x + itCP2->rt.width / 2;
		pt.y = itCP2->rt.y + itCP2->rt.height / 2;
		vecFixNewPt.push_back(pt);
	}

	Point2f srcTri[3];
	Point2f dstTri[3];
	Mat rot_mat(2, 3, CV_32FC1);
	Mat warp_mat(2, 3, CV_32FC1);
	Mat warp_dst, warp_rotate_dst;
	for (int i = 0; i < vecFixPt.size(); i++)
	{
		srcTri[i] = vecFixNewPt[i];
		dstTri[i] = vecFixPt[i];
	}

//	warp_dst = Mat::zeros(matCompPic.rows, matCompPic.cols, matCompPic.type());
	warp_mat = getAffineTransform(srcTri, dstTri);
	warpAffine(matCompPic, matCompPic, warp_mat, matCompPic.size(), 1, 0, Scalar(255, 255, 255));

	return 1;
}
#endif
bool CRecognizeThread::RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
	bool bResult = true;
	if (pModelInfo->pModel->nHasHead != 0)	//有同步头的，不需要进行定点识别
		return bResult;

	RECTLIST::iterator itCP = pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.begin();
	for (; itCP != pModelInfo->pModel->vecPaperModel[nPic]->lSelFixRoi.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		std::vector<Rect>RectCompList;
		try
		{
			Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
			sharpenImage1(matCompRoi, matCompRoi);

			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
			cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(6, 6)	普通空白框可识别
			dilate(matCompRoi, matCompRoi, element);

#if 1
			//		std::vector<std::vector<cv::Point> > vecContours;		//轮廓信息存储
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

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				RectCompList.push_back(rm);
			}
#else
			//		std::vector<std::vector<cv::Point> > vecContours;		//轮廓信息存储
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
				// 				TRACE("抛弃不符合条件的矩形块,(%d, %d, %d, %d)\n", rm.x, rm.y, rm.width, rm.height);
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
			std::string strLog = "识别定点异常: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;						//找到问题点
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
			TRACE("定点矩形: (%d,%d,%d,%d)\n", rtFix.x, rtFix.y, rtFix.width, rtFix.height);
		}
		if (bFindRect)
		{
			bResult = false;						//找到问题点
			pPic->bFindIssue = true;
			pPic->lIssueRect.push_back(rc);
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别定点失败, 图片名: %s\n", pPic->strPicName.c_str());
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

	m_vecH_Head.clear();
	RECTLIST::iterator itRoi = pModelInfo->pModel->vecPaperModel[nPic]->lSelHTracker.begin();
	for (; itRoi != pModelInfo->pModel->vecPaperModel[nPic]->lSelHTracker.end(); itRoi++)
	{
		RECTINFO rc = *itRoi;
#if 1
		std::vector<Rect>RectCompList;
		try
		{
			Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
			sharpenImage1(matCompRoi, matCompRoi);

			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
			cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(6, 6)	普通空白框可识别
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
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
		}
		catch (cv::Exception& exc)
		{
			std::string strLog = "识别水平同步头异常: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;
			break;
		}
		if (RectCompList.size() == 0)
			bResult = false;
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
		}
#else
		GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic].lFix, rc.rt);
		bool bFindRect = Recog(nPic, rc, matCompPic, pPic, pModelInfo);

		if (bFindRect)
		{
			bResult = false;						//找到问题点
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
		sprintf_s(szLog, "识别水平同步头失败, 图片名: %s\n", pPic->strPicName.c_str());
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

	m_vecV_Head.clear();
	RECTLIST::iterator itRoi = pModelInfo->pModel->vecPaperModel[nPic]->lSelVTracker.begin();
	for (; itRoi != pModelInfo->pModel->vecPaperModel[nPic]->lSelVTracker.end(); itRoi++)
	{
		RECTINFO rc = *itRoi;
#if 1
		std::vector<Rect>RectCompList;
		try
		{
			Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
			sharpenImage1(matCompRoi, matCompRoi);

			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
			cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(6, 6)	普通空白框可识别
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
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
		}
		catch (cv::Exception& exc)
		{
			std::string strLog = "识别垂直同步头异常: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;
			break;
		}
		if (RectCompList.size() == 0)
			bResult = false;
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
		}
#else
		GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic].lFix, rc.rt);
		bool bFindRect = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
		if (bFindRect)
		{
			bResult = false;						//找到问题点
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
		sprintf_s(szLog, "识别垂直同步头失败, 图片名: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
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
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
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
				g_pLogger->information(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}
		
		bResult = false;						//找到问题点
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		break;		
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别科目失败, 图片名: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
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
				((pST_PaperInfo)pPic->pPaper)->bQKFlag = true;			//设置学生缺考
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}

		bResult = false;						//找到问题点
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		break;
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别缺考失败, 图片名: %s\n", pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CRecognizeThread::RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo)
{
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
				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				g_pLogger->information(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}

		bResult = false;						//找到问题点
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		break;
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别灰度校验点失败, 图片名: %s\n", pPic->strPicName.c_str());
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
				sprintf_s(szLog, "校验失败, 灰度百分比: %f, 问题点: (%d,%d,%d,%d)\n", rc.fRealValuePercent * 100, rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
				g_pLogger->information(szLog);
				TRACE(szLog);
			}
		}
		else
		{
			char szLog[MAX_PATH] = { 0 };
			sprintf_s(szLog, "校验失败, 异常结束, 问题点: (%d,%d,%d,%d)\n", rc.rt.x, rc.rt.y, rc.rt.width, rc.rt.height);
			g_pLogger->information(szLog);
			TRACE(szLog);
		}

		bResult = false;						//找到问题点
		pPic->bFindIssue = true;
		pPic->lIssueRect.push_back(rc);
		break;
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别空白校验点失败, 图片名: %s\n", pPic->strPicName.c_str());
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
			sprintf_s(szTmp, "图片名: %s, SN: 第%d位, 选项=%d, 识别实际比例=%.3f, val=%.2f, 识别标准=%.3f, val=%.2f, 是否成功:%d\n", pPic->strPicName.c_str(),\
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
			sprintf_s(szLog, "识别准考证号第%d位失败,识别出结果%d位(%s), 图片名: %s\n", pSnItem->nItem, vecItemVal.size(), szVal, pPic->strPicName.c_str());
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
		sprintf_s(szLog, "识别准考证号完成(%s), 图片名: %s\n", (static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.c_str(), pPic->strPicName.c_str());
		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别准考证号失败, 图片名: %s\n", pPic->strPicName.c_str());
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
		std::vector<int> vecVal_calcHist;		//直方图灰度计算的识别结果
		std::vector<int> vecVal_threshold;		//二值化计算的识别结果
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
#if 1
			bool bResult_Recog = Recog(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > rc.fStandardValuePercent)
				{
					vecVal_calcHist.push_back(rc.nAnswer);
//					omrResult.lSelAnswer.push_back(rc);
				}
			}
// 			char szTmp[300] = {0};
// 			sprintf_s(szTmp, "图片名: %s, OMR: 题号=%d, 选项=%c, 识别实际比例=%.3f, val=%.2f, 识别标准=%.3f, val=%.2f, 是否成功:%d\n", pPic->strPicName.c_str(),\
// 				pOmrQuestion->nTH, rc.nAnswer + 65, rc.fRealValuePercent, rc.fRealValue, rc.fStandardValuePercent, rc.fStandardValue, rc.fRealValuePercent > rc.fStandardValuePercent);
// 			TRACE(szTmp);

			bool bResult_Recog2 = RecogVal(nPic, rc, matCompPic, pPic, pModelInfo);
			if (bResult_Recog2)
			{
				vecVal_threshold.push_back(rc.nAnswer);
			}
			omrResult.lSelAnswer.push_back(rc);
#endif
		}
		std::string strRecogAnswer1;
		for (int i = 0; i < vecVal_calcHist.size(); i++)
		{
			char szVal[5] = { 0 };
			sprintf_s(szVal, "%c", vecVal_calcHist[i] + 65);
			strRecogAnswer1.append(szVal);	
		}
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
// 			strcpy_s(szSingle, "单选题");
// 		else
// 			strcpy_s(szSingle, "多选题");
// 		char szLog[MAX_PATH] = { 0 };
// 		sprintf_s(szLog, "识别OMR第%d题(%s),识别结果(%s), 图片名: %s\n", pOmrQuestion->nTH, szSingle, strRecogAnswer.c_str(), pPic->strPicName.c_str());
// 		g_pLogger->information(szLog);
// 		TRACE(szLog);
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别OMR失败, 图片名: %s\n", pPic->strPicName.c_str());
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

		//图片二值化
		threshold(matCompRoi, matCompRoi, 240, 255, THRESH_BINARY_INV);				//200, 255
#if 1
// 		if (rc.nTH == 26)
// 		{
// 			namedWindow("threshold", 1);
// 			imshow("threshold", matCompRoi);
// 		}

		//确定腐蚀和膨胀核的大小
		Mat element = getStructuringElement(MORPH_RECT, Size(4, 4));	//Size(4, 4)
		//膨胀操作
		dilate(matCompRoi, matCompRoi, element);

// 		if (rc.nTH == 26)
// 		{
// 			namedWindow("dilate", 1);
// 			imshow("dilate", matCompRoi);
// 		}

		Mat element2 = getStructuringElement(MORPH_RECT, Size(15, 15));	//Size(4, 4)
		//腐蚀操作1
		erode(matCompRoi, matCompRoi, element2);

// 		if (rc.nTH == 26)
// 		{
// 			namedWindow("erode", 1);
// 			imshow("erode", matCompRoi);
// 		}

		Mat element3 = getStructuringElement(MORPH_RECT, Size(4, 4));	//Size(4, 4)
		//腐蚀操作2
		erode(matCompRoi, matCompRoi, element3);

// 		if (rc.nTH == 26)
// 		{
// 			namedWindow("erode2", 1);
// 			imshow("erode2", matCompRoi);
// 		}


		Mat element4 = getStructuringElement(MORPH_RECT, Size(5, 5));	//Size(4, 4)
		dilate(matCompRoi, matCompRoi, element4);

// 		if(rc.nTH == 26)
// 		{
// 			namedWindow("dilate2", 1);
// 			imshow("dilate2", matCompRoi);
// 			waitKey(0);
// 		}

#else
		//确定腐蚀和膨胀核的大小
		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));	//Size(4, 4)
		//腐蚀操作1
		erode(matCompRoi, matCompRoi, element);

		//确定腐蚀和膨胀核的大小
		Mat element2 = getStructuringElement(MORPH_RECT, Size(3, 3));	//Size(4, 4)
		//腐蚀操作2
		erode(matCompRoi, matCompRoi, element2);

		//膨胀操作
		dilate(matCompRoi, matCompRoi, element2);
		//膨胀操作
		dilate(matCompRoi, matCompRoi, element);
#endif
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
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

// 		if (RectCompList.size() > 0 && rc.nSnVal == 7)
// 		{
// 			namedWindow("5轮廓化resultImage", 1);
// 			cv::imshow("5轮廓化resultImage", matShow);
// 		}
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



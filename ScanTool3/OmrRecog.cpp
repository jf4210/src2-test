#include "stdafx.h"
#include "OmrRecog.h"


COmrRecog::COmrRecog()
{
}


COmrRecog::~COmrRecog()
{
}

cv::Rect COmrRecog::GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Rect rtResult;
	if (nOrientation == 1)	//matSrc正向
	{
		rtResult = rt;
	}
	else if (nOrientation == 2)	//matSrc右转90度
	{
		cv::Point pt1, pt2;
		pt1.x = nH - rt.tl().y;
		pt1.y = rt.tl().x;
		pt2.x = nH - rt.br().y;
		pt2.y = rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 3)	//matSrc左转90度
	{
		cv::Point pt1, pt2;
		pt1.x = rt.tl().y;
		pt1.y = nW - rt.tl().x;
		pt2.x = rt.br().y;
		pt2.y = nW - rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 4)	//matSrc右转180度
	{
		cv::Point pt1, pt2;
		pt1.x = nW - rt.tl().x;
		pt1.y = nH - rt.tl().y;
		pt2.x = nW - rt.br().x;
		pt2.y = nH - rt.br().y;
		rtResult = cv::Rect(pt1, pt2);
	}
	return rtResult;
}

bool COmrRecog::RecogFixCP(int nPic, cv::Mat& matCompPic, RECTLIST& rlFix, pMODEL pModel, int nOrientation)
{
	bool bResult = true;

	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lSelFixRoi.begin();
	for (int i = 0; itCP != pModel->vecPaperModel[nPic]->lSelFixRoi.end(); i++, itCP++)
	{
		RECTINFO rc = *itCP;
		//获取按给定方向旋转图像后的矩形
		cv::Rect rtModelPic;
		rtModelPic.width = pModel->vecPaperModel[nPic]->nPicW;
		rtModelPic.height = pModel->vecPaperModel[nPic]->nPicH;
		rc.rt = GetRectByOrientation(rtModelPic, rc.rt, nOrientation);

		std::vector<cv::Rect>RectCompList;
		try
		{
		#if 1
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
		#else
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
			cv::Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);
		#endif

			cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)
			SharpenImage(matCompRoi, matCompRoi, rc.nSharpKernel);

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
			const int histSize[1] = { nRealThreshold };
			float hranges[2] = { 0, nRealThreshold };
			const float* ranges[1];
			ranges[0] = hranges;
			cv::MatND hist;
			calcHist(&matCompRoi, 1, channels, cv::Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			int nThreshold = nRealThreshold;
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

			if (nThreshold > nRealThreshold) nThreshold = nRealThreshold;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			cv::Mat element_Anticlutter = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别	Size(_nDilateKernel_, _nDilateKernel_)
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
				cv::Rect rm = aRect;
				rm = rm + rc.rt.tl();
				RectCompList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);
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
// 			pPic->bFindIssue = true;
// 			pPic->lIssueRect.push_back(rc);
			break;
		}
		bool bFindRect = false;
		if (RectCompList.size() == 0)
			bFindRect = true;
		else
		{
			std::sort(RectCompList.begin(), RectCompList.end(), SortByArea);
			cv::Rect& rtFix = RectCompList[0];

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
			if (RectCompList.size() == 1)	//只发现一个矩形时，就默认就是定点了
				bOnlyOne = true;

			//通过灰度值来判断
			for (int i = 0; i < RectCompList.size(); i++)
			{
				RECTINFO rcTmp = rcFix;
				rcTmp.rt = RectCompList[i];
				Recog(nPic, rcTmp, matCompPic, NULL, NULL);
				float fArea = rcTmp.fRealArea / rcTmp.fStandardArea;
				float fDensity = rcTmp.fRealDensity / rcTmp.fStandardDensity;
				float fWper = (float)rcTmp.rt.width / rcFix.rt.width;			//查找的矩形的宽度与模板对应定点的宽度之比
				float fHper = (float)rcTmp.rt.height / rcFix.rt.height;			//查找的矩形的宽度与模板对应定点的高度之比
				
				if ((bOnlyOne && fArea > 0.4 && fArea < 2.5 && fDensity > rcTmp.fStandardValuePercent * 0.9 && fWper < 2.0 && fWper > 0.4 && fHper < 2.0 && fHper > 0.4) || \
					(fArea > 0.5 && fArea < 2.0 && fDensity > rcTmp.fStandardValuePercent && fWper < 2.0 && fWper > 0.4 && fHper < 2.0 && fHper > 0.4))	//fArea > 0.7 && fArea < 1.5 && fDensity > 0.6
				{
					bFind = true;
					rtFix = RectCompList[i];
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
//				pPic->lFix.push_back(rcFixInfo);
				rlFix.push_back(rcFixInfo);
				TRACE("定点矩形: (%d,%d,%d,%d)\n", rtFix.x, rtFix.y, rtFix.width, rtFix.height);
			}
		}
		if (bFindRect)
		{
			bResult = false;						//找到问题点
// 			pPic->bFindIssue = true;
// 			pPic->lIssueRect.push_back(rc);
		}
	}
	if (!bResult)
	{
// 		char szLog[MAX_PATH] = { 0 };
// 		sprintf_s(szLog, "识别定点失败, 图片名: %s\n", pPic->strPicName.c_str());
// 		g_pLogger->information(szLog);
// 		TRACE(szLog);
	}
	return bResult;
}

bool COmrRecog::RecogRtVal(RECTINFO& rc, cv::Mat& matCompPic)
{
	return Recog(0, rc, matCompPic, NULL, NULL);
}

bool COmrRecog::RecogZkzh(int nPic, cv::Mat& matCompPic, pMODEL	pModel, int nOrientation)
{
	bool bResult = false;
	SNLIST::iterator itSN = pModel->vecPaperModel[nPic]->lSNInfo.begin();
	for (; itSN != pModel->vecPaperModel[nPic]->lSNInfo.end(); itSN++)
	{
		pSN_ITEM pSnItem = *itSN;

		RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
		for (; itSnItem != pSnItem->lSN.end(); itSnItem++)		//最多一个矩形块，列表中最多只有一个元素
		{
			RECTINFO rc = *itSnItem;

			//获取按给定方向旋转图像后的矩形
			cv::Rect rtModelPic;
			rtModelPic.width = pModel->vecPaperModel[nPic]->nPicW;
			rtModelPic.height = pModel->vecPaperModel[nPic]->nPicH;
			rc.rt = GetRectByOrientation(rtModelPic, rc.rt, nOrientation);

			try
			{
				if (rc.rt.x < 0) rc.rt.x = 0;
				if (rc.rt.y < 0) rc.rt.y = 0;
				if (rc.rt.br().x > matCompPic.cols)
					rc.rt.width = matCompPic.cols - rc.rt.x;
				if (rc.rt.br().y > matCompPic.rows)
					rc.rt.height = matCompPic.rows - rc.rt.y;

				cv::Mat matCompRoi;
				matCompRoi = matCompPic(rc.rt);

				cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

				string strTypeName;
				string strResult = GetQR(matCompRoi, strTypeName);

				std::string strLog;
				if (strResult != "")
				{
					bResult = true;
				}
			}
			catch (cv::Exception& exc)
			{
				break;
			}
		}
	}

	return bResult;
}

bool COmrRecog::Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel)
{
	cv::Mat matCompRoi;
	cv::Rect rt = rc.rt;
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

		cv::Mat imag_src, img_comp;
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
			hranges[1] = g_nRecogGrayMax_White;	//255		//256时可统计完全空白的点，即RGB值为255的完全空白点;255时只能统计到RGB为254的值，255的值统计不到
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
			hranges[1] = g_nRecogGrayMax_White;	//255		//256时可统计完全空白的点，即RGB值为255的完全空白点;255时只能统计到RGB为254的值，255的值统计不到
			ranges[0] = hranges;
		}
#endif
		cv::MatND src_hist, comp_hist;
		cv::calcHist(&matCompRoi, 1, channels, cv::Mat(), comp_hist, 1, histSize, ranges, false);

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


		cv::MatND src_hist2;
		const int histSize2[1] = { 256 };	//rc.nThresholdValue - g_nRecogGrayMin
		const float* ranges2[1];
		float hranges2[2];
		hranges2[0] = 0;
		hranges2[1] = 255;
		ranges2[0] = hranges2;
		cv::calcHist(&matCompRoi, 1, channels, cv::Mat(), src_hist2, 1, histSize2, ranges2, true, false);
		int nCount = 0;
		for (int i = 0; i < 256; i++)
		{
			nCount += i * src_hist2.at<float>(i);
		}
		rc.fRealMeanGray = nCount / rc.fRealArea;
	}
	catch (cv::Exception &exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "COmrRecog::Recog error. detail: %s\n", exc.msg);
		g_pLogger->information(szLog);
		TRACE(szLog);
	}

	return bResult;
}

int COmrRecog::GetRightPicOrientation(cv::Mat& matSrc, int n, bool bDoubleScan)
{
	int nResult = CheckOrientation(matSrc, n, bDoubleScan);
	switch (nResult)	//1:针对模板图像需要进行的旋转，正向，不需要旋转，2：右转90(模板图像旋转), 3：左转90(模板图像旋转), 4：右转180(模板图像旋转)
	{
		case 1:	break;
		case 2:
		{
				  cv::Mat dst;
				  transpose(matSrc, dst);	//左旋90，镜像 
				  flip(dst, matSrc, 0);		//左旋90，模板图像需要右旋90，原图即需要左旋90
		}
			break;
		case 3:
		{
				  cv::Mat dst;
				  transpose(matSrc, dst);	//左旋90，镜像 
				  flip(dst, matSrc, 1);		//右旋90，模板图像需要左旋90，原图即需要右旋90
		}
			break;
		case 4:
		{
				  cv::Mat dst;
				  transpose(matSrc, dst);	//左旋90，镜像 
				  cv::Mat dst2;
				  flip(dst, dst2, 1);
				  cv::Mat dst5;
				  transpose(dst2, dst5);
				  flip(dst5, matSrc, 1);	//右旋180
		}
			break;
		default: 
			break;
	}
	return nResult;
}

int COmrRecog::CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan)
{
	clock_t start, end;
	start = clock();

	//*********************************
	//*********	测试结论 **************
	//前提：双面扫描
	//1、正面不需要旋转 ==> 反面也不需要旋转
	//2、正面需要右转90度 ==> 反面需要左转90度
	//3、正面需要左转90度 ==> 反面需要右转90度
	//4、正面需要旋转180度 ==> 反面也需要旋转180度
	//*********************************
	int nResult = 1;	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180

	static int nFristOrientation = 1;
	if (bDoubleScan && n % 2 != 0)	//双面扫描, 且属于双面扫描的第二面的情况
	{
		int nCountBK = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
		if (nCountBK == 0)
		{
			if (nFristOrientation == 1) nResult = 1;
			else if (nFristOrientation == 2) nResult = 3;
			else if (nFristOrientation == 3) nResult = 2;
			else if (nFristOrientation == 4) nResult = 4;
			end = clock();
			TRACE("判断旋转方向时间: %dms\n", end - start);

			std::string strDirection;
			switch (nResult)
			{
				case 1: strDirection = "正向，不需要旋转"; break;
				case 2: strDirection = "右旋90"; break;
				case 3: strDirection = "左旋90"; break;
				case 4: strDirection = "右旋180"; break;
			}
			std::string strLog = "双面扫描第二面，根据第一面方向判断结果：" + strDirection;
			g_pLogger->information(strLog);
			TRACE("%s\n", strLog.c_str());
			return nResult;
		}
	}

	cv::Mat matCom = matSrc.clone();
	if (_pModel_->nHasHead)
		nResult = CheckOrientation4Head(matCom, n);
	else
		nResult = CheckOrientation4Fix(matCom, n);

	if (bDoubleScan && n % 2 == 0)		//双面扫描，且属于扫描的第一面
		nFristOrientation = nResult;

	end = clock();
	TRACE("判断旋转方向时间: %dms\n", end - start);

	std::string strDirection;
	switch (nResult)
	{
		case 1: strDirection = "正向，不需要旋转"; break;
		case 2: strDirection = "右旋90"; break;
		case 3: strDirection = "左旋90"; break;
		case 4: strDirection = "右旋180"; break;
	}
	std::string strLog = "方向判断结果：" + strDirection;
	g_pLogger->information(strLog);
	TRACE("%s\n", strLog.c_str());

	return nResult;
}


int COmrRecog::CheckOrientation4Head(cv::Mat& matSrc, int n)
{
	bool bFind = false;
	int nResult = 1;	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180

	if (!_pModel_->nHasHead)
		return nResult;

	const float fMinPer = 0.5;		//识别矩形数/模板矩形数 低于最小值，认为不合格
	const float fMaxPer = 1.5;		//识别矩形数/模板矩形数 超过最大值，认为不合格
	const float fMidPer = 0.8;

	cv::Rect rtModelPic;
	rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
	rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matSrc.cols;
	rtSrcPic.height = matSrc.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matSrc.cols / matSrc.rows;

	cv::Rect rt1 = _pModel_->vecPaperModel[n]->rtHTracker;
	cv::Rect rt2 = _pModel_->vecPaperModel[n]->rtVTracker;
	TRACE("水平橡皮筋:(%d,%d,%d,%d), 垂直橡皮筋(%d,%d,%d,%d)\n", rt1.x, rt1.y, rt1.width, rt1.height, rt2.x, rt2.y, rt2.width, rt2.height);

	float fFirst_H, fFirst_V, fSecond_H, fSecond_V;
	fFirst_H = fFirst_V = fSecond_H = fSecond_V = 0.0;
	if (nModelPicPersent == nSrcPicPercent)	//与模板图片方向一致，需判断正向还是反向一致
	{
		TRACE("与模板图片方向一致\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			TRACE("查水平同步头\n");
			cv::Rect rtH = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rtHTracker, i);
			int nHead_H = GetRects(matSrc, rtH, _pModel_, n, i, 1);		//查水平同步头数量
			int nSum_H = _pModel_->vecPaperModel[n]->lH_Head.size();

			float fSimilarity_H = (float)nHead_H / nSum_H;
			if (fSimilarity_H < fMinPer || fSimilarity_H > fMaxPer)
				continue;

			if (i == 1)
				fFirst_H = fSimilarity_H;
			else
				fSecond_H = fSimilarity_H;

			TRACE("查垂直同步头\n");
			cv::Rect rtH2 = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rtVTracker, i);
			int nHead_V = GetRects(matSrc, rtH2, _pModel_, n, i, 2);		//查垂直同步头数量
			int nSum_V = _pModel_->vecPaperModel[n]->lV_Head.size();

			float fSimilarity_V = (float)nHead_V / nSum_V;

			char szLog[300] = { 0 };
			sprintf_s(szLog, "rtH = (%d,%d,%d,%d), rtH2 = (%d,%d,%d,%d),\nnHead_H = %d, nHead_V = %d, nSum_H = %d, nSum_V = %d, H=%.2f, V=%.2f\n", rtH.tl().x, rtH.tl().y, rtH.width, rtH.height, rtH2.tl().x, rtH2.tl().y, rtH2.width, rtH2.height, \
					  nHead_H, nHead_V, nSum_H, nSum_V, fSimilarity_H, fSimilarity_V);
			g_pLogger->information(szLog);
			TRACE(szLog);

			if (fSimilarity_H > fMidPer)
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (fSimilarity_V > fMidPer)
				{
					bFind = true;
					nResult = i;
					break;
				}
				else    //fSimilarity_V in [0.5,0.8]	有可能，再进行进一步判断
				{
					if (i == 1)
						fFirst_V = fSimilarity_V;
					else
						fSecond_V = fSimilarity_V;
				}
			}
			else	//fSimilarity_H in [0.5,0.8]	有可能，再进行进一步判断
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (i == 1)
					fFirst_V = fSimilarity_V;
				else
					fSecond_V = fSimilarity_V;
			}
		}

		if (!bFind)
		{
			if (fFirst_H > fSecond_H && fFirst_V > fSecond_V)
			{
				nResult = 1;
			}
			else if (fFirst_H < fSecond_H && fFirst_V < fSecond_V)
			{
				nResult = 4;
			}
			else
			{
				TRACE("无法判断图片方向\n");
				g_pLogger->information("无法判断图片方向");
				nResult = 1;
			}
		}
	}
	else	//与模板图片方向不一致，需判断向右旋转90还是向左旋转90
	{
		TRACE("与模板图片方向不一致\n");
		for (int i = 2; i <= 3; i++)
		{
			TRACE("查水平同步头\n");
			cv::Rect rtH = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rtHTracker, i);
			int nHead_H = GetRects(matSrc, rtH, _pModel_, n, i, 1);		//查水平同步头数量
			int nSum_H = _pModel_->vecPaperModel[n]->lH_Head.size();

			float fSimilarity_H = (float)nHead_H / nSum_H;
			if (fSimilarity_H < fMinPer || fSimilarity_H > fMaxPer)
				continue;

			if (i == 2)
				fFirst_H = fSimilarity_H;
			else
				fSecond_H = fSimilarity_H;

			TRACE("查垂直同步头\n");
			cv::Rect rtH2 = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rtVTracker, i);
			int nHead_V = GetRects(matSrc, rtH2, _pModel_, n, i, 2);		//查垂直同步头数量
			int nSum_V = _pModel_->vecPaperModel[n]->lV_Head.size();

			float fSimilarity_V = (float)nHead_V / nSum_V;

			char szLog[300] = { 0 };
			sprintf_s(szLog, "rtH = (%d,%d,%d,%d), rtH2 = (%d,%d,%d,%d),\nnHead_H = %d, nHead_V = %d, nSum_H = %d, nSum_V = %d, H=%.2f, V=%.2f\n", rtH.tl().x, rtH.tl().y, rtH.width, rtH.height, rtH2.tl().x, rtH2.tl().y, rtH2.width, rtH2.height, \
					  nHead_H, nHead_V, nSum_H, nSum_V, fSimilarity_H, fSimilarity_V);
			g_pLogger->information(szLog);
			TRACE(szLog);

			if (fSimilarity_H > fMidPer)
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (fSimilarity_V > fMidPer)
				{
					bFind = true;
					nResult = i;
					break;
				}
				else    //fSimilarity_V in [0.5,0.8]	有可能，再进行进一步判断
				{
					if (i == 2)
						fFirst_V = fSimilarity_V;
					else
						fSecond_V = fSimilarity_V;
				}
			}
			else	//fSimilarity_H in [0.5,0.8]	有可能，再进行进一步判断
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (i == 2)
					fFirst_V = fSimilarity_V;
				else
					fSecond_V = fSimilarity_V;
			}
		}

		if (!bFind)
		{
			if (fFirst_H > fSecond_H && fFirst_V > fSecond_V)
			{
				nResult = 2;
			}
			else if (fFirst_H < fSecond_H && fFirst_V < fSecond_V)
			{
				nResult = 3;
			}
			else
			{
				TRACE("无法判断图片方向\n");
				g_pLogger->information("无法判断图片方向");
				nResult = 1;
			}
		}
	}
	return nResult;
}

int COmrRecog::CheckOrientation4Fix(cv::Mat& matSrc, int n)
{
	bool bFind = false;
	int nResult = 1;	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180

	if (_pModel_->nHasHead)
		return nResult;

	std::string strLog;

	cv::Rect rtModelPic;
	rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
	rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matSrc.cols;
	rtSrcPic.height = matSrc.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matSrc.cols / matSrc.rows;

	if (_pModel_->nZkzhType == 2)			//使用条码的时候，先通过条码来判断方向
	{
		if (nModelPicPersent == nSrcPicPercent)
		{
			TRACE("与模板图片方向一致\n");
			for (int i = 1; i <= 4; i = i + 3)
			{
				bool bResult = RecogZkzh(n, matSrc, _pModel_, i);
				if (!bResult)
					continue;

				bFind = true;
				nResult = i;
				break;
			}
		}
		else
		{
			TRACE("与模板图片方向不一致\n");
			for (int i = 2; i <= 3; i++)
			{
				bool bResult = RecogZkzh(n, matSrc, _pModel_, i);
				if (!bResult)
					continue;

				bFind = true;
				nResult = i;
				break;
			}
		}
		if (bFind)
			return nResult;

		strLog.append("通过条形码或二维码判断试卷旋转方向失败，下面通过定位点判断\n");
	}

	int nCount = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
	if (nCount == 0)
		return nResult;

	if (nModelPicPersent == nSrcPicPercent)	//与模板图片方向一致，需判断正向还是反向一致
	{
		TRACE("与模板图片方向一致\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			//先查定点
			RECTLIST lFix;
			bool bResult = RecogFixCP(n, matSrc, lFix, _pModel_, i);
			// 			if (!bResult)
			// 				continue;
#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			PicTransfer(0, matSrc, lFix, _pModel_->vecPaperModel[n]->lFix, inverseMat);
#endif

			RECTLIST lModelTmp;
			if (lFix.size() < 3)
			{
				RECTLIST::iterator itFix = lFix.begin();
				for (auto itFix : lFix)
				{
					RECTLIST::iterator itModel = _pModel_->vecPaperModel[n]->lFix.begin();
					for (int j = 0; itModel != _pModel_->vecPaperModel[n]->lFix.end(); j++, itModel++)
					{
						if (j == itFix.nTH)
						{
							RECTINFO rcModel = *itModel;

							cv::Rect rtModelPic;
							rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
							rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
							rcModel.rt = GetRectByOrientation(rtModelPic, rcModel.rt, i);

							lModelTmp.push_back(rcModel);
							break;
						}
					}
				}
			}

			TRACE("查灰度校验点\n");
			bool bContinue = false;
			int nRtCount = 0;
			for (auto rcGray : _pModel_->vecPaperModel[n]->lGray)
			{
				RECTINFO rcItem = rcGray;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
					rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
					rcItem.rt = GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//根据实际定点个数获取矩形的相对位置，定点数为3或4时获取的实际上还是模板位置
				}
				else
					GetPosition(lFix, _pModel_->vecPaperModel[n]->lFix, rcItem.rt);		//根据实际定点个数获取矩形的相对位置，定点数为3或4时获取的实际上还是模板位置

				if (RecogRtVal(rcItem, matSrc))
				{
					if (rcItem.fRealDensity / rcGray.fStandardDensity > rcGray.fStandardValuePercent && rcItem.fRealValue / rcGray.fStandardValue > rcGray.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("判断灰度校验点的密度百分比: %f, 低于要求的: %f\n", rcItem.fRealValuePercent, rcGray.fStandardValuePercent);
						// 						bContinue = true;
						// 						break;
					}
				}
				else
				{
					// 					bContinue = true;
					// 					break;
				}
			}
			if (bContinue)
				continue;

			TRACE("科目校验点\n");
			bContinue = false;
			for (auto rcSubject : _pModel_->vecPaperModel[n]->lCourse)
			{
				RECTINFO rcItem = rcSubject;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
					rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
					rcItem.rt = GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//根据实际定点个数获取矩形的相对位置，定点数为3或4时获取的实际上还是模板位置
				}
				else
					GetPosition(lFix, _pModel_->vecPaperModel[n]->lFix, rcItem.rt);		//根据实际定点个数获取矩形的相对位置，定点数为3或4时获取的实际上还是模板位置

				if (RecogRtVal(rcItem, matSrc))
				{
					if (rcItem.fRealDensity / rcSubject.fStandardDensity > rcSubject.fStandardValuePercent && rcItem.fRealValue / rcSubject.fStandardValue > rcSubject.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("判断科目校验点的密度百分比: %f, 低于要求的: %f\n", rcItem.fRealValuePercent, rcSubject.fStandardValuePercent);
						// 						bContinue = true;
						// 						break;
					}
				}
				else
				{
					// 					bContinue = true;
					// 					break;
				}
			}
			if (bContinue)
				continue;

			//判断总数
			int nAllCount = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
			if (nAllCount <= 2)
			{
				if (nRtCount >= nAllCount)
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("总校验点数=%d, 实际识别校验点数=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
			else
			{
				if (nRtCount >= (int)(nAllCount * 0.9))
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("总校验点数=%d, 实际识别校验点数=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
		}

		if (!bFind)
		{
			TRACE("无法判断图片方向\n");
			strLog.append("无法判断图片方向\n");
			g_pLogger->information(strLog);
			nResult = 1;
		}
	}
	else	//与模板图片方向不一致，需判断向右旋转90还是向左旋转90
	{
		TRACE("与模板图片方向不一致\n");
		for (int i = 2; i <= 3; i++)
		{
			//先查定点
			RECTLIST lFix;
			bool bResult = RecogFixCP(n, matSrc, lFix, _pModel_, i);
			// 			if (!bResult)
			// 				continue;
#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			cv::Mat matDst;
			PicTransfer2(0, matSrc, matDst, lFix, _pModel_->vecPaperModel[n]->lFix, inverseMat);
#endif

			RECTLIST lModelTmp;
			if (lFix.size() < 3)
			{
				matDst = matSrc;

				RECTLIST::iterator itFix = lFix.begin();
				for (auto itFix : lFix)
				{
					RECTLIST::iterator itModel = _pModel_->vecPaperModel[n]->lFix.begin();
					for (int j = 0; itModel != _pModel_->vecPaperModel[n]->lFix.end(); j++, itModel++)
					{
						if (j == itFix.nTH)
						{
							RECTINFO rcModel = *itModel;

							cv::Rect rtModelPic;
							rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
							rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
							rcModel.rt = GetRectByOrientation(rtModelPic, rcModel.rt, i);

							lModelTmp.push_back(rcModel);
							break;
						}
					}
				}
			}

			TRACE("查灰度校验点\n");
			bool bContinue = false;
			int nRtCount = 0;
			for (auto rcGray : _pModel_->vecPaperModel[n]->lGray)
			{
				RECTINFO rcItem = rcGray;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
					rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
					rcItem.rt = GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//根据实际定点个数获取矩形的相对位置，定点数为3或4时获取的实际上还是模板位置
				}
				else
					GetPosition(lFix, _pModel_->vecPaperModel[n]->lFix, rcItem.rt);		//根据实际定点个数获取矩形的相对位置，定点数为3或4时获取的实际上还是模板位置

				if (RecogRtVal(rcItem, matDst))
				{
					if (rcItem.fRealDensity / rcGray.fStandardDensity > rcGray.fStandardValuePercent && rcItem.fRealValue / rcGray.fStandardValue > rcGray.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("判断灰度校验点的密度百分比: %f, 低于要求的: %f\n", rcItem.fRealValuePercent, rcGray.fStandardValuePercent);
						// 						bContinue = true;
						// 						break;
					}
				}
				else
				{
					// 					bContinue = true;
					// 					break;
				}
			}
			if (bContinue)
				continue;

			TRACE("科目校验点\n");
			bContinue = false;
			for (auto rcSubject : _pModel_->vecPaperModel[n]->lCourse)
			{
				RECTINFO rcItem = rcSubject;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
					rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
					rcItem.rt = GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//根据实际定点个数获取矩形的相对位置，定点数为3或4时获取的实际上还是模板位置
				}
				else
					GetPosition(lFix, _pModel_->vecPaperModel[n]->lFix, rcItem.rt);		//根据实际定点个数获取矩形的相对位置，定点数为3或4时获取的实际上还是模板位置

				if (RecogRtVal(rcItem, matDst))
				{
					if (rcItem.fRealDensity / rcSubject.fStandardDensity > rcSubject.fStandardValuePercent && rcItem.fRealValue / rcSubject.fStandardValue > rcSubject.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("判断科目校验点的密度百分比: %f, 低于要求的: %f\n", rcItem.fRealValuePercent, rcSubject.fStandardValuePercent);
						// 						bContinue = true;
						// 						break;
					}
				}
				else
				{
					// 					bContinue = true;
					// 					break;
				}
			}
			if (bContinue)
				continue;

			//判断总数
			int nAllCount = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
			if (nAllCount <= 2)
			{
				if (nRtCount >= nAllCount)
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("总校验点数=%d, 实际识别校验点数=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
			else
			{
				if (nRtCount >= (int)(nAllCount * 0.9))
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("总校验点数=%d, 实际识别校验点数=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
		}

		if (!bFind)
		{
			TRACE("无法判断图片方向，采用默认右旋90度的方向\n");
			strLog.append("无法判断图片方向，采用默认右旋90度的方向\n");
			g_pLogger->information(strLog);
			nResult = 2;	//如果出现无法判断图像方向时，默认模板需要右旋90度变成此图像方向，即默认返回方向为右旋90度，因为方向只有右旋90或者左旋90度两种选择，此处不返回默认的1，返回2
		}
	}

	return nResult;
}

int COmrRecog::GetRects(cv::Mat& matSrc, cv::Rect rt, pMODEL pModel, int nPic, int nOrientation, int nHead)
{
	int nResult = 0;
	std::vector<cv::Rect>RectCompList;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matSrc.cols)
		{
			rt.width = matSrc.cols - rt.x;
		}
		if (rt.br().y > matSrc.rows)
		{
			rt.height = matSrc.rows - rt.y;
		}

		cv::Mat matCompRoi;
		matCompRoi = matSrc(rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
		sharpenImage1(matCompRoi, matCompRoi, 3);

#ifdef USES_GETTHRESHOLD_ZTFB
		const int channels[1] = { 0 };
		const int histSize[1] = { 150 };
		float hranges[2] = { 0, 150 };
		const float* ranges[1];
		ranges[0] = hranges;
		cv::MatND hist;
		calcHist(&matCompRoi, 1, channels, cv::Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		float fMean = (float)nSum / nCount;		//均值

		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nDevSum += pow(h - fMean, 2)*binVal;
		}
		float fStdev = sqrt(nDevSum / nCount);	//标准差
		int nThreshold = fMean + 2 * fStdev;
		if (fStdev > fMean)
			nThreshold = fMean + fStdev;

		if (nThreshold > 150) nThreshold = 150;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);

		// 		int blockSize = 25;		//25
		// 		int constValue = 10;
		// 		cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
#else
		threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
		//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
		cv::Mat element_Anticlutter = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
		dilate(matCompRoi, matCompRoi, element_Anticlutter);
		erode(matCompRoi, matCompRoi, element_Anticlutter);

		cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));	//Size(6, 6)	普通空白框可识别
		dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
#if 1
		//模板图像的水平同步头平均长宽
		RECTLIST::iterator itBegin;
		if (nHead == 1)	//检测水平同步头
			itBegin = pModel->vecPaperModel[nPic]->lH_Head.begin();
		else
			itBegin = pModel->vecPaperModel[nPic]->lV_Head.begin();
		RECTINFO rcFist = *itBegin;
		RECTINFO rcSecond = *(++itBegin);

		int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
		int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

		float fPer_W, fPer_H;	//模板第二个点与第一个点的宽、高的比例，用于最小值控制
		cv::Rect rtFirst, rtSecond;
		if (nOrientation == 1 || nOrientation == 4)
		{
			rtSecond = rcSecond.rt;
			rtFirst = rcFist.rt;
			fPer_W = 0.5;
			fPer_H = 0.25;
		}
		else if (nOrientation == 2 || nOrientation == 3)
		{
			rtSecond.width = rcSecond.rt.height;
			rtSecond.height = rcSecond.rt.width;

			rtFirst.width = rcFist.rt.height;
			rtFirst.height = rcFist.rt.width;
			fPer_W = 0.25;
			fPer_H = 0.5;
		}

		if (pModel->nType == 1)
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

			// 			float fOffset = 0.1;
			// 			int nMid_modelW = rtSecond.width + 2;		//加2是因为制卷模板框框没有经过查边框运算，经过查边框后，外框会包含整个矩形，需要加上上下各1个单位的线宽
			// 			int nMid_modelH = rtSecond.height + 2;
			// 			if (nMid_modelW < rtFirst.width * fPer_W + 0.5)	nMid_modelW = rtFirst.width * fPer_W + 0.5;
			// 			if (nMid_modelH < rtFirst.height * fPer_H + 0.5)	nMid_modelH = rtFirst.height * fPer_H + 0.5;
			// 			nMid_minW = nMid_modelW * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			// 			nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			// 			nMid_minH = nMid_modelH * (1 - fOffset);				//同上
			// 			nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//同上
			// 
			// 			nHead_minW = rtFirst.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
			// 			nHead_maxW = rtFirst.width * (1 + fOffset * 4) + 0.5;		//同上
			// 			nHead_minH = rtFirst.height * (1 - fOffset);				//同上
			// 			nHead_maxH = rtFirst.height * (1 + fOffset * 4) + 0.5;		//同上
		}
		else
		{
			float fOffset = 0.2;
			nMid_minW = rtSecond.width * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_maxW = rtSecond.width * (1 + fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_minH = rtSecond.height * (1 - fOffset);		//同上
			nMid_maxH = rtSecond.height * (1 + fOffset);		//同上

			nHead_minW = rtFirst.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nHead_maxW = rtFirst.width * (1 + fOffset);		//同上
			nHead_minH = rtFirst.height * (1 - fOffset);		//同上
			nHead_maxH = rtFirst.height * (1 + fOffset);		//同上
		}
		TRACE("w(%d, %d), h(%d, %d)，最大点:w(%d, %d), h(%d, %d)\n", nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, \
			  nHead_minW, nHead_maxW, nHead_minH, nHead_maxH);

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;

			if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
			{
				if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//排除第一个或最后一个大的同步头
				{
					TRACE("过滤同步头(%d,%d,%d,%d), 要求范围W:[%d,%d], H[%d,%d], 参考大小(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
					continue;
				}
				else
				{
					TRACE("首尾同步头(即定位点同步头)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
				}
			}
			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
#else
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;
			if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50 || rm.area() < 70)	//********** 需要寻找一种新的方法来过滤矩形	********
			{
				//				TRACE("过滤矩形:(%d,%d,%d,%d), 面积: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
				continue;
			}
			RectCompList.push_back(rm);
		}
#endif
		nResult = RectCompList.size();
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "识别同步头异常: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		nResult = -1;
	}
	return nResult;
}

float COmrRecog::GetRtDensity(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod)
{
	cv::Mat matCompRoi;
	matCompRoi = matSrc(rt);
	cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);
	cv::GaussianBlur(matCompRoi, matCompRoi, cv::Size(rcMod.nGaussKernel, rcMod.nGaussKernel), 0, 0);
	sharpenImage1(matCompRoi, matCompRoi, rcMod.nSharpKernel);

	const int channels[1] = { 0 };
	const float* ranges[1];
	const int histSize[1] = { 1 };
	float hranges[2];
	hranges[0] = g_nRecogGrayMin;
	hranges[1] = static_cast<float>(rcMod.nThresholdValue);
	ranges[0] = hranges;

	cv::MatND src_hist;
	cv::calcHist(&matCompRoi, 1, channels, cv::Mat(), src_hist, 1, histSize, ranges, false);

	float fRealVal = src_hist.at<float>(0);
	float fRealArea = rt.area();
	float fRealDensity = fRealVal / fRealArea;

	return fRealVal;
}

bool COmrRecog::bGetMaxRect(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod, cv::Rect& rtMax)
{
	clock_t start, end;
	start = clock();

	bool bResult = false;

	int nResult = 0;
	std::vector<cv::Rect>RectCompList;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matSrc.cols)
		{
			rt.width = matSrc.cols - rt.x;
		}
		if (rt.br().y > matSrc.rows)
		{
			rt.height = matSrc.rows - rt.y;
		}

		cv::Mat matCompRoi;
		matCompRoi = matSrc(rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(rcMod.nGaussKernel, rcMod.nGaussKernel), 0, 0);
		sharpenImage1(matCompRoi, matCompRoi, rcMod.nSharpKernel);

#ifdef USES_GETTHRESHOLD_ZTFB
		const int channels[1] = { 0 };
		const int histSize[1] = { 150 };
		float hranges[2] = { 0, 150 };
		const float* ranges[1];
		ranges[0] = hranges;
		cv::MatND hist;
		calcHist(&matCompRoi, 1, channels, cv::Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		int nThreshold = 150;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		if (nCount > 0)
		{
			float fMean = (float)nSum / nCount;		//均值

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);	//标准差
			nThreshold = fMean + 2 * fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;
		}
		if (nThreshold > 150) nThreshold = 150;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);

		// 		int blockSize = 25;		//25
		// 		int constValue = 10;
		// 		cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
#else
		threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
		//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
		cv::Mat element_Anticlutter = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
		dilate(matCompRoi, matCompRoi, element_Anticlutter);
		erode(matCompRoi, matCompRoi, element_Anticlutter);

		cv::Canny(matCompRoi, matCompRoi, 0, rcMod.nCannyKernel, 5);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(6, 6));	//Size(6, 6)	普通空白框可识别
		dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
#if 0
		//模板图像的水平同步头平均长宽

		RECTLIST::iterator itBegin;
		if (nHead == 1)	//检测水平同步头
			itBegin = pModel->vecPaperModel[nPic]->lH_Head.begin();
		else if (nHead == 2)
			itBegin = pModel->vecPaperModel[nPic]->lV_Head.begin();
		RECTINFO rcFist = *itBegin;
		RECTINFO rcSecond = *(++itBegin);

		int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
		int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

		float fPer_W, fPer_H;	//模板第二个点与第一个点的宽、高的比例，用于最小值控制
		cv::Rect rtFirst, rtSecond;
		if (nOrientation == 1 || nOrientation == 4)
		{
			rtSecond = rcSecond.rt;
			rtFirst = rcFist.rt;
			fPer_W = 0.5;
			fPer_H = 0.25;
		}
		else if (nOrientation == 2 || nOrientation == 3)
		{
			rtSecond.width = rcSecond.rt.height;
			rtSecond.height = rcSecond.rt.width;

			rtFirst.width = rcFist.rt.height;
			rtFirst.height = rcFist.rt.width;
			fPer_W = 0.25;
			fPer_H = 0.5;
		}

		if (pModel->nType == 1)
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
		}
		else
		{
			float fOffset = 0.2;
			nMid_minW = rtSecond.width * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_maxW = rtSecond.width * (1 + fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_minH = rtSecond.height * (1 - fOffset);		//同上
			nMid_maxH = rtSecond.height * (1 + fOffset);		//同上

			nHead_minW = rtFirst.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nHead_maxW = rtFirst.width * (1 + fOffset);		//同上
			nHead_minH = rtFirst.height * (1 - fOffset);		//同上
			nHead_maxH = rtFirst.height * (1 + fOffset);		//同上
		}

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;

			if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
			{
				if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//排除第一个或最后一个大的同步头
				{
					TRACE("过滤同步头(%d,%d,%d,%d), 要求范围W:[%d,%d], H[%d,%d], 参考大小(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
					continue;
				}
				else
				{
					TRACE("首尾同步头(即定位点同步头)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
				}
			}
			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
#else
		int nMaxArea = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;
			if (rm.area() > nMaxArea)
			{
				rtMax = rm;
				nMaxArea = rm.area();
			}
		}
#endif
		if (nMaxArea > 0)
			bResult = true;
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "识别校验点矩形异常: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		nResult = -1;
	}
	end = clock();
	TRACE("计算矩形数量时间: %d\n", end - start);

	return bResult;
}

void COmrRecog::sharpenImage1(const cv::Mat &image, cv::Mat &result, int nKernel)
{
	//创建并初始化滤波模板
	cv::Mat kernel(nKernel, nKernel, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = 5;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//对图像进行滤波
	cv::filter2D(image, result, image.depth(), kernel);
}

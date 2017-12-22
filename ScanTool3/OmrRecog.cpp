#include "stdafx.h"
#include "OmrRecog.h"


COmrRecog::COmrRecog()
{
	_nFristOrientation = 1;
}


COmrRecog::~COmrRecog()
{
}

cv::Rect COmrRecog::GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Rect rtResult;
	if (nOrientation == 1)	//matSrc正向	模板图像的旋转方向
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
	TRACE("==> recogFixCP\n");

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

			TRACE("------>Recog Fix Sel 1: %d, rtTmp:(%d, %d, %d, %d), matCompRoi(%d, %d)\n", i, rtTmp.x, rtTmp.y, rtTmp.width, rtTmp.height, matCompRoi.cols, matCompRoi.rows);

			if (matCompRoi.channels() == 3)
				cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

			TRACE("------>Recog Fix Sel 2: %d\n", i);
			GaussianBlur(matCompRoi, matCompRoi, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//cv::Size(_nGauseKernel_, _nGauseKernel_)

			TRACE("------>Recog Fix Sel 3: %d\n", i);
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

			TRACE("------>Recog Fix Sel 4: %d\n", i);
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
			TRACE("------>Recog Fix Sel 5: %d\n", i);

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

				//根据定点左上点与右下点位置判断是否在试卷的边线上，如果在，则可能是折角或者边上有损坏
				cv::Point pt1 = RectCompList[i].tl();
				cv::Point pt2 = RectCompList[i].br();
				int nDiff = 4;	//与图像边界的距离间隔在这个值之内，认为属于边界线上
				if (pt1.x < nDiff || pt1.y < nDiff || matCompPic.cols - pt2.x < nDiff || matCompPic.rows - pt2.y < nDiff)
				{
					TRACE("矩形(%d,%d,%d,%d)位置距离边线太近，可能是折角或损坏\n", RectCompList[i].x, RectCompList[i].y, RectCompList[i].width, RectCompList[i].height);
					continue;
				}
				TRACE("--->recog gray val.\n");
				Recog(nPic, rcTmp, matCompPic, NULL, NULL);
				TRACE("<---recog gray val done.\n");
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
				
				if(matCompRoi.channels() == 3)
					cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

				string strTypeName;
				string strResult = GetQR(matCompRoi, strTypeName);

				std::string strLog;
				if (strResult != "")
				{
					bResult = true;
					TRACE("GetQR: %s\n", strResult.c_str());
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

cv::Mat COmrRecog::GetRotMat(RECTLIST lFixRealPic, RECTLIST lFixModelPic)
{
	cv::Mat matResult;
	if (lFixRealPic.size() == 3)	//仿射变换
	{
		std::vector<cv::Point2f> vecFixPt;
		std::vector<cv::Point2f> vecFixNewPt;
		RECTLIST::iterator itCP2 = lFixRealPic.begin();
		for (; itCP2 != lFixRealPic.end(); itCP2++)
		{
			cv::Point2f pt;

			pt.x = itCP2->rt.x + itCP2->rt.width / 2;
			pt.y = itCP2->rt.y + itCP2->rt.height / 2;

			vecFixNewPt.push_back(pt);
			//获取该定点属于第几个模板定点
			RECTLIST::iterator itCP = lFixModelPic.begin();
			for (int i = 0; itCP != lFixModelPic.end(); i++, itCP++)
			{
				if (i == itCP2->nTH)
				{
					cv::Point2f pt2;

					pt2.x = itCP->rt.x + itCP->rt.width / 2;
					pt2.y = itCP->rt.y + itCP->rt.height / 2;

					vecFixPt.push_back(pt2);
					break;
				}
			}
		}
		cv::Point2f srcTri[3];
		cv::Point2f dstTri[3];
		cv::Mat rot_mat(2, 3, CV_32FC1);
		for (int i = 0; i < vecFixPt.size(); i++)
		{
			srcTri[i] = vecFixNewPt[i];
			dstTri[i] = vecFixPt[i];
		}
		rot_mat = cv::getAffineTransform(srcTri, dstTri);

		//反向计算模板上点对应的实际图像的点
		cv::Mat rot_mat_inv = rot_mat.inv();	//逆矩阵
		matResult = rot_mat_inv;
	}
	else if (lFixRealPic.size() == 4)	//透视逆变换
	{
		std::vector<cv::Point2f> vecFixPt;
		RECTLIST::iterator itCP = lFixModelPic.begin();
		for (; itCP != lFixModelPic.end(); itCP++)
		{
			cv::Point2f pt;
			pt.x = itCP->rt.x + itCP->rt.width / 2;
			pt.y = itCP->rt.y + itCP->rt.height / 2;
			vecFixPt.push_back(pt);
		}
		std::vector<cv::Point2f> vecFixNewPt;
		RECTLIST::iterator itCP2 = lFixRealPic.begin();
		for (; itCP2 != lFixRealPic.end(); itCP2++)
		{
			cv::Point2f pt;
			pt.x = itCP2->rt.x + itCP2->rt.width / 2;
			pt.y = itCP2->rt.y + itCP2->rt.height / 2;
			vecFixNewPt.push_back(pt);
		}

		cv::Point2f srcTri[4];
		cv::Point2f dstTri[4];
		cv::Mat rot_mat(3, 3, CV_32FC1);		//warp_mat(2, 3, CV_32FC1);
		for (int i = 0; i < vecFixPt.size(); i++)
		{
			srcTri[i] = vecFixNewPt[i];
			dstTri[i] = vecFixPt[i];
		}
		rot_mat = cv::getPerspectiveTransform(srcTri, dstTri);

		cv::Mat rot_mat_inv = rot_mat.inv();	//逆矩阵
		matResult = rot_mat_inv;
	}
	return matResult;
}

cv::Rect COmrRecog::GetRealRtFromModel(cv::Rect rtModel, RECTLIST lFixRealPic, RECTLIST lFixModelPic, cv::Mat rot_mat_inv)
{
	cv::Rect rtResult;
	if (lFixRealPic.size() <= 2)
	{
		rtResult = rtModel;
		GetPosition(lFixRealPic, lFixModelPic, rtResult);
	}
	if (lFixRealPic.size() == 3 || lFixRealPic.size() == 4)
	{
		//cv::Mat rot_mat_inv = GetRotMat(lFixRealPic, lFixModelPic);

		cv::Point2f pt1 = rtModel.tl();
		cv::Point2f pt2 = rtModel.br();
		cv::Point2f p1 = cv::Point2f(0, 0);
		cv::Point2f p2 = cv::Point2f(0, 0);
		p1.x = rot_mat_inv.ptr<double>(0)[0] * pt1.x + rot_mat_inv.ptr<double>(0)[1] * pt1.y + rot_mat_inv.ptr<double>(0)[2];
		p1.y = rot_mat_inv.ptr<double>(1)[0] * pt1.x + rot_mat_inv.ptr<double>(1)[1] * pt1.y + rot_mat_inv.ptr<double>(1)[2];

		p2.x = rot_mat_inv.ptr<double>(0)[0] * pt2.x + rot_mat_inv.ptr<double>(0)[1] * pt2.y + rot_mat_inv.ptr<double>(0)[2];
		p2.y = rot_mat_inv.ptr<double>(1)[0] * pt2.x + rot_mat_inv.ptr<double>(1)[1] * pt2.y + rot_mat_inv.ptr<double>(1)[2];
		rtResult = cv::Rect(p1, p2);
	}

	return rtResult;
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
		if (matCompRoi.channels() == 3)
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
	_strLog.clear();
	clock_t sTime, eTime;
	sTime = clock();
	int nResult = CheckOrientation(matSrc, n, bDoubleScan, _strLog);
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
	eTime = clock();
	TRACE("判断方向总时间: %d\n", eTime - sTime);
	return nResult;
}

int COmrRecog::CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan, std::string& strLog)
{
	clock_t start, end;
	start = clock();
	strLog.append("************>\n检测图像方向:\n");

	//*********************************
	//*********	测试结论 **************
	//前提：双面扫描
	//1、正面不需要旋转 ==> 反面也不需要旋转
	//2、正面需要右转90度 ==> 反面需要左转90度
	//3、正面需要左转90度 ==> 反面需要右转90度
	//4、正面需要旋转180度 ==> 反面也需要旋转180度
	//0、正面无法判断旋转方向，采用默认方向，不需要旋转==> 反面也采用默认方向，不需要旋转
	//*********************************
	int nResult = 1;	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180

	if (bDoubleScan && n % 2 != 0)	//双面扫描, 且属于双面扫描的第二面的情况
	{
		int nCountBK = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
		if (nCountBK == 0)
		{
			if (_nFristOrientation == 1) nResult = 1;
			else if (_nFristOrientation == 2) nResult = 3;
			else if (_nFristOrientation == 3) nResult = 2;
			else if (_nFristOrientation == 4) nResult = 4;
			//else if (_nFristOrientation == 0) nResult = 0;
			end = clock();
			TRACE("判断旋转方向时间: %dms\n", end - start);

			std::string strDirection;
			switch (nResult)
			{
				case 1: strDirection = "正向，不需要旋转"; break;
				case 2: strDirection = "右旋90"; break;
				case 3: strDirection = "左旋90"; break;
				case 4: strDirection = "右旋180"; break;
				//case 0: strDirection = "正向，无法识别，故不旋转"; break;
			}
			strLog.append("双面扫描第二面，根据第一面方向判断结果：" + strDirection);
			strLog.append("<************\n");
			//g_pLogger->information(strLog);
			TRACE("%s\n", strLog.c_str());
			return nResult;
		}
	}
#if 1
	cv::Mat matCom = matSrc.clone();
	try
	{
		if (matCom.channels() == 3)
			cvtColor(matCom, matCom, CV_BGR2GRAY);
	}
	catch (cv::Exception& exc)
	{
		TRACE("err: cvtColor. %s\n", exc.what());
	}
#else
	cv::Mat matCom;
	try
	{
		matCom = matSrc.clone();
	}
	catch (cv::Exception& exc)
	{
		TRACE("err: matSrc.clone(). %s\n", exc.what());
		matCom = matSrc;
	}
#endif
	if (_pModel_->nHasHead)
		nResult = CheckOrientation4Head(matCom, n);
	else
		nResult = CheckOrientation4Fix(matCom, n, strLog);

	if (bDoubleScan && n % 2 == 0)		//双面扫描，且属于扫描的第一面
		_nFristOrientation = nResult;

	end = clock();
	std::string strTmp = Poco::format("判断旋转方向时间: %dms\n", (int)(end - start));
	TRACE("判断旋转方向时间: %dms\n", end - start);

	std::string strDirection;
	switch (nResult)
	{
		case 1: strDirection = "正向，不需要旋转"; break;
		case 2: strDirection = "右旋90"; break;
		case 3: strDirection = "左旋90"; break;
		case 4: strDirection = "右旋180"; break;
		//case 0: strDirection = "正向，无法识别，故不旋转"; break;
	}
	strLog.append("方向判断结果：" + strDirection);
	strLog.append("\t" + strTmp);
	strLog.append("<************\n");
	//g_pLogger->information(strLog);
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

int COmrRecog::CheckOrientation4Word(cv::Mat& matSrc, int n)
{
	bool bFind = false;
	int nResult = 1;	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180

	if (_pModel_->vecPaperModel[n]->lCharacterAnchorArea.size() == 0)
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
		if (RecogCodeOrientation(matSrc, n, _pModel_, nResult))
			return nResult;

		strLog.append("通过条形码或二维码判断试卷旋转方向失败，下面通过定位点判断\n");
	}

	if (nModelPicPersent == nSrcPicPercent)	//与模板图片方向一致，需判断正向还是反向一致
	{
		TRACE("与模板图片方向一致\n");

		for (int i = 1; i <= 4; i = i + 3)
		{
			bFind = RecogWordOrientationByRectCount(matSrc, n, i, nResult, strLog);
			if (bFind)
				break;
		}

		if (!bFind)
		{
			for (int i = 1; i <= 4; i = i + 3)
			{
				bFind = RecogWordOrientationByMatchTempl(matSrc, n, i, nResult, strLog);
				if (bFind)
					break;
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
	else
	{
		TRACE("与模板图片方向不一致\n");

		for (int i = 2; i <= 3; i++)
		{
			bFind = RecogWordOrientationByRectCount(matSrc, n, i, nResult, strLog);
			if (bFind)
				break;
		}
		if (!bFind)
		{
			for (int i = 2; i <= 3; i++)
			{
				bFind = RecogWordOrientationByMatchTempl(matSrc, n, i, nResult, strLog);
				if (bFind)
					break;
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
	//------------------------------------------
	if(bFind)
		g_pLogger->information(strLog);
	//------------------------------------------
	return nResult;
}

int COmrRecog::CheckOrientation4Fix(cv::Mat& matSrc, int n, std::string& strLog)
{
	bool bFind = false;
	int nResult = 1;	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180

	if (_pModel_->nHasHead)
		return nResult;

	if (_pModel_->vecPaperModel[n]->lCharacterAnchorArea.size() > 0)	//有文字定位时，不用定点
	{
		nResult = CheckOrientation4Word(matSrc, n);
		return nResult;
	}
	
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
		if (RecogCodeOrientation(matSrc, n, _pModel_, nResult))
			return nResult;
		strLog.append("通过条形码或二维码判断试卷旋转方向失败，下面通过定位点判断\n");
	}

	int nCount = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
	if (nCount == 0)
	{
		if (nModelPicPersent == nSrcPicPercent)	//与模板图片方向一致，需判断正向还是反向一致
		{
			for (int i = 1; i <= 4; i = i + 3)
			{
				bFind = RecogWordOrientationByRectCount(matSrc, n, i, nResult, strLog);
				if (bFind)
					break;
			}
		}
		else
		{
			for (int i = 2; i <= 3; i++)
			{
				bFind = RecogWordOrientationByRectCount(matSrc, n, i, nResult, strLog);
				if (bFind)
					break;
			}
		}
		return nResult;
	}

	cv::Mat matComp = matSrc.clone();
	if (nModelPicPersent == nSrcPicPercent)	//与模板图片方向一致，需判断正向还是反向一致
	{
		TRACE("与模板图片方向一致\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			//先查定点
			RECTLIST lFix;
			bool bResult = RecogFixCP(n, matComp, lFix, _pModel_, i);
			// 			if (!bResult)
			// 				continue;
			TRACE("recogFixCP done.\n");
#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			PicTransfer(0, matComp, lFix, _pModel_->vecPaperModel[n]->lFix, inverseMat);
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

				if (RecogRtVal(rcItem, matComp))
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

				if (RecogRtVal(rcItem, matComp))
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

		if (!bFind)		//通过校验点无法判断方向时，尝试矩形数量来判断
		{
			for (int i = 1; i <= 4; i = i + 3)
			{
				bFind = RecogWordOrientationByRectCount(matSrc, n, i, nResult, strLog);
				if (bFind)
					break;
			}
		}
		if (!bFind)
		{
			TRACE("无法判断图片方向\n");
			strLog.append("无法判断图片方向\n");
			//g_pLogger->information(strLog);
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
			bool bResult = RecogFixCP(n, matComp, lFix, _pModel_, i);
			// 			if (!bResult)
			// 				continue;
#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			cv::Mat matDst;
			PicTransfer2(0, matComp, matDst, lFix, _pModel_->vecPaperModel[n]->lFix, inverseMat);
#endif

			RECTLIST lModelTmp;
			if (lFix.size() < 3)
			{
				matDst = matComp;

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

		if (!bFind)	//通过校验点无法判断方向时，尝试矩形数量来判断
		{
			for (int i = 2; i <= 3; i++)
			{
				bFind = RecogWordOrientationByRectCount(matSrc, n, i, nResult, strLog);
				if (bFind)
					break;
			}
		}
		if (!bFind)
		{
			TRACE("无法判断图片方向，采用默认右旋90度的方向\n");
			strLog.append("无法判断图片方向，采用默认右旋90度的方向\n");
			//g_pLogger->information(strLog);
			nResult = 2;	//如果出现无法判断图像方向时，默认模板需要右旋90度变成此图像方向，即默认返回方向为右旋90度，因为方向只有右旋90或者左旋90度两种选择，此处不返回默认的1，返回2
		}
	}

	return nResult;
}

bool COmrRecog::RecogCodeOrientation(cv::Mat& matSrc, int n, pMODEL pModel, int& nResult)
{
	bool bFind = false;
//	int nResult = 1;	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180

	if (_pModel_->nZkzhType != 2)
		return bFind;
	
	cv::Rect rtModelPic;
	rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
	rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matSrc.cols;
	rtSrcPic.height = matSrc.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matSrc.cols / matSrc.rows;

	if (nModelPicPersent == nSrcPicPercent)
	{
		TRACE("与模板图片方向一致\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			bool bResult = RecogZkzh(n, matSrc, pModel, i);
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
			bool bResult = RecogZkzh(n, matSrc, pModel, i);
			if (!bResult)
				continue;

			bFind = true;
			nResult = i;
			break;
		}
	}
// 	if (bFind)
// 		return nResult;
	
	return bFind;
}

void COmrRecog::PicRotate(cv::Mat& matSrc, int n)
{
	clock_t start, end;
	start = clock();
	switch (n)	//1:实际图像需要进行的旋转，正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
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
	end = clock();
	TRACE("图像旋转time = %dms\n", end - start);
}

bool COmrRecog::RecogWordOrientationByMatchTempl(cv::Mat& matSrc, int n, int nRotation, int& nResult, std::string& strLog)
{
	bool bFind = false;
	clock_t start, end;
	start = clock();

	int nRtCount = 0;
	std::vector<ST_WORDMATCHTEMPL> vecMatchTemplPoint;	//模板匹配到的点
	int nModelWordPoint = 0;

	//----------------将图像旋转到模板一样的方向，方便进行模板匹配
	cv::Mat matRealPic = matSrc.clone();
	PicRotate(matRealPic, nRotation);
	//----------------

	//根据文字定位的个数进行模板匹配，然后计算多个匹配的灰度、密度、位置等信息并进行比较
	int nMaxCompWordPint = 8;	//最多匹配模板定点的个数
	for (auto itCharactArea : _pModel_->vecPaperModel[n]->lCharacterAnchorArea)
	{
		cv::Mat SrcCharactArea = matRealPic(itCharactArea->rt);
		
		for (auto itCharactPoint : itCharactArea->vecCharacterRt)
		{
			if(nModelWordPoint >= nMaxCompWordPint)
				break;
			cv::Mat CharactPointTempl = _pModel_->vecPaperModel[n]->matModel(itCharactPoint->rc.rt);
			cv::Point ptResult1;
			MatchingMethod(0, SrcCharactArea, CharactPointTempl, ptResult1);

			cv::Point ptResult2;
			MatchingMethod(1, SrcCharactArea, CharactPointTempl, ptResult2);
			
			bool bMatchRight1 = abs(ptResult1.x - ptResult2.x) < itCharactPoint->rc.rt.width * 0.5 ? (abs(ptResult1.y - ptResult2.y) < itCharactPoint->rc.rt.height * 0.5 ? true : false) : false;

// 			cv::Mat SrcTmp = SrcCharactArea.clone();
// 			rectangle(SrcTmp, ptResult1, cv::Point(ptResult1.x + CharactPointTempl.cols, ptResult1.y + CharactPointTempl.rows), CV_RGB(255, 0, 0), 2, 8, 0);
// 			rectangle(SrcTmp, ptResult2, cv::Point(ptResult2.x + CharactPointTempl.cols, ptResult2.y + CharactPointTempl.rows), CV_RGB(0, 0, 255), 2, 8, 0);
			
			++nModelWordPoint;

			bool bMatchRight2 = false;
			bool bMatchRight3 = false;
			cv::Point ptResult3;
			if (!bMatchRight1)
			{
				MatchingMethod(5, SrcCharactArea, CharactPointTempl, ptResult3);
				//rectangle(SrcTmp, ptResult3, cv::Point(ptResult3.x + CharactPointTempl.cols, ptResult3.y + CharactPointTempl.rows), CV_RGB(0, 255, 0), 2, 8, 0);

				//3中匹配中，2种以上位置差不多就认为匹配成功
				bMatchRight2 = abs(ptResult1.x - ptResult3.x) < itCharactPoint->rc.rt.width * 0.5 ? (abs(ptResult1.y - ptResult3.y) < itCharactPoint->rc.rt.height * 0.5 ? true : false) : false;
				bMatchRight3 = abs(ptResult3.x - ptResult2.x) < itCharactPoint->rc.rt.width * 0.5 ? (abs(ptResult3.y - ptResult2.y) < itCharactPoint->rc.rt.height * 0.5 ? true : false) : false;
				if (!bMatchRight1 && !bMatchRight2 && !bMatchRight3)
					continue;
			}

			cv::Point ptResultTmp;
			if (bMatchRight1) ptResultTmp = ptResult1;
			if (bMatchRight2) ptResultTmp = ptResult1;
			if (bMatchRight3) ptResultTmp = ptResult3;

			//计算密度、灰度、位置比例信息
			cv::Point ptReal(itCharactArea->rt.x + ptResultTmp.x, itCharactArea->rt.y + ptResultTmp.y);

			RECTINFO rcItem;
			rcItem = itCharactPoint->rc;
			rcItem.rt = cv::Rect(ptReal, cv::Point(ptReal.x + itCharactPoint->rc.rt.width, ptReal.y + itCharactPoint->rc.rt.height));
			if (ptReal.x + itCharactPoint->rc.rt.width > itCharactArea->rt.x + itCharactArea->rt.width)
				rcItem.rt.width = itCharactArea->rt.x + itCharactArea->rt.width - ptReal.x;
			if (ptReal.y + itCharactPoint->rc.rt.height > itCharactArea->rt.y + itCharactArea->rt.height)
				rcItem.rt.height = itCharactArea->rt.y + itCharactArea->rt.height - ptReal.y;

			if (RecogRtVal(rcItem, matRealPic))
			{
				if (rcItem.fRealDensity / itCharactPoint->rc.fStandardDensity > 0.9 && rcItem.fRealValue / itCharactPoint->rc.fStandardValue > 0.9 && \
					rcItem.rt.area() / itCharactPoint->rc.rt.area() > 0.9)
				{
					ST_WORDMATCHTEMPL stWordMatchTempl;
					stWordMatchTempl.rtModel = itCharactPoint->rc.rt;
					stWordMatchTempl.rcReal = rcItem;
					vecMatchTemplPoint.push_back(stWordMatchTempl);
				}
				else
				{
					TRACE("判断文字识别点的密度百分比: %f, 低于要求的: %f\n", rcItem.fRealValuePercent, itCharactPoint->rc.fStandardValuePercent);
				}
			}
		}
		if (nModelWordPoint >= nMaxCompWordPint)
			break;
	}

	if (nModelWordPoint <= 3)
	{
		if (vecMatchTemplPoint.size() >= nModelWordPoint)
		{
			bFind = true;
			nResult = nRotation;
		}
	}
	else
	{
		int nMinComp = (int)(nModelWordPoint * 0.7) > 3 ? (int)(nModelWordPoint * 0.7) : 3;
		if (vecMatchTemplPoint.size() >= nMinComp)
		{
			bFind = true;
			nResult = nRotation;
		}
	}
	end = clock();
	int nTime = end - start;
	std::string strTmpLog = Poco::format("总文字定点数=%d, 实际识别文字定点数=%d\n", nModelWordPoint, (int)vecMatchTemplPoint.size());
	strLog.append(strTmpLog);

	std::string strTmp = Poco::format("文字模板匹配判断旋转方向时间: %dms\n", (int)(end - start));
	strLog.append(strTmp);

	return bFind;
}

bool COmrRecog::RecogWordOrientationByRectCount(cv::Mat& matSrc, int n, int nRotation, int& nResult, std::string& strLog)
{
	bool bFind = false;
	clock_t start, end;
	start = clock();

	if (_pModel_->vecPaperModel[n]->lSNInfo.size() > 0 && _pModel_->nZkzhType == 1)
	{
		//查找zkzh区域的矩形数量，根据准考证号的Omr框大小过滤矩形	**************************
		SNLIST::iterator itSNGroup = _pModel_->vecPaperModel[n]->lSNInfo.begin();
		if ((*itSNGroup)->lSN.size() > 0)
		{
			RECTLIST::iterator itSN = (*itSNGroup)->lSN.begin();

			cv::Rect rtModelPic;
			rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
			rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
			cv::Rect rtReal = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rcSNTracker.rt, nRotation);

			try
			{
				if (rtReal.x < 0) rtReal.x = 0;
				if (rtReal.y < 0) rtReal.y = 0;
				if (rtReal.br().x > matSrc.cols)
					rtReal.width = matSrc.cols - rtReal.x;
				if (rtReal.br().y > matSrc.rows)
					rtReal.height = matSrc.rows - rtReal.y;

				cv::Mat matRealSnArea = matSrc(rtReal);
				RECTINFO rcModelSnTracker = _pModel_->vecPaperModel[n]->rcSNTracker;
				rcModelSnTracker.nDilateKernel = itSN->nDilateKernel;
				int nMinW = itSN->rt.width * 0.7;
				int nMaxW = itSN->rt.width * 1.3;
				int nMinH = itSN->rt.height * 0.7;
				int nMaxH = itSN->rt.height * 1.3;
				if (nRotation == 2 || nRotation == 3)
				{
					nMinW = itSN->rt.height * 0.7;
					nMaxW = itSN->rt.height * 1.3;
					nMinH = itSN->rt.width * 0.7;
					nMaxH = itSN->rt.width * 1.3;
				}
				int nRealSzRects = GetRectsInArea(matRealSnArea, rcModelSnTracker, nMinW, nMaxW, nMinH, nMaxH, CV_RETR_LIST) / 2;	//查找内外轮廓矩形，实际数量要除2
				int nModelSzRects = _pModel_->vecPaperModel[n]->lSNInfo.size() * (*itSNGroup)->lSN.size();
				if (nRealSzRects > nModelSzRects * 0.7)
				{
					bFind = true;
					nResult = nRotation;
				}

				std::string strTmpLog = Poco::format("总zkzh矩形数=%d, 实际识别zkzh矩形数=%d\n", nModelSzRects, nRealSzRects);
				strLog.append(strTmpLog);
			}
			catch (cv::Exception& exc)
			{
				char szLog[300] = { 0 };
				sprintf_s(szLog, "RecogWordOrientationByRectCount error1. detail: %s\n", exc.msg);
				strLog.append(szLog);
				//g_pLogger->information(szLog);
				TRACE(szLog);
			}
		}
	}

	if (!bFind)
	{
		try
		{
			int nModelAnchorAreaRects = 0;
			int nRealAnchorAreaRects = 0;
			for (auto itCharactArea : _pModel_->vecPaperModel[n]->lCharacterAnchorArea)
			{
				cv::Rect rtModelPic;
				rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
				rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
				cv::Rect rtReal = GetRectByOrientation(rtModelPic, itCharactArea->rt, nRotation);

				if (rtReal.x < 0) rtReal.x = 0;
				if (rtReal.y < 0) rtReal.y = 0;
				if (rtReal.br().x > matSrc.cols)
					rtReal.width = matSrc.cols - rtReal.x;
				if (rtReal.br().y > matSrc.rows)
					rtReal.height = matSrc.rows - rtReal.y;

				cv::Mat SrcCharactArea = matSrc(rtReal);

				RECTINFO rcCharactArea = itCharactArea->vecCharacterRt[0]->rc;
				int nMinW = itCharactArea->vecCharacterRt[0]->rc.rt.width * 0.7;
				int nMaxW = itCharactArea->vecCharacterRt[0]->rc.rt.width * 1.3;
				int nMinH = itCharactArea->vecCharacterRt[0]->rc.rt.height * 0.7;
				int nMaxH = itCharactArea->vecCharacterRt[0]->rc.rt.height * 1.3;
				if (nRotation == 2 || nRotation == 3)
				{
					nMinW = itCharactArea->vecCharacterRt[0]->rc.rt.height * 0.7;
					nMaxW = itCharactArea->vecCharacterRt[0]->rc.rt.height * 1.3;
					nMinH = itCharactArea->vecCharacterRt[0]->rc.rt.width * 0.7;
					nMaxH = itCharactArea->vecCharacterRt[0]->rc.rt.width * 1.3;
				}
				nRealAnchorAreaRects += GetRectsInArea(SrcCharactArea, rcCharactArea, nMinW, nMaxW, nMinH, nMaxH);

				nModelAnchorAreaRects = itCharactArea->nRects;
			}
			if (nModelAnchorAreaRects > 0 && _pModel_->vecPaperModel[n]->lCharacterAnchorArea.size() > 0)
			{
				int nMinCount = 0;
				if (nModelAnchorAreaRects < 3)
					nMinCount = 2;
				else
					nMinCount = nModelAnchorAreaRects * 0.8;
				if (nRealAnchorAreaRects > nMinCount)
				{
					bFind = true;
					nResult = nRotation;
				}
				std::string strTmpLog = Poco::format("总文字定点矩形数=%d, 实际识别文字定点矩形数=%d\n", nModelAnchorAreaRects, nRealAnchorAreaRects);
				strLog.append(strTmpLog);
			}
		}
		catch (cv::Exception& exc)
		{
			char szLog[300] = { 0 };
			sprintf_s(szLog, "RecogWordOrientationByRectCount error2. detail: %s\n", exc.msg);
			strLog.append(szLog);
			//g_pLogger->information(szLog);
			TRACE(szLog);
		}
	}
	end = clock();
	int nTime = end - start;

	std::string strTmp = Poco::format("矩形数量判断旋转方向时间: %dms\n", (int)(end - start));
	strLog.append(strTmp);

	TRACE("%s\n", strLog.c_str());

	return bFind;
}

bool COmrRecog::MatchingMethod(int method, cv::Mat& src, cv::Mat& templ, cv::Point& ptResult)
{
	double t1, t2;
	t1 = cvGetTickCount();

	/// Create the result matrix
	int result_cols = src.cols - templ.cols + 1;
	int result_rows = src.rows - templ.rows + 1;

	cv::Mat result;
	result.create(result_rows, result_cols, CV_32FC1);

	/// Do the Matching and Normalize
	cv::matchTemplate(src, templ, result, method);
	cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

	/// Localizing the best match with minMaxLoc
	double minVal; double maxVal; 
	cv::Point minLoc; 
	cv::Point maxLoc;
	cv::Point matchLoc;

	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

	/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
	if (method == CV_TM_SQDIFF || method == CV_TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
	}
	else
	{
		matchLoc = maxLoc;
	}

	t2 = cvGetTickCount();
	double dTime1 = (t2 - t1) / 1000.0 / cvGetTickFrequency();
//	TRACE("模板匹配时间: %fms\n", dTime1);

	ptResult = matchLoc;
	return true;
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

		if(matCompRoi.channels() == 3)
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

int COmrRecog::GetRectsInArea(cv::Mat& matSrc, RECTINFO rc, int nMinW, int nMaxW, int nMinH, int nMaxH, int nFindContoursModel)
{
	clock_t start, end;
	start = clock();

	cv::Mat imgResult = matSrc;

	if (imgResult.channels() == 3)
		cv::cvtColor(imgResult, imgResult, CV_BGR2GRAY);
	cv::GaussianBlur(imgResult, imgResult, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);
	sharpenImage1(imgResult, imgResult, rc.nSharpKernel);

	cv::threshold(imgResult, imgResult, rc.nThresholdValue, 255, cv::THRESH_OTSU | cv::THRESH_BINARY);

	cv::Canny(imgResult, imgResult, 0, rc.nCannyKernel, 5);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别
	cv::dilate(imgResult, imgResult, element);

	IplImage ipl_img(imgResult);

	//the parm. for cvFindContours  
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;

	//提取轮廓  
	cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), nFindContoursModel, CV_CHAIN_APPROX_SIMPLE);

	cv::Rect rtMax;		//记录最大矩形，识别同步头时用来排除非同步头框
	bool bResult = false;
	std::vector<cv::Rect>RectCompList;
	
	for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
	{
		CvRect aRect = cvBoundingRect(contour, 0);
		cv::Rect rm = aRect;

		if (rm.width < nMinW || rm.height < nMinH || rm.width > nMaxW || rm.height > nMaxH)
		{
			//TRACE("过滤矩形:(%d,%d,%d,%d), 面积: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
			continue;
		}
		
		RectCompList.push_back(rm);
	}
	cvReleaseMemStorage(&storage);

	end = clock();
//	TRACE("计算区域内矩形数--time = %dms, rect count = %d \n", end - start, RectCompList.size());

// 	std::sort(RectCompList.begin(), RectCompList.end(), [](cv::Rect& rt1, cv::Rect& rt2) {
// 		bool bResult = true;
// 
// 		if (abs(rt1.y - rt2.y) > 9)
// 		{
// 			return rt1.y < rt2.y ? true : false;
// 		}
// 		else
// 		{
// 			bResult = rt1.x < rt2.x ? true : false;
// 			if (!bResult)
// 				bResult = rt1.x == rt2.x ? rt1.y < rt2.y : false;
// 		}
// 		return bResult;
// 	});
// 	for (int i = 0; i < RectCompList.size(); i++)
// 	{
// 		rectangle(matSrc, RectCompList[i], CV_RGB(255, 0, 0), 2);
// 		TRACE("识别矩形框: (%d,%d,%d,%d)\n", RectCompList[i].x, RectCompList[i].y, RectCompList[i].width, RectCompList[i].height);
// 	}

	return RectCompList.size();
}

bool COmrRecog::IsFirstPic(int nPic, cv::Mat& matCompPic, pMODEL pModel)
{
	bool bResult = false;

	_strLog.clear();

	clock_t sTime, eTime;
	sTime = clock();

	int nFrontPage = floor(nPic / 2) * 2;		//对双面扫描，和图像的正面进行判断，对于多页的情况，都和当前纸张的正面比较
												//如第0、1页，使用模板的第0页信息来判断，第2、3页使用模板的第2页来判断
	char szTmp1[200] = { 0 };
	sprintf_s(szTmp1, "===========>\n检测第%d页正反面\n", nPic + 1);
	_strLog.append(szTmp1);

	cv::Rect rtModelPic;
	rtModelPic.width = pModel->vecPaperModel[nFrontPage]->nPicW;
	rtModelPic.height = pModel->vecPaperModel[nFrontPage]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matCompPic.cols;
	rtSrcPic.height = matCompPic.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matCompPic.cols / matCompPic.rows;

	int nRotateResult;	//方向
	if (pModel->nZkzhType == 2)			//使用条码的时候，先通过条码来判断
	{
		bResult = RecogCodeOrientation(matCompPic, nFrontPage, pModel, nRotateResult);

		if (!bResult)
		{
			_strLog.append("通过条形码或二维码判断试卷是否为第一面失败\n");

			//使用条码时，需要找一种方法判断属于条码，如直线
		}
	}

	if (!bResult)
	{
		//通过准考证号的矩形数量判断	
		bool bFind = false;
		if (nModelPicPersent == nSrcPicPercent)	//与模板图片方向一致，需判断正向还是反向一致
		{
			_strLog.append("与模板图片方向一致\n");

			for (int i = 1; i <= 4; i = i + 3)
			{
				bResult = RecogWordOrientationByRectCount(matCompPic, nFrontPage, i, nRotateResult, _strLog);
				if (bResult)
					break;
			}

			if (!bResult && pModel->vecPaperModel[nFrontPage]->lCharacterAnchorArea.size() > 0)	//通过文字定位点判断
			{
				for (int i = 1; i <= 4; i = i + 3)
				{
					bResult = RecogWordOrientationByMatchTempl(matCompPic, nFrontPage, i, nRotateResult, _strLog);
					if (bResult)
						break;
				}
			}

			if (!bResult)
				TRACE("无法判断图片正反面\n");
		}
		else
		{
			_strLog.append("与模板图片方向不一致\n");

			for (int i = 2; i <= 3; i++)
			{
				bResult = RecogWordOrientationByRectCount(matCompPic, nFrontPage, i, nRotateResult, _strLog);
				if (bResult)
					break;
			}
			if (!bResult && pModel->vecPaperModel[nFrontPage]->lCharacterAnchorArea.size() > 0)	//通过文字定位点判断
			{
				for (int i = 2; i <= 3; i++)
				{
					bResult = RecogWordOrientationByMatchTempl(matCompPic, nFrontPage, i, nRotateResult, _strLog);
					if (bResult)
						break;
				}
			}
			if (!bResult)
				TRACE("无法判断图片正反面，采用默认右旋90度的方向\n");
		}
	}

	eTime = clock();
	char szTmp[200] = { 0 };
	sprintf_s(szTmp, "第%d页的判断正反面结果: %s, 耗时: %d\n<===========\n", nPic + 1, bResult ? "正面" : "反面(失败，无法判断是正面)", (int)(eTime - sTime));
	_strLog.append(szTmp);
	TRACE(_strLog.c_str());
	//g_pLogger->information(strLog);

	return bResult;
}

std::string COmrRecog::GetRecogLog()
{
	return _strLog;
}

//------------------------------------------------------
CAdjustPaperPic::CAdjustPaperPic()
{
}

CAdjustPaperPic::~CAdjustPaperPic()
{
}

std::string CAdjustPaperPic::GetLog()
{
	return _strLog;
}

void CAdjustPaperPic::AdjustScanPaperToModel(pST_SCAN_PAPER pScanPaperTask)
{
	_strLog.clear();

	clock_t sTime, mT0, mT1, mT2, mT3, eTime;
	sTime = clock();
	std::stringstream ssLog;
	ssLog << "检测试卷(S" << pScanPaperTask->nPaperID << ")的图像正反面:\n";

	if (pScanPaperTask->vecScanPic.size() <= 1)
	{
		ssLog << "这张试卷只有" << pScanPaperTask->vecScanPic.size() << "页图片, 不需要判断正反，直接判断旋转方向\n";
		if (pScanPaperTask->vecScanPic.size())
		{
			pST_SCAN_PIC pScanPic = pScanPaperTask->vecScanPic[0];

			clock_t sT, eT1, eT2;
			sT = clock();
			COmrRecog chkRotationObj;
			int nResult1 = chkRotationObj.GetRightPicOrientation(pScanPic->mtPic, 0, pScanPaperTask->bDoubleScan);
			eT1 = clock();
			ssLog << "新图像" << pScanPic->strPicName << "方向调整: " << nResult1 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

// 			imwrite(pScanPic->strPicPath, pScanPic->mtPic);
// 			eT2 = clock();
// 			ssLog << "新图像" << pScanPic->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
		}
		else
			ssLog << "错误：该试卷没有发现有扫描的图片信息\n";
	}
	else
	{
		//COmrRecog chkRotationObj;
		bool bPreviousFirstPic = false;	//检测一张试卷时，上一页试卷是否为正面
		pST_SCAN_PIC pPreviousPic = NULL;
		for (int i = 0; i < pScanPaperTask->vecScanPic.size(); i++)
		{
			pST_SCAN_PIC pScanPic = pScanPaperTask->vecScanPic[i];

			//一张试卷的前一页是正面，或者已经是最后一页单独的试卷
			if (bPreviousFirstPic || (i == pScanPaperTask->vecScanPic.size() - 1 && floor(i / 2) * 2 == i))
			{
				if (bPreviousFirstPic)
				{
					bPreviousFirstPic = false;
					ssLog << "图像" << pScanPic->strPicName << "属于这张试卷的第2页(考生总试卷的第" << i + 1 << "面)，不需要检测正反面，上一页试卷是正面，开始旋转方向判断\n";
				}
				else if (i == pScanPaperTask->vecScanPic.size() - 1 && floor(i / 2) * 2 == i)
					ssLog << "这份试卷的最后一页" << pScanPic->strPicName << "(" << i + 1 << "/" << pScanPaperTask->vecScanPic.size() << "), 属于最后一张试卷只有单独一面的情况，不需要检测正反面. 开始旋转方向判断\n";

				clock_t sT, eT1, eT2;
				sT = clock();
				int nResult1 = GetRightPicOrientation(pScanPic->mtPic, i, pScanPaperTask->bDoubleScan);
				eT1 = clock();
				ssLog << "新图像" << pScanPic->strPicName << "方向调整: " << nResult1 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

// 				imwrite(pScanPic->strPicPath, pScanPic->mtPic);
// 				eT2 = clock();
// 				ssLog << "新图像" << pScanPic->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
				continue;
			}

			mT1 = clock();
			bool bResult = IsFirstPic(i, pScanPic->mtPic, _pModel_);
			ssLog << GetRecogLog();
			mT2 = clock();

			if (bResult)
			{
				int nFrontPage = floor(i / 2) * 2;
				if (i % 2 == 0)
				{
					//第1页为正面，第2页可以不需要判断
					pPreviousPic = pScanPic;
					bPreviousFirstPic = true;
					ssLog << "图像" << pScanPic->strPicName << "检测到属于这张试卷的正面, 下一页试卷不需要检测正反. 开始旋转方向判断. " << (int)(mT2 - mT1) << "ms\n";

					clock_t sT, eT1, eT2;
					sT = clock();
					int nResult1 = GetRightPicOrientation(pScanPic->mtPic, i, pScanPaperTask->bDoubleScan);
					eT1 = clock();
					ssLog << "新图像" << pScanPic->strPicName << "方向调整: " << nResult1 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

// 					imwrite(pScanPic->strPicPath, pScanPic->mtPic);
// 					eT2 = clock();
// 					ssLog << "新图像" << pScanPic->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
				}
				else
				{
					//第2页为正面
					ssLog << "图像" << pScanPic->strPicName << "检测到属于这张试卷的正面, 开始与上一页试卷" << pPreviousPic->strPicName << "调换. " << (int)(mT2 - mT1) << "ms\n";
					//图像重命名
					bool bPicsExchangeSucc = true;
					pST_SCAN_PIC pPic1 = pScanPic;
					pST_SCAN_PIC pPic2 = pPreviousPic;
				#if 1	//在这里，原图像数据还没有写到文件，直接交换图像存储的路径
					std::string strTmp = pPic1->strPicPath;
					pPic1->strPicPath = pPic2->strPicPath;
					pPic2->strPicPath = strTmp;
				#else
					try
					{
						Poco::File fPic1(pPic1->strPicPath);
						fPic1.renameTo(pPic1->strPicPath + "_tmp");

						Poco::File fPic2(pPic2->strPicPath);
						fPic2.renameTo(pPic1->strPicPath);

						fPic1.renameTo(pPic2->strPicPath);
					}
					catch (Poco::Exception &e)
					{
						bPicsExchangeSucc = false;
						std::string strErr = e.displayText();

						ssLog << "图像" << pScanPic->strPicName << "与" << pPreviousPic->strPicName << "调换重命名失败: " << strErr << "\n";
					}
				#endif
					mT3 = clock();

					if (bPicsExchangeSucc)
					{
						ssLog << "图像调换成功，开始重写原图. " << (int)(mT3 - mT2) << "ms\n";
						clock_t sT, eT1, eT2;
						sT = clock();

						bool bDoubleScan = pScanPaperTask->bDoubleScan;	//使用此功能必须是双面扫描。	 m_pModel->vecPaperModel.size() % 2 == 0 ? true : false;
						int nResult1 = GetRightPicOrientation(pScanPic->mtPic, nFrontPage, bDoubleScan);
						eT1 = clock();
						ssLog << "新图像" << pPic1->strPicName << "方向调整: " << nResult1 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

						std::string strPicPath1 = pPic2->strPicPath;
						//if (nResult1 >= 2 && nResult1 <= 4)
						//{
// 						imwrite(strPicPath1, pScanPic->mtPic);
// 						eT2 = clock();
// 						ssLog << "新图像" << pPic1->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
						//}

						sT = clock();
						int nResult2 = GetRightPicOrientation(pPreviousPic->mtPic, i, bDoubleScan);
						eT1 = clock();
						ssLog << "新图像" << pPic2->strPicName << "方向调整: " << nResult2 << "(1:不需要旋转，2：右转90, 3：左转90, 4：右转180). " << (int)(eT1 - sT) << "ms\n";

						std::string strPicPath2 = pPic1->strPicPath;
						//if (nResult2 >= 2 && nResult2 <= 4)
						//{
// 						imwrite(strPicPath2, pPreviousPic->mtPic);
// 						eT2 = clock();
// 						ssLog << "新图像" << pPic2->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
						//}

						bPreviousFirstPic = false;	//这张试卷的正反已经判断完了，重置

						//交换原始图像信息
						cv::Mat mtTmp = pPreviousPic->mtPic;
						pPreviousPic->mtPic = pScanPic->mtPic;
						pScanPic->mtPic = mtTmp;
						pScanPic->strPicPath = strPicPath1;
						pPreviousPic->strPicPath = strPicPath2;
					}
				}
			}
			else
			{
				pPreviousPic = pScanPic;
				bPreviousFirstPic = false;
				ssLog << "判断" << pScanPic->strPicName << "属于这张试卷的正面失败，不能确定为正面. " << (int)(mT2 - mT1) << "ms\n";
			}
		}
	}
	eTime = clock();
	ssLog << "判断试卷(S" << pScanPaperTask->nPaperID << ")正反面结束(" << (int)(eTime - sTime) << "ms)\n";
	_strLog.append(ssLog.str());
}

void CAdjustPaperPic::SaveScanPaperPic(pST_SCAN_PAPER pScanPaperTask)
{
	clock_t sT, eT1, eT2;
	sT = clock();
	std::stringstream ssLog; 
	ssLog << "开始保存试卷(S" << pScanPaperTask->nPaperID << ")到文件.\n";
	#pragma omp parallel for
	for (int i = 0; i < pScanPaperTask->vecScanPic.size(); i++)
	{
		eT1 = clock();
		pST_SCAN_PIC pScanPic = pScanPaperTask->vecScanPic[i];
		imwrite(pScanPic->strPicPath, pScanPic->mtPic);
		eT2 = clock();
		#pragma omp critical
		{
			ssLog << "\t新图像" << pScanPic->strPicName << "写文件完成. " << (int)(eT2 - eT1) << "ms\n";
		}
	}
	ssLog << "试卷(S" << pScanPaperTask->nPaperID << ")保存每页图片数据完成. " << (int)(eT2 - sT) << "ms\n";
	_strLog.append(ssLog.str());
}

bool CAdjustPaperPic::RecogPanination(pST_SCAN_PAPER pScanPaperTask)
{
	bool bResult = false;

	return bResult;
}

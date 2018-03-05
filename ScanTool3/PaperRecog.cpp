#include "stdafx.h"
#include "PaperRecog.h"

using namespace cv;
CPaperRecog::CPaperRecog()
{
}


CPaperRecog::~CPaperRecog()
{
}

bool CPaperRecog::Recog(RECTINFO& rc, cv::Mat& matCompPic, std::string& strLog)
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
		Mat matCompRoi2 = matCompRoi.clone();
		if (matCompRoi2.channels() == 3)
			cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);

		Mat imag_src, img_comp;
		cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);
		cv::GaussianBlur(matCompRoi2, matCompRoi2, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);	//_nGauseKernel_
		SharpenImage(matCompRoi2, matCompRoi2, rc.nSharpKernel);

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
		MatND src_hist, comp_hist;
		cv::calcHist(&matCompRoi2, 1, channels, Mat(), comp_hist, 1, histSize, ranges, false);

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

		MatND mean;
		MatND stddev;
		meanStdDev(matCompRoi2, mean, stddev);

		IplImage *src;
		src = &IplImage(mean);
		rc.fRealMeanGray = cvGetReal2D(src, 0, 0);

		IplImage *src2;
		src2 = &IplImage(stddev);
		rc.fRealStddev = cvGetReal2D(src2, 0, 0);
	}
	catch (cv::Exception &exc)
	{
		char szLog[300] = { 0 };
		sprintf_s(szLog, "\n\tRecog error. detail: %s\n", exc.msg);
		strLog.append(szLog);
		TRACE(szLog);
	}

	return bResult;
}

bool CPaperRecog::Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog)
{
	bool bResult_Recog = false;

	if (!pModel->nHasHead)	//同步头模式不需要判断是否矩形区框选到了其他区域
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

			//图片二值化
			// 局部自适应阈值的图像二值化
			int blockSize = 25;		//25
			int constValue = 10;
			cv::Mat local;
			//			cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
			cv::threshold(matCompRoi, matCompRoi, rc.nThresholdValue, 255, THRESH_BINARY);
			cv::Canny(matCompRoi, matCompRoi, 0, _nCannyKernel_, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));	//Size(6, 6)	普通空白框可识别
			dilate(matCompRoi, matCompRoi, element);

			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
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

#else		//这里先不进行选项框的裁剪(裁剪目的是放在选项框到题目或其他选项的矩形区)，起码这里是有问题的，选项有填涂的时候，如果矩形区全都框选到，会识别出多个矩形，这个需要解决
			if (i > 1)
				rc.rt = rt;
#endif
		}
		catch (cv::Exception &exc)
		{
			bResult_Recog = false;
		}
	}
	bResult_Recog = Recog(rc, matCompPic, strLog);
	return bResult_Recog;
}

bool CPaperRecog::RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, RECTLIST& lSelInfo, std::string& strResult)
{
	Mat matCompRoi;

	bool bResult = false;

	//omr框的大小高度
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
	Rect rt = cv::Rect(pt1, pt2);	//ABCD整个题目的选项区

	RECTLIST::iterator itFirst = itItem++;
#if 0
	int nSpace = 0;		//选项间的空格距离(可能是宽，可能是高)
	switch (itItem->nRecogFlag)	//获取选项间的空白区域宽度或高度
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
	//根据大小、面积先过滤一下可能框选到题号的情况

	try
	{
		cv::Point ptNew1, ptNew2;
		if (!pModel->nHasHead)	//无同步头时，进行坐标重定位
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

			Mat matCompRoi2 = matCompRoi.clone();
			if (matCompRoi2.channels() == 3)
				cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);

			//图片二值化
			threshold(matCompRoi2, matCompRoi2, _nThreshold_Recog2_, 255, THRESH_BINARY_INV);				//200, 255

			IplImage ipl_img(matCompRoi2);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			bool bFirst = true;
			std::vector<Rect> RectBaseList;
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				cv::Rect rm = aRect;

				if (rm.width < nOmrMinW || rm.height < nOmrMinH || rm.area() < nAreaMin)		//************	针对中括号的选项会存在问题	************
					continue;

				//需要去除可能包含的情况


				if (bFirst)
				{
					bFirst = false;
					ptNew1 = rm.tl() + pt1;
					ptNew2 = rm.br() + pt1;
				}
				if (itItem->nRecogFlag >= 37)	//选项为竖向的
				{
					if (rm.tl().y + pt1.y < ptNew1.y)	ptNew1 = rm.tl() + pt1;
					if (rm.br().y + pt1.y > ptNew2.y)	ptNew2 = rm.br() + pt1;
				}
				else  //选项为横向的
				{
					if (rm.tl().x + pt1.x < ptNew1.x)	ptNew1 = rm.tl() + pt1;
					if (rm.br().x + pt1.x > ptNew2.x)	ptNew2 = rm.br() + pt1;
				}
				RectBaseList.push_back(rm);
			}
			cvReleaseMemStorage(&storage);

			if (RectBaseList.size() <= lSelInfo.size())	//识别出的矩形区个数和选项一致，可能有删除的情况，还是去新的起点与原起点的中间值
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
		//根据新的坐标点计算新选项区矩形的填涂情况
		//		matCompRoi.deallocate();
		matCompRoi = matCompPic(cv::Rect(ptNew1, ptNew2));

		Mat matCompRoi2 = matCompRoi.clone();
		if (matCompRoi2.channels() == 3)
			cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);
		cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);

		// 		GaussianBlur(matCompRoi, matCompRoi, cv::Size(_nGauseKernel_, _nGauseKernel_), 0, 0);
		// 		sharpenImage1(matCompRoi, matCompRoi);

		//++先获取均值和标准差，再计算新的二值化阀值	2017.4.27
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
			float fMean = (float)nSum / nCount;		//均值

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

		//图片二值化
		threshold(matCompRoi2, matCompRoi2, nThreshold, 255, THRESH_BINARY_INV);				//200, 255

		//这里进行开闭运算
		//确定腐蚀和膨胀核的大小
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
																		//腐蚀操作
		erode(matCompRoi2, matCompRoi2, element);

		IplImage ipl_img2(matCompRoi2);

		//the parm. for cvFindContours  
		CvMemStorage* storage2 = cvCreateMemStorage(0);
		CvSeq* contour2 = 0;

		//提取轮廓  
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
			int nMinArea = nMaxArea * 0.3;				//在所有二值化识别出来的矩形中再过滤一遍，根据最大识别矩形的面积
			std::vector<Rect>::iterator itRect = RectCompList.begin();
			for (; itRect != RectCompList.end();)
			{
				if (itRect->area() < nMinArea)
					itRect = RectCompList.erase(itRect);
				else
					itRect++;
			}
		}

		//接下来根据位置信息判断abcd
		float fThreod;
		if (pModel->nHasHead)
			fThreod = _dCompThread_Head_;
		else
			fThreod = _dCompThread_Fix_;

		std::string strRecogAnswer;
		if (RectCompList.size())
		{
			if (itItem->eCPType == OMR && itItem->nRecogFlag >= 37)	//选项竖直排列
			{
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().y > itItem->rt.tl().y && itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
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

				//下面逻辑和上面有重复，先测试上面的，没问题就可以删除下面
				// 				if (strTmpVal.length())
				// 				{
				// 					RECTLIST::iterator itItem = lSelInfo.begin();
				// 					for (; itItem != lSelInfo.end(); itItem++)
				// 					{
				// 						for (int i = 0; i < RectCompList.size(); i++)
				// 						{
				// 							if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().x > itItem->rt.tl().x)
				// 							{
				// 								if (itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
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

				//以下可用
				// 				RECTLIST::iterator itItem = lSelInfo.begin();
				// 				for (; itItem != lSelInfo.end(); itItem++)
				// 				{
				// 					for (int i = 0; i < RectCompList.size(); i++)
				// 					{
				// 						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().x > itItem->rt.tl().x)
				// 						{
				// 							float fThreod = 1.0;
				// 							if (RectCompList.size() == 1)		//当只识别出只有一个选项时，降低灰度值标准
				// 								fThreod = 0.95;
				// 
				// 							if (itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
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
			else if (itItem->eCPType == OMR && itItem->nRecogFlag < 37)	//选项横向排列
			{
#if 1
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x && itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
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
				// 								if (itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
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
							if (!bFind)								//当单个选项识别出多个小识别块时
							{
								bFind = true;
								if (RectCompList.size() == 1)		//当只识别出只有一个选项时，降低灰度值标准
									fThreod = 0.95;
							}
							else
							{
								fThreod = 0.95;
							}

							if (itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
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
			else if (itItem->eCPType == SN && itItem->nRecogFlag >= 9)	//竖直排列
			{
				std::string strTmpVal;
				RECTLIST::iterator itItem = lSelInfo.begin();
				for (; itItem != lSelInfo.end(); itItem++)
				{
					bool bFind = false;
					for (int i = 0; i < RectCompList.size(); i++)
					{
						if (RectCompList[i].tl().y < itItem->rt.br().y && RectCompList[i].br().y > itItem->rt.tl().y && itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
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
						if (RectCompList[i].tl().x < itItem->rt.br().x && RectCompList[i].br().x > itItem->rt.tl().x && itItem->fRealValuePercent > fThreod)	//灰度值必须大于1.0才认为有填涂
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
		//g_pLogger->information(szLog);
		strResult.append(szLog);
		TRACE(szLog);
	}

	return bResult;
}

bool CPaperRecog::RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog)
{
	clock_t start, end;
	start = clock();
	strLog = Poco::format("图片%s\n", pPic->strPicName);

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
					strTmpLog = "识别准考证号完成(" + strResult + "), 图片名: " + pPic->strPicName;
					(static_cast<CDialog*>((static_cast<pST_PaperInfo>(pPic->pPaper))->pSrcDlg))->PostMessage(MSG_ZKZH_RECOG, (WPARAM)pPic->pPaper, (LPARAM)(static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers);
				}
				else
				{
					strTmpLog = "识别准考证号失败, 图片名:" + pPic->strPicName;
					//					bResult = false;
				}
				(static_cast<pST_PaperInfo>(pPic->pPaper))->strSN = strResult;
				strLog.append(strTmpLog);
			}
			catch (cv::Exception& exc)
			{
				std::string strTmpLog = "识别二维码或条码失败(" + pPic->strPicName + "): " + exc.msg;
				strLog.append(strTmpLog);
				break;
			}
			catch (...)
			{
				std::string strTmpLog = "识别二维码或条码失败(" + pPic->strPicName + "), 未知异常.\n";
				strLog.append(strTmpLog);
				break;
			}
		}
	}
// 	if ((static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.empty())
// 	{
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.lock();
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->nSnNull++;
// 		(static_cast<pPAPERSINFO>((static_cast<pST_PaperInfo>(pPic->pPaper))->pPapers))->fmSnStatistics.unlock();
// 	}
	end = clock();
	std::string strTime = Poco::format("识别考号时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	//g_pLogger->information(strLog);
	return bResult;
}

bool CPaperRecog::RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog)
{
	bool bRecogAll = true;
	bool bResult = true;
	std::vector<int> vecSN;

	clock_t start, end;
	start = clock();
	strLog = Poco::format("图片%s\n", pPic->strPicName);

	if (pModel->vecPaperModel[nPic]->lSNInfo.size() == 0)
	{
		strLog = Poco::format("图片%s没有考号需要识别", pPic->strPicName);
		g_pLogger->information(strLog);
		return true;
	}

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

			if (pModel->nHasHead)
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
				GetRecogPosition(nPic, pPic, pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
#if 1
			bool bResult_Recog = Recog2(nPic, rc, matCompPic, pPic, pModel, strLog);
			if (bResult_Recog)
			{
				if (rc.fRealValuePercent > rc.fStandardValuePercent)
					vecItemVal.push_back(rc.nSnVal);
			}
			char szTmp[300] = { 0 };
			sprintf_s(szTmp, "图片名: %s, SN: 第%d位, 选项=%d, 识别实际比例=%.3f, val=%.2f, 识别标准=%.3f, val=%.2f, 是否成功:%d\n", pPic->strPicName.c_str(), \
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
#ifdef PaintOmrSnRect	//打印OMR、SN位置
			pPic->lNormalRect.push_back(rc);
#endif
		}
		if (!bResult)
			break;

#if 1	//根据选项差值判断选中
		std::vector<pRECTINFO> vecItemsDesc;
		std::vector<ST_ITEM_DIFF> vecSnItemDiff;
		calcSnDensityDiffVal(pSn, vecItemsDesc, vecSnItemDiff);

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
#if 1
		vecItemVal.clear();
		if (vecItemsDesc[0]->fRealValuePercent > fCompThread)
		{
			vecItemVal.push_back(vecItemsDesc[0]->nSnVal);
		}
#else
		for (int i = 0; i < vecSnItemDiff.size(); i++)
		{
			//根据所有选项灰度值排序，相邻灰度值差值超过阀值，同时其中第一个最大的灰度值超过1.0，就认为这个区间为选中的阀值区间
			//(大于1.0是防止最小的灰度值很小的时候影响阀值判断)
			float fDiff = (fCompThread - vecSnItemDiff[i].fFirst) * 0.1;
			if ((vecSnItemDiff[i].fDiff >= fDiffThread && vecSnItemDiff[i].fFirst > fCompThread) ||
				(vecSnItemDiff[i].fDiff >= fDiffThread + fDiff && vecSnItemDiff[i].fFirst > (fCompThread - 0.1) && fDiff > 0))
			{
				nFlag = i;
				fThreld = vecSnItemDiff[i].fFirst;
				if (vecSnItemDiff[i].fDiff > fDiffExit)	//灰度值变化较大，直接退出
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
			sprintf_s(szTmpLog, "第%d位SN[", pSn->nItem);
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
			for (auto itSn : pSn->lSN)
				if (itSn.nSnVal == pSn->nRecogVal)
				{
					pSn->rt = itSn.rt;
					break;
				}
		}
		else
		{
#if 1	//第二种ZKZH识别方法 test
			std::vector<int> vecItemVal2;
			RecogVal_Sn2(nPic, matCompPic, pPic, pModelInfo, pSn, vecItemVal2);
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
				RecogVal_Sn3(nPic, matCompPic, pPic, pModelInfo, pSn, vecItemVal3);
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
					sprintf_s(szLog, "识别准考证号第%d位失败,识别出结果%d位(%s), 图片名: %s\n", pSnItem->nItem, vecItemVal.size(), szVal, pPic->strPicName.c_str());
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
		sprintf_s(szLog, "识别准考证号完成(%s), 图片名: %s\n", (static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.c_str(), pPic->strPicName.c_str());
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
				szTmp[0] = '#';		//未识别出来的或者识别到多个的用#代替，后面做模糊查找
			if (!bAllEmpty)			//只要识别到一部分，就将此识别到的结果放入模糊搜索字段中
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
		sprintf_s(szLog, "识别准考证号失败, 图片名: %s\n", pPic->strPicName.c_str());
		strLog.append(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别考号时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	g_pLogger->information(strLog);
	return bResult;
}

bool CPaperRecog::RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, pSN_ITEM pSn, std::vector<int>& vecItemVal)
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

bool CPaperRecog::RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;
	// 	if (pModelInfo->pModel->nHasHead != 0)	//有同步头的，不需要进行定点识别
	// 		return bResult;

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
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别	Size(_nDilateKernel_, _nDilateKernel_)
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

bool CPaperRecog::RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;
	if (pModel->nHasHead == 0)
		return true;

	std::string strErrDesc;
	m_vecH_Head.clear();
	RECTLIST::iterator itRoi = pModel->vecPaperModel[nPic]->lSelHTracker.begin();
	for (; itRoi != pModel->vecPaperModel[nPic]->lSelHTracker.end(); itRoi++)
	{
		RECTINFO rc = *itRoi;

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
			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			//模板图像的水平同步头平均长宽
			RECTLIST::iterator itBegin = pModel->vecPaperModel[nPic]->lH_Head.begin();
			RECTINFO rcFist = *itBegin;
			RECTINFO rcSecond = *(++itBegin);

			int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
			int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

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

				// 				float fOffset = 0.1;
				// 				float fPer_W, fPer_H;	//模板第二个点与第一个点的宽、高的比例，用于最小值控制
				// 				fPer_W = 0.5;
				// 				fPer_H = 0.25;
				// 				int nMid_modelW = rcSecond.rt.width + 2;		//加2是因为制卷模板框框没有经过查边框运算，经过查边框后，外框会包含整个矩形，需要加上上下各1个单位的线宽
				// 				int nMid_modelH = rcSecond.rt.height + 2;
				// 				if (nMid_modelW < rcFist.rt.width * fPer_W + 0.5)	nMid_modelW = rcFist.rt.width * fPer_W + 0.5;
				// 				if (nMid_modelH < rcFist.rt.height * fPer_H + 0.5)	nMid_modelH = rcFist.rt.height * fPer_H + 0.5;
				// 				nMid_minW = nMid_modelW * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				// 				nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				// 				nMid_minH = nMid_modelH * (1 - fOffset);				//同上
				// 				nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//同上
				// 
				// 				nHead_minW = rcFist.rt.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
				// 				nHead_maxW = rcFist.rt.width * (1 + fOffset * 4) + 0.5;		//同上
				// 				nHead_minH = rcFist.rt.height * (1 - fOffset);				//同上
				// 				nHead_maxH = rcFist.rt.height * (1 + fOffset * 4) + 0.5;	//同上
			}
			else
			{
				float fOffset_mid = 0.4;
				nMid_minW = rcSecond.rt.width * (1 - fOffset_mid);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nMid_maxW = rcSecond.rt.width * (1 + fOffset_mid);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nMid_minH = rcSecond.rt.height * (1 - fOffset_mid);		//同上
				nMid_maxH = rcSecond.rt.height * (1 + fOffset_mid);		//同上

				float fOffset_Head = 0.3;
				nHead_minW = rcFist.rt.width * (1 - fOffset_Head);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nHead_maxW = rcFist.rt.width * (1 + fOffset_Head);		//同上
				nHead_minH = rcFist.rt.height * (1 - fOffset_Head);		//同上
				nHead_maxH = rcFist.rt.height * (1 + fOffset_Head);		//同上
			}

			int nYSum = 0;
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm.x = rm.x + rc.rt.x;
				rm.y = rm.y + rc.rt.y;

				if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
				{
					if (!(rm.width > nHead_minW && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//排除第一个或最后一个大的同步头
					{
						TRACE("过滤水平同步头(%d,%d,%d,%d), 要求范围W:[%d,%d], H[%d,%d], 参考大小(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
						continue;
					}
					else
					{
						TRACE("首尾水平同步头(即定位点同步头)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
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
			std::string strTmpLog = "识别水平同步头异常: " + exc.msg;
			//g_pLogger->information(strLog);
			strLog.append(strTmpLog);
			TRACE(strTmpLog.c_str());

			bResult = false;
			break;
		}
		if (RectCompList.size() == 0)
		{
			bResult = false;
			strErrDesc = "水平同步头数量为0.";
		}
		else
		{
			//二次过滤
			std::sort(RectCompList.begin(), RectCompList.end(), SortByPositionX2);
			std::vector<Rect>::iterator itHead = RectCompList.begin();
			for (int i = 0; itHead != RectCompList.end(); i++)
			{
				RECTINFO rcHead1;
				RECTLIST::iterator itModelHead = pModel->vecPaperModel[nPic]->lH_Head.begin();
				for (int j = 0; itModelHead != pModel->vecPaperModel[nPic]->lH_Head.end(); j++, itModelHead++)
				{
					if (j == i)
					{
						rcHead1 = *itModelHead;
						break;
					}
				}
				RECTINFO rcTmp = rcHead1;
				rcTmp.rt = RectCompList[i];
				Recog(rcTmp, matCompPic, strLog);
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
			std::sort(m_vecH_Head.begin(), m_vecH_Head.end(), [](RECTINFO& rc1, RECTINFO& rc2)
			{
				bool bResult = true;
				bResult = rc1.rt.x < rc2.rt.x ? true : false;
				if (!bResult)
				{
					if (rc1.rt.x == rc2.rt.x)
						bResult = rc1.rt.y < rc2.rt.y ? true : false;
				}
				return bResult;
			});
		}
		if (m_vecH_Head.size() != pModel->vecPaperModel[nPic]->lH_Head.size())
		{
			bResult = false;
			pPic->bFindIssue = true;
			for (int i = 0; i < m_vecH_Head.size(); i++)
				pPic->lIssueRect.push_back(m_vecH_Head[i]);

			strErrDesc = Poco::format("水平同步头数量为%u, 与模板水平同步头数量(%u)不一致", m_vecH_Head.size(), pModel->vecPaperModel[nPic]->lH_Head.size());
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别水平同步头失败, 原因: %s, 图片名: %s\n", strErrDesc.c_str(), pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CPaperRecog::RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;
	if (pModel->nHasHead == 0)
		return true;

	m_vecV_Head.clear();
	std::string strErrDesc;
	RECTLIST::iterator itRoi = pModel->vecPaperModel[nPic]->lSelVTracker.begin();
	for (; itRoi != pModel->vecPaperModel[nPic]->lSelVTracker.end(); itRoi++)
	{
		RECTINFO rc = *itRoi;

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
			if (nThreshold > 150) nThreshold = 150;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//去除干扰信息，先膨胀后腐蚀还原, 可去除一些线条干扰
			Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			Mat element = getStructuringElement(MORPH_RECT, Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element);
			IplImage ipl_img(matCompRoi);

			//the parm. for cvFindContours  
			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;

			//提取轮廓  
			cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			//模板图像的水平同步头平均长宽
			RECTLIST::iterator itBegin = pModel->vecPaperModel[nPic]->lV_Head.begin();
			RECTINFO rcFist = *itBegin;
			RECTINFO rcSecond = *(++itBegin);

			int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
			int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

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
				// 				float fOffset = 0.1;
				// 				float fPer_W, fPer_H;	//模板第二个点与第一个点的宽、高的比例，用于最小值控制
				// 				fPer_W = 0.5;
				// 				fPer_H = 0.25;
				// 				int nMid_modelW = rcSecond.rt.width + 2;		//加2是因为制卷模板框框没有经过查边框运算，经过查边框后，外框会包含整个矩形，需要加上上下各1个单位的线宽
				// 				int nMid_modelH = rcSecond.rt.height + 2;
				// 				if (nMid_modelW < rcFist.rt.width * fPer_W + 0.5)	nMid_modelW = rcFist.rt.width * fPer_W + 0.5;
				// 				if (nMid_modelH < rcFist.rt.height * fPer_H + 0.5)	nMid_modelH = rcFist.rt.height * fPer_H + 0.5;
				// 				nMid_minW = nMid_modelW * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				// 				nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				// 				nMid_minH = nMid_modelH * (1 - fOffset);				//同上
				// 				nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//同上
				// 
				// 				nHead_minW = rcFist.rt.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
				// 				nHead_maxW = rcFist.rt.width * (1 + fOffset * 4) + 0.5;		//同上
				// 				nHead_minH = rcFist.rt.height * (1 - fOffset);				//同上
				// 				nHead_maxH = rcFist.rt.height * (1 + fOffset * 4) + 0.5;	//同上
			}
			else
			{
				float fOffset_mid = 0.4;
				nMid_minW = rcSecond.rt.width * (1 - fOffset_mid);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nMid_maxW = rcSecond.rt.width * (1 + fOffset_mid);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nMid_minH = rcSecond.rt.height * (1 - fOffset_mid);		//同上
				nMid_maxH = rcSecond.rt.height * (1 + fOffset_mid);		//同上

				float fOffset_Head = 0.3;
				nHead_minW = rcFist.rt.width * (1 - fOffset_Head);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
				nHead_maxW = rcFist.rt.width * (1 + fOffset_Head);		//同上
				nHead_minH = rcFist.rt.height * (1 - fOffset_Head);		//同上
				nHead_maxH = rcFist.rt.height * (1 + fOffset_Head);		//同上
			}

			int nXMidSum = 0;	//x轴中线坐标总和
			for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
			{
				CvRect aRect = cvBoundingRect(contour, 0);
				Rect rm = aRect;
				rm.x = rm.x + rc.rt.x;
				rm.y = rm.y + rc.rt.y;

				if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
				{
					if (!(rm.width > nHead_minW && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//排除第一个或最后一个大的同步头
					{
						TRACE("过滤垂直同步头(%d,%d,%d,%d), 要求范围W:[%d,%d], H[%d,%d], 参考大小(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
						continue;
					}
					else
					{
						TRACE("首尾垂直同步头(即定位点同步头)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
					}
				}
				RectCompList.push_back(rm);
				nXMidSum += (rm.x + rm.width / 2);
			}
			cvReleaseMemStorage(&storage);
			int nXMean = nXMidSum / RectCompList.size();

			//根据坐标位置过滤
			// 			int nDeviation = 10;
			// 			int xMin = nXMean - (rcFist.rt.width / 2 + nDeviation);
			// 			int xMax = nXMean + (rcFist.rt.width / 2 + nDeviation);
			// 			TRACE("垂直同步头中线X = %d, 偏差[%d,%d]\n", nXMean, xMin, xMax);
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
		}
		catch (cv::Exception& exc)
		{
			std::string strTmpLog = "识别垂直同步头异常: " + exc.msg;
			//g_pLogger->information(strLog);
			strLog.append(strTmpLog);
			TRACE(strTmpLog.c_str());

			bResult = false;
			break;
		}
		if (RectCompList.size() == 0)
		{
			bResult = false;
			strErrDesc = "垂直同步头数量为0.";
		}
		else
		{
			//二次过滤
			std::sort(RectCompList.begin(), RectCompList.end(), [](cv::Rect& rt1, cv::Rect& rt2)
			{
				bool bResult = true;
				bResult = rt1.y < rt2.y ? true : false;
				if (!bResult)
				{
					if (rt1.y == rt2.y)
						bResult = rt1.x < rt2.x ? true : false;
				}
				return bResult;
			});
			std::vector<Rect>::iterator itHead = RectCompList.begin();
			for (int i = 0; itHead != RectCompList.end(); i++)
			{
				RECTINFO rcHead1;
				RECTLIST::iterator itModelHead = pModel->vecPaperModel[nPic]->lV_Head.begin();
				for (int j = 0; itModelHead != pModel->vecPaperModel[nPic]->lV_Head.end(); j++, itModelHead++)
				{
					if (j == i)
					{
						rcHead1 = *itModelHead;
						break;
					}
				}
				RECTINFO rcTmp = rcHead1;
				rcTmp.rt = RectCompList[i];
				Recog(rcTmp, matCompPic, strLog);
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
		if (m_vecV_Head.size() != pModel->vecPaperModel[nPic]->lV_Head.size())
		{
			bResult = false;
			pPic->bFindIssue = true;
			for (int i = 0; i < m_vecV_Head.size(); i++)
				pPic->lIssueRect.push_back(m_vecV_Head[i]);

			strErrDesc = Poco::format("垂直同步头数量为%u, 与模板垂直同步头数量(%u)不一致", m_vecV_Head.size(), pModel->vecPaperModel[nPic]->lV_Head.size());
		}
	}
	if (!bResult)
	{
		char szLog[MAX_PATH] = { 0 };
		sprintf_s(szLog, "识别垂直同步头失败, 原因: %s, 图片名: %s\n", strErrDesc.c_str(), pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CPaperRecog::RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("识别AB卷型\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lABModel.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lABModel.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModel->nHasHead)
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
			GetRecogPosition(nPic, pPic, pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
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

bool CPaperRecog::RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
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

		if (pModel->nHasHead)
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
			GetRecogPosition(nPic, pPic, pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
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
		//		g_pLogger->information(szLog);
		TRACE(szLog);
	}
	end = clock();
	std::string strTime = Poco::format("识别科目校验点时间: %dms\n", (int)(end - start));
	strLog.append(strTime);
	return bResult;
}

bool CPaperRecog::RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("识别缺考\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lQK_CP.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lQK_CP.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModel->nHasHead)
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
			GetRecogPosition(nPic, pPic, pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(rc, matCompPic, strLog);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				((pST_PaperInfo)pPic->pPaper)->nQKFlag = 1;			//设置学生缺考
				pPic->nQKFlag = 1;
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
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
		sprintf_s(szLog, "识别缺考失败, 图片名: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CPaperRecog::RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	TRACE("识别违纪\n");
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lWJ_CP.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lWJ_CP.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModel->nHasHead)
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
			GetRecogPosition(nPic, pPic, pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
		bool bResult_Recog = Recog(rc, matCompPic, strLog);
		if (bResult_Recog)
		{
			if (rc.fRealValuePercent >= rc.fStandardValuePercent)
			{
				((pST_PaperInfo)pPic->pPaper)->nWJFlag = 1;			//设置学生违纪
				pPic->nWJFlag = 1;
			}
			pPic->lNormalRect.push_back(rc);
			bResult = true;
			continue;
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
		sprintf_s(szLog, "识别违纪失败, 图片名: %s\n", pPic->strPicName.c_str());
		//g_pLogger->information(szLog);
		strLog.append(szLog);
		TRACE(szLog);
	}
	return bResult;
}

bool CPaperRecog::RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
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

		if (pModel->nHasHead)
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
			GetRecogPosition(nPic, pPic, pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
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

bool CPaperRecog::RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;
	RECTLIST::iterator itCP = pModel->vecPaperModel[nPic]->lWhite.begin();
	for (; itCP != pModel->vecPaperModel[nPic]->lWhite.end(); itCP++)
	{
		RECTINFO rc = *itCP;

		if (pModel->nHasHead)
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
			GetRecogPosition(nPic, pPic, pModel, rc.rt);		//GetPosition(pPic->lFix, pModelInfo->pModel->vecPaperModel[nPic]->lFix, rc.rt);
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

bool CPaperRecog::RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog)
{
	bool bResult = true;
	if (pModel->nZkzhType == 2)
		bResult = RecogSn_code(nPic, matCompPic, pPic, pModel);
	else
		bResult = RecogSn_omr(nPic, matCompPic, pPic, pModel);
	return bResult;
}

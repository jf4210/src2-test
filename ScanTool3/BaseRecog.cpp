#include "stdafx.h"
#include "BaseRecog.h"

using namespace cv;
CBaseRecog::CBaseRecog()
{
}


CBaseRecog::~CBaseRecog()
{
}

bool CBaseRecog::Recog(RECTINFO& rc, cv::Mat& matCompPic, std::string& strLog)
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
		Mat matCompRoi2 = matCompRoi.clone();
		if (matCompRoi2.channels() == 3)
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
		cv::MatND src_hist, comp_hist;
		cv::calcHist(&matCompRoi2, 1, channels, Mat(), comp_hist, 1, histSize, ranges, false);

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

bool CBaseRecog::Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog)
{
	bool bResult_Recog = false;

	if (!pModel->nHasHead)	//ͬ��ͷģʽ����Ҫ�ж��Ƿ��������ѡ������������
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

			Mat matCompRoi2 = matCompRoi.clone();
			if (matCompRoi2.channels() == 3)
				cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);

			//ͼƬ��ֵ��
			// �ֲ�����Ӧ��ֵ��ͼ���ֵ��
			int blockSize = 25;		//25
			int constValue = 10;
			cv::Mat local;
			//			cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
			cv::threshold(matCompRoi2, matCompRoi2, rc.nThresholdValue, 255, THRESH_BINARY);
			cv::Canny(matCompRoi2, matCompRoi2, 0, _nCannyKernel_, 5);
			Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));	//Size(6, 6)	��ͨ�հ׿��ʶ��
			dilate(matCompRoi2, matCompRoi2, element);

			IplImage ipl_img(matCompRoi2);

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
	bResult_Recog = Recog(rc, matCompPic, strLog);
	return bResult_Recog;
}

bool CBaseRecog::RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, RECTLIST& lSelInfo, std::string& strResult)
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
		if (!pModel->nHasHead)	//��ͬ��ͷʱ�����������ض�λ
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

			//ͼƬ��ֵ��
			threshold(matCompRoi2, matCompRoi2, _nThreshold_Recog2_, 255, THRESH_BINARY_INV);				//200, 255

			IplImage ipl_img(matCompRoi2);

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

		Mat matCompRoi2 = matCompRoi.clone();
		if (matCompRoi2.channels() == 3)
			cv::cvtColor(matCompRoi2, matCompRoi2, CV_BGR2GRAY);

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
		threshold(matCompRoi2, matCompRoi2, nThreshold, 255, THRESH_BINARY_INV);				//200, 255

		//������п�������
		//ȷ����ʴ�����ͺ˵Ĵ�С
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(4, 4)
																		//��ʴ����
		erode(matCompRoi2, matCompRoi2, element);

		IplImage ipl_img2(matCompRoi2);

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
		if (pModel->nHasHead)
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
			else if (itItem->eCPType == OMR && itItem->nRecogFlag < 37)	//ѡ���������
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
		//g_pLogger->information(szLog);
		strResult.append(szLog);
		TRACE(szLog);
	}

	return bResult;
}

void CBaseRecog::SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel)
{
	//��������ʼ���˲�ģ��
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = nSharpKernel;		//_nSharpKernel_
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//��ͼ������˲�
	cv::filter2D(image, result, image.depth(), kernel);
}

//----------------------------------------------------------------------
CAssistRecog::CAssistRecog()
{
}

CAssistRecog::~CAssistRecog()
{
}

bool CAssistRecog::GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, cv::Rect& rt)
{
#ifdef USE_TESSERACT
	//����ģ��ȽϵĶ���
	if (pModel->vecPaperModel[nPic]->lCharacterAnchorArea.size() > 0)
	{
		if (pPic->lFix.size() < 3)
		{
			clock_t start, end;
			start = clock();
			cv::Rect rtLT, rtRB;	//���ϣ�������������
			rtLT = rt;
			rtRB = rt;
			rtRB.x += rtRB.width;
			rtRB.y += rtRB.height;
			GetPosition(pPic->lFix, pPic->lModelWordFix, rtLT);
			GetPosition(pPic->lFix, pPic->lModelWordFix, rtRB);

			int nWidth = abs(rtRB.x - rtLT.x);
			int nHeight = abs(rtRB.y - rtLT.y);
			rt.x = rtLT.x + nWidth / 2 - rt.width / 2;
			rt.y = rtLT.y + nHeight / 2 - rt.height / 2;

			end = clock();
			TRACE("�������λ��ʱ��: %dms\n", (int)(end - start));
			return true;
		}
		else
		{
			clock_t start, end;
			start = clock();
			VEC_NEWRTBY2FIX vecNewRt;
#if 1		//���ݾ��붥����Զ�ĵ�������λ�ã�����Ĭ�Ϸ�ֹ���е�һ��
			RECTLIST::iterator itFix = pPic->lFix.begin();
			RECTLIST::iterator itModelFix = pPic->lModelWordFix.begin();
			if (itModelFix != pPic->lModelWordFix.end())
			{
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
					stNewRt.rt = rt;
					GetPosition(lTmpFix, lTmpModelFix, stNewRt.rt);
					vecNewRt.push_back(stNewRt);
				}
			}
#else
			VEC_FIXRECTINFO lFixRtInfo;
			RECTLIST::iterator itFix = pPic->lFix.begin();
			RECTLIST::iterator itModelFix = pPic->lModelWordFix.begin();
			for (; itFix != pPic->lFix.end(); itFix++, itModelFix++)
			{
				GetNewRt((*itFix), (*itModelFix), lFixRtInfo, vecNewRt, rt);
			}
#endif
			int nRidus = rt.width < rt.height ? rt.width * 0.5 : rt.height * 0.5;
			nRidus = nRidus > 3 ? 3 : nRidus;
			VEC_POINTDISTWEIGHT vecPointDistWeight;
			for (auto newRt : vecNewRt)
			{
				GetPointDistWeight(nRidus, newRt.rt.tl(), vecPointDistWeight);
			}
			VEC_POINTDISTWEIGHT::iterator itPoint = vecPointDistWeight.begin();
			for (; itPoint != vecPointDistWeight.end(); )
			{
				if (itPoint->nWeight < 1)
					itPoint = vecPointDistWeight.erase(itPoint);
				else
					itPoint++;
			}

			int nXCount = 0, nYCount = 0;
			int nCount = 0;
			if (vecPointDistWeight.size() > 0)
			{
				for (auto newPt : vecPointDistWeight)
				{
					nXCount += newPt.pt.x;
					nYCount += newPt.pt.y;
				}
				nCount = vecPointDistWeight.size();
			}
			else
			{
				for (auto newRt : vecNewRt)
				{
					nXCount += newRt.rt.x;
					nYCount += newRt.rt.y;
				}
				nCount = vecNewRt.size();
			}
			if (nCount > 0)
			{
				rt.x = nXCount / nCount;
				rt.y = nYCount / nCount;
			}
			end = clock();
			//			TRACE("�������λ��ʱ��: %dms\n", (int)(end - start));
			return true;
		}
	}
	else
		return GetPosition(pPic->lFix, pModel->vecPaperModel[nPic]->lFix, rt);
#endif
	return GetPosition(pPic->lFix, pModel->vecPaperModel[nPic]->lFix, rt);
}

bool CAssistRecog::GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, RECTINFO& rc)
{
	bool bResult = false;	//��֧��ͬ��ͷģʽ������
	return bResult;

// 	if (rc.nHItem >= m_vecH_Head.size() || rc.nVItem >= m_vecV_Head.size())
// 	{
// 		bResult = false;
// 		pPic->bFindIssue = true;
// 		pPic->lIssueRect.push_back(rc);
// 		break;
// 	}
// 	rc.rt.x = m_vecH_Head[rc.nHItem].rt.tl().x;
// 	rc.rt.y = m_vecV_Head[rc.nVItem].rt.tl().y;
// 	rc.rt.width = m_vecH_Head[rc.nHItem].rt.width;
// 	rc.rt.height = m_vecV_Head[rc.nVItem].rt.height;
}

bool CAssistRecog::GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW /*= 0*/, int nPicH /*= 0*/)
{
	if (lModelFix.size() == 1)
	{
		if (lFix.size() < 1)
			return false;
		RECTLIST::iterator it = lFix.begin();
		RECTLIST::iterator itModel = lModelFix.begin();
		RECTINFO rc = *it;
		RECTINFO rcModel = *itModel;
		cv::Point pt, ptModel;
		pt.x = rc.rt.x + rc.rt.width / 2 + 0.5;
		pt.y = rc.rt.y + rc.rt.height / 2 + 0.5;
		ptModel.x = rcModel.rt.x + rcModel.rt.width / 2 + 0.5;
		ptModel.y = rcModel.rt.y + rcModel.rt.height / 2 + 0.5;
		int x = pt.x - ptModel.x;
		int y = pt.y - ptModel.y;
		rt.x = rt.x + x;
		rt.y = rt.y + y;
	}
	if (lModelFix.size() == 2)
	{
		if (lFix.size() < 2)
			return false;
		RECTLIST::iterator it = lFix.begin();
		RECTINFO rcA = *it++;
		RECTINFO rcB = *it;
		RECTLIST::iterator itModel = lModelFix.begin();
		RECTINFO rcModelA = *itModel++;
		RECTINFO rcModelB = *itModel;

		cv::Point ptA, ptB, ptA0, ptB0, ptC0;
		cv::Point2d ptC;

		if (nPicW != 0 && nPicH != 0)
		{
			int nCenterX = nPicW / 2 + 0.5;
			int nCenterY = nPicH / 2 + 0.5;
			if (rcModelA.rt.x < nCenterX)
			{
				if (rcModelA.rt.y < nCenterY)
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.8;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.8;
					ptA.x = rcA.rt.x + rcA.rt.width * 0.8;
					ptA.y = rcA.rt.y + rcA.rt.height * 0.8;
				}
				else
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.8;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.2;
					ptA.x = rcA.rt.x + rcA.rt.width * 0.8;
					ptA.y = rcA.rt.y + rcA.rt.height * 0.2;
				}
			}
			else
			{
				if (rcModelA.rt.y < nCenterY)
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.2;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.8;
					ptA.x = rcA.rt.x + rcA.rt.width * 0.2;
					ptA.y = rcA.rt.y + rcA.rt.height * 0.8;
				}
				else
				{
					ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.2;
					ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.2;
					ptA.x = rcA.rt.x + rcA.rt.width * 0.2;
					ptA.y = rcA.rt.y + rcA.rt.height * 0.2;
				}
			}

			if (rcModelB.rt.x < nCenterX)
			{
				if (rcModelB.rt.y < nCenterY)
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.8;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.8;
					ptB.x = rcB.rt.x + rcB.rt.width * 0.8;
					ptB.y = rcB.rt.y + rcB.rt.height * 0.8;
				}
				else
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.8;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.2;
					ptB.x = rcB.rt.x + rcB.rt.width * 0.8;
					ptB.y = rcB.rt.y + rcB.rt.height * 0.2;
				}
			}
			else
			{
				if (rcModelB.rt.y < nCenterY)
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.2;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.8;
					ptB.x = rcB.rt.x + rcB.rt.width * 0.2;
					ptB.y = rcB.rt.y + rcB.rt.height * 0.8;
				}
				else
				{
					ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.2;
					ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.2;
					ptB.x = rcB.rt.x + rcB.rt.width * 0.2;
					ptB.y = rcB.rt.y + rcB.rt.height * 0.2;
				}
			}
		}
		else
		{
			ptA0.x = rcModelA.rt.x + rcModelA.rt.width * 0.8;
			ptA0.y = rcModelA.rt.y + rcModelA.rt.height * 0.8;
			ptB0.x = rcModelB.rt.x + rcModelB.rt.width * 0.8;
			ptB0.y = rcModelB.rt.y + rcModelB.rt.height * 0.8;

			ptA.x = rcA.rt.x + rcA.rt.width * 0.8;
			ptA.y = rcA.rt.y + rcA.rt.height * 0.8;
			ptB.x = rcB.rt.x + rcB.rt.width * 0.8;
			ptB.y = rcB.rt.y + rcB.rt.height * 0.8;
		}
		ptC0.x = rt.x;
		ptC0.y = rt.y;

		ptC = TriangleCoordinate(ptA0, ptB0, ptC0, ptA, ptB);
		rt.x = ptC.x;
		rt.y = ptC.y;

		//���µĵ�Ҳ����
		// 		cv::Point ptC1;
		// 		ptC1.x = ptC0.x + rt.width;
		// 		ptC1.y = ptC0.y + rt.height;
		// 		ptC = TriangleCoordinate(ptA0, ptB0, ptC1, ptA, ptB);
		// 		rt.width = ptC1.x - rt.x;
		// 		rt.height = ptC1.y - rt.y;

		//		TRACE("����1(%d, %d), ����2(%d, %d),�µ�C��(%d, %d), C��(%d, %d), ԭ����1(%d, %d), ����2(%d, %d)\n", ptA.x, ptA.y, ptB.x, ptB.y, ptC.x, ptC.y, ptC0.x, ptC0.y, ptA0.x, ptA0.y, ptB0.x, ptB0.y);
	}
	else if (lModelFix.size() == 3)
	{
		if (lFix.size() < 3)
		{
			if (lFix.size() > 0)
			{
				RECTLIST lModelTmp;
				RECTLIST::iterator itFix = lFix.begin();
				for (int i = 0; itFix != lFix.end(); i++, itFix++)
				{
					RECTLIST::iterator itModel = lModelFix.begin();
					for (int j = 0; itModel != lModelFix.end(); j++, itModel++)
					{
						if (j == itFix->nTH)
						{
							lModelTmp.push_back(*itModel);
							break;
						}
					}
				}
				return GetPosition(lFix, lModelTmp, rt, nPicW, nPicH);
			}
			else
				return false;
		}
#ifdef WarpAffine_TEST
		return true;
#else
		RECTLIST::iterator it = lFix.begin();
		RECTINFO rcA = *it++;
		RECTINFO rcB = *it++;
		RECTINFO rcC = *it;
		RECTLIST::iterator itModel = lModelFix.begin();
		RECTINFO rcModelA = *itModel++;
		RECTINFO rcModelB = *itModel++;
		RECTINFO rcModelC = *itModel;

		cv::Point2f ptA, ptB, ptC, ptA0, ptB0, ptC0;
		cv::Point2f ptChk;

		ptA0.x = rcModelA.rt.x + (double)rcModelA.rt.width / 2;
		ptA0.y = rcModelA.rt.y + (double)rcModelA.rt.height / 2;
		ptB0.x = rcModelB.rt.x + (double)rcModelB.rt.width / 2;
		ptB0.y = rcModelB.rt.y + (double)rcModelB.rt.height / 2;
		ptC0.x = rcModelC.rt.x + (double)rcModelC.rt.width / 2;
		ptC0.y = rcModelC.rt.y + (double)rcModelC.rt.height / 2;

		ptA.x = rcA.rt.x + (double)rcA.rt.width / 2;
		ptA.y = rcA.rt.y + (double)rcA.rt.height / 2;
		ptB.x = rcB.rt.x + (double)rcB.rt.width / 2;
		ptB.y = rcB.rt.y + (double)rcB.rt.height / 2;
		ptC.x = rcC.rt.x + (double)rcC.rt.width / 2;
		ptC.y = rcC.rt.y + (double)rcC.rt.height / 2;

		ptChk.x = rt.x;
		ptChk.y = rt.y;
		cv::Point2d ptResult;
#ifdef TriangleSide_TEST
		ptResult = TriangleSide(ptChk, ptA0, ptB0, ptC0, ptA, ptB, ptC);
#else
		long double dMax_X = 0, dMax_Y = 0, dMin_X = 0, dMin_Y = 0;
		long double dSumX = 0;
		long double dSumY = 0;
		long double v1 = 0, v2 = 0, v3 = 0;
		int nCount = 0;
		cv::Point2d ptResult1 = TriangleCentroid(ptChk, ptA0, ptB0, ptA, ptB);
		if (ptResult1.x != 0 && ptResult1.y != 0)
		{
			dSumX += ptResult1.x;
			dSumY += ptResult1.y;
			nCount++;

			dMin_X = ptResult1.x;
			dMin_Y = ptResult1.y;
			if (dMax_X < ptResult1.x) dMax_X = ptResult1.x;
			if (dMax_Y < ptResult1.y) dMax_Y = ptResult1.y;
			if (dMin_X > ptResult1.x) dMin_X = ptResult1.x;
			if (dMin_Y > ptResult1.y) dMin_Y = ptResult1.y;

			v1 = 1;
		}
		cv::Point2d ptResult2 = TriangleCentroid(ptChk, ptA0, ptC0, ptA, ptC);
		if (ptResult2.x != 0 && ptResult2.y != 0)
		{
			dSumX += ptResult2.x;
			dSumY += ptResult2.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult2.x;
			if (dMin_Y == 0) dMin_Y = ptResult2.y;
			if (dMax_X < ptResult2.x) dMax_X = ptResult2.x;
			if (dMax_Y < ptResult2.y) dMax_Y = ptResult2.y;
			if (dMin_X > ptResult2.x) dMin_X = ptResult2.x;
			if (dMin_Y > ptResult2.y) dMin_Y = ptResult2.y;

			v2 = 1;
		}
		cv::Point2d ptResult3 = TriangleCentroid(ptChk, ptB0, ptC0, ptB, ptC);
		if (ptResult3.x != 0 && ptResult3.y != 0)
		{
			dSumX += ptResult3.x;
			dSumY += ptResult3.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult3.x;
			if (dMin_Y == 0) dMin_Y = ptResult3.y;
			if (dMax_X < ptResult3.x) dMax_X = ptResult3.x;
			if (dMax_Y < ptResult3.y) dMax_Y = ptResult3.y;
			if (dMin_X > ptResult3.x) dMin_X = ptResult3.x;
			if (dMin_Y > ptResult3.y) dMin_Y = ptResult3.y;

			v3 = 1;
		}

		long double da = sqrt(pow(ptChk.x - ptA.x, 2) + pow(ptChk.y - ptA.y, 2));
		long double db = sqrt(pow(ptChk.x - ptB.x, 2) + pow(ptChk.y - ptB.y, 2));
		long double dc = sqrt(pow(ptChk.x - ptC.x, 2) + pow(ptChk.y - ptC.y, 2));

		long double x1 = (ptResult1.x / (da + db) + ptResult2.x / (da + dc) + ptResult3.x / (db + dc)) / (v1 / (da + db) + v2 / (da + dc) + v3 / (db + dc));
		long double y1 = (ptResult1.y / (da + db) + ptResult2.y / (da + dc) + ptResult3.y / (db + dc)) / (v1 / (da + db) + v2 / (da + dc) + v3 / (db + dc));
		long double x = (ptResult1.x * (v1 / da + v2 / db) + ptResult2.x * (v1 / da + v3 / dc) + ptResult3.x * (v2 / db + v3 / dc)) / (2 * (v1 / da + v2 / db + v3 / dc));
		long double y = (ptResult1.y * (v1 / da + v2 / db) + ptResult2.y * (v1 / da + v3 / dc) + ptResult3.y * (v2 / db + v3 / dc)) / (2 * (v1 / da + v2 / db + v3 / dc));

		if (nCount > 0)
		{
			// 			ptResult.x = dSumX / nCount;
			// 			ptResult.y = dSumY / nCount;
			ptResult.x = x;
			ptResult.y = y;
		}
		else
		{
			std::string strLog = "���ļ���ʧ�ܣ�û�н���";
		}
		TRACE("���������㷨: chk(%f,%f),ptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f),��������(%f,%f)���õ�(%f,%f)\n", ptChk.x, ptChk.y, \
			  ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult.x, ptResult.y, x1, y1);
#endif
		rt.x = ptResult.x;
		rt.y = ptResult.y;
#endif
	}
	else if (lModelFix.size() == 4)
	{
		if (lFix.size() < 4)
		{
			if (lFix.size() > 0)
			{
				RECTLIST lModelTmp;
				RECTLIST::iterator itFix = lFix.begin();
				for (int i = 0; itFix != lFix.end(); i++, itFix++)
				{
					RECTLIST::iterator itModel = lModelFix.begin();
					for (int j = 0; itModel != lModelFix.end(); j++, itModel++)
					{
						if (j == itFix->nTH)
						{
							lModelTmp.push_back(*itModel);
							break;
						}
					}
				}
				return GetPosition(lFix, lModelTmp, rt, nPicW, nPicH);
			}
			else
				return false;
		}
#ifdef WarpAffine_TEST
		return true;
#else
		RECTLIST::iterator it = lFix.begin();
		RECTINFO rcA = *it++;
		RECTINFO rcB = *it++;
		RECTINFO rcC = *it++;
		RECTINFO rcD = *it;
		RECTLIST::iterator itModel = lModelFix.begin();
		RECTINFO rcModelA = *itModel++;
		RECTINFO rcModelB = *itModel++;
		RECTINFO rcModelC = *itModel++;
		RECTINFO rcModelD = *itModel;

		cv::Point2f ptA, ptB, ptC, ptD, ptA0, ptB0, ptC0, ptD0;
		cv::Point2f ptChk;

		ptA0.x = rcModelA.rt.x + (double)rcModelA.rt.width / 2;
		ptA0.y = rcModelA.rt.y + (double)rcModelA.rt.height / 2;
		ptB0.x = rcModelB.rt.x + (double)rcModelB.rt.width / 2;
		ptB0.y = rcModelB.rt.y + (double)rcModelB.rt.height / 2;
		ptC0.x = rcModelC.rt.x + (double)rcModelC.rt.width / 2;
		ptC0.y = rcModelC.rt.y + (double)rcModelC.rt.height / 2;
		ptD0.x = rcModelD.rt.x + (double)rcModelD.rt.width / 2;
		ptD0.y = rcModelD.rt.y + (double)rcModelD.rt.height / 2;

		ptA.x = rcA.rt.x + (double)rcA.rt.width / 2;
		ptA.y = rcA.rt.y + (double)rcA.rt.height / 2;
		ptB.x = rcB.rt.x + (double)rcB.rt.width / 2;
		ptB.y = rcB.rt.y + (double)rcB.rt.height / 2;
		ptC.x = rcC.rt.x + (double)rcC.rt.width / 2;
		ptC.y = rcC.rt.y + (double)rcC.rt.height / 2;
		ptD.x = rcD.rt.x + (double)rcD.rt.width / 2;
		ptD.y = rcD.rt.y + (double)rcD.rt.height / 2;

		ptChk.x = rt.x;
		ptChk.y = rt.y;
		cv::Point2d ptResult;
#ifdef TriangleSide_TEST
		long double dSumX = 0;
		long double dSumY = 0;
		int nCount = 0;
		//		ptResult = TriangleSide(ptChk, ptA0, ptB0, ptC0, ptA, ptB, ptC);
		cv::Point2d ptResult1 = TriangleSide(ptChk, ptA0, ptB0, ptC0, ptA, ptB, ptC);
		if (ptResult1.x != 0 && ptResult1.y != 0)
		{
			dSumX += ptResult1.x;
			dSumY += ptResult1.y;
			nCount++;
		}
		cv::Point2d ptResult2 = TriangleSide(ptChk, ptA0, ptB0, ptD0, ptA, ptB, ptD);
		if (ptResult2.x != 0 && ptResult2.y != 0)
		{
			dSumX += ptResult2.x;
			dSumY += ptResult2.y;
			nCount++;
		}
		cv::Point2d ptResult3 = TriangleSide(ptChk, ptB0, ptC0, ptD0, ptB, ptC, ptD);
		if (ptResult3.x != 0 && ptResult3.y != 0)
		{
			dSumX += ptResult3.x;
			dSumY += ptResult3.y;
			nCount++;
		}
		cv::Point2d ptResult4 = TriangleSide(ptChk, ptA0, ptC0, ptD0, ptA, ptC, ptD);
		if (ptResult4.x != 0 && ptResult4.y != 0)
		{
			dSumX += ptResult4.x;
			dSumY += ptResult4.y;
			nCount++;
		}
#if 0
		double d1 = abs(ptResult1.x - ptResult2.x);
		double d2 = abs(ptResult1.x - ptResult3.x);
		double d3 = abs(ptResult2.x - ptResult3.x);
		if (d1 < d2)
		{
			if (d1 < d3) { ptResult.x = (ptResult1.x + ptResult2.x) / 2; }
			else if (d1 > d3) { ptResult.x = (ptResult3.x + ptResult2.x) / 2; }
			else { ptResult.x = (ptResult1.x + ptResult2.x + ptResult3.x) / 3; }
		}
		else if (d1 > d2)
		{
			if (d2 < d3) { ptResult.x = (ptResult1.x + ptResult3.x) / 2; }
			else if (d2 > d3) { ptResult.x = (ptResult3.x + ptResult2.x) / 2; }
			else { ptResult.x = (ptResult1.x + ptResult2.x + ptResult3.x) / 3; }
		}
		else
		{
			ptResult.x = (ptResult1.x + ptResult2.x + ptResult3.x) / 3;
		}
		double d4 = abs(ptResult1.y - ptResult2.y);
		double d5 = abs(ptResult1.y - ptResult3.y);
		double d6 = abs(ptResult2.y - ptResult3.y);
		if (d4 < d5)
		{
			if (d4 < d6) { ptResult.y = (ptResult1.y + ptResult2.y) / 2; }
			else if (d4 > d6) { ptResult.y = (ptResult3.y + ptResult2.y) / 2; }
			else { ptResult.y = (ptResult1.y + ptResult2.y + ptResult3.y) / 3; }
		}
		else if (d4 > d5)
		{
			if (d5 < d6) { ptResult.y = (ptResult1.y + ptResult3.y) / 2; }
			else if (d5 > d6) { ptResult.y = (ptResult3.y + ptResult2.y) / 2; }
			else { ptResult.y = (ptResult1.y + ptResult2.y + ptResult3.y) / 3; }
		}
		else
		{
			ptResult.y = (ptResult1.y + ptResult2.y + ptResult3.y) / 3;
		}
#else
		if (nCount > 0)
		{
			ptResult.x = dSumX / nCount;
			ptResult.y = dSumY / nCount;
		}
		else
		{
			std::string strLog = "���ļ���ʧ�ܣ�û�н���";
		}
#endif
		TRACE("���߶�λ�㷨: ptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f), ptResult4(%f,%f)��������(%f,%f)\n", \
			  ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult4.x, ptResult4.y, ptResult.x, ptResult.y);
#else
		long double dMax_X = 0, dMax_Y = 0, dMin_X = 0, dMin_Y = 0;
		long double dSumX = 0;
		long double dSumY = 0;
		int nCount = 0;
		cv::Point2d ptResult1 = TriangleCentroid(ptChk, ptA0, ptB0, ptA, ptB);
		if (ptResult1.x > 0 && ptResult1.y > 0)
		{
			dSumX += ptResult1.x;
			dSumY += ptResult1.y;
			nCount++;

			dMin_X = ptResult1.x;
			dMin_Y = ptResult1.y;
			if (dMax_X < ptResult1.x) dMax_X = ptResult1.x;
			if (dMax_Y < ptResult1.y) dMax_Y = ptResult1.y;
			if (dMin_X > ptResult1.x) dMin_X = ptResult1.x;
			if (dMin_Y > ptResult1.y) dMin_Y = ptResult1.y;
		}

		cv::Point2d ptResult2 = TriangleCentroid(ptChk, ptA0, ptC0, ptA, ptC);
		if (ptResult2.x > 0 && ptResult2.y > 0)
		{
			dSumX += ptResult2.x;
			dSumY += ptResult2.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult2.x;
			if (dMin_Y == 0) dMin_Y = ptResult2.y;
			if (dMax_X < ptResult2.x) dMax_X = ptResult2.x;
			if (dMax_Y < ptResult2.y) dMax_Y = ptResult2.y;
			if (dMin_X > ptResult2.x) dMin_X = ptResult2.x;
			if (dMin_Y > ptResult2.y) dMin_Y = ptResult2.y;
		}

		cv::Point2d ptResult3 = TriangleCentroid(ptChk, ptB0, ptC0, ptB, ptC);
		if (ptResult3.x > 0 && ptResult3.y > 0)
		{
			dSumX += ptResult3.x;
			dSumY += ptResult3.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult3.x;
			if (dMin_Y == 0) dMin_Y = ptResult3.y;
			if (dMax_X < ptResult3.x) dMax_X = ptResult3.x;
			if (dMax_Y < ptResult3.y) dMax_Y = ptResult3.y;
			if (dMin_X > ptResult3.x) dMin_X = ptResult3.x;
			if (dMin_Y > ptResult3.y) dMin_Y = ptResult3.y;
		}

		cv::Point2d ptResult4 = TriangleCentroid(ptChk, ptA0, ptD0, ptA, ptD);
		if (ptResult4.x > 0 && ptResult4.y > 0)
		{
			dSumX += ptResult4.x;
			dSumY += ptResult4.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult4.x;
			if (dMin_Y == 0) dMin_Y = ptResult4.y;
			if (dMax_X < ptResult4.x) dMax_X = ptResult4.x;
			if (dMax_Y < ptResult4.y) dMax_Y = ptResult4.y;
			if (dMin_X > ptResult4.x) dMin_X = ptResult4.x;
			if (dMin_Y > ptResult4.y) dMin_Y = ptResult4.y;
		}

		cv::Point2d ptResult5 = TriangleCentroid(ptChk, ptB0, ptD0, ptB, ptD);
		if (ptResult5.x > 0 && ptResult5.y > 0)
		{
			dSumX += ptResult5.x;
			dSumY += ptResult5.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult5.x;
			if (dMin_Y == 0) dMin_Y = ptResult5.y;
			if (dMax_X < ptResult5.x) dMax_X = ptResult5.x;
			if (dMax_Y < ptResult5.y) dMax_Y = ptResult5.y;
			if (dMin_X > ptResult5.x) dMin_X = ptResult5.x;
			if (dMin_Y > ptResult5.y) dMin_Y = ptResult5.y;
		}

		cv::Point2d ptResult6 = TriangleCentroid(ptChk, ptC0, ptD0, ptC, ptD);
		if (ptResult6.x != 0 && ptResult6.y != 0)
		{
			dSumX += ptResult6.x;
			dSumY += ptResult6.y;
			nCount++;

			if (dMin_X == 0) dMin_X = ptResult6.x;
			if (dMin_Y == 0) dMin_Y = ptResult6.y;
			if (dMax_X < ptResult6.x) dMax_X = ptResult6.x;
			if (dMax_Y < ptResult6.y) dMax_Y = ptResult6.y;
			if (dMin_X > ptResult6.x) dMin_X = ptResult6.x;
			if (dMin_Y > ptResult6.y) dMin_Y = ptResult6.y;
		}

		dSumX -= dMax_X;
		dSumX -= dMin_X;
		dSumY -= dMax_Y;
		dSumY -= dMin_Y;
		nCount -= 2;

		long double da = sqrt(pow(ptChk.x - ptA.x, 2) + pow(ptChk.y - ptA.y, 2));
		long double db = sqrt(pow(ptChk.x - ptB.x, 2) + pow(ptChk.y - ptB.y, 2));
		long double dc = sqrt(pow(ptChk.x - ptC.x, 2) + pow(ptChk.y - ptC.y, 2));
		long double dd = sqrt(pow(ptChk.x - ptD.x, 2) + pow(ptChk.y - ptD.y, 2));
#if 0	//���������3����������3�����У�ȡ����������2��ֱ�ߣ���ABC�������3���㣬����A�����������AB��AC����ĵ�
		cv::Point2f pt[3];
		cv::Point2f pt2[3];
		GetMaxMin(ptChk, ptA, ptB, ptC, ptD, ptA0, ptB0, ptC0, ptD0, pt, pt2);

		long double dSx = 0;
		long double dSy = 0;
		int nCount2 = 0;
		cv::Point2d ptResultT1 = TriangleCentroid(ptChk, pt2[0], pt2[1], pt[0], pt[1]);
		if (ptResultT1.x > 0 && ptResultT1.y > 0)
		{
			dSx += ptResultT1.x;
			dSy += ptResultT1.y;
			nCount2++;
		}
		cv::Point2d ptResultT2 = TriangleCentroid(ptChk, pt2[0], pt2[2], pt[0], pt[2]);
		if (ptResultT2.x > 0 && ptResultT2.y > 0)
		{
			dSx += ptResultT2.x;
			dSy += ptResultT2.y;
			nCount2++;
		}
		ptResult.x = dSx / nCount2;
		ptResult.y = dSy / nCount2;
		TRACE("�ο�(%f,%f), ptResultT1((%f,%f), ptResultT1((%f,%f)\n", dSumX / nCount, dSumY / nCount, ptResultT1.x, ptResultT1.y, ptResultT2.x, ptResultT2.y);
#endif


		if (nCount > 0)
		{
			ptResult.x = dSumX / nCount;
			ptResult.y = dSumY / nCount;
		}
		else
		{
			std::string strLog = "���ļ���ʧ�ܣ�û�н���";
		}
		TRACE("���������㷨: chk(%f,%f), da=%f,db=%f,dc=%f,dd=%f,\nptResult1(%f,%f),ptResult2(%f,%f),ptResult3(%f,%f),ptResult4(%f,%f),ptResult5(%f,%f),ptResult6(%f,%f),��������(%f,%f)\n", ptChk.x, ptChk.y, \
			  da, db, dc, dd, ptResult1.x, ptResult1.y, ptResult2.x, ptResult2.y, ptResult3.x, ptResult3.y, ptResult4.x, ptResult4.y, ptResult5.x, ptResult5.y, ptResult6.x, ptResult6.y, ptResult.x, ptResult.y);
#endif
		rt.x = ptResult.x;
		rt.y = ptResult.y;
#endif
	}
	return true;
}

cv::Point2d CAssistRecog::TriangleCoordinate(cv::Point ptA, cv::Point ptB, cv::Point ptC, cv::Point ptNewA, cv::Point ptNewB)
{
	// 	clock_t start, end;
	// 	start = clock();
	long double c02 = pow((ptB.x - ptA.x), 2) + pow((ptB.y - ptA.y), 2);
	long double b02 = pow((ptC.x - ptA.x), 2) + pow((ptC.y - ptA.y), 2);
	long double a02 = pow((ptC.x - ptB.x), 2) + pow((ptC.y - ptB.y), 2);
	long double c2 = pow((ptNewB.x - ptNewA.x), 2) + pow((ptNewB.y - ptNewA.y), 2);

	long double m = sqrt(c2 / c02);	//�������α߳���ԭ�����εı���

	long double a2 = pow(m, 2) * a02;
	long double b2 = pow(m, 2) * b02;
	long double dT1 = 2 * b2 * (a2 + c2) - pow(a2 - c2, 2) - pow(b2, 2);
	//++C��ABֱ�ߵľ��� 
	long double A = ptB.y - ptA.y;
	long double B = ptA.x - ptB.x;
	long double C = ptB.x * ptA.y - ptA.x * ptB.y;
	long double dC2AB = abs((A * ptC.x + B * ptC.y + C) / sqrt(pow(A, 2) + pow(B, 2)));		//C��ABֱ�ߵľ���
																							//--


	long double k_ab;		//ԭABֱ��б��
	long double dDx;			//ԭC�㴹ֱ��AB��D��
	long double dDy;
	long double dFlag;		//��ʶԭC��λ��AB���Ϸ������·�
#if 1	//ͨ����ά��������жϷ���
	cv::Point2f Xab;	//����AB
	cv::Point2f Xac;	//����AC
	Xab.x = ptB.x - ptA.x;
	Xab.y = ptB.y - ptA.y;
	Xac.x = ptC.x - ptA.x;
	Xac.y = ptC.y - ptA.y;
	dFlag = Xab.x * Xac.y - Xab.y * Xac.x;	//����AB * ����AC�Ĳ��

	// 	cv::Point2f Xac;	//����AC
	// 	cv::Point2f Xbc;	//����BC
	// 	Xac.x = ptC.x - ptA.x;
	// 	Xac.y = ptC.y - ptA.y;
	// 	Xbc.x = ptC.x - ptA.x;
	// 	Xbc.y = ptB.y - ptB.y;
	// 	dFlag = Xab.x * Xac.y - Xab.y * Xac.x;	//����AB * ����AC�Ĳ��
	// 	cv::Point2f Xab;	//����AB
	// 	cv::Point2f Xac;	//����AC
	// 	Xab.x = ptB.x - ptA.x;
	// 	Xab.y = ptB.y - ptB.y;
	// 	Xac.x = ptC.x - ptA.x;
	// 	Xac.y = ptC.y - ptA.y;
	// 	dFlag = Xab.x * Xac.y - Xab.y * Xac.x;	//����AB * ����AC�Ĳ��


	// 	//ͨ��D = Ax + By + C�ж���ֱ����һ�࣬D<0��ֱ����࣬D>0��ֱ���Ҳ࣬D=0��ֱ����
	// 	long double DptC = A * ptC.x + B * ptC.y + C;
#else
	if (ptA.x != ptB.x)
	{
		k_ab = (long double)(ptB.y - ptA.y) / (ptB.x - ptA.x);
		dDx = (ptC.x + ptC.y * k_ab - k_ab * ptA.y + pow(k_ab, 2) * ptA.x) / (pow(k_ab, 2) + 1);
		dDy = k_ab * ((ptC.x + ptC.y * k_ab - k_ab * ptA.y - ptA.x) / (pow(k_ab, 2) + 1)) + ptA.y;
		dFlag = k_ab*(ptC.x - ptA.x) + ptA.y - ptC.y;
	}
	else
	{
		dDx = ptA.x;
		dDy = ptC.y;
		dFlag = ptC.x - ptA.x;
	}
#endif

	long double dTmp1 = (ptNewA.x - ptNewB.x) * sqrt(2 * b2 * (a2 + c2) - pow(a2 - c2, 2) - pow(b2, 2)) / (2 * c2);
	long double dTmp2 = (pow(m, 2) * (a02 - b02) * (ptNewB.y - ptNewA.y) - (ptNewA.y + ptNewB.y) * c2) / (2 * c2);

	long double dK1 = (pow(m, 2) * (a02 - b02) + pow(ptNewA.x, 2) - pow(ptNewB.x, 2) + pow(ptNewA.y, 2) - pow(ptNewB.y, 2)) / (2 * (ptNewA.x - ptNewB.x));
	long double dK2 = (long double)(ptNewB.y - ptNewA.y) / (ptNewA.x - ptNewB.x);

	long double dTmp3 = sqrt((pow(dK2, 2) + 1) * pow(m, 2) * a02 - pow(dK2 * ptNewB.y + dK1 - ptNewB.x, 2)) / (pow(dK2, 2) + 1);
	long double dTmp4 = (dK1 * dK2 - ptNewB.x * dK2 - ptNewB.y) / (pow(dK2, 2) + 1);

	long double dYc1;	//�µ�C�������1
	long double dXc1;
	long double dYc2;	//�µ�C�������2
	long double dXc2;
	if (ptNewA.x == ptNewB.x)	//��ʱdK1, dK2Ϊ���޴�
	{
		dYc1 = dTmp1 - dTmp2;
		dXc1 = ptNewA.x + dC2AB * m;

		dYc2 = -dTmp1 - dTmp2;
		dXc2 = ptNewA.x - dC2AB * m;
	}
	else
	{
		dYc1 = dTmp1 - dTmp2;
		dXc1 = dK1 + dK2 * dYc1;

		dYc2 = -dTmp1 - dTmp2;
		dXc2 = dK1 + dK2 * dYc2;
	}
	cv::Point2d ptNewC;
#if 1
	cv::Point2f Xa1b1;		//����A1B1
	cv::Point2f Xa1c1;		//����A1C1
	cv::Point2f Xa1c2;		//����A1C2
	Xa1b1.x = ptNewB.x - ptNewA.x;
	Xa1b1.y = ptNewB.y - ptNewA.y;
	Xa1c1.x = dXc1 - ptNewA.x;
	Xa1c1.y = dYc1 - ptNewA.y;
	Xa1c2.x = dXc2 - ptNewA.x;
	Xa1c2.y = dYc2 - ptNewA.y;
	long double dNewFlag = Xa1b1.x * Xa1c1.y - Xa1b1.y * Xa1c1.x;	//����A1B1 * ����A1C1�Ĳ��
	long double dNewFlag2 = Xa1b1.x * Xa1c2.y - Xa1b1.y * Xa1c2.x;	//����A1B1 * ����A1C2�Ĳ��

	// 	//ͨ��D = Ax + By + C�ж���ֱ����һ�࣬D<0��ֱ����࣬D>0��ֱ���Ҳ࣬D=0��ֱ����
	// 	long double A1 = ptNewB.y - ptNewA.y;
	// 	long double B1 = ptNewA.x - ptNewB.x;
	// 	long double C1 = ptNewB.x * ptNewA.y - ptNewA.x * ptNewB.y;
	// 	long double DptC1 = A1 * dXc1 + B1 * dYc1 + C1;
	// 	long double DptC2 = A1 * dXc2 + B1 * dYc2 + C1;
#else
	long double k_newAB = (double)(ptNewB.y - ptNewA.y) / (ptNewB.x - ptNewA.x);
	long double dNewFlag = k_newAB*(dXc1 - ptNewA.x) + ptNewA.y - dYc1;
	long double dNewFlag2 = k_newAB*(dXc2 - ptNewA.x) + ptNewA.y - dYc2;
#endif
	if (dFlag >= 0)
	{
		if (dNewFlag >= 0)		//xy����Ҫ������������
		{
			ptNewC.x = dXc1;
			ptNewC.y = dYc1;
		}
		else if (dNewFlag < 0)
		{
			ptNewC.x = dXc2;
			ptNewC.y = dYc2;
		}
	}
	else if (dFlag < 0)
	{
		if (dNewFlag >= 0)
		{
			ptNewC.x = dXc2;
			ptNewC.y = dYc2;
		}
		else if (dNewFlag < 0)
		{
			ptNewC.x = dXc1;
			ptNewC.y = dYc1;
		}
	}
	// 	end = clock();
	// 	TRACE("�µ�C������(%f, %f)����(%f, %f),ȷ����Ϊ(%f,%f)��ʱ: %d\n", dXc1, dYc1, dXc2, dYc2, ptNewC.x, ptNewC.y, end - start);
	return ptNewC;
}

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
	if (nOrientation == 1)	//matSrc����
	{
		rtResult = rt;
	}
	else if (nOrientation == 2)	//matSrc��ת90��
	{
		cv::Point pt1, pt2;
		pt1.x = nH - rt.tl().y;
		pt1.y = rt.tl().x;
		pt2.x = nH - rt.br().y;
		pt2.y = rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 3)	//matSrc��ת90��
	{
		cv::Point pt1, pt2;
		pt1.x = rt.tl().y;
		pt1.y = nW - rt.tl().x;
		pt2.x = rt.br().y;
		pt2.y = nW - rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 4)	//matSrc��ת180��
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
		//��ȡ������������תͼ���ľ���
		cv::Rect rtModelPic;
		rtModelPic.width = pModel->vecPaperModel[nPic]->nPicW;
		rtModelPic.height = pModel->vecPaperModel[nPic]->nPicH;
		rc.rt = GetRectByOrientation(rtModelPic, rc.rt, nOrientation);

		std::vector<cv::Rect>RectCompList;
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

			cv::Mat matCompRoi;
			matCompRoi = matCompPic(rc.rt);

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

			if (nThreshold > nRealThreshold) nThreshold = nRealThreshold;
			threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);
#else
			threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
			//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
			cv::Mat element_Anticlutter = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
			dilate(matCompRoi, matCompRoi, element_Anticlutter);
			erode(matCompRoi, matCompRoi, element_Anticlutter);

			cv::Canny(matCompRoi, matCompRoi, 0, rc.nCannyKernel, 5);	//_nCannyKernel_
			cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	��ͨ�հ׿��ʶ��	Size(_nDilateKernel_, _nDilateKernel_)
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
				cv::Rect rm = aRect;
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
			std::string strLog = "ʶ�𶨵��쳣: " + exc.msg;
			g_pLogger->information(strLog);
			TRACE(strLog.c_str());

			bResult = false;						//�ҵ������
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

			bool bOnlyOne = false;		//ֻ��һ�����Σ���Ҫ�ж�����ͻҶȣ����Ǳ������Խ���
			bool bFind = false;
			if (RectCompList.size() == 1)	//ֻ����һ������ʱ����Ĭ�Ͼ��Ƕ�����
				bOnlyOne = true;

			//ͨ���Ҷ�ֵ���ж�
			for (int i = 0; i < RectCompList.size(); i++)
			{
				RECTINFO rcTmp = rcFix;
				rcTmp.rt = RectCompList[i];
				Recog(nPic, rcTmp, matCompPic, NULL, NULL);
				float fArea = rcTmp.fRealArea / rcTmp.fStandardArea;
				float fDensity = rcTmp.fRealDensity / rcTmp.fStandardDensity;
				float fWper = (float)rcTmp.rt.width / rcFix.rt.width;			//���ҵľ��εĿ����ģ���Ӧ����Ŀ��֮��
				float fHper = (float)rcTmp.rt.height / rcFix.rt.height;			//���ҵľ��εĿ����ģ���Ӧ����ĸ߶�֮��
				
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
				rcFixInfo.nTH = i;			//��������ģ���϶����б�ĵڼ���
				rcFixInfo.rt = rtFix;
//				pPic->lFix.push_back(rcFixInfo);
				rlFix.push_back(rcFixInfo);
				TRACE("�������: (%d,%d,%d,%d)\n", rtFix.x, rtFix.y, rtFix.width, rtFix.height);
			}
		}
		if (bFindRect)
		{
			bResult = false;						//�ҵ������
// 			pPic->bFindIssue = true;
// 			pPic->lIssueRect.push_back(rc);
		}
	}
	if (!bResult)
	{
// 		char szLog[MAX_PATH] = { 0 };
// 		sprintf_s(szLog, "ʶ�𶨵�ʧ��, ͼƬ��: %s\n", pPic->strPicName.c_str());
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
		for (; itSnItem != pSnItem->lSN.end(); itSnItem++)		//���һ�����ο飬�б������ֻ��һ��Ԫ��
		{
			RECTINFO rc = *itSnItem;

			//��ȡ������������תͼ���ľ���
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
		cv::calcHist(&matCompRoi, 1, channels, cv::Mat(), comp_hist, 1, histSize, ranges, false);

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

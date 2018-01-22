#include "stdafx.h"
#include "CoordinationConvert.h"


CCoordinationConvert::CCoordinationConvert(cv::Mat matSrcPic)
	:rtPic(0, 0, matSrcPic.cols, matSrcPic.rows)
{
}


CCoordinationConvert::~CCoordinationConvert()
{
}

void CCoordinationConvert::SetPicRect(int nW, int nH)
{
	rtPic.width = nW;
	rtPic.height = nH;
}

cv::Rect CCoordinationConvert::GetSrcSaveRect(cv::Rect rt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Rect rtResult;
	if (nOrientation == 1)	//matSrc正向	模板图像的旋转方向
	{
		rtResult = rt;
	}
	else if (nOrientation == 2)	//matSrc右转90度，实际保存坐标进行左旋90
	{
		cv::Point pt1, pt2;
		pt1.x = rt.tl().y;
		pt1.y = nW - rt.tl().x;
		pt2.x = rt.br().y;
		pt2.y = nW - rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 3)	//matSrc左转90度，实际保存坐标进行右旋90
	{
		cv::Point pt1, pt2;
		pt1.x = nH - rt.tl().y;
		pt1.y = rt.tl().x;
		pt2.x = nH - rt.br().y;
		pt2.y = rt.br().x;
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

cv::Point CCoordinationConvert::GetPointToSave(cv::Point pt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Point ptResult;
	if (nOrientation == 1)	//matSrc正向	模板图像的旋转方向
	{
		ptResult = pt;
	}
	else if (nOrientation == 2)	//matSrc右转90度，实际保存坐标进行左旋90
	{
		cv::Point pt1, pt2;
		pt1.x = pt.y;
		pt1.y = nW - pt.x;
		ptResult = pt1;
	}
	else if (nOrientation == 3)	//matSrc左转90度，实际保存坐标进行右旋90
	{
		cv::Point pt1, pt2;
		pt1.x = nH - pt.y;
		pt1.y = pt.x;
		ptResult = pt1;
	}
	else if (nOrientation == 4)	//matSrc右转180度
	{
		cv::Point pt1, pt2;
		pt1.x = nW - pt.x;
		pt1.y = nH - pt.y;
		ptResult = pt1;
	}
	return ptResult;
}

cv::Rect CCoordinationConvert::GetShowFakePosRect(cv::Rect rt, int nOrientation)
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

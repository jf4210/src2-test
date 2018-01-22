#pragma once
#include <opencv2\opencv.hpp>

class CCoordinationConvert
{
public:
	CCoordinationConvert(cv::Mat matSrcPic);
	~CCoordinationConvert();

	void	SetPicRect(int nW, int nH);		//改变参照图像的大小
	cv::Rect GetSrcSaveRect(cv::Rect rt, int nOrientation);			//（从假坐标到真坐标），获取真实的矩形位置
	cv::Point GetPointToSave(cv::Point pt, int nOrientation);		//（从假坐标到真坐标），获取真实的坐标点位置
	cv::Rect GetShowFakePosRect(cv::Rect rt, int nOrientation);		//(从真坐标到假坐标)，获取显示的矩形位置
private:
	cv::Rect rtPic;
};


#pragma once
#include <opencv2\opencv.hpp>

class CCoordinationConvert
{
public:
	CCoordinationConvert(cv::Mat matSrcPic);
	~CCoordinationConvert();

	void	SetPicRect(int nW, int nH);		//�ı����ͼ��Ĵ�С
	cv::Rect GetSrcSaveRect(cv::Rect rt, int nOrientation);			//���Ӽ����굽�����꣩����ȡ��ʵ�ľ���λ��
	cv::Point GetPointToSave(cv::Point pt, int nOrientation);		//���Ӽ����굽�����꣩����ȡ��ʵ�������λ��
	cv::Rect GetShowFakePosRect(cv::Rect rt, int nOrientation);		//(�������굽������)����ȡ��ʾ�ľ���λ��
private:
	cv::Rect rtPic;
};


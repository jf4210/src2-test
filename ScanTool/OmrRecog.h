#pragma once
#include "global.h"

class COmrRecog
{
public:
	COmrRecog();
	~COmrRecog();

	bool RecogFixCP(int nPic, cv::Mat& matCompPic, RECTLIST& rlFix, pMODEL	pModel, int nOrientation = 1);		//ʶ�𶨵�, Ĭ�����򣬲�������ת
	bool RecogRtVal(RECTINFO& rc, cv::Mat& matCompPic);			//ʶ�������һ�����εĻҶȡ��ܶȵ���Ϣ
protected:
	//����һ�����Σ�������������ת��ľ���
	//1:���򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180
	cv::Rect GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation);		
	//ʶ����λҶȡ��ܶȵ���Ϣ
	inline bool Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL	pModel);
};


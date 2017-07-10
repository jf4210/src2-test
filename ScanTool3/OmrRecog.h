#pragma once
#include "global.h"

class COmrRecog
{
public:
	COmrRecog();
	~COmrRecog();

	//给定一个矩形，按给定方向旋转后的矩形
	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
	cv::Rect GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation);

	bool RecogFixCP(int nPic, cv::Mat& matCompPic, RECTLIST& rlFix, pMODEL	pModel, int nOrientation = 1);		//识别定点, 默认正向，不进行旋转
	bool RecogRtVal(RECTINFO& rc, cv::Mat& matCompPic);			//识别给定的一个矩形的灰度、密度等信息
	bool RecogZkzh(int nPic, cv::Mat& matCompPic, pMODEL pModel, int nOrientation);	//识别二维码或者条码，根据识别成功与否来判断图像方向

	//获取图片的正确方向，和模板一样
	int GetRightPicOrientation(cv::Mat& matSrc, int n, bool bDoubleScan);
protected:	
	//识别矩形灰度、密度等信息
	inline bool Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL	pModel);

	//-------------------------------------------------------------
	void		sharpenImage1(const cv::Mat &image, cv::Mat &result, int nKernel);
	int			GetRects(cv::Mat& matSrc, cv::Rect rt, pMODEL pModel, int nPic, int nOrientation, int nHead);
//	cv::Rect	GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation);
	float		GetRtDensity(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod);
	bool		bGetMaxRect(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod, cv::Rect& rtMax);
	int		CheckOrientation4Fix(cv::Mat& matSrc, int n);	//定点模式下的方向
	int		CheckOrientation4Head(cv::Mat& matSrc, int n);	//同步头模式下的方向
	//检查图像旋转方向，在获取图像后将模板图像进行旋转
	//1:针对模板图像需要进行的旋转，正向，不需要旋转，2：右转90(模板图像旋转), 3：左转90(模板图像旋转), 4：右转180(模板图像旋转)
	int		CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan);
	//-------------------------------------------------------------
};


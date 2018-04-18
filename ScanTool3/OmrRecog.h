#pragma once
#include "global.h"

typedef struct _WordMatchTempl_
{
	cv::Rect rtModel;	//对应文字的模板位置信息
	RECTINFO rcReal;	//实际进行模板匹配后的信息
}ST_WORDMATCHTEMPL, *pST_WORDMATCHTEMPL;

class COmrRecog
{
public:
	COmrRecog();
	virtual ~COmrRecog();

	//给定一个矩形，按给定方向旋转后的矩形
	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
	cv::Rect GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation);

	bool RecogFixCP(int nPic, cv::Mat& matCompPic, RECTLIST& rlFix, pMODEL	pModel, int nOrientation = 1);		//识别定点, 默认正向，不进行旋转
	bool RecogRtVal(RECTINFO& rc, cv::Mat& matCompPic);			//识别给定的一个矩形的灰度、密度等信息
	bool RecogZkzh(int nPic, cv::Mat& matCompPic, pMODEL pModel, int nOrientation);	//识别二维码或者条码，根据识别成功与否来判断图像方向

	//=============================================================================================
	//进行仿射变换或透视变换，获取转换矩阵
	cv::Mat		GetRotMat(RECTLIST lFixRealPic, RECTLIST lFixModelPic);
	//根据模板上矩形来获取实际图像上的矩形
	cv::Rect GetRealRtFromModel(cv::Rect rtModel, RECTLIST lFixRealPic, RECTLIST lFixModelPic, cv::Mat rot_mat_inv);
	//=============================================================================================

	//获取图片的正确方向，和模板一样
	//1:针对模板图像需要进行的旋转，正向，不需要旋转，2：右转90(模板图像旋转), 3：左转90(模板图像旋转), 4：右转180(模板图像旋转), #不用0-正向，无法识别，故不旋转
	int GetRightPicOrientation(cv::Mat& matSrc, int n, bool bDoubleScan);

	//判断一张图像的正反面
	//参数nPic为模板上的含准考证号区的页面
	bool IsFirstPic(int nPic, cv::Mat& matCompPic, pMODEL pModel);

	std::string GetRecogLog();	//获取识别过程的日志
protected:	
	//识别矩形灰度、密度等信息
	inline bool Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL	pModel);

	//-------------------------------------------------------------
	void		sharpenImage1(const cv::Mat &image, cv::Mat &result, int nKernel);
	int			GetRects(cv::Mat& matSrc, cv::Rect rt, pMODEL pModel, int nPic, int nOrientation, int nHead);
//	cv::Rect	GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation);
	float		GetRtDensity(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod);
	bool		bGetMaxRect(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod, cv::Rect& rtMax);
	bool		RecogCodeOrientation(cv::Mat& matSrc, int n, pMODEL pModel, int& nResult);	//条码方向判断
	void		PicRotate(cv::Mat& matSrc, int n);	//图像旋转，实际图像需要进行的旋转，1：正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
	bool		RecogWordOrientationByMatchTempl(cv::Mat& matSrc, int n, int nRotation, int& nResult, std::string& strLog); //通过模板的文字匹配来识别文字的方向，具体操作
	bool		RecogWordOrientationByRectCount(cv::Mat& matSrc, int n, int nRotation, int& nResult, std::string& strLog); //通过矩形数量来识别文字的方向，具体操作

	bool	MatchingMethod(int method, cv::Mat& src, cv::Mat& templ, cv::Point& ptResult);	//模板匹配操作

	int		CheckOrientation4Fix(cv::Mat& matSrc, int n, std::string& strLog);	//定点模式下的方向
	int		CheckOrientation4Head(cv::Mat& matSrc, int n);	//同步头模式下的方向
	int		CheckOrientation4Word(cv::Mat& matSrc, int n);	//文字定位模式下的方向判断
	//检查图像旋转方向，在获取图像后将模板图像进行旋转
	//1:针对模板图像需要进行的旋转，正向，不需要旋转，2：右转90(模板图像旋转), 3：左转90(模板图像旋转), 4：右转180(模板图像旋转)
	int		CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan, std::string& strLog);
	//-------------------------------------------------------------


	int			GetRectsInArea(cv::Mat& matSrc, RECTINFO rc, int nMinW, int nMaxW, int nMinH, int nMaxH, int nFindContoursModel = CV_RETR_EXTERNAL);	//获取给定区域内的矩形数量
	std::string GetQR(cv::Mat img, std::string& strTypeName);	//获取准考证号，根据准考证号来判断正反、方向
private:
	//*********************************
	//*********	测试结论 **************
	//前提：双面扫描
	//1、正面不需要旋转 ==> 反面也不需要旋转
	//2、正面需要右转90度 ==> 反面需要左转90度
	//3、正面需要左转90度 ==> 反面需要右转90度
	//4、正面需要旋转180度 ==> 反面也需要旋转180度
	//#不用0、正面无法判断旋转方向，采用默认方向，不需要旋转==> 反面也采用默认方向，不需要旋转
	//*********************************
	int		_nFristOrientation;
	std::string _strLog;
};

class CAdjustPaperPic : public COmrRecog
{

public:
	CAdjustPaperPic();
	~CAdjustPaperPic();

	std::string GetLog();
	void AdjustScanPaperToModel(pST_SCAN_PAPER pScanPaperTask);	//将扫描的图片调整到和模板一致(正反、方向)
	void SaveScanPaperPic(pST_SCAN_PAPER pScanPaperTask);
	bool RecogPagination(pST_SCAN_PAPER pScanPaperTask);

private:
	std::string _strLog;
};
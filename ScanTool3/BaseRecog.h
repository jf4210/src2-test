#pragma once
#include "global.h"

class CAssistRecog
{
public:
	CAssistRecog();
	~CAssistRecog();

	//定点模式
	bool GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, cv::Rect& rt);
	//同步头模式
	bool GetRecogPosition(int nPic, pST_PicInfo pPic, pMODEL pModel, RECTINFO& rc);
private:
	bool	GetPosition(RECTLIST& lFix, RECTLIST& lModelFix, cv::Rect& rt, int nPicW = 0, int nPicH = 0);

	inline cv::Point2d TriangleCoordinate(cv::Point ptA, cv::Point ptB, cv::Point ptC, cv::Point ptNewA, cv::Point ptNewB);
};

class CBaseRecog
{
public:
	CBaseRecog();
	virtual ~CBaseRecog();

protected:
	bool Recog(RECTINFO& rc, cv::Mat& matCompPic, std::string& strLog);
	bool Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//先识别此选项有几个矩形区，选取最大的一个矩形区进行识别，此方法用于防止出现框选到其他选项区的情况
	bool RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, RECTLIST& lSelInfo, std::string& strResult);	//通过识别整个题的选项区，来判断选择项

	//计算列表的密度降序结果、列表选项间的密度差值排列
	int calcDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecItemDiff);
	//计算列表的灰度值升序排列结果，选项间的灰度差值排列
	int calcGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsAsc, std::vector<ST_ITEM_DIFF>& vecItemGrayDiff);

	void SharpenImage(const cv::Mat &image, cv::Mat &result, int nSharpKernel);
public:
	CAssistRecog AssistObj;
};

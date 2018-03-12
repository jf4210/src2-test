#pragma once
#include "WritePointRecog.h"
class COmrPoint :
	public CWritePointRecog
{
public:
	COmrPoint() {};
	~COmrPoint() {};
	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);

protected:
	bool RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, OMR_RESULT& omrResult);		//第二种识别OMR的方法
	bool RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, OMR_RESULT& omrResult);		//第三种识别OMR的方法

};

class CElectOmrPoint :
	public CWritePointRecog
{
public:
	CElectOmrPoint() {};
	~CElectOmrPoint() {};
	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);
protected:
	int calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
};
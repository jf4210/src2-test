#pragma once
#include "WritePointRecog.h"
class CSNPoint :
	public CWritePointRecog
{
public:
	CSNPoint() {};
	~CSNPoint() {};
	bool RecogPrintPoint(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);

protected:
	bool RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//通过条码、二维码识别SN
	bool RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//通过OMR识别sz
	int calcSnDensityDiffVal(pSN_ITEM pSn, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemGrayDiff);

	bool RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//第二种识别SN的方法
	bool RecogVal_Sn3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, pSN_ITEM pSn, std::vector<int>& vecItemVal);
};


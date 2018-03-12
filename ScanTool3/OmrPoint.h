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
	bool RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, OMR_RESULT& omrResult);		//�ڶ���ʶ��OMR�ķ���
	bool RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, OMR_RESULT& omrResult);		//������ʶ��OMR�ķ���

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
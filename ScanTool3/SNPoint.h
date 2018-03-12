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
	bool RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//ͨ�����롢��ά��ʶ��SN
	bool RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//ͨ��OMRʶ��sz

	bool RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//�ڶ���ʶ��SN�ķ���
	bool RecogVal_Sn3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, pSN_ITEM pSn, std::vector<int>& vecItemVal);
};


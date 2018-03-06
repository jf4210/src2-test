#pragma once
#include "global.h"

class CPaperRecog
{
public:
	CPaperRecog();
	~CPaperRecog();


	bool RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);		//ʶ�𶨵�
	bool RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);		//ʶ��ˮƽͬ��ͷ
	bool RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);		//ʶ��ֱͬ��ͷ
	bool RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);		//ʶ�����
	bool RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);		//ʶ���Ŀ
	bool RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);			//ʶ��ȱ��
	bool RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);			//ʶ��Υ��
	bool RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);		//ʶ��Ҷ�У���
	bool RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);		//ʶ��հ�У���
	bool RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);			//ʶ��׼��֤��
	bool RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);			//ʶ��OMR��Ϣ
	bool RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, int nRecogMode, std::string& strLog);		//ʶ��ѡ����OMR��Ϣ

protected:
	inline bool Recog(RECTINFO& rc, cv::Mat& matCompPic, std::string& strLog);
	inline bool Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//��ʶ���ѡ���м�����������ѡȡ����һ������������ʶ�𣬴˷������ڷ�ֹ���ֿ�ѡ������ѡ���������
	inline bool RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, RECTLIST& lSelInfo, std::string& strResult);	//ͨ��ʶ���������ѡ���������ж�ѡ����

	bool RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//ͨ�����롢��ά��ʶ��SN
	bool RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, std::string& strLog);		//ͨ��OMRʶ��sz

	int calcSnDensityDiffVal(pSN_ITEM pSn, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemGrayDiff);

	bool RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//�ڶ���ʶ��SN�ķ���
	bool RecogVal_Sn3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, pSN_ITEM pSn, std::vector<int>& vecItemVal);
	bool RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, OMR_RESULT& omrResult);		//�ڶ���ʶ��OMR�ķ���
	bool RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel, OMR_RESULT& omrResult);		//������ʶ��OMR�ķ���
private:
	std::string _strLog;
	std::vector<RECTINFO> m_vecH_Head;		//ͬ��ͷģʽ��ˮƽͬ��ͷ
	std::vector<RECTINFO> m_vecV_Head;		//ͬ��ͷģʽ����ֱͬ��ͷ
};


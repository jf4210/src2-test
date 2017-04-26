#pragma once
#include "global.h"

typedef struct _ModelInfo_
{
	pMODEL	pModel;
	cv::Mat matSrc;
	std::vector<cv::Mat> vecMatSrc;
}MODELINFO,*pMODELINFO;
class CRecognizeThread : public Poco::Runnable
{
public:
	CRecognizeThread();
	~CRecognizeThread();

	virtual void run();

	bool HandleTask(pRECOGTASK pTask);

	bool LoadModel(pMODELINFO pModelInfo);

	void PaperRecognise(pST_PaperInfo pPaper, pMODELINFO pModelInfo);

	void sharpenImage1(const cv::Mat &image, cv::Mat &result);
	inline bool Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);
	inline bool Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//��ʶ���ѡ���м�����������ѡȡ����һ������������ʶ�𣬴˷������ڷ�ֹ���ֿ�ѡ������ѡ���������
	inline bool RecogVal(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);
	inline bool RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, RECTLIST& lSelInfo, std::string& strResult);	//ͨ��ʶ���������ѡ���������ж�ѡ����

	bool RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//�ڶ���ʶ��SN�ķ���
	bool RecogVal_Sn3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//������ʶ��SN�ķ���
	bool RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult);		//�ڶ���ʶ��OMR�ķ���
	bool RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult);		//������ʶ��OMR�ķ���
	bool RecogVal_ElectOmr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pELECTOMR_QUESTION pOmrQuestion, OMR_RESULT& omrResult);		//�ڶ���ʶ��ѡ����OMR�ķ���

	bool RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ��ˮƽͬ��ͷ
	bool RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ��ˮƽͬ��ͷ
	bool RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//ʶ��ֱͬ��ͷ
	bool RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ�����
	bool RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ���Ŀ
	bool RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//ʶ��ȱ��
	bool RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ��Ҷ�У���
	bool RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ��հ�У���
	bool RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//ʶ��׼��֤��
	bool RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//ʶ��OMR��Ϣ
	bool RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ��ѡ����OMR��Ϣ

	bool RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);	//ͨ�����롢��ά��ʶ��SN
	bool RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ͨ��OMRʶ��sz

//	int FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix);		//������з���任

	int calcSnDensityDiffVal(pSN_ITEM pSn, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);

	void HandleWithErrPaper(pST_PaperInfo pPaper);	//����ʶ�������Ծ�

	Poco::Event		eExit;
private:
	std::map<pMODEL, pMODELINFO> _mapModel;		//���߳��Ѿ���ȡ��ģ���б���ֹͬ����ģ���ζ�ȡ�˷�ʱ��
	cv::Point m_ptFixCP;
	std::vector<std::vector<cv::Point>> m_vecContours;
	std::vector<RECTINFO> m_vecH_Head;
	std::vector<RECTINFO> m_vecV_Head;
};


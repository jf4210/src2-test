#pragma once
#include "DMTDef.h"
#include "OmrRecog.h"

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

	inline bool Recog(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);
	inline bool Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//��ʶ���ѡ���м�����������ѡȡ����һ������������ʶ�𣬴˷������ڷ�ֹ���ֿ�ѡ������ѡ���������
	inline bool RecogVal(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);
	inline bool RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, RECTLIST& lSelInfo, std::string& strResult);	//ͨ��ʶ���������ѡ���������ж�ѡ����

	bool AutoContractBright(cv::Mat& matComPic);

	bool RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//�ڶ���ʶ��SN�ķ���
	bool RecogVal_Sn3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//������ʶ��SN�ķ���
	bool RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult);		//�ڶ���ʶ��OMR�ķ���
	bool RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult);		//������ʶ��OMR�ķ���

	bool ChkPicRotation(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, std::string& strLog);
	bool RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, int nRecogMode, std::string& strLog);		//ʶ�𶨵㣬nRecogMode��2-�ϸ�ģʽ��1-��ģʽ
	bool RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ��ˮƽͬ��ͷ
	bool RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//ʶ��ֱͬ��ͷ
	bool RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//ʶ�����
	bool RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, int nRecogMode, std::string& strLog);		//ʶ���Ŀ��nRecogMode��2-�ϸ�ģʽ��1-��ģʽ
	bool RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, int nRecogMode, std::string& strLog);			//ʶ��ȱ����nRecogMode��2-�ϸ�ģʽ��1-��ģʽ
	bool RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, int nRecogMode, std::string& strLog);			//ʶ��Υ�ͣ�nRecogMode��2-�ϸ�ģʽ��1-��ģʽ
	bool RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, int nRecogMode, std::string& strLog);		//ʶ��Ҷ�У��㣬nRecogMode��2-�ϸ�ģʽ��1-��ģʽ
	bool RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, int nRecogMode, std::string& strLog);		//ʶ��հ�У��㣬nRecogMode��2-�ϸ�ģʽ��1-��ģʽ
	bool RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, std::string& strLog);			//ʶ��׼��֤��
	bool RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, std::string& strLog);			//ʶ��OMR��Ϣ
	bool RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, std::string& strLog);		//ʶ��ѡ����OMR��Ϣ

	bool RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, std::string& strLog);	//ͨ�����롢��ά��ʶ��SN
	bool RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, std::string& strLog);		//ͨ��OMRʶ��sz

//	int FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix);		//������з���任


	void HandleWithErrPaper(pST_PaperInfo pPaper);	//����ʶ�������Ծ�

	int calcSnDensityDiffVal(pSN_ITEM pSn, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcSnGrayDiffVal(pSN_ITEM pSn, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	Poco::Event		eExit;
private:
	Poco::FastMutex	_mapModelLock;
	std::map<pMODEL, pMODELINFO> _mapModel;		//���߳��Ѿ���ȡ��ģ���б���ֹͬ����ģ���ζ�ȡ�˷�ʱ��
	std::vector<std::vector<cv::Point>> m_vecContours;
	std::vector<RECTINFO> m_vecH_Head;
	std::vector<RECTINFO> m_vecV_Head;

	int		m_nContract;	//�Աȶ�
	int		m_nBright;		//����

	COmrRecog	_chkRotationObj;
};


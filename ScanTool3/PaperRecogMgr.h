#pragma once
#include "PrintPointRecog.h"
#include "WritePointRecog.h"
class CPaperRecogMgr
{
public:
	CPaperRecogMgr(int nRecogModel);
	~CPaperRecogMgr();

	bool RecogCharacter(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);	//ʶ������
	bool RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//ʶ��ˮƽͬ��ͷ
// 	bool RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//ʶ��ˮƽͬ��ͷ
// 	bool RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//ʶ��ֱͬ��ͷ
	bool RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//ʶ�����
	bool RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//ʶ���Ŀ
	bool RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);			//ʶ��ȱ��
	bool RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);			//ʶ��Υ��
	bool RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//ʶ��Ҷ�У���
	bool RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//ʶ��հ�У���
	bool RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);			//ʶ��׼��֤��
	bool RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);			//ʶ��OMR��Ϣ
	bool RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//ʶ��ѡ����OMR��Ϣ

	std::string GetLog();
	void SetTesseractObj(tesseract::TessBaseAPI* pTess);
private:
	CPrintPointRecog* pPrintPointRecogObj;	//ӡˢ��ʶ�����
	CWritePointRecog* pWritePointRecogObj;	//��Ϳ��ʶ�����
	int		_nRecogModel;
	std::string _strLog;
#ifdef USE_TESSERACT
	tesseract::TessBaseAPI* m_pTess;
#endif
};


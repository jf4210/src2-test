#pragma once
#include "PrintPointRecog.h"
#include "WritePointRecog.h"
class CPaperRecogMgr
{
public:
	CPaperRecogMgr(int nRecogModel);
	~CPaperRecogMgr();
	
	//识别整份试卷，包含所有试卷页的识别
	//bool bMustRecog: 强制识别
	bool RecogPaper(pST_PaperInfo pPaper, pMODEL pModel, bool bMustRecog = false);
	bool RecogPic(int nPic, pST_PicInfo pPic, pMODEL pModel);

	//图像识别步骤
	//==============================================================================================================
	bool InitPic(pST_PicInfo pPic, cv::Mat& matCompSrcPic);
	bool RecogCharacter(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);	//识别文字
	bool RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//识别水平同步头
// 	bool RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//识别水平同步头
// 	bool RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//识别垂直同步头
	bool RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//识别卷型
	bool RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//识别科目
	bool RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);			//识别缺考
	bool RecogWJCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);			//识别违纪
	bool RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//识别灰度校验点
	bool RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//识别空白校验点
	bool RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);			//识别准考证号
	bool RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);			//识别OMR信息
	bool RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODEL pModel);		//识别选做题OMR信息
	//==============================================================================================================


	std::string GetLog();
	void SetTesseractObj(tesseract::TessBaseAPI* pTess);
protected:
	void ClearPicRecogData(pST_PicInfo pPic);
	void ClearPaperRecogData(pST_PaperInfo pPaper);
private:
	CPrintPointRecog* pPrintPointRecogObj;	//印刷点识别对象
	CWritePointRecog* pWritePointRecogObj;	//填涂点识别对象
	int		_nRecogModel;
	std::string _strLog;
#ifdef USE_TESSERACT
	tesseract::TessBaseAPI* m_pTess;
#endif
};


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
	inline bool Recog2(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//先识别此选项有几个矩形区，选取最大的一个矩形区进行识别，此方法用于防止出现框选到其他选项区的情况
	inline bool RecogVal(int nPic, RECTINFO& rc, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);
	inline bool RecogVal2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, RECTLIST& lSelInfo, std::string& strResult);	//通过识别整个题的选项区，来判断选择项

	bool RecogVal_Sn2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//第二种识别SN的方法
	bool RecogVal_Sn3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, pSN_ITEM pSn, std::vector<int>& vecItemVal);		//第三种识别SN的方法
	bool RecogVal_Omr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult);		//第二种识别OMR的方法
	bool RecogVal_Omr3(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo, OMR_RESULT& omrResult);		//第三种识别OMR的方法
	bool RecogVal_ElectOmr2(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pELECTOMR_QUESTION pOmrQuestion, OMR_RESULT& omrResult);		//第二种识别选做题OMR的方法

	bool RecogFixCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//识别水平同步头
	bool RecogHHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//识别水平同步头
	bool RecogVHead(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//识别垂直同步头
	bool RecogABModel(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//识别卷型
	bool RecogCourse(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//识别科目
	bool RecogQKCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//识别缺考
	bool RecogGrayCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//识别灰度校验点
	bool RecogWhiteCP(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//识别空白校验点
	bool RecogSN(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//识别准考证号
	bool RecogOMR(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);			//识别OMR信息
	bool RecogElectOmr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//识别选做题OMR信息

	bool RecogSn_code(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);	//通过条码、二维码识别SN
	bool RecogSn_omr(int nPic, cv::Mat& matCompPic, pST_PicInfo pPic, pMODELINFO pModelInfo);		//通过OMR识别sz

//	int FixWarpAffine(int nPic, cv::Mat& matCompPic, RECTLIST& lFix, RECTLIST& lModelFix);		//定点进行仿射变换

	int calcSnDensityDiffVal(pSN_ITEM pSn, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrDensityDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);
	int calcOmrGrayDiffVal(RECTLIST& rectList, std::vector<pRECTINFO>& vecItemsDesc, std::vector<ST_ITEM_DIFF>& vecOmrItemDiff);

	void HandleWithErrPaper(pST_PaperInfo pPaper);	//处理识别错误的试卷

	Poco::Event		eExit;
private:
	std::map<pMODEL, pMODELINFO> _mapModel;		//本线程已经读取的模板列表，防止同样的模板多次读取浪费时间
	cv::Point m_ptFixCP;
	std::vector<std::vector<cv::Point>> m_vecContours;
	std::vector<RECTINFO> m_vecH_Head;
	std::vector<RECTINFO> m_vecV_Head;
};


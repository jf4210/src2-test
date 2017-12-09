#pragma once
#include "global.h"
#include "PicShow.h"
#include "MyCodeConvert.h"
#include "RecogInfoDlg.h"
#include "OmrInfoDlg.h"
//#include "TwainCpp.h"
#include "SNInfoSetDlg.h"
#include "ElectOmrDlg.h"
#include "ZipObj.h"
#include "ScanThread.h"
#include "ModelInfoDlg.h"
// CMakeModelDlg 对话框

typedef struct _ProjectionInfo_
{
	int		nStartIndex;	//原图上的起始点
	int		nEndIndex;		//原图上的结束点
	int		nArea;			//点的面积
//	cv::Mat matProjection;	
}ST_PROJECTION, *pST_PROJECTION;
typedef std::vector<ST_PROJECTION> VEC_PROJECT;

typedef struct _PaperModelInfo_
{
	bool bFirstH;		//第一次展示水平同步头
	bool bFirstV;		//第一次展示垂直同步头
	int nPaper;			//第几张试卷
	int			nPicW;					//图片宽
	int			nPicH;					//图片高

	int		nDirection;			//图像默认显示方向，1:针对原始图像需要进行的旋转，正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
	int		nRotateTimes;		//记录往左往右旋转次数

	std::string	strModelPicName;	//模板图片的名称
	CString		strModelPicPath;	//模板图片路径
	cv::Mat		matSrcImg;			//原始图像的Mat
	cv::Mat		matDstImg;			//显示图像的Mat
	cv::Rect	rtHTracker;
	cv::Rect	rtVTracker;
//	cv::Rect	rtSNTracker;
	RECTINFO	rcSNTracker;
	SNLIST		lSN;
	std::vector<RECTINFO> vecHTracker;	//水平橡皮筋区域
	std::vector<RECTINFO> vecVTracker;	//垂直橡皮筋区域
	std::vector<RECTINFO> vecRtSel;				//存储选择的矩形,框的大矩形，用来框定点
	std::vector<RECTINFO> vecRtFix;				//存储定点矩形
	std::vector<RECTINFO>	vecH_Head;				//水平校验点列表
	std::vector<RECTINFO>	vecV_Head;				//垂直同步头列表
	std::vector<RECTINFO>	vecABModel;				//卷型校验点
	std::vector<RECTINFO>	vecCourse;				//科目校验点
	std::vector<RECTINFO>	vecQK_CP;				//缺考校验点
	std::vector<RECTINFO>	vecWJ_CP;				//违纪校验点
	std::vector<RECTINFO>	vecGray;				//灰度校验点
	std::vector<RECTINFO>	vecWhite;				//空白校验点
	std::vector<RECTINFO>	vecPagination;			//页码
	std::vector<OMR_QUESTION> vecOmr2;
	std::vector<ELECTOMR_QUESTION> vecElectOmr;		//选做题信息
	std::vector<pST_CHARACTER_ANCHOR_AREA> vecCharacterLocation;	//文字定位区域
	_PaperModelInfo_()
	{
		bFirstH = true;
		bFirstV = true;
		nPaper = 0;
		nPicW = -1;
		nPicH = -1;
		nDirection = 1;
		nRotateTimes = 0;
	}
	~_PaperModelInfo_()
	{
		SNLIST::iterator itSn = lSN.begin();
		for (; itSn != lSN.end();)
		{
			pSN_ITEM pSNItem = *itSn;
			itSn = lSN.erase(itSn);
			SAFE_RELEASE(pSNItem);
		}
 		for (int i = 0; i < vecCharacterLocation.size(); i++)		//这里不能释放，从模板加载后只有一份数据，在模板析构时释放
 			SAFE_RELEASE(vecCharacterLocation[i]);
	}
}PaperModelInfo, *pPaperModelInfo;

class CMakeModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CMakeModelDlg)

public:
	CMakeModelDlg(pMODEL pModel = NULL, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMakeModelDlg();

// 对话框数据
	enum { IDD = IDD_MAKEMODELDLG };
public:
	int			m_nModelPicNums;		//模板图片数，即一份模板有多少图片，对应多少试卷
	CTabCtrl	m_tabModelPicCtrl;		//模板图片显示Tab控件
	pMODEL		m_pModel;				//模板信息
	CPicShow*	m_pModelPicShow;		//图片显示控件
	cv::Mat		m_matSrcImg;			//原始图像的Mat
	cv::Mat		m_matDstImg;			//显示图像的Mat
	CComboBox	m_comboCheckPointType;	//校验点类型
	CListCtrl	m_cpListCtrl;			//校验点列表控件
	CRecogInfoDlg*	m_pRecogInfoDlg;	//矩形识别信息的窗口
	COmrInfoDlg*	m_pOmrInfoDlg;		//OMR选项信息设置窗口
	CSNInfoSetDlg*	m_pSNInfoDlg;		//SN信息设置窗口
	ElectOmrDlg*	m_pElectOmrDlg;		//选做题OMR信息设置窗口

// 	CString		m_strCPTypeName;		//校验点的类型名
// 	int			m_nThresholdVal;		//校验点的识别阀值
// 	float		m_fThresholdValPercent;	//校验点达到阀值的比例
	int	m_nFixVal;
	int	m_nHeadVal;				//水平垂直同步头的阀值
	int	m_nABModelVal;
	int	m_nCourseVal;
	int	m_nQK_CPVal;
	int m_nWJ_CPVal;
	int	m_nGrayVal;
	int	m_nWhiteVal;
	int m_nOMR;
	int	m_nSN;
	int m_nCharacterThreshold;
	float m_fFixThresholdPercent;
	float m_fHeadThresholdPercent;	//同步头达到阀值的比例
	float m_fABModelThresholdPercent;
	float m_fCourseThresholdPercent;
	float m_fQK_CPThresholdPercent_Head;	//通过同步头识别
	float m_fWJ_CPThresholdPercent_Head;	//通过同步头识别
	float m_fQK_CPThresholdPercent_Fix;		//通过定点识别（无同步头）
	float m_fWJ_CPThresholdPercent_Fix;		//通过定点识别（无同步头）
	float m_fGrayThresholdPercent;
	float m_fWhiteThresholdPercent;
	float m_fOMRThresholdPercent_Fix;	//通过定点识别（无同步头）
	float m_fSNThresholdPercent_Fix;	//通过定点识别（无同步头）
	float m_fOMRThresholdPercent_Head;	//通过同步头识别
	float m_fSNThresholdPercent_Head;	//通过同步头识别

	float m_fDefPersentFix;
	float m_fDefPersentGray;
	float m_fDefPersentQkWj;
	float m_fDefPersentSN;		//(定点模式)
	float m_fDefPersentOmr;

	int	 m_nCharacterConfidence;		//文字识别的信心阀值

	std::vector<CPicShow*>	m_vecPicShow;	//存储图片显示窗口指针，有多个模板图片时，对应到不同的tab控件页面
	int						m_nCurrTabSel;	//当前Tab控件选择的页面

	std::vector<pPaperModelInfo> m_vecPaperModelInfo;	//存储每张试卷的模板信息

	int	m_nGaussKernel;			//高斯变换核因子
	int m_nSharpKernel;			//锐化核因子
	int m_nThresholdKernel;		//二值化核因子
	int m_nCannyKernel;			//轮廓化核因子
	int m_nDilateKernel;		//膨胀核因子
	int m_nErodeKernel;			//腐蚀核因子

	int m_nDilateKernel_Sn;		//ZKZH识别时用的膨胀因子
	int	m_nDilateKernel_Common;	//通用膨胀核因子，SN时不用
	int m_nDilateKernel_DefSn;		//ZKZH识别时用的膨胀因子，默认值，从配置文件读取
	int	m_nDilateKernel_DefCommon;	//通用膨胀核因子，SN时不用，默认值，从配置文件读取
	int m_nThreshold_DefFix;
	int m_nThreshold_DefGray;
	int m_nThreshold_DefSn;			//默认ZKZH二值化识别阀值
	int m_nThreshold_DefOmr;		//默认OMR二值化识别阀值
	
	CString m_strModelPicPath;	
	HZIP hz;					//压缩

	cv::Point	m_ptRBtnUp;			//右键抬起坐标，用于右键删除图像显示区选中的识别点
	cv::Point	m_ptHTracker1;		//水平橡皮筋的起点tl坐标
	cv::Point	m_ptHTracker2;		//水平橡皮筋的终点br坐标
	cv::Point	m_ptVTracker1;		//垂直橡皮筋的起点tl坐标
	cv::Point	m_ptVTracker2;		//垂直橡皮筋的终点br坐标
	cv::Point	m_ptSNTracker1;		//考号区橡皮筋按下的tl实际坐标
	cv::Point	m_ptSNTracker2;		//考号区橡皮筋按下的br实际坐标
	bool		m_bFistHTracker;	//第一次生成水平橡皮筋
	bool		m_bFistVTracker;	//第一次生成垂直橡皮筋
	bool		m_bFistSNTracker;	//第一次生成SN橡皮筋

	bool		m_bNewModelFlag;	//是否是新创建的模板标识
	bool		m_bSavedModelFlag;	//是否新模板是否已经保存
	CPType		m_eCurCPType;		//当前校验点类型
	int			m_ncomboCurrentSel; //当前校验点的所选项
	int			m_nCurListCtrlSel;	//当前List列表所选的项
	pRECTINFO	m_pCurRectInfo;		//当前点击时选择的识别点，真坐标
	std::vector<std::vector<cv::Point> > m_vecContours;
	std::vector<RECTINFO>	m_vecTmp;	//有同步头时，保存选择的矩形用于显示，真坐标

	int			m_nStartTH;			//添加OMR时的起始题号

	bool		m_bShiftKeyDown;	//shift按键是否按下
#ifdef USE_TESSERACT
	tesseract::TessBaseAPI* m_pTess;
#endif

#ifdef TEST_SCAN_THREAD
	CScanThread m_scanThread;
	CScanThread* m_pScanThread;
#endif
	//扫描
	//----------------------
	CString		m_strScanSavePath;
	CScanThread		m_scanThread;
	pTW_IDENTITY GetScanSrc(int nIndex);
	TwainApp*		_pTWAINApp;
	std::vector<CString> m_vecScanSrc;
	void	InitScanner();

	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);
	//----------------------
// 	BOOL m_bTwainInit;
// 	CString		m_strScanSavePath;
// 	CArray<TW_IDENTITY, TW_IDENTITY> m_scanSourceArry;
// 	BOOL ScanSrcInit();

// 	void CopyImage(HANDLE hBitmap, TW_IMAGEINFO& info);
// 	void SetImage(HANDLE hBitmap, int bits);
// 	void ScanDone(int nStatus);

	void ReInitModel(pMODEL pModel);
	void CreateNewModel(std::vector<MODELPICPATH>& vecPath);
	void SaveNewModel();
public:
	bool RecogNewGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc);							//在修改阀值后重新计算矩形区的灰度值
	void horizontalProjectionMat(cv::Mat srcImg, VEC_PROJECT& vecResult);				//水平投影
	void verticalProjectionMat(cv::Mat srcImg, VEC_PROJECT& vecResult);					//垂直投影  
private:
	void InitUI();
	void InitTab();
	void InitCtrlPosition();
	void InitConf();
	void InitParam();
	bool UploadModel(CString strModelPath, pMODEL pModel);		//上传模板文件

	void UpdataCPList();
	void UpdateCPListByType();		//根据校验点类型更改显示样式
	CPType GetComboSelCpType();
	bool RecogByHead(cv::Rect rtOri);			//通过同步头来识别点, 传入真坐标
	bool RecogCharacterArea(cv::Rect rtOri);	//标题区识别操作，传入真坐标
	bool RecogWhiteAreaGray(cv::Rect rtOri);	//识别空白区灰度，传入真坐标
	bool Recognise(cv::Rect rtOri);				//识别操作，传入真坐标
	inline bool RecogGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc);						//识别灰度值
	
//	bool PicRectify(cv::Mat& src, cv::Mat& dst, cv::Mat& rotMat);						//图片纠偏
	bool PicRotate();							//图像偏移操作
	void sharpenImage1(const cv::Mat &image, cv::Mat &result);		//锐化

	void GetOmrArry(std::vector<cv::Rect>& rcList);	//黑白卡时获取框选的OMR排列数组
	void GetSNArry(std::vector<cv::Rect>& rcList);	//黑白卡时获取框选的考号排列数组
	void GetElectOmrInfo(std::vector<cv::Rect>& rcList);	//黑白卡时获取框选的选做题信息
	void GetStandardValue(RECTINFO& rc);	//获取自建提卡的标准实验值

	int GetRectsInArea(cv::Mat& matSrc, RECTINFO rc, int nMinW, int nMaxW, int nMinH, int nMaxH, int nFindContoursModel = CV_RETR_EXTERNAL);

	inline void GetThreshold(cv::Mat& matSrc, cv::Mat& matDst);			//二值化计算
//	inline void ShowDetailRectInfo();
//	inline int GetStandardVal(CPType eType);
	inline int GetRectInfoByPoint(cv::Point pt, CPType eType, RECTINFO*& pRc);	//传入真坐标
	bool ShowRectByPoint(cv::Point pt);		//传入假坐标
	void ShowRectByItem(int nItem);
	void ShowRectByCPType(CPType eType);
	void ShowTmpRect();
	void SortRect();							//将识别出来的矩形进行坐标排序
	void ShowRectTracker();						//显示橡皮筋类矩形
	void	InitShowSnOmrDlg(CPType eType);		//点击SN或OMR时，显示对于窗口
	LRESULT RoiLBtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT RoiLBtnDown(WPARAM wParam, LPARAM lParam);
	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT HTrackerChange(WPARAM wParam, LPARAM lParam);
	LRESULT VTrackerChange(WPARAM wParam, LPARAM lParam);
	LRESULT SNTrackerChange(WPARAM wParam, LPARAM lParam);

	LRESULT ShiftKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT ShiftKeyUp(WPARAM wParam, LPARAM lParam);

	//给定一个矩形，按给定方向旋转后的矩形
	//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
	//（从真坐标到假坐标）
	cv::Rect GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation);
	cv::Point GetPointByOrientation(cv::Rect& rtPic, cv::Point pt, int nOrientation);
	//（从假坐标到真坐标）
	cv::Rect GetRectToSave(cv::Rect& rtPic, cv::Rect rt, int nOrientation);
	cv::Point GetPointToSave(cv::Rect& rtPic, cv::Point pt, int nOrientation);
	void	RotateImg(cv::Mat& imgMat, int nDirection /*= 1*/);		//1:正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
	//真坐标：实际保存的坐标，原始图像没有旋转的坐标，假坐标：经过旋转后的图像坐标
	cv::Rect GetShowFakePosRect(cv::Rect rt);		//获取显示的假矩形（从真坐标到假坐标）的位置，内部根据方向自动获取矩形新位置
	cv::Point GetShowFakePosPoint(cv::Point pt);	//获取显示的假点（从真坐标到假坐标）的位置，内部根据方向自动获取点新位置
	cv::Rect GetSrcSaveRect(cv::Rect rt);		//获取保存时原始矩形，（从假坐标到真坐标）
	cv::Point GetSrcSavePoint(cv::Point pt);	//获取保存时原始坐标，（从假坐标到真坐标）
	//获取考号显示方向标识
	int GetSnShowFakePosFlag(int nRecogFlag);	//从真实图像方向获取显示的图像方向标识（从真坐标到假坐标）
	int GetSnSavePosFlag(int nRecogFlag);		//从显示的图像标识获取到真实保存的图像标识（从假坐标到真坐标）
	int GetOmrShowFakePosFlag(int nRecogFlag);	//从真实图像方向获取显示的图像方向标识（从真坐标到假坐标）
	int GetOmrSavePosFlag(int nRecogFlag);		//从显示的图像标识获取到真实保存的图像标识（从假坐标到真坐标）

	void	LeftRotate();
	void	RightRotate();

	inline bool checkOverlap(CPType eType, cv::Rect rtSrc);		//区域重叠检测，传入真坐标

	bool checkValidity();						//保存模板前合法性检查
	bool SaveModelFile(pMODEL pModel);			//保存模板到文件
	void setUploadModelInfo(CString& strName, CString& strModelPath, int nExamId, int nSubjectId);					//设置上传模板的信息

	void DelRectInfoOnPic();					//删除图像控件上的选中识别点
	void DeleteRectInfoOnList();				//删除List列表选中的识别点
	BOOL DeleteRectInfo(CPType eType, int nItem);				//删除选中的识别点
	void RecognizeRectTracker();				//识别橡皮筋区域
	void AddRecogRectToList();					//添加识别出来的区域到对应的列表中，针对有同步头的情况
	void AddRecogSN();							//添加考号识别区域

	void RecogFixWithHead(int i);				//在同步头模式，识别定点信息

#ifdef TEST_SCAN_THREAD
	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog(); 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnSelpic();
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedBtnExitmodeldlg();
	afx_msg void OnBnClickedBtnNew();
	afx_msg void OnTcnSelchangeTabModelpic(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeComboCptype();
	afx_msg void OnNMRClickListCheckpoint(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListCheckpoint(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownListCheckpoint(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnBnClickedBtnSaverecoginfo();
//	afx_msg void OnBnClickedBtnuploadmodel();
	afx_msg void OnBnClickedBtnScanmodel();
	afx_msg void OnDestroy();
	afx_msg void OnNMHoverListCheckpoint(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnAdvancedsetting();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

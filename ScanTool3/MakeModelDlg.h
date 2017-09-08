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
// CMakeModelDlg �Ի���

typedef struct _ProjectionInfo_
{
	int		nStartIndex;	//ԭͼ�ϵ���ʼ��
	int		nEndIndex;		//ԭͼ�ϵĽ�����
	int		nArea;			//������
//	cv::Mat matProjection;	
}ST_PROJECTION, *pST_PROJECTION;
typedef std::vector<ST_PROJECTION> VEC_PROJECT;

typedef struct _PaperModelInfo_
{
	bool bFirstH;		//��һ��չʾˮƽͬ��ͷ
	bool bFirstV;		//��һ��չʾ��ֱͬ��ͷ
	int nPaper;			//�ڼ����Ծ�
	int			nPicW;					//ͼƬ��
	int			nPicH;					//ͼƬ��

	std::string	strModelPicName;	//ģ��ͼƬ������
	CString		strModelPicPath;	//ģ��ͼƬ·��
	cv::Mat		matSrcImg;			//ԭʼͼ���Mat
	cv::Mat		matDstImg;			//��ʾͼ���Mat
	cv::Rect	rtHTracker;
	cv::Rect	rtVTracker;
//	cv::Rect	rtSNTracker;
	RECTINFO	rcSNTracker;
	SNLIST		lSN;
	std::vector<RECTINFO> vecHTracker;	//ˮƽ��Ƥ������
	std::vector<RECTINFO> vecVTracker;	//��ֱ��Ƥ������
	std::vector<RECTINFO> vecRtSel;				//�洢ѡ��ľ���,��Ĵ���Σ������򶨵�
	std::vector<RECTINFO> vecRtFix;				//�洢�������
	std::vector<RECTINFO>	vecH_Head;				//ˮƽУ����б�
	std::vector<RECTINFO>	vecV_Head;				//��ֱͬ��ͷ�б�
	std::vector<RECTINFO>	vecABModel;				//����У���
	std::vector<RECTINFO>	vecCourse;				//��ĿУ���
	std::vector<RECTINFO>	vecQK_CP;				//ȱ��У���
	std::vector<RECTINFO>	vecWJ_CP;				//Υ��У���
	std::vector<RECTINFO>	vecGray;				//�Ҷ�У���
	std::vector<RECTINFO>	vecWhite;				//�հ�У���
	std::vector<OMR_QUESTION> vecOmr2;
	std::vector<ELECTOMR_QUESTION> vecElectOmr;		//ѡ������Ϣ
	std::vector<pST_CHARACTER_ANCHOR_AREA> vecCharacterLocation;	//���ֶ�λ����
	_PaperModelInfo_()
	{
		bFirstH = true;
		bFirstV = true;
		nPaper = 0;
		nPicW = -1;
		nPicH = -1;
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
// 		for (int i = 0; i < vecCharacterLocation.size(); i++)		//���ﲻ���ͷţ���ģ����غ�ֻ��һ�����ݣ���ģ������ʱ�ͷ�
// 			SAFE_RELEASE(vecCharacterLocation[i]);
	}
}PaperModelInfo, *pPaperModelInfo;

class CMakeModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CMakeModelDlg)

public:
	CMakeModelDlg(pMODEL pModel = NULL, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMakeModelDlg();

// �Ի�������
	enum { IDD = IDD_MAKEMODELDLG };
public:
	int			m_nModelPicNums;		//ģ��ͼƬ������һ��ģ���ж���ͼƬ����Ӧ�����Ծ�
	CTabCtrl	m_tabModelPicCtrl;		//ģ��ͼƬ��ʾTab�ؼ�
	pMODEL		m_pModel;				//ģ����Ϣ
	CPicShow*	m_pModelPicShow;		//ͼƬ��ʾ�ؼ�
	cv::Mat		m_matSrcImg;			//ԭʼͼ���Mat
	cv::Mat		m_matDstImg;			//��ʾͼ���Mat
	CComboBox	m_comboCheckPointType;	//У�������
	CListCtrl	m_cpListCtrl;			//У����б�ؼ�
	CRecogInfoDlg*	m_pRecogInfoDlg;	//����ʶ����Ϣ�Ĵ���
	COmrInfoDlg*	m_pOmrInfoDlg;		//OMRѡ����Ϣ���ô���
	CSNInfoSetDlg*	m_pSNInfoDlg;		//SN��Ϣ���ô���
	ElectOmrDlg*	m_pElectOmrDlg;		//ѡ����OMR��Ϣ���ô���

// 	CString		m_strCPTypeName;		//У����������
// 	int			m_nThresholdVal;		//У����ʶ��ֵ
// 	float		m_fThresholdValPercent;	//У���ﵽ��ֵ�ı���
	int	m_nFixVal;
	int	m_nHeadVal;				//ˮƽ��ֱͬ��ͷ�ķ�ֵ
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
	float m_fHeadThresholdPercent;	//ͬ��ͷ�ﵽ��ֵ�ı���
	float m_fABModelThresholdPercent;
	float m_fCourseThresholdPercent;
	float m_fQK_CPThresholdPercent_Head;	//ͨ��ͬ��ͷʶ��
	float m_fWJ_CPThresholdPercent_Head;	//ͨ��ͬ��ͷʶ��
	float m_fQK_CPThresholdPercent_Fix;		//ͨ������ʶ����ͬ��ͷ��
	float m_fWJ_CPThresholdPercent_Fix;		//ͨ������ʶ����ͬ��ͷ��
	float m_fGrayThresholdPercent;
	float m_fWhiteThresholdPercent;
	float m_fOMRThresholdPercent_Fix;	//ͨ������ʶ����ͬ��ͷ��
	float m_fSNThresholdPercent_Fix;	//ͨ������ʶ����ͬ��ͷ��
	float m_fOMRThresholdPercent_Head;	//ͨ��ͬ��ͷʶ��
	float m_fSNThresholdPercent_Head;	//ͨ��ͬ��ͷʶ��

	std::vector<CPicShow*>	m_vecPicShow;	//�洢ͼƬ��ʾ����ָ�룬�ж��ģ��ͼƬʱ����Ӧ����ͬ��tab�ؼ�ҳ��
	int						m_nCurrTabSel;	//��ǰTab�ؼ�ѡ���ҳ��

	std::vector<pPaperModelInfo> m_vecPaperModelInfo;	//�洢ÿ���Ծ��ģ����Ϣ

	int	m_nGaussKernel;			//��˹�任������
	int m_nSharpKernel;			//�񻯺�����
	int m_nThresholdKernel;		//��ֵ��������
	int m_nCannyKernel;			//������������
	int m_nDilateKernel;		//���ͺ�����
	int m_nErodeKernel;			//��ʴ������

	int m_nDilateKernel_Sn;		//ZKZHʶ��ʱ�õ���������
	int	m_nDilateKernel_Common;	//ͨ�����ͺ����ӣ�SNʱ����
	int m_nDilateKernel_DefSn;		//ZKZHʶ��ʱ�õ��������ӣ�Ĭ��ֵ���������ļ���ȡ
	int	m_nDilateKernel_DefCommon;	//ͨ�����ͺ����ӣ�SNʱ���ã�Ĭ��ֵ���������ļ���ȡ

	CString m_strModelPicPath;	
	HZIP hz;					//ѹ��

	cv::Point	m_ptRBtnUp;			//�Ҽ�̧�����꣬�����Ҽ�ɾ��ͼ����ʾ��ѡ�е�ʶ���
	cv::Point	m_ptHTracker1;		//ˮƽ��Ƥ������tl����
	cv::Point	m_ptHTracker2;		//ˮƽ��Ƥ����յ�br����
	cv::Point	m_ptVTracker1;		//��ֱ��Ƥ������tl����
	cv::Point	m_ptVTracker2;		//��ֱ��Ƥ����յ�br����
	cv::Point	m_ptSNTracker1;		//��������Ƥ��µ�tlʵ������
	cv::Point	m_ptSNTracker2;		//��������Ƥ��µ�brʵ������
	bool		m_bFistHTracker;	//��һ������ˮƽ��Ƥ��
	bool		m_bFistVTracker;	//��һ�����ɴ�ֱ��Ƥ��
	bool		m_bFistSNTracker;	//��һ������SN��Ƥ��

	bool		m_bNewModelFlag;	//�Ƿ����´�����ģ���ʶ
	bool		m_bSavedModelFlag;	//�Ƿ���ģ���Ƿ��Ѿ�����
	CPType		m_eCurCPType;		//��ǰУ�������
	int			m_ncomboCurrentSel; //��ǰУ������ѡ��
	int			m_nCurListCtrlSel;	//��ǰList�б���ѡ����
	pRECTINFO	m_pCurRectInfo;		//��ǰ���ʱѡ���ʶ���
	cv::Point	m_ptFixCP;			//���������
	std::vector<std::vector<cv::Point> > m_vecContours;
	std::vector<RECTINFO>	m_vecTmp;	//��ͬ��ͷʱ������ѡ��ľ���������ʾ

	int			m_nStartTH;			//���OMRʱ����ʼ���

	bool		m_bShiftKeyDown;	//shift�����Ƿ���
#ifdef USE_TESSERACT
	tesseract::TessBaseAPI* m_pTess;
#endif

#ifdef TEST_SCAN_THREAD
	CScanThread m_scanThread;
	CScanThread* m_pScanThread;
#endif
	//ɨ��
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
	bool RecogNewGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc);							//���޸ķ�ֵ�����¼���������ĻҶ�ֵ
	void horizontalProjectionMat(cv::Mat srcImg, VEC_PROJECT& vecResult);				//ˮƽͶӰ
	void verticalProjectionMat(cv::Mat srcImg, VEC_PROJECT& vecResult);					//��ֱͶӰ  
private:
	void InitUI();
	void InitTab();
	void InitCtrlPosition();
	void InitConf();
	void InitParam();
	bool UploadModel(CString strModelPath, pMODEL pModel);		//�ϴ�ģ���ļ�

	void UpdataCPList();
	void UpdateCPListByType();		//����У������͸�����ʾ��ʽ
	CPType GetComboSelCpType();
	bool RecogByHead(cv::Rect rtOri);			//ͨ��ͬ��ͷ��ʶ���
	bool RecogCharacterArea(cv::Rect rtOri);			//������ʶ�����
	bool Recognise(cv::Rect rtOri);				//ʶ�����
	inline bool RecogGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc);						//ʶ��Ҷ�ֵ
	
//	bool PicRectify(cv::Mat& src, cv::Mat& dst, cv::Mat& rotMat);						//ͼƬ��ƫ
	bool PicRotate();							//ͼ��ƫ�Ʋ���
	void sharpenImage1(const cv::Mat &image, cv::Mat &result);		//��

	void GetOmrArry(std::vector<cv::Rect>& rcList);	//�ڰ׿�ʱ��ȡ��ѡ��OMR��������
	void GetSNArry(std::vector<cv::Rect>& rcList);	//�ڰ׿�ʱ��ȡ��ѡ�Ŀ�����������
	void GetElectOmrInfo(std::vector<cv::Rect>& rcList);	//�ڰ׿�ʱ��ȡ��ѡ��ѡ������Ϣ
	void GetStandardValue(RECTINFO& rc);	//��ȡ�Խ��Ῠ�ı�׼ʵ��ֵ

	inline void GetThreshold(cv::Mat& matSrc, cv::Mat& matDst);			//��ֵ������
//	inline void ShowDetailRectInfo();
	inline int GetStandardVal(CPType eType);
	inline int GetRectInfoByPoint(cv::Point pt, CPType eType, RECTINFO*& pRc);
	bool ShowRectByPoint(cv::Point pt);
	void ShowRectByItem(int nItem);
	void ShowRectByCPType(CPType eType);
	void ShowTmpRect();
	void SortRect();							//��ʶ������ľ��ν�����������
	void ShowRectTracker();						//��ʾ��Ƥ�������
	void	InitShowSnOmrDlg(CPType eType);		//���SN��OMRʱ����ʾ���ڴ���
	LRESULT RoiLBtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT RoiLBtnDown(WPARAM wParam, LPARAM lParam);
	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT HTrackerChange(WPARAM wParam, LPARAM lParam);
	LRESULT VTrackerChange(WPARAM wParam, LPARAM lParam);
	LRESULT SNTrackerChange(WPARAM wParam, LPARAM lParam);

	LRESULT ShiftKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT ShiftKeyUp(WPARAM wParam, LPARAM lParam);

	inline bool checkOverlap(CPType eType, cv::Rect rtSrc);		//�����ص����

	bool checkValidity();						//����ģ��ǰ�Ϸ��Լ��
	bool SaveModelFile(pMODEL pModel);			//����ģ�嵽�ļ�
	void setUploadModelInfo(CString& strName, CString& strModelPath, int nExamId, int nSubjectId);					//�����ϴ�ģ�����Ϣ

	void DelRectInfoOnPic();					//ɾ��ͼ��ؼ��ϵ�ѡ��ʶ���
	void DeleteRectInfoOnList();				//ɾ��List�б�ѡ�е�ʶ���
	BOOL DeleteRectInfo(CPType eType, int nItem);				//ɾ��ѡ�е�ʶ���
	void RecognizeRectTracker();				//ʶ����Ƥ������
	void AddRecogRectToList();					//���ʶ����������򵽶�Ӧ���б��У������ͬ��ͷ�����
	void AddRecogSN();							//��ӿ���ʶ������

	void RecogFixWithHead(int i);				//��ͬ��ͷģʽ��ʶ�𶨵���Ϣ

#ifdef TEST_SCAN_THREAD
	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
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

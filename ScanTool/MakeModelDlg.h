#pragma once
#include "global.h"
#include "PicShow.h"
#include "MyCodeConvert.h"
#include "RecogInfoDlg.h"
#include "OmrInfoDlg.h"
#include "TwainCpp.h"
// CMakeModelDlg �Ի���


typedef struct 
{
	int nPaper;			//�ڼ����Ծ�
	CString		strModelPicName;	//ģ��ͼƬ������
	CString		strModelPicPath;	//ģ��ͼƬ·��
	cv::Mat		matSrcImg;			//ԭʼͼ���Mat
	cv::Mat		matDstImg;			//��ʾͼ���Mat
	std::vector<RECTINFO> vecHTracker;	//ˮƽ��Ƥ������
	std::vector<RECTINFO> vecVTracker;	//��ֱ��Ƥ������
	std::vector<RECTINFO> vecRtSel;				//�洢ѡ��ľ���,��Ĵ���Σ������򶨵�
	std::vector<RECTINFO> vecRtFix;				//�洢�������
	std::vector<RECTINFO> vecOmr;				//�洢ʶ������Ĵ𰸾���
	std::vector<OMR_QUESTION> vecOmr2;
	std::vector<RECTINFO>	vecH_Head;				//ˮƽУ����б�
	std::vector<RECTINFO>	vecV_Head;				//��ֱͬ��ͷ�б�
	std::vector<RECTINFO>	vecABModel;				//����У���
	std::vector<RECTINFO>	vecCourse;				//��ĿУ���
	std::vector<RECTINFO>	vecQK_CP;				//ȱ��У���
	std::vector<RECTINFO>	vecGray;				//�Ҷ�У���
	std::vector<RECTINFO>	vecWhite;				//�հ�У���
}PaperModelInfo, *pPaperModelInfo;

class CMakeModelDlg : public CDialog, public CTwain
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


// 	CString		m_strCPTypeName;		//У����������
// 	int			m_nThresholdVal;		//У����ʶ��ֵ
// 	float		m_fThresholdValPercent;	//У���ﵽ��ֵ�ı���
	const int	m_nFixVal;
	const int	m_nHeadVal;				//ˮƽ��ֱͬ��ͷ�ķ�ֵ
	const int	m_nABModelVal;
	const int	m_nCourseVal;
	const int	m_nQK_CPVal;
	const int	m_nGrayVal;
	const int	m_nWhiteVal;
	const int   m_nOMR;
	const float m_fFixThresholdPercent;
	const float m_fHeadThresholdPercent;	//ͬ��ͷ�ﵽ��ֵ�ı���
	const float m_fABModelThresholdPercent;
	const float m_fCourseThresholdPercent;
	const float m_fQK_CPThresholdPercent;
	const float m_fGrayThresholdPercent;
	const float m_fWhiteThresholdPercent;
	const float m_fOMRThresholdPercent;

	std::vector<CPicShow*>	m_vecPicShow;	//�洢ͼƬ��ʾ����ָ�룬�ж��ģ��ͼƬʱ����Ӧ����ͬ��tab�ؼ�ҳ��
	int						m_nCurrTabSel;	//��ǰTab�ؼ�ѡ���ҳ��

	std::vector<pPaperModelInfo> m_vecPaperModelInfo;	//�洢ÿ���Ծ��ģ����Ϣ

	int	m_nGaussKernel;			//��˹�任������
	int m_nSharpKernel;			//�񻯺�����
	int m_nThresholdKernel;		//��ֵ��������
	int m_nCannyKernel;			//������������
	int m_nDelateKernel;		//���ͺ�����
	int m_nErodeKernel;			//��ʴ������

	CString m_strModelPicPath;	
	HZIP hz;					//ѹ��

	cv::Point	m_ptRBtnUp;			//�Ҽ�̧�����꣬�����Ҽ�ɾ��ͼ����ʾ��ѡ�е�ʶ���
	cv::Point	m_ptHTracker1;		//ˮƽ��Ƥ������tl����
	cv::Point	m_ptHTracker2;		//ˮƽ��Ƥ����յ�br����
	cv::Point	m_ptVTracker1;		//��ֱ��Ƥ������tl����
	cv::Point	m_ptVTracker2;		//��ֱ��Ƥ����յ�br����
	bool		m_bFistHTracker;	//��һ������ˮƽ��Ƥ��
	bool		m_bFistVTracker;	//��һ�����ɴ�ֱ��Ƥ��

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

	//ɨ��
	BOOL m_bTwainInit;
	CString		m_strScanSavePath;
	CArray<TW_IDENTITY, TW_IDENTITY> m_scanSourceArry;
	bool	ScanSrcInit();

	void CopyImage(HANDLE hBitmap, TW_IMAGEINFO& info);
	void SetImage(HANDLE hBitmap, int bits);
	void ScanDone(int nStatus);
public:
	bool RecogNewGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc);							//���޸ķ�ֵ�����¼���������ĻҶ�ֵ

private:
	void InitUI();
	void InitTab();
	void InitCtrlPosition();
	void InitConf();
	void UpdataCPList();
	CPType GetComboSelCpType();
	bool RecogByHead(cv::Rect rtOri);			//ͨ��ͬ��ͷ��ʶ���
	bool Recognise(cv::Rect rtOri);				//ʶ�����
	inline bool RecogGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc);						//ʶ��Ҷ�ֵ
	
	bool PicRotate();							//ͼ��ƫ�Ʋ���
	void sharpenImage1(const cv::Mat &image, cv::Mat &result);		//��

	void GetOmrArry(std::vector<cv::Rect>& rcList);	//�ڰ׿�ʱ��ȡ��ѡ��OMR��������

	inline void GetThreshold(cv::Mat& matSrc, cv::Mat& matDst);			//��ֵ������
//	inline void ShowDetailRectInfo();
	inline int GetStandardVal(CPType eType);
	inline int GetRectInfoByPoint(cv::Point pt, CPType eType, RECTINFO*& pRc);
	void ShowRectByPoint(cv::Point pt);
	void ShowRectByItem(int nItem);
	void ShowRectByCPType(CPType eType);
	void ShowTmpRect();
	void SortRect();							//��ʶ������ľ��ν�����������
	void ShowRectTracker();						//��ʾ��Ƥ�������
	LRESULT RoiLBtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT RoiLBtnDown(WPARAM wParam, LPARAM lParam);
	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);
	LRESULT HTrackerChange(WPARAM wParam, LPARAM lParam);
	LRESULT VTrackerChange(WPARAM wParam, LPARAM lParam);
	
	bool SaveModelFile(pMODEL pModel);			//����ģ�嵽�ļ�
	void setUploadModelInfo(CString& strName, CString& strModelPath, int nExamId, int nSubjectId);					//�����ϴ�ģ�����Ϣ

	void DelRectInfoOnPic();					//ɾ��ͼ��ؼ��ϵ�ѡ��ʶ���
	void DeleteRectInfoOnList();				//ɾ��List�б�ѡ�е�ʶ���
	BOOL DeleteRectInfo(CPType eType, int nItem);				//ɾ��ѡ�е�ʶ���
	void RecognizeRectTracker();				//ʶ����Ƥ������
	void AddRecogRectToList();					//���ʶ����������򵽶�Ӧ���б��У������ͬ��ͷ�����
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
	afx_msg void OnBnClickedBtnuploadmodel();
	afx_msg void OnBnClickedBtnScanmodel();
};

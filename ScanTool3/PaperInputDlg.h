#pragma once
#include "global.h"
#include "PicShow.h"
#include "MyCodeConvert.h"
#include "FileUpLoad.h"
#include "ComboBoxExt.h"
#include "XListCtrl.h"
#include "StudentMgr.h"
#include "AnswerShowDlg.h"
// CPaperInputDlg 对话框

class CPaperInputDlg : public CDialog
{
	DECLARE_DYNAMIC(CPaperInputDlg)

public:
	CPaperInputDlg(pMODEL pModel, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPaperInputDlg();

// 对话框数据
	enum { IDD = IDD_PAPERINPUTDLG };

public:
	CXListCtrl	m_lPapersCtrl;		//试卷袋列表
	CXListCtrl	m_lPaperCtrl;		//试卷袋中的试卷列表
//	CXListCtrl	m_lIssuePaperCtrl;	//问题试卷列表
	CComboBoxExt	m_comboModel;		//模板下拉列表
	CButton		m_btnBroswer;		//浏览按钮
	
	CShowPicDlg* m_pShowPicDlg;
	CAnswerShowDlg* m_pAnswerShowDlg;

	CString		m_strPapersPath;	//试卷袋文件夹路径
	int			m_nModelPicNums;	//模板图片数，即一份模板有多少图片，对应多少试卷

	CString		m_strModelName;		//此试卷袋所用模板的名称
	CString		m_strPapersName;	//此试卷袋的名称
	CString		m_strPapersDesc;	//此试卷袋的描述信息
	
	pMODEL		m_pOldModel;				//旧模板信息，切换combox的时候不能把它释放
	pMODEL		m_pModel;					//模板信息

	int				m_nCurrItemPapers;		//试卷袋列表当前选择的项
	int				m_ncomboCurrentSel;		//下拉列表当前选择项

	int				m_nCurrItemPaperList;	//当前试卷列表选中的项
	std::vector<std::string> m_vecCHzkzh;	//重号的准考证号，在同一袋试卷时进行检查当前试卷袋中所有重号的试卷（防止出现第1份试卷正常后面出现重号的，在前面重号的试卷无法检测的问题）

	void	ReShowPapers();
	void	ReInitData(pMODEL pModel);

	void	SetStatusShowInfo(CString strMsg, BOOL bWarn = FALSE);	//设置状态栏显示的消息
private:
	void	InitUI();
	void	InitCtrlPosition(); 
	void	InitParam();

	void	SeachModel();
// 	bool	UnZipModel(CString strZipPath);
// 	pMODEL	LoadModelFile(CString strModelPath);			//加载模板文件

// 	int		CheckOrientation4Fix(cv::Mat& matSrc, int n);	//定点模式下的方向
// 	int		CheckOrientation4Head(cv::Mat& matSrc, int n);	//同步头模式下的方向
// 	int		CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan);	//检测图片方向

// 	void	PaintRecognisedRect(pST_PaperInfo pPaper);		//画已识别的矩形
// 	void	PaintIssueRect(pST_PaperInfo pPaper);			//画出已识别出来的问题矩形位置

	COLORREF		crOldText, crOldBackground;
	void	SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//设置列表高亮显示
	void	UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//取消列表高亮显示

	void	ShowPaperByItem(int nItem);
	void	ShowPapers(pPAPERSINFO pPapers);
	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);

	void	InitTmpSubjectBmk();
	STUDENT_LIST		m_lBmkStudent;

	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色
	pST_PaperInfo	m_pCurrentShowPaper;
	pPAPERSINFO		m_pCurrentPapers;

	CStudentMgr*	m_pStudentMgr;			//报名库管理对象
	
	void	ShowRectByPoint(cv::Point pt, pST_PaperInfo pPaper);
	LRESULT RoiLBtnDown(WPARAM wParam, LPARAM lParam);		//鼠标左键按下的通知
	int		GetRectInfoByPoint(cv::Point pt, pST_PicInfo pPic, RECTINFO*& pRc);

	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);		//准考证号识别完成时的通知

	void	SetFontSize(int nSize);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnBroswer();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnCbnSelchangeComboModellist();
	afx_msg void OnNMDblclkListPapers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnSave();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLvnKeydownListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNMHoverListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListPapers(NMHDR *pNMHDR, LRESULT *pResult);
};

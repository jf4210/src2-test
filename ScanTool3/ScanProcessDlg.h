#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ScanReminderDlg.h"
#include "ShowPicDlg.h"
#include "BmpButton.h"
#include "ctrl/skinscrollwnd.h"
#include "StudentMgr.h"
//#include "ModifyZkzhDlg.h"

// CScanProcessDlg 对话框

class CScanProcessDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanProcessDlg)

public:
	CScanProcessDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanProcessDlg();

// 对话框数据
	enum { IDD = IDD_SCANPROCESSDLG };

	void	AddPaper(int nID, pST_PaperInfo pPaper);
	void	ResetPicList();
	void	InitShow();
	void	UpdateChildInfo(bool bScanDone = false);
	void	ScanCompleted();
	void	SetStatusShow(int nType, CString strShowInfo, bool bWarn = false);	//1--扫描状态信息，2--保存试卷袋信息
	void	UpdateExamBmk();		//将当前试卷袋的扫描信息更新到考试的总报名库中
	void	ReShowCurrPapers();
	void	ShowSinglePic(cv::Mat& matPic);
	void	InitTmpSubjectBmk();		//初始化临时报名库
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	EnableBtn(BOOL bEnable);
	bool	WriteJsonFile();
	void	SetFontSize();

	void	ShowPapers(pPAPERSINFO pPapers);
	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);

	void TestData(bool bReset);	//测试数据，可以删

	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);		//准考证号识别完成时的通知
private:
	CXListCtrl	m_lcPicture;	//CXListCtrl
	CRect		m_rtChildDlg;	//子控件位置
	CScanReminderDlg*	m_pReminderDlg;	//扫描提示框窗口
	CShowPicDlg*		m_pShowPicDlg;
	CStudentMgr*		m_pStudentMgr;
//	CModifyZkzhDlg*		m_pModifyZkzhDlg;

	STUDENT_LIST		m_lBmkStudent;

	CFont			m_fontBtn1;				//按钮字体
	CFont			m_fontBtn2;
	CBmpButton		m_bmpBtnScanProcess;
	CBmpButton		m_bmpBtnScanAgain;
	CBmpButton		m_bmpBtnSave;
	HBITMAP			m_bitmap_scrollbar;

	int				m_nCurrentScanCount;	//当前扫描需要扫描试卷数量
	std::string		m_strCurrPicSavePath;		//gb2312
	std::vector<std::string> m_vecCHzkzh;	//重号的准考证号，在同一袋试卷时进行检查当前试卷袋中所有重号的试卷（防止出现第1份试卷正常后面出现重号的，在前面重号的试卷无法检测的问题）
	//std::vector<std::string> m_vecKC;		//记录当前试卷袋的所有试卷的考场列表，可能是多个考场的试卷一起扫描
	std::set<std::string> m_setKC;			//记录当前试卷袋的所有试卷的考场列表，可能是多个考场的试卷一起扫描
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnScanagain();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnDestroy();
	afx_msg void OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnScanprocess();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

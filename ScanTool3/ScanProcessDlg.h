#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ScanReminderDlg.h"
#include "ShowPicDlg.h"
#include "BmpButton.h"
#include "ctrl/skinscrollwnd.h"
#include "StudentMgr.h"

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
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	EnableBtn(BOOL bEnable);
	void	WriteJsonFile();
	void	SetFontSize();

	void	ShowPapers(pPAPERSINFO pPapers);

	LRESULT MsgZkzhRecog(WPARAM wParam, LPARAM lParam);		//准考证号识别完成时的通知
private:
	CXListCtrl	m_lcPicture;	//CXListCtrl
	CRect		m_rtChildDlg;	//子控件位置
	CScanReminderDlg*	m_pReminderDlg;	//扫描提示框窗口
	CShowPicDlg*		m_pShowPicDlg;
	CStudentMgr*		m_pStudentMgr;

	CFont			m_fontBtn1;				//按钮字体
	CFont			m_fontBtn2;
	CBmpButton		m_bmpBtnScanProcess;
	CBmpButton		m_bmpBtnScanAgain;
	CBmpButton		m_bmpBtnSave;
	HBITMAP			m_bitmap_scrollbar;

	int				m_nCurrentScanCount;	//当前扫描需要扫描试卷数量
	std::string		m_strCurrPicSavePath;		//gb2312
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

#pragma once
#include "global.h"
#include "ScanDlg.h"
#include "WaitDownloadDlg.h"
#include "ScanProcessDlg.h"
#include "ScanThread.h"

// CScanMgrDlg 对话框

class CScanMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanMgrDlg)

public:
	CScanMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanMgrDlg();

// 对话框数据
	enum { IDD = IDD_SCANMGRDLG };

	CScanThread		m_scanThread;

	void	ShowDlg();
	bool	SearchModel();		//遍历所有模板
	bool	DownLoadModel();	//下载模板
	void	ShowChildDlg(int n);
	void	ResetChildDlg();	//子窗口重置
	void	UpdateChildDlgInfo();	//更新子窗口信息
private:
	void	InitData();
	void	InitCtrlPosition();
	void	InitChildDlg();
	void	ReleaseDlg();
	void	SetFontSize(int nSize);
	void	DrawBorder(CDC *pDC);	//绘制边框线

	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);

private:
	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

	CRect		m_rtChildDlg;		//显示子窗口的区域
	CWaitDownloadDlg*	m_pWaitDownloadDlg;
	CScanDlg*			m_pScanDlg;
	CScanProcessDlg*	m_pScanProcessDlg;

	CString			m_strExamName;
	CComboBoxExt		m_comboSubject;
//	CComboBox		m_comboSubject;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnDatacheck();
	afx_msg void OnBnClickedBtnScanrecord();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedBtnChangeexam();
};

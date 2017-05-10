#pragma once
#include "global.h"
#include "ScanDlg.h"

// CScanMgrDlg 对话框

class CScanMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanMgrDlg)

public:
	CScanMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanMgrDlg();

// 对话框数据
	enum { IDD = IDD_SCANMGRDLG };

	void	UpdateInfo();
private:
	void	InitCtrlPosition();
	void	InitChildDlg();
	void	ReleaseDlg();
private:
	CRect		m_rtChildDlg;		//显示子窗口的区域
	CScanDlg*	m_pScanDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

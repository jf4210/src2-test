#pragma once
#include "XListCtrl.h"

// CScanProcessDlg 对话框

class CScanProcessDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanProcessDlg)

public:
	CScanProcessDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanProcessDlg();

// 对话框数据
	enum { IDD = IDD_SCANPROCESSDLG };

private:
	void	InitUI();
	void	InitCtrlPosition();

private:
	CXListCtrl	m_lcPicture;
	CRect		m_rtChildDlg;	//子控件位置
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

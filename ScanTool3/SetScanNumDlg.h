#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"


// CSetScanNumDlg 对话框

class CSetScanNumDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CSetScanNumDlg)

public:
	CSetScanNumDlg(int nScanNum, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSetScanNumDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETSCANNUMDLG };
#endif

	CBmpButton		m_bmpBtnClose;
	int				m_nScanNum;

	CFont	fontStatus;
	void	SetFontSize();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCloseSetscannumdlg();
	afx_msg void OnBnClickedBtnOkSetscannumdlg();
};

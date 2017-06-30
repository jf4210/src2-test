#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"

// CTHSetDlg 对话框

class CTHSetDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CTHSetDlg)

public:
	CTHSetDlg(int nStartTH = 0, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTHSetDlg();

// 对话框数据
	enum { IDD = IDD_THSETDLG };

	CBmpButton		m_bmpBtnClose;
public:
	int		m_nStartTH;	//起始题号
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnClose();
};

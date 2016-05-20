#pragma once


// CTHSetDlg 对话框

class CTHSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CTHSetDlg)

public:
	CTHSetDlg(int nStartTH = 0, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTHSetDlg();

// 对话框数据
	enum { IDD = IDD_THSETDLG };

public:
	int		m_nStartTH;	//起始题号
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

#pragma once


// CParamSetDlg 对话框

class CParamSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CParamSetDlg)

public:
	CParamSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CParamSetDlg();

// 对话框数据
	enum { IDD = IDD_PARAMSETDLG };

public:
	CString		m_strFileIP;
	CString		m_strCmdIP;
	int			m_nFilePort;
	int			m_nCmdPort;

private:
	CIPAddressCtrl	m_cFileAddr;
	CIPAddressCtrl	m_cCmdAddr;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOk();
};

#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"


// CChkIdentityDlg 对话框

class CChkIdentityDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CChkIdentityDlg)

public:
	CChkIdentityDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CChkIdentityDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHKIDENTITYDLG };
#endif
	CString		m_strUser;
	CString		m_strPwd;
private:
	CBmpButton		m_bmpBtnClose;
	CBmpButton		m_bmpBtnOK;

	void	InitUI();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

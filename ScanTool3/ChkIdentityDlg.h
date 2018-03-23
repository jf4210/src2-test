#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"


// CChkIdentityDlg �Ի���

class CChkIdentityDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CChkIdentityDlg)

public:
	CChkIdentityDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CChkIdentityDlg();

// �Ի�������
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

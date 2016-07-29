#pragma once


// CParamSetDlg �Ի���

class CParamSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CParamSetDlg)

public:
	CParamSetDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CParamSetDlg();

// �Ի�������
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOk();
};

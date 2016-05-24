#pragma once
#include "global.h"

// CLoginDlg �Ի���

class CLoginDlg : public CDialog
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CString strIP, int nPort, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CLoginDlg();

// �Ի�������
	enum { IDD = IDD_LOGINDLG };

public:
	CString		m_strUserName;
	CString		m_strPwd;
	CString		m_strEzs;

	bool		m_bLogin;
	int			m_nRecvLen;
	int			m_nWantLen;
	char		m_szRecvBuff[1024 * 10];
	Poco::Net::StreamSocket m_ss;

	int		RecvData(CString& strResultInfo);
	int		GetExamInfo();

private:
	CString		m_strServerIP;
	int			m_nServerPort;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLogin();
};

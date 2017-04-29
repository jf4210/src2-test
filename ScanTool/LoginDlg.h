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
	CString		m_strUser;
	CString		m_strNickName;
	CString		m_strPersonId;		//����ר��
	int			m_nTeacherId;
	int			m_nUserId;

	bool		m_bLogin;
	int			m_nRecvLen;
	int			m_nWantLen;
	char		m_szRecvBuff[1024];

	char*		m_pRecvBuff;
	Poco::Net::StreamSocket m_ss;

	int		RecvData(CString& strResultInfo);
	int		GetExamInfo();
	int		GetBmkInfo();

#ifdef TO_WHTY
	VEC_PLATFORM_TY _vecPlatformList;
	void	LoginPlatform4TY(pST_PLATFORMINFO p);
#endif


private:
	CString		m_strServerIP;
	int			m_nServerPort;
	int			nRecvBuffSize;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog(); 
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLogin();
};

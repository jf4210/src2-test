#pragma once
#include "global.h"

// CLoginDlg 对话框

class CLoginDlg : public CDialog
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CString strIP, int nPort, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLoginDlg();

// 对话框数据
	enum { IDD = IDD_LOGINDLG };

public:
	CString		m_strUserName;
	CString		m_strPwd;
	CString		m_strEzs;
	CString		m_strUser;
	CString		m_strNickName;
	CString		m_strPersonId;		//天喻专用
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog(); 
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLogin();
};

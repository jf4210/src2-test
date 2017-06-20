#pragma once
#include "global.h"
#include "BmpButton.h"
#include "IconEdit.h"

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
	CBitmap		m_bmpBkg;
	CBitmap		m_bmpTitle;
	CBmpButton	m_bmpBtnMin;
	CBmpButton	m_bmpBtnExit;
	CBmpButton	m_bmpBtnLogin;
	CIconEdit	m_icnEditUser;
	CIconEdit	m_icnEditPwd;

	CString		m_strUserName;
	CString		m_strPwd;
	CString		m_strEzs;
	CString		m_strUser;
	CString		m_strNickName;
	CString		m_strPersonId;		//天喻专用
	CString		m_strSchoolID;		//天喻专用
	int			m_nTeacherId;
	int			m_nUserId;

	bool		m_bRecordPwd;
	bool		m_bLogin;
	int			m_nRecvLen;
	int			m_nWantLen;
	char		m_szRecvBuff[1024];

	char*		m_pRecvBuff;
	Poco::Net::StreamSocket m_ss;

	int		RecvData(CString& strResultInfo);
	int		GetExamInfo();
	int		GetBmkInfo();
	void	GetFileAddrs();

	void	InitUI();
//#ifdef TO_WHTY
#if 1
	VEC_PLATFORM_TY _vecPlatformList;
	void	LoginPlatform4TY(pST_PLATFORMINFO p);
#endif


private:
	CString		m_strServerIP;
	int			m_nServerPort;
	int			nRecvBuffSize;
	CString		m_strTitle;
	CString		m_strVersion;

	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontTitle;			//标题字体
	CFont			m_fontVersion;			//版本字体
	CFont			m_fontLogin;
	COLORREF		m_colorStatus;			//状态栏字体颜色
	void	SetFontSize(int nSize);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedChkRecordpwd();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnMin();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStaticCheckbox();
};

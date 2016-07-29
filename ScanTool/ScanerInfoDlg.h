#pragma once


// CScanerInfoDlg 对话框

class CScanerInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanerInfoDlg)

public:
	CScanerInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanerInfoDlg();

// 对话框数据
	enum { IDD = IDD_SCANERINFODLG };

public:
	void			setShowInfo(CString& strUser, CString& strNickName);

private:
	CString		m_strUser;
	CString		m_strNickName;

	int				m_nStatusSize;			//字体大小
	CFont			m_fontStatus;			//字体
	CFont			m_fontUnLogin;
	COLORREF		m_colorStatus;			//字体颜色

	void			SetFontSize(int nSize);
	void			InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

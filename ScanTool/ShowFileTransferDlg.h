#pragma once


// CShowFileTransferDlg 对话框

class CShowFileTransferDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowFileTransferDlg)

public:
	CShowFileTransferDlg(void* pDlg, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowFileTransferDlg();

// 对话框数据
	enum { IDD = IDD_SHOWFILETRANSFERDLG };

private:
	CListCtrl	m_lFileTranser;
	int			m_nCurListItem;
	CString		m_strCmdChannel;
	CString		m_strFileChannel;
	void*		m_pDlg;

	int				m_nStatusSize;			//字体大小
	CFont			m_fontStatus;			//字体
	COLORREF		m_colorCmdStatus;			//字体颜色
	COLORREF		m_colorFileStatus;			//字体颜色
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	ShowFileTransferList();
	void	ReSendFile();
	void	PkgToPapers();

	void	SetFontSize(int nSize);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMHoverListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMRClickListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

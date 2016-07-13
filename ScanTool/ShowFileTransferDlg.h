#pragma once


// CShowFileTransferDlg 对话框

class CShowFileTransferDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowFileTransferDlg)

public:
	CShowFileTransferDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowFileTransferDlg();

// 对话框数据
	enum { IDD = IDD_SHOWFILETRANSFERDLG };

private:
	CListCtrl	m_lFileTranser;
	int			m_nCurListItem;
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	ShowFileTransferList();
	void	ReSendFile();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMHoverListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMRClickListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult);
};

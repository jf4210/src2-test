#pragma once


// CShowFileTransferDlg �Ի���

class CShowFileTransferDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowFileTransferDlg)

public:
	CShowFileTransferDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShowFileTransferDlg();

// �Ի�������
	enum { IDD = IDD_SHOWFILETRANSFERDLG };

private:
	CListCtrl	m_lFileTranser;

private:
	void	InitUI();
	void	InitCtrlPosition();
	void	ShowFileTransferList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMHoverListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

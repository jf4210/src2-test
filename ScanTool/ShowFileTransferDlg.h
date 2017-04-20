#pragma once


// CShowFileTransferDlg �Ի���

class CShowFileTransferDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowFileTransferDlg)

public:
	CShowFileTransferDlg(void* pDlg, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShowFileTransferDlg();

// �Ի�������
	enum { IDD = IDD_SHOWFILETRANSFERDLG };

private:
	CListCtrl	m_lFileTranser;
	int			m_nCurListItem;
	CString		m_strCmdChannel;
	CString		m_strFileChannel;
	void*		m_pDlg;

	int				m_nStatusSize;			//�����С
	CFont			m_fontStatus;			//����
	COLORREF		m_colorCmdStatus;			//������ɫ
	COLORREF		m_colorFileStatus;			//������ɫ
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	ShowFileTransferList();
	void	ReSendFile();
	void	PkgToPapers();

	void	SetFontSize(int nSize);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMHoverListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMRClickListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

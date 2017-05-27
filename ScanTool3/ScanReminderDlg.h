#pragma once


// CScanReminderDlg �Ի���

class CScanReminderDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanReminderDlg)

public:
	CScanReminderDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanReminderDlg();

// �Ի�������
	enum { IDD = IDD_SCANREMINDERDLG };

	void		SetShowTips(CString str);
	void		UpdataScanCount(int nCount);
private:
	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	CFont			m_fontStatus2;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ
	
	CString			m_strScanCount;
	CString			m_strShowTips;

	CBitmap			m_bmpBk;

	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	void	DrawBorder(CDC *pDC);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

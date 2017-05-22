#pragma once


// CScanReminderDlg 对话框

class CScanReminderDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanReminderDlg)

public:
	CScanReminderDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanReminderDlg();

// 对话框数据
	enum { IDD = IDD_SCANREMINDERDLG };

private:
	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	void	DrawBorder(CDC *pDC);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


// ScanToolStartDlg.h : 头文件
//

#pragma once


// CScanToolStartDlg 对话框
class CScanToolStartDlg : public CDialogEx
{
// 构造
public:
	CScanToolStartDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SCANTOOLSTART_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	void InitCtrlPosition();

public:
	int				m_nStatusSize;			//字体大小
	CFont			m_fontStatus;			//字体
	COLORREF		m_colorStatus;			//字体颜色

	CString			m_strAppPath;

	void			SetFontSize(int nSize);
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSimple();
	afx_msg void OnBnClickedBtnSpecialty();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

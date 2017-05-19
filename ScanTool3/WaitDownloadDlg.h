#pragma once
#include "global.h"

// CWaitDownloadDlg 对话框

class CWaitDownloadDlg : public CDialog
{
	DECLARE_DYNAMIC(CWaitDownloadDlg)

public:
	CWaitDownloadDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWaitDownloadDlg();

// 对话框数据
	enum { IDD = IDD_WAITDOWNLOADDLG };

private:
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

	CString		m_strWaitInfo;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

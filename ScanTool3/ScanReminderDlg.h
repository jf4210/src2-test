#pragma once
#include "ctrl/TransparentStatic.h"

// CScanReminderDlg 对话框

class CScanReminderDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanReminderDlg)

public:
	CScanReminderDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanReminderDlg();

// 对话框数据
	enum { IDD = IDD_SCANREMINDERDLG };

	void		SetShowTips(CString str, bool bWarn = false);
	void		UpdataScanCount(int nCount);
	void		SetShowScanCount(bool bShow);	//是否需要显示扫描数量的提示
private:
	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	CFont			m_fontStatus2;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色
	
	int				m_nShowType;			//1--显示扫描过程中的状态，此时状态控件高度低，2--显示保存过程或其他过程的状态，此时控件的高度高
	CString			m_strScanCount;
	CString			m_strShowTips;

	CBitmap			m_bmpBk;
	CTransparentStatic	m_staticShowTips;

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

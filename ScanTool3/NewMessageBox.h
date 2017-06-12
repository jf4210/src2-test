#pragma once
#include "resource.h"
#include "BmpButton.h"
#include "ctrl/TransparentStatic.h"
#include "TipBaseDlg.h"
#include <string>

// CTestDlg 对话框

class CNewMessageBox : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CNewMessageBox)

public:
	CNewMessageBox(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNewMessageBox();

// 对话框数据
	enum { IDD = IDD_NEWMESSAGEBOXDLG };

	int		m_nResult;
	void	setShowInfo(int nType, int nBtns, std::string strMsg);		//nType=1 显示无报名库信息，2-显示无模板信息
private:
	int		m_nBtn;				//显示几个按钮
	int		m_nShowType;		//nType=1 显示无报名库信息，2-显示无模板信息
	CBmpButton		m_bmpBtnOK;
	CBmpButton		m_bmpBtnClose;
	CTransparentStatic	m_staticMsgShow;

	CString			m_strMsgShow;
	CBitmap			m_bmpBk;

	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	CFont			m_fontBtn;				//按钮字体

	void	SetFontSize(int nSize);
	void	InitUI();
	void	InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

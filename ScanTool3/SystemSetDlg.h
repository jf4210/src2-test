#pragma once
#include "modelInfo.h"
#include "TipBaseDlg.h"
#include "BmpButton.h"
#include "BaseTabDlg.h"
#include "ctrl/XPGroupBox.h"


// CSystemSetDlg 对话框

class CSystemSetDlg : public CBaseTabDlg
{
	DECLARE_DYNAMIC(CSystemSetDlg)

public:
	CSystemSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSystemSetDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYSTEMSETDLG };
#endif

	int		m_nChkLostCorner;
	int		m_nUseRectFit;

	CButton		m_chkLostCorner;	//缺角检测
	CButton		m_chkUseRectFit;	//使用矩形拟合
	int		m_nRectFitInterval;


	void	InitData(AdvanceParam& stParam);
	BOOL	SaveParamData(AdvanceParam& stParam);
private:
	void	InitUI();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedChkLostcorner();
	afx_msg void OnBnClickedChkUserectfit();
};

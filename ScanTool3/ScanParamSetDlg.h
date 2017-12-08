#pragma once
#include "modelInfo.h"
#include "TipBaseDlg.h"
#include "BmpButton.h"
#include "BaseTabDlg.h"
#include "ctrl/XPGroupBox.h"
// CAdvancedSetDlg 对话框

class CScanParamSetDlg : public CBaseTabDlg
{
	DECLARE_DYNAMIC(CScanParamSetDlg)

public:
	CScanParamSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanParamSetDlg();

// 对话框数据
	enum { IDD = IDD_ADVANCEDSETDLG };

public:
	CComboBox	m_combo_ScanDpi;
	CComboBox	m_combo_PaperSize;
	CComboBox	m_combo_ScanType;
	CButton		m_chkAutoCut;
	CXPGroupBox m_groupScanInfo;
	CXPGroupBox m_groupModelInfo;
	CButton		m_chkUseWordAnchorPoint;	//使用文字作为定点
	CButton		m_chkUsePagination;			//使用页码，多页试卷的情况

	int			m_nScanDpi;
	int			m_nScanPaperSize;
	int			m_nScanType;
	int			m_nAutoCut;		//是否自动裁剪
	int			m_nUseWordAnchorPoint;	//是否使用文字作为定点
	int			m_nUsePagination;	//使用页码，针对多页试卷情况

	void	InitData(AdvanceParam& stParam);
	BOOL SaveParamData(AdvanceParam& stParam);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboScandpi();
	afx_msg void OnBnClickedChkAutocut();
	afx_msg void OnCbnSelchangeComboPapersize();
	afx_msg void OnCbnSelchangeComboScantype();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedChkUsewordanchorpoint();
	afx_msg void OnBnClickedChkUsepagination();
};

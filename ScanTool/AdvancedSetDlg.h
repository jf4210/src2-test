#pragma once
#include "modelInfo.h"

// CAdvancedSetDlg 对话框

class CAdvancedSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CAdvancedSetDlg)

public:
	CAdvancedSetDlg(pMODEL	pModel, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAdvancedSetDlg();

// 对话框数据
	enum { IDD = IDD_ADVANCEDSETDLG };

public:
	CComboBox	m_combo_ScanDpi;
	CComboBox	m_combo_PaperSize;
	CComboBox	m_combo_ScanType;
	CButton		m_chkAutoCut;

	CSpinButtonCtrl m_Spin_Zkzh;
	CSpinButtonCtrl m_Spin_Omr;
	int			m_nSensitiveZkzh;
	int			m_nSensitiveOmr;

	int			m_nScanDpi;
	int			m_nScanPaperSize;
	int			m_nScanType;
	int			m_nAutoCut;		//是否自动裁剪

private:
	pMODEL		m_pModel;

	void	InitData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboScandpi();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedChkAutocut();
	afx_msg void OnCbnSelchangeComboPapersize();
	afx_msg void OnCbnSelchangeComboScantype();
	afx_msg void OnBnClickedBtnDefParam();
	afx_msg void OnDeltaposSpinZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinOmr(NMHDR *pNMHDR, LRESULT *pResult);
};

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
	int			m_nScanDpi;
private:
	pMODEL		m_pModel;

	void	InitData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboScandpi();
};

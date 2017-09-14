#pragma once
#include "BmpButton.h"
#include "TipBaseDlg.h"
#include "AdvancedSetDlg.h"


// CAdanceSetMgrDlg 对话框

class CAdanceSetMgrDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CAdanceSetMgrDlg)

public:
	CAdanceSetMgrDlg(pMODEL	pModel, ST_SENSITIVE_PARAM stSensitiveParam, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAdanceSetMgrDlg();

// 对话框数据
	enum { IDD = IDD_ADANCESETMGRDLG };

	void	InitUI();
	void	InitCtrlPosition();

	CTabCtrl		m_tabParamMgr;
	CBmpButton		m_bmpBtnClose;
	std::vector<CDialog*>	m_vecTabDlg;
	int				m_nCurrTabSel;

	ST_SENSITIVE_PARAM _stSensitiveParam;
private:
	pMODEL		m_pModel;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
	afx_msg void OnTcnSelchangeTabParam(NMHDR *pNMHDR, LRESULT *pResult);
};

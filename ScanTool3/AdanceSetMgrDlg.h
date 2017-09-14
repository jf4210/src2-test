#pragma once
#include "BmpButton.h"
#include "TipBaseDlg.h"
#include "AdvancedSetDlg.h"


// CAdanceSetMgrDlg �Ի���

class CAdanceSetMgrDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CAdanceSetMgrDlg)

public:
	CAdanceSetMgrDlg(pMODEL	pModel, ST_SENSITIVE_PARAM stSensitiveParam, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAdanceSetMgrDlg();

// �Ի�������
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
	afx_msg void OnTcnSelchangeTabParam(NMHDR *pNMHDR, LRESULT *pResult);
};

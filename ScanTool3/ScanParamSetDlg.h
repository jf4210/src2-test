#pragma once
#include "modelInfo.h"
#include "TipBaseDlg.h"
#include "BmpButton.h"
#include "BaseTabDlg.h"
#include "ctrl/XPGroupBox.h"
// CAdvancedSetDlg �Ի���

class CScanParamSetDlg : public CBaseTabDlg
{
	DECLARE_DYNAMIC(CScanParamSetDlg)

public:
	CScanParamSetDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanParamSetDlg();

// �Ի�������
	enum { IDD = IDD_ADVANCEDSETDLG };

public:
	CComboBox	m_combo_ScanDpi;
	CComboBox	m_combo_PaperSize;
	CComboBox	m_combo_ScanType;
	CButton		m_chkAutoCut;
	CXPGroupBox m_groupScanInfo;
	
	int			m_nScanDpi;
	int			m_nScanPaperSize;
	int			m_nScanType;
	int			m_nAutoCut;		//�Ƿ��Զ��ü�

	void	InitData(AdvanceParam& stParam);
	BOOL SaveParamData(AdvanceParam& stParam);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboScandpi();
	afx_msg void OnBnClickedChkAutocut();
	afx_msg void OnCbnSelchangeComboPapersize();
	afx_msg void OnCbnSelchangeComboScantype();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
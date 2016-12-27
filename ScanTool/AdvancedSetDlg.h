#pragma once
#include "modelInfo.h"

// CAdvancedSetDlg �Ի���

class CAdvancedSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CAdvancedSetDlg)

public:
	CAdvancedSetDlg(pMODEL	pModel, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAdvancedSetDlg();

// �Ի�������
	enum { IDD = IDD_ADVANCEDSETDLG };

public:
	CComboBox	m_combo_ScanDpi;
	CComboBox	m_combo_PaperSize;
	CButton		m_chkAutoCut;
	int			m_nScanDpi;
	int			m_nScanPaperSize;
	int			m_nAutoCut;		//�Ƿ��Զ��ü�

private:
	pMODEL		m_pModel;

	void	InitData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboScandpi();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedChkAutocut();
	afx_msg void OnCbnSelchangeComboPapersize();
};

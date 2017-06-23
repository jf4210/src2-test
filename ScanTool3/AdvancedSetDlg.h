#pragma once
#include "modelInfo.h"
#include "TipBaseDlg.h"

// CAdvancedSetDlg 对话框
typedef struct _SensitiveParam_
{
	int nCurrentZkzhSensitivity;
	int nCurrentOmrSensitivity;
	int nDefZkzhSensitivity;
	int nDefOmrSensitivity;
	_SensitiveParam_()
	{
		nCurrentZkzhSensitivity = 2;
		nCurrentOmrSensitivity = 5;
		nDefZkzhSensitivity = 2;
		nDefOmrSensitivity = 5;
	}
}ST_SENSITIVE_PARAM;

class CAdvancedSetDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CAdvancedSetDlg)

public:
	CAdvancedSetDlg(pMODEL	pModel, ST_SENSITIVE_PARAM stSensitiveParam, CWnd* pParent = NULL);   // 标准构造函数
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
	int			m_nDefSensitiveZkzh;	//默认考号灵敏度
	int			m_nDefSensitiveOmr;		//默认Omr灵敏度

	int			m_nScanDpi;
	int			m_nScanPaperSize;
	int			m_nScanType;
	int			m_nAutoCut;		//是否自动裁剪

private:
	pMODEL		m_pModel;
	ST_SENSITIVE_PARAM _stSensitiveParam;

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
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

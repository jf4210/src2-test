#pragma once
#include "TwainCpp.h"

typedef CArray<TW_IDENTITY, TW_IDENTITY> SCANSRCARRY;

// CScanCtrlDlg 对话框

class CScanCtrlDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanCtrlDlg)

public:
	CScanCtrlDlg(SCANSRCARRY& srcArry, bool bShowPaperSize = false, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanCtrlDlg();

// 对话框数据
	enum { IDD = IDD_SCANCTRLDLG };

	bool	ScanSrcInit();
	bool	InitUI();
public:
	CComboBox	m_comboScanSrc;		//扫描仪源
	CComboBox	m_comboDuplex;		//单双面下拉
	CComboBox	m_comboModelPaperSize;	//模板纸张类型
	int		m_nStudentNum;		//学生数量

	bool	m_bAdvancedSetting;	//是否进行高级设置

	SCANSRCARRY& sourceArry;
	int			m_nCurrScanSrc;		//当前选择的扫描仪源
	int			m_nCurrDuplex;		//当前选择的单双面
	int			m_nCurrPaperSize;	//当前纸张类型

private:
	bool	m_bShowPaperSize;
	void	InitCtrlPosition();
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboScansrc();
	afx_msg void OnCbnSelchangeComboDuplex();
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnBnClickedBtnAdvanced();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangeComboModelpapersize();
};

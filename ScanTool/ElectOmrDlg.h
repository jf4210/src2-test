#pragma once
#include "modelInfo.h"

// ElectOmrDlg 对话框

class ElectOmrDlg : public CDialog
{
	DECLARE_DYNAMIC(ElectOmrDlg)

public:
	ElectOmrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ElectOmrDlg();

// 对话框数据
	enum { IDD = IDD_ELECTOMRDLG };

public:
	std::vector<pELECTOMRGROUPINFO> m_vecElectOmrInfo;

	CComboBox m_comboGroup;
	int		m_nAllCount;	//总选项数
	int		m_nRealItem;	//有效数

	int		m_nCurrentSel;

private:
	void InitCtrlPosition();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnNew();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnCbnSelchangeComboGroup();
};

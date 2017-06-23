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
	std::vector<pELECTOMRGROUPINFO> m_vecElectOmrInfoAll;	//所有选做题信息组，可能有新建后有删除的，都保存在此
	std::vector<pELECTOMRGROUPINFO> m_vecElectOmrInfoReal;	//实际有效的选做题信息

	CComboBox m_comboGroup;
	int		m_nAllCount;	//总选项数
	int		m_nRealItem;	//有效数

	int		m_nCurrentSel;
	pELECTOMRGROUPINFO	m_pCurrentGroup;

	int		m_nCurrPaperId;	//当前页面ID

	bool checkValid();		//检测当前组信息是否是有效的信息 
	void showUI(int nGroup);
	void InitGroupInfo(std::vector<ELECTOMR_QUESTION>& vecElectOmr, int nPaperId);
	void ReleaseData();
private:
	void InitCtrlPosition();
	void InitUI();

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
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

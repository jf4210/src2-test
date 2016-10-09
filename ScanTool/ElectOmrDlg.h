#pragma once
#include "modelInfo.h"

// ElectOmrDlg �Ի���

class ElectOmrDlg : public CDialog
{
	DECLARE_DYNAMIC(ElectOmrDlg)

public:
	ElectOmrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~ElectOmrDlg();

// �Ի�������
	enum { IDD = IDD_ELECTOMRDLG };

public:
	std::vector<pELECTOMRGROUPINFO> m_vecElectOmrInfo;

	CComboBox m_comboGroup;
	int		m_nAllCount;	//��ѡ����
	int		m_nRealItem;	//��Ч��

	int		m_nCurrentSel;

private:
	void InitCtrlPosition();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
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

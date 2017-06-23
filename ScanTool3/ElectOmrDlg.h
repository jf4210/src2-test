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
	std::vector<pELECTOMRGROUPINFO> m_vecElectOmrInfoAll;	//����ѡ������Ϣ�飬�������½�����ɾ���ģ��������ڴ�
	std::vector<pELECTOMRGROUPINFO> m_vecElectOmrInfoReal;	//ʵ����Ч��ѡ������Ϣ

	CComboBox m_comboGroup;
	int		m_nAllCount;	//��ѡ����
	int		m_nRealItem;	//��Ч��

	int		m_nCurrentSel;
	pELECTOMRGROUPINFO	m_pCurrentGroup;

	int		m_nCurrPaperId;	//��ǰҳ��ID

	bool checkValid();		//��⵱ǰ����Ϣ�Ƿ�����Ч����Ϣ 
	void showUI(int nGroup);
	void InitGroupInfo(std::vector<ELECTOMR_QUESTION>& vecElectOmr, int nPaperId);
	void ReleaseData();
private:
	void InitCtrlPosition();
	void InitUI();

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
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

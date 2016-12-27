#pragma once
#include "TwainCpp.h"

typedef CArray<TW_IDENTITY, TW_IDENTITY> SCANSRCARRY;

// CScanCtrlDlg �Ի���

class CScanCtrlDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanCtrlDlg)

public:
	CScanCtrlDlg(SCANSRCARRY& srcArry, bool bShowPaperSize = false, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanCtrlDlg();

// �Ի�������
	enum { IDD = IDD_SCANCTRLDLG };

	bool	ScanSrcInit();
	bool	InitUI();
public:
	CComboBox	m_comboScanSrc;		//ɨ����Դ
	CComboBox	m_comboDuplex;		//��˫������
	CComboBox	m_comboModelPaperSize;	//ģ��ֽ������
	int		m_nStudentNum;		//ѧ������

	bool	m_bAdvancedSetting;	//�Ƿ���и߼�����

	SCANSRCARRY& sourceArry;
	int			m_nCurrScanSrc;		//��ǰѡ���ɨ����Դ
	int			m_nCurrDuplex;		//��ǰѡ��ĵ�˫��
	int			m_nCurrPaperSize;	//��ǰֽ������

private:
	bool	m_bShowPaperSize;
	void	InitCtrlPosition();
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboScansrc();
	afx_msg void OnCbnSelchangeComboDuplex();
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnBnClickedBtnAdvanced();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangeComboModelpapersize();
};

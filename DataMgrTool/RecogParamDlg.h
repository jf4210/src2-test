#pragma once
#include "DMTDef.h"

// CRecogParamDlg �Ի���

class CRecogParamDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecogParamDlg)

public:
	CRecogParamDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRecogParamDlg();

// �Ի�������
	enum { IDD = IDD_RECOGPARAMDLG };

public:
	int		m_nChkSN;
	int		m_nChkOmr;
	int		m_nChkElectOmr;
	CString		m_strEzsAddr;

	int		m_nHandleResult;
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChkSn();
	afx_msg void OnBnClickedChkOmr();
	afx_msg void OnBnClickedChkElecomr();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadioCompresspkg();
	afx_msg void OnBnClickedRadioSendezs();
};

#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"


// CSetScanNumDlg �Ի���

class CSetScanNumDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CSetScanNumDlg)

public:
	CSetScanNumDlg(int nScanNum, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSetScanNumDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETSCANNUMDLG };
#endif

	CBmpButton		m_bmpBtnClose;
	int				m_nScanNum;

	CFont	fontStatus;
	void	SetFontSize();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCloseSetscannumdlg();
	afx_msg void OnBnClickedBtnOkSetscannumdlg();
};

#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"

// CTHSetDlg �Ի���

class CTHSetDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CTHSetDlg)

public:
	CTHSetDlg(int nStartTH = 0, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CTHSetDlg();

// �Ի�������
	enum { IDD = IDD_THSETDLG };

	CBmpButton		m_bmpBtnClose;
public:
	int		m_nStartTH;	//��ʼ���
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnClose();
};

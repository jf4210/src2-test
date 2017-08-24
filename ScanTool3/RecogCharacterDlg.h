#pragma once
#include "BmpButton.h"
#include "TipBaseDlg.h"

// CRecogCharacterDlg �Ի���

class CRecogCharacterDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CRecogCharacterDlg)

public:
	CRecogCharacterDlg(CString strRecoged, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRecogCharacterDlg();

// �Ի�������
	enum { IDD = IDD_RECOGCHARACTERDLG };

	CString		m_strVal;
	CString		m_strValBk;
private:
	CBmpButton m_bmpBtnClose;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedOk();
};

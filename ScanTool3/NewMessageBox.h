#pragma once
#include "BmpButton.h"
#include "TipBaseDlg.h"

// CTestDlg �Ի���

class CNewMessageBox : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CNewMessageBox)

public:
	CNewMessageBox(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CNewMessageBox();

// �Ի�������
	enum { IDD = IDD_NEWMESSAGEBOXDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
};

#pragma once
#include "BmpButton.h"
#include "TipBaseDlg.h"

// CTestDlg 对话框

class CNewMessageBox : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CNewMessageBox)

public:
	CNewMessageBox(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNewMessageBox();

// 对话框数据
	enum { IDD = IDD_NEWMESSAGEBOXDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
};

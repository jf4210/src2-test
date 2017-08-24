#pragma once
#include "BmpButton.h"
#include "TipBaseDlg.h"

// CRecogCharacterDlg 对话框

class CRecogCharacterDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CRecogCharacterDlg)

public:
	CRecogCharacterDlg(CString strRecoged, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRecogCharacterDlg();

// 对话框数据
	enum { IDD = IDD_RECOGCHARACTERDLG };

	CString		m_strVal;
	CString		m_strValBk;
private:
	CBmpButton m_bmpBtnClose;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedOk();
};

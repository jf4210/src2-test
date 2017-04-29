#pragma once
#include "global.h"


// CPapersInfoSave4TyDlg 对话框

class CPapersInfoSave4TyDlg : public CDialog
{
	DECLARE_DYNAMIC(CPapersInfoSave4TyDlg)

public:
	CPapersInfoSave4TyDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPapersInfoSave4TyDlg();

// 对话框数据
	enum { IDD = IDD_PAPERSINFOSAVE4TYDLG };
public:
	CComboBox	m_comboExamName;

	CString		m_strExamID;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeComboExamlist4ty();
};

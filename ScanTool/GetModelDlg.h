#pragma once
#include "global.h"

// CGetModelDlg 对话框

class CGetModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CGetModelDlg)

public:
	CGetModelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CGetModelDlg();

// 对话框数据
	enum { IDD = IDD_GETMODELDLG };

public:
	CComboBox	m_comboExamName;
	CComboBox	m_comboSubject;

	int			m_nExamID;
	CString		m_strExamTypeName;
	CString		m_strGradeName;
	int			m_SubjectID;
	CString		m_strScanModelName;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboExamname();
	afx_msg void OnCbnSelchangeComboSubjectname();
	afx_msg void OnBnClickedButton1();
};

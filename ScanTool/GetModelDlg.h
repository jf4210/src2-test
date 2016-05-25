#pragma once
#include "global.h"

// CGetModelDlg �Ի���

class CGetModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CGetModelDlg)

public:
	CGetModelDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CGetModelDlg();

// �Ի�������
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboExamname();
	afx_msg void OnCbnSelchangeComboSubjectname();
	afx_msg void OnBnClickedButton1();
};

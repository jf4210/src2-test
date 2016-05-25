#pragma once
#include "global.h"


// CModelSaveDlg �Ի���

class CModelSaveDlg : public CDialog
{
	DECLARE_DYNAMIC(CModelSaveDlg)

public:
	CModelSaveDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CModelSaveDlg();

// �Ի�������
	enum { IDD = IDD_MODELSAVEDLG };
public:
	CString m_strModelName;

	CComboBox	m_comboExamName;
	CComboBox	m_comboSubject;

	int			m_nExamID;
	CString		m_strExamTypeName;
	CString		m_strGradeName;
	int			m_SubjectID;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSavemodeldlg();
	afx_msg void OnCbnSelchangeComboExamname();
	afx_msg void OnCbnSelchangeComboSubjectname();
};

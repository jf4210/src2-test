#pragma once
#include "global.h"


// CPapersInfoSave4TyDlg �Ի���

class CPapersInfoSave4TyDlg : public CDialog
{
	DECLARE_DYNAMIC(CPapersInfoSave4TyDlg)

public:
	CPapersInfoSave4TyDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CPapersInfoSave4TyDlg();

// �Ի�������
	enum { IDD = IDD_PAPERSINFOSAVE4TYDLG };
public:
	CComboBox	m_comboExamName;

	CString		m_strExamID;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeComboExamlist4ty();
};

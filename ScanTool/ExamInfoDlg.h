#pragma once
#include "modelinfo.h"
#include "ComboBoxExt.h"
// CExamInfoDlg �Ի���

class CExamInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CExamInfoDlg)

public:
	CExamInfoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CExamInfoDlg();

// �Ի�������
	enum { IDD = IDD_EXAMINFODLG };

	CComboBoxExt	m_comboExamName;
	CComboBoxExt	m_comboSubject;

	int			m_nExamID;
	CString		m_strExamTypeName;
	CString		m_strGradeName;
	int			m_SubjectID;

	bool	InitShow(pMODEL pModel);
private:
	BOOL InitData();
	void InitUI();
	void InitCtrlPosition();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangeComboExamname();
	afx_msg void OnCbnSelchangeComboSubjectname();
};

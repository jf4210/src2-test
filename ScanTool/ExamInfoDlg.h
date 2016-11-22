#pragma once
#include "modelinfo.h"
#include "ComboBoxExt.h"
// CExamInfoDlg 对话框

class CExamInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CExamInfoDlg)

public:
	CExamInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CExamInfoDlg();

// 对话框数据
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangeComboExamname();
	afx_msg void OnCbnSelchangeComboSubjectname();
};

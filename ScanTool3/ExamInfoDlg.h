#pragma once
#include "modelinfo.h"
#include "ComboBoxExt.h"
#include "TipBaseDlg.h"
#include "BmpButton.h"
// CExamInfoDlg 对话框

class CExamInfoDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CExamInfoDlg)

public:
	CExamInfoDlg(pMODEL pModel = NULL, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CExamInfoDlg();

// 对话框数据
	enum { IDD = IDD_EXAMINFODLG };

	CComboBoxExt	m_comboExamName;
	CComboBoxExt	m_comboSubject;

	CString		m_strExamName;
	CString		m_strSubjectName;

	int			m_nExamID;
	CString		m_strExamID;
	CString		m_strExamTypeName;
	CString		m_strGradeName;
	int			m_SubjectID;

	CBmpButton	m_bmpBtnClose;
	bool	InitShow(pMODEL pModel);
private:
	BOOL InitData();
	void InitUI();
	void InitCtrlPosition();

	pMODEL	m_pModel;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangeComboExamname();
	afx_msg void OnCbnSelchangeComboSubjectname();
	afx_msg void OnBnClickedBtnSavenewmodel();
	afx_msg void OnBnClickedBtnClose();
};

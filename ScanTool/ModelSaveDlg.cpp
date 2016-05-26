// ModelSaveDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ModelSaveDlg.h"
#include "afxdialogex.h"


// CModelSaveDlg 对话框

IMPLEMENT_DYNAMIC(CModelSaveDlg, CDialog)

CModelSaveDlg::CModelSaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModelSaveDlg::IDD, pParent)
	, m_strExamTypeName(_T("")), m_strGradeName(_T("")), m_SubjectID(0), m_nExamID(0)
{

}

CModelSaveDlg::~CModelSaveDlg()
{
}

void CModelSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ModelName, m_strModelName);
	DDX_Text(pDX, IDC_EDIT_ExamID, m_nExamID);
	DDX_Text(pDX, IDC_EDIT_ExamTypeName, m_strExamTypeName);
	DDX_Text(pDX, IDC_EDIT_GradeName, m_strGradeName);
	DDX_Text(pDX, IDC_EDIT_SubjectID, m_SubjectID);
	DDX_Control(pDX, IDC_COMBO_ExamName, m_comboExamName);
	DDX_Control(pDX, IDC_COMBO_SubjectName, m_comboSubject);
}


BEGIN_MESSAGE_MAP(CModelSaveDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_SaveModelDlg, &CModelSaveDlg::OnBnClickedBtnSavemodeldlg)
	ON_CBN_SELCHANGE(IDC_COMBO_ExamName, &CModelSaveDlg::OnCbnSelchangeComboExamname)
	ON_CBN_SELCHANGE(IDC_COMBO_SubjectName, &CModelSaveDlg::OnCbnSelchangeComboSubjectname)
END_MESSAGE_MAP()


// CModelSaveDlg 消息处理程序

BOOL CModelSaveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	if (g_lExamList.size() == 0)
		return TRUE;

	EXAM_LIST::iterator itExam = g_lExamList.begin();
	for (; itExam != g_lExamList.end(); itExam++)
	{
		CString strName = A2T(itExam->strExamName.c_str());
		m_comboExamName.AddString(strName);
		int nCount = m_comboExamName.GetCount();
		m_comboExamName.SetItemData(nCount - 1, (DWORD_PTR)&(*itExam));
	}
	m_comboExamName.SetCurSel(0);

	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemData(0);
	if (pExamInfo)
	{
		m_comboSubject.ResetContent();
		SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
		for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
		{
			EXAM_SUBJECT* pSubject = &(*itSub);
			CString strSubjectName = A2T(itSub->strSubjName.c_str());
			m_comboSubject.AddString(strSubjectName);
			int nCount = m_comboSubject.GetCount();
			m_comboSubject.SetItemData(nCount - 1, (DWORD_PTR)pSubject);

			if (i == 0)
			{
				m_SubjectID = itSub->nSubjID;
			}
		}
		m_comboSubject.SetCurSel(0);

		m_nExamID = pExamInfo->nExamID;
		m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
		m_strGradeName = pExamInfo->strGradeName.c_str();
	}

	UpdateData(FALSE);
	return TRUE;
}


void CModelSaveDlg::OnBnClickedBtnSavemodeldlg()
{
	UpdateData(TRUE);

	OnOK();
}


void CModelSaveDlg::OnCbnSelchangeComboExamname()
{
	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemData(n);
	if (!pExamInfo)
		return;

	USES_CONVERSION;
	m_comboSubject.ResetContent();
	SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
	for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
	{
		EXAM_SUBJECT* pSubject = &(*itSub);
		CString strSubjectName = A2T(itSub->strSubjName.c_str());
		m_comboSubject.AddString(strSubjectName);
		int nCount = m_comboSubject.GetCount();
		m_comboSubject.SetItemData(nCount - 1, (DWORD_PTR)pSubject);

		if (i == 0)
		{
			m_SubjectID = itSub->nSubjID;
		}
	}
	m_comboSubject.SetCurSel(0);

	m_nExamID = pExamInfo->nExamID;
	m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
	m_strGradeName = pExamInfo->strGradeName.c_str();
	UpdateData(FALSE);
}


void CModelSaveDlg::OnCbnSelchangeComboSubjectname()
{
	int n2 = m_comboSubject.GetCurSel();

	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemData(n);
	if (!pExamInfo)
		return;

	SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
	for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
	{
		if (i == n2)
		{
			m_SubjectID = itSub->nSubjID;
		}
	}

	UpdateData(FALSE);
}

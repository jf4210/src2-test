// PapersInfoSaveDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "PapersInfoSaveDlg.h"
#include "afxdialogex.h"


// CPapersInfoSaveDlg 对话框

IMPLEMENT_DYNAMIC(CPapersInfoSaveDlg, CDialog)

CPapersInfoSaveDlg::CPapersInfoSaveDlg(pPAPERSINFO pPapers, CWnd* pParent /*=NULL*/)
	: CDialog(CPapersInfoSaveDlg::IDD, pParent)
	, m_nPaperCount(0), m_strPapersName(_T("")), m_strPapersDetail(_T("")), m_pPapers(pPapers)
	, m_strExamTypeName(_T("")), m_strGradeName(_T("")), m_SubjectID(0), m_nExamID(0)
{

}

CPapersInfoSaveDlg::~CPapersInfoSaveDlg()
{
}

void CPapersInfoSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PapersName, m_strPapersName);
	DDX_Text(pDX, IDC_EDIT_PaperCount, m_nPaperCount);
	DDX_Text(pDX, IDC_EDIT_PapersDetail, m_strPapersDetail);
	DDX_Text(pDX, IDC_EDIT_ExamID, m_nExamID);
	DDX_Text(pDX, IDC_EDIT_ExamTypeName, m_strExamTypeName);
	DDX_Text(pDX, IDC_EDIT_GradeName, m_strGradeName);
	DDX_Text(pDX, IDC_EDIT_SubjectID, m_SubjectID);
	DDX_Control(pDX, IDC_COMBO_ExamName, m_comboExamName);
	DDX_Control(pDX, IDC_COMBO_SubjectName, m_comboSubject);
}


BEGIN_MESSAGE_MAP(CPapersInfoSaveDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CPapersInfoSaveDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_ExamName, &CPapersInfoSaveDlg::OnCbnSelchangeComboExamname)
	ON_CBN_SELCHANGE(IDC_COMBO_SubjectName, &CPapersInfoSaveDlg::OnCbnSelchangeComboSubjectname)
END_MESSAGE_MAP()

BOOL CPapersInfoSaveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	m_nPaperCount = m_pPapers->nPaperCount;
	m_strPapersDetail = m_pPapers->strPapersDesc.c_str();
	m_strPapersName = m_pPapers->strPapersName.c_str();

	if (g_lExamList.size() == 0)
	{
		UpdateData(FALSE);
		return FALSE;
	}

	EXAM_LIST::iterator itExam = g_lExamList.begin();
	for (; itExam != g_lExamList.end(); itExam++)
	{
		CString strName = A2T(itExam->strExamName.c_str());

		int nCount = m_comboExamName.GetCount();
		m_comboExamName.InsertString(nCount, strName);

		m_comboExamName.SetItemDataPtr(nCount, (void*)&(*itExam));
	}
	m_comboExamName.SetCurSel(0);


	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemDataPtr(0);
	if (pExamInfo)
	{
		m_comboSubject.ResetContent();
		SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
		for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
		{
			EXAM_SUBJECT* pSubject = &(*itSub);
			CString strSubjectName = A2T(itSub->strSubjName.c_str());

			int nCount = m_comboSubject.GetCount();
			m_comboSubject.InsertString(nCount, strSubjectName);
			m_comboSubject.SetItemDataPtr(nCount, pSubject);

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


// CPapersInfoSaveDlg 消息处理程序


void CPapersInfoSaveDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	USES_CONVERSION;
	m_pPapers->strPapersName = T2A(m_strPapersName);
	m_pPapers->strPapersDesc = T2A(m_strPapersDetail);

	CDialog::OnOK();
}


void CPapersInfoSaveDlg::OnCbnSelchangeComboExamname()
{
	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemDataPtr(n);
	if (!pExamInfo)
		return;

	USES_CONVERSION;
	m_comboSubject.ResetContent();
	SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
	for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
	{
		EXAM_SUBJECT* pSubject = &(*itSub);
		CString strSubjectName = A2T(itSub->strSubjName.c_str());
		int nCount = m_comboSubject.GetCount();
		m_comboSubject.InsertString(nCount, strSubjectName);
		m_comboSubject.SetItemDataPtr(nCount, pSubject);

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


void CPapersInfoSaveDlg::OnCbnSelchangeComboSubjectname()
{
	int n2 = m_comboSubject.GetCurSel();

	USES_CONVERSION;
	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemDataPtr(n);
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

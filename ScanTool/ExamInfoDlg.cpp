// ExamInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ExamInfoDlg.h"
#include "afxdialogex.h"
#include "global.h"


// CExamInfoDlg 对话框

IMPLEMENT_DYNAMIC(CExamInfoDlg, CDialog)

CExamInfoDlg::CExamInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExamInfoDlg::IDD, pParent)
	, m_strExamTypeName(_T("")), m_strGradeName(_T("")), m_SubjectID(0), m_nExamID(0)
{

}

CExamInfoDlg::~CExamInfoDlg()
{
}

void CExamInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ExamID, m_nExamID);
	DDX_Text(pDX, IDC_EDIT_ExamTypeName, m_strExamTypeName);
	DDX_Text(pDX, IDC_EDIT_GradeName, m_strGradeName);
	DDX_Text(pDX, IDC_EDIT_SubjectID, m_SubjectID);
	DDX_Control(pDX, IDC_COMBO_ExamName, m_comboExamName);
	DDX_Control(pDX, IDC_COMBO_SubjectName, m_comboSubject);
}


BEGIN_MESSAGE_MAP(CExamInfoDlg, CDialog)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO_ExamName, &CExamInfoDlg::OnCbnSelchangeComboExamname)
	ON_CBN_SELCHANGE(IDC_COMBO_SubjectName, &CExamInfoDlg::OnCbnSelchangeComboSubjectname)
END_MESSAGE_MAP()

BOOL CExamInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	InitData();

//	m_comboExamName.SetListTooltip(TRUE, TRUE, TRUE);
	m_comboExamName.AdjustDroppedWidth();
//	m_comboSubject.SetListTooltip(TRUE, TRUE, TRUE);
	m_comboSubject.AdjustDroppedWidth();

	return TRUE;
}

void CExamInfoDlg::InitUI()
{
	InitCtrlPosition();
}

void CExamInfoDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 2;
	const int nLeftGap = 2;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	int nLeft_Group = nLeftGap + nGap * 5;
	if (GetDlgItem(IDC_STATIC_Group)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Group)->MoveWindow(nCurrentLeft, nCurrentTop, cx - nRightGap, cy - nBottomGap);
		nCurrentLeft = nLeft_Group;
		nCurrentTop = nCurrentTop + nGap * 10;
	}

	int nStaticW = (cx - nRightGap) / 7;
	int nStaticH = 25;
	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_COMBO_ExamName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_COMBO_ExamName)->MoveWindow(nCurrentLeft, nCurrentTop, 3 * nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + 3 * nStaticW + 5 * nGap;
	}
	if (GetDlgItem(IDC_STATIC_SubjectName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_SubjectName)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_COMBO_SubjectName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_COMBO_SubjectName)->MoveWindow(nCurrentLeft, nCurrentTop, 1 * nStaticW, nStaticH);
		nCurrentLeft = nLeft_Group;
		nCurrentTop = nCurrentTop + nStaticH + nGap * 3;
	}
	//-----------------
	if (GetDlgItem(IDC_STATIC_ExamID)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ExamID)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_EDIT_ExamID)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_ExamID)->MoveWindow(nCurrentLeft, nCurrentTop, 2 * nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + 2 * nStaticW + 5 * nGap;
	}
	if (GetDlgItem(IDC_STATIC_SubjectID)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_SubjectID)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_EDIT_SubjectID)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_SubjectID)->MoveWindow(nCurrentLeft, nCurrentTop, 2 * nStaticW, nStaticH);
		nCurrentLeft = nLeft_Group;
		nCurrentTop = nCurrentTop + nStaticH + nGap * 3;
	}
	//--------------------
	if (GetDlgItem(IDC_STATIC_TypeName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TypeName)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_EDIT_ExamTypeName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_ExamTypeName)->MoveWindow(nCurrentLeft, nCurrentTop, 2 * nStaticW, nStaticH);
		nCurrentLeft = nLeft_Group;
		nCurrentTop = nCurrentTop + nStaticH + nGap * 3;
	}
	//----------------------
	if (GetDlgItem(IDC_STATIC_Grade)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Grade)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_EDIT_GradeName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_GradeName)->MoveWindow(nCurrentLeft, nCurrentTop, 2 * nStaticW, nStaticH);
		nCurrentLeft = nLeft_Group;
		nCurrentTop = nCurrentTop + nStaticH + nGap * 3;
	}
}



// CExamInfoDlg 消息处理程序


void CExamInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

BOOL CExamInfoDlg::InitData()
{
	if (g_lExamList.size() == 0)
	{
		UpdateData(FALSE);
		return FALSE;
	}

	USES_CONVERSION;
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


void CExamInfoDlg::OnCbnSelchangeComboExamname()
{
	UpdateData(TRUE);
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


void CExamInfoDlg::OnCbnSelchangeComboSubjectname()
{
	UpdateData(TRUE);
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

bool CExamInfoDlg::InitShow(pMODEL pModel)
{
	if (!pModel)
		return false;

	if (g_lExamList.size() == 0)
	{
		UpdateData(FALSE);
		return false;
	}

	int nExamShowItem = 0;
	int nSubjectShowItem = 0;

	USES_CONVERSION;
	EXAM_LIST::iterator itExam = g_lExamList.begin();
	for (; itExam != g_lExamList.end(); itExam++)
	{
		CString strName = A2T(itExam->strExamName.c_str());

		int nCount = m_comboExamName.GetCount();
		m_comboExamName.InsertString(nCount, strName);

		m_comboExamName.SetItemDataPtr(nCount, (void*)&(*itExam));

		if (itExam->nExamID == pModel->nExamID)
			nExamShowItem = nCount;
	}
	m_comboExamName.SetCurSel(nExamShowItem);

	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemDataPtr(nExamShowItem);
	if (pExamInfo)
	{
		m_comboSubject.ResetContent();
		int nShowItem = 0;
		SUBJECT_LIST::iterator itSub = pExamInfo->lSubjects.begin();
		for (int i = 0; itSub != pExamInfo->lSubjects.end(); itSub++, i++)
		{
			EXAM_SUBJECT* pSubject = &(*itSub);
			CString strSubjectName = A2T(itSub->strSubjName.c_str());

			int nCount = m_comboSubject.GetCount();
			m_comboSubject.InsertString(nCount, strSubjectName);
			m_comboSubject.SetItemDataPtr(nCount, pSubject);

			if (pSubject->nSubjID == pModel->nSubjectID)
			{
				m_SubjectID = itSub->nSubjID;
				nShowItem = i;
			}
		}
		m_comboSubject.SetCurSel(nShowItem);

		m_nExamID = pExamInfo->nExamID;
		m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
		m_strGradeName = pExamInfo->strGradeName.c_str();
	}

	UpdateData(FALSE);

	return true;
}

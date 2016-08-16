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
	, m_pExamInfoDlg(NULL)
{
}

CPapersInfoSaveDlg::~CPapersInfoSaveDlg()
{
	SAFE_RELEASE(m_pExamInfoDlg);
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
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CPapersInfoSaveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	if (m_pPapers)
	{
		m_nPaperCount = m_pPapers->nPaperCount;
		m_strPapersDetail = m_pPapers->strPapersDesc.c_str();
		m_strPapersName = m_pPapers->strPapersName.c_str();
	}
#if 1

#ifndef TO_WHTY
	m_pExamInfoDlg = new CExamInfoDlg(this);
	m_pExamInfoDlg->Create(CExamInfoDlg::IDD, this);
	m_pExamInfoDlg->ShowWindow(SW_SHOW);

	MoveWindow(0, 0, 450, 350);
	CenterWindow();
#else
	MoveWindow(0, 0, 450, 200);
	CenterWindow();
#endif
	InitCtrlPosition();

#else
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
#endif
	UpdateData(FALSE);
	return TRUE;
}


// CPapersInfoSaveDlg 消息处理程序


void CPapersInfoSaveDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	if (m_pExamInfoDlg)
	{
		m_nExamID = m_pExamInfoDlg->m_nExamID;
		m_SubjectID = m_pExamInfoDlg->m_SubjectID;
		m_strGradeName = m_pExamInfoDlg->m_strGradeName;
		m_strExamTypeName = m_pExamInfoDlg->m_strExamTypeName;
	}	

	if (g_lExamList.size() == 0)
	{
		if (m_strPapersName == _T(""))
		{
			AfxMessageBox(_T("试卷袋名称未设置"));
			return;
		}
	}

	USES_CONVERSION;
	if (m_pPapers)
	{
		m_pPapers->strPapersName = T2A(m_strPapersName);
		m_pPapers->strPapersDesc = T2A(m_strPapersDetail);
	}
	CDialog::OnOK();
}


void CPapersInfoSaveDlg::OnCbnSelchangeComboExamname()
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


void CPapersInfoSaveDlg::OnCbnSelchangeComboSubjectname()
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


void CPapersInfoSaveDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CPapersInfoSaveDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 10;
	const int nLeftGap = 4;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 4;	//右边的空白间隔
	const int nGap = 4;			//普通控件的间隔

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	int nStaticW = (cx - nLeftGap - nRightGap) / 6;
	int nStaticH = 25;
	if (GetDlgItem(IDC_STATIC_PapersName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PapersName)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_EDIT_PapersName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_PapersName)->MoveWindow(nCurrentLeft, nCurrentTop, 2 * nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + 2 * nStaticW + 2 * nGap;
	}
	if (GetDlgItem(IDC_STATIC_PapersCount)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PapersCount)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_EDIT_PaperCount)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_PaperCount)->MoveWindow(nCurrentLeft, nCurrentTop, 2 * nStaticW - 4 * nGap, nStaticH);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nStaticH + nGap * 3;
	}
	//-------------
	if (GetDlgItem(IDC_STATIC_Detail)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Detail)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH * 2);
		nCurrentLeft = nCurrentLeft + nStaticW + nGap;
	}
	if (GetDlgItem(IDC_EDIT_PapersDetail)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_PapersDetail)->MoveWindow(nCurrentLeft, nCurrentTop, cx - nCurrentLeft - nRightGap, nStaticH * 2);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nStaticH * 2 + nGap * 3;
	}
	//-------------------
	int nExamInfo_H = rcClient.Height() - nCurrentTop - nBottomGap - 50;
	if (m_pExamInfoDlg && m_pExamInfoDlg->GetSafeHwnd())
	{
		m_pExamInfoDlg->MoveWindow(nCurrentLeft, nCurrentTop, rcClient.Width() - nLeftGap - nRightGap, nExamInfo_H);
		nCurrentTop = nCurrentTop + nExamInfo_H + nGap * 3;
	}

	//---------------
	if (GetDlgItem(IDOK)->GetSafeHwnd())
	{
		GetDlgItem(IDOK)->MoveWindow(cx * 0.4, nCurrentTop, 50, 25);
	}
}

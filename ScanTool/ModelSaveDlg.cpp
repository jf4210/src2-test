// ModelSaveDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ModelSaveDlg.h"
#include "afxdialogex.h"


// CModelSaveDlg 对话框

IMPLEMENT_DYNAMIC(CModelSaveDlg, CDialog)

CModelSaveDlg::CModelSaveDlg(pMODEL pModel, CWnd* pParent /*=NULL*/)
	: CDialog(CModelSaveDlg::IDD, pParent)
	, m_strExamTypeName(_T("")), m_strGradeName(_T("")), m_strSubjectName(_T("")), m_SubjectID(0), m_nExamID(0), m_nSaveMode(1), m_pModel(pModel), m_strExamName(_T(""))
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
	DDX_Text(pDX, IDC_EDIT_ModelDesc, m_strLocalModelDesc);
	DDX_Control(pDX, IDC_COMBO_ExamName, m_comboExamName);
	DDX_Control(pDX, IDC_COMBO_SubjectName, m_comboSubject);
}


BEGIN_MESSAGE_MAP(CModelSaveDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_SaveModelDlg, &CModelSaveDlg::OnBnClickedBtnSavemodeldlg)
	ON_CBN_SELCHANGE(IDC_COMBO_ExamName, &CModelSaveDlg::OnCbnSelchangeComboExamname)
	ON_CBN_SELCHANGE(IDC_COMBO_SubjectName, &CModelSaveDlg::OnCbnSelchangeComboSubjectname)
	ON_BN_CLICKED(IDC_RADIO_LocalMode, &CModelSaveDlg::OnBnClickedRadioLocalmode)
	ON_BN_CLICKED(IDC_RADIO_RemoteMode, &CModelSaveDlg::OnBnClickedRadioRemotemode)
END_MESSAGE_MAP()


// CModelSaveDlg 消息处理程序

BOOL CModelSaveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	if (g_lExamList.size() > 0)
	{
		EXAM_LIST::iterator itExam = g_lExamList.begin();
		for (; itExam != g_lExamList.end(); itExam++)
		{
			CString strName = A2T(itExam->strExamName.c_str());
			m_strExamName = strName;

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
					m_strSubjectName = itSub->strSubjName.c_str();
				}
			}
			m_comboSubject.SetCurSel(0);

			m_nExamID = pExamInfo->nExamID;
			m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
			m_strGradeName = pExamInfo->strGradeName.c_str();
		}
	}	
	m_nSaveMode = m_pModel->nSaveMode;
	if (m_nSaveMode == 1)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LocalMode))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_RemoteMode))->SetCheck(0);
		SetLocalModelEnable(TRUE);
		SetNetModelEnable(FALSE);
	}
	else if (m_nSaveMode == 2)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LocalMode))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_RemoteMode))->SetCheck(1);
		SetLocalModelEnable(FALSE);
		SetNetModelEnable(TRUE);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LocalMode))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_RemoteMode))->SetCheck(0);
	}
	if (m_pModel)
	{
		m_strLocalModelDesc = A2T(m_pModel->strModelDesc.c_str());
		m_strModelName		= A2T(m_pModel->strModelName.c_str());
	}	

	UpdateData(FALSE);
	return TRUE;
}


void CModelSaveDlg::OnBnClickedBtnSavemodeldlg()
{
	UpdateData(TRUE);

	if (m_nSaveMode == 1)
	{
		if (m_strModelName == _T(""))
		{
			AfxMessageBox(_T("请设置模板保存名称！"));
			return;
		}
	}
	else if (m_nSaveMode == 2)
	{
		if (m_nExamID == 0 || m_SubjectID == 0)
		{
			AfxMessageBox(_T("当前考试信息不完整，请重新选择"));
			return;
		}
	}
	else
	{
		AfxMessageBox(_T("请设置模板保存方式！"));
		return;
	}

	OnOK();
}


void CModelSaveDlg::OnCbnSelchangeComboExamname()
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
			m_strSubjectName = itSub->strSubjName.c_str();
		}
	}
	m_comboSubject.SetCurSel(0);

	m_strExamName = pExamInfo->strExamName.c_str();
	m_nExamID = pExamInfo->nExamID;
	m_strExamTypeName = pExamInfo->strExamTypeName.c_str();
	m_strGradeName = pExamInfo->strGradeName.c_str();
	UpdateData(FALSE);
}


void CModelSaveDlg::OnCbnSelchangeComboSubjectname()
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
			m_strSubjectName = itSub->strSubjName.c_str();
		}
	}

	UpdateData(FALSE);
}


void CModelSaveDlg::OnBnClickedRadioLocalmode()
{
	if (m_nSaveMode != 1)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LocalMode))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_RemoteMode))->SetCheck(0);
		m_nSaveMode = 1;

		SetLocalModelEnable(TRUE);
		SetNetModelEnable(FALSE);
	}
}


void CModelSaveDlg::OnBnClickedRadioRemotemode()
{
	if (m_nSaveMode != 2)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LocalMode))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_RemoteMode))->SetCheck(1);
		m_nSaveMode = 2;

		SetLocalModelEnable(FALSE);
		SetNetModelEnable(TRUE);
	}
}

void CModelSaveDlg::SetLocalModelEnable(BOOL bEnable)
{
	GetDlgItem(IDC_EDIT_ModelName)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_ModelDesc)->EnableWindow(bEnable);
}

void CModelSaveDlg::SetNetModelEnable(BOOL bEnable)
{
	GetDlgItem(IDC_COMBO_ExamName)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_SubjectName)->EnableWindow(bEnable);
}

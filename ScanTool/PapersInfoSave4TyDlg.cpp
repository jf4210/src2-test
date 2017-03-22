// PapersInfoSave4TyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "PapersInfoSave4TyDlg.h"
#include "afxdialogex.h"


// CPapersInfoSave4TyDlg 对话框

IMPLEMENT_DYNAMIC(CPapersInfoSave4TyDlg, CDialog)

CPapersInfoSave4TyDlg::CPapersInfoSave4TyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPapersInfoSave4TyDlg::IDD, pParent)
	, m_strExamID(_T(""))
{

}

CPapersInfoSave4TyDlg::~CPapersInfoSave4TyDlg()
{
}

void CPapersInfoSave4TyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_ExamList4Ty, m_comboExamName);
	DDX_Text(pDX, IDC_EDIT_ExamId4Ty, m_strExamID);
}

BEGIN_MESSAGE_MAP(CPapersInfoSave4TyDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CPapersInfoSave4TyDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_ExamList4Ty, &CPapersInfoSave4TyDlg::OnCbnSelchangeComboExamlist4ty)
END_MESSAGE_MAP()


// CPapersInfoSave4TyDlg 消息处理程序
BOOL CPapersInfoSave4TyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;

	int nItem = 0;
	char* ret;
	ret = new char[20];
	ret[0] = '\0';
	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "papersSave4Ty", ret) == 0)
	{
		nItem = atoi(ret);
	}
	SAFE_RELEASE_ARRY(ret);

	EXAM_LIST::iterator itExam = g_lExamList.begin();
	for (int i = 0; itExam != g_lExamList.end(); itExam++, i++)
	{
		CString strName = A2T(itExam->strExamName.c_str());

		int nCount = m_comboExamName.GetCount();
		m_comboExamName.InsertString(nCount, strName);

		m_comboExamName.SetItemDataPtr(nCount, (void*)&(*itExam));
		if (nItem == i)
		{
			m_strExamID = itExam->strExamID.c_str();
		}
	}
	if (m_comboExamName.GetCount() > nItem)
		m_comboExamName.SetCurSel(nItem);

	UpdateData(FALSE);
	return TRUE;
}


void CPapersInfoSave4TyDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	char szRet[20] = { 0 };
	sprintf_s(szRet, "%d", m_comboExamName.GetCurSel());
	WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "papersSave4Ty", szRet);

	CDialog::OnOK();
}


void CPapersInfoSave4TyDlg::OnCbnSelchangeComboExamlist4ty()
{
	UpdateData(TRUE);
	int n = m_comboExamName.GetCurSel();
	EXAMINFO* pExamInfo = (EXAMINFO*)m_comboExamName.GetItemDataPtr(n);
	if (!pExamInfo)
		return;

	m_strExamID = pExamInfo->strExamID.c_str();
	UpdateData(FALSE);
}

// RecogParamDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DataMgrTool.h"
#include "RecogParamDlg.h"
#include "afxdialogex.h"


// CRecogParamDlg 对话框

IMPLEMENT_DYNAMIC(CRecogParamDlg, CDialog)

CRecogParamDlg::CRecogParamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecogParamDlg::IDD, pParent)
	, m_nChkSN(1), m_nChkElectOmr(1), m_nChkOmr(1)
{

}

CRecogParamDlg::~CRecogParamDlg()
{
}

void CRecogParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_EZS, m_strEzsAddr);
}


BEGIN_MESSAGE_MAP(CRecogParamDlg, CDialog)
	ON_BN_CLICKED(IDC_CHK_SN, &CRecogParamDlg::OnBnClickedChkSn)
	ON_BN_CLICKED(IDC_CHK_OMR, &CRecogParamDlg::OnBnClickedChkOmr)
	ON_BN_CLICKED(IDC_CHK_ElecOmr, &CRecogParamDlg::OnBnClickedChkElecomr)
	ON_BN_CLICKED(IDOK, &CRecogParamDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CRecogParamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	m_strEzsAddr = A2T(g_strUploadUri.c_str());

	((CButton*)GetDlgItem(IDC_CHK_SN))->SetCheck(m_nChkSN);
	((CButton*)GetDlgItem(IDC_CHK_OMR))->SetCheck(m_nChkSN);
	((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->SetCheck(m_nChkSN);

	UpdateData(FALSE);

	return TRUE;
}


// CRecogParamDlg 消息处理程序


void CRecogParamDlg::OnBnClickedChkSn()
{
	if (((CButton*)GetDlgItem(IDC_CHK_SN))->GetCheck())
	{
		((CButton*)GetDlgItem(IDC_CHK_SN))->SetCheck(0);
		m_nChkSN = 0;
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHK_SN))->SetCheck(1);
		m_nChkSN = 1;
	}
}


void CRecogParamDlg::OnBnClickedChkOmr()
{
	if (((CButton*)GetDlgItem(IDC_CHK_OMR))->GetCheck())
	{
		((CButton*)GetDlgItem(IDC_CHK_OMR))->SetCheck(0);
		m_nChkOmr = 0;
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHK_OMR))->SetCheck(1);
		m_nChkOmr = 1;
	}
}


void CRecogParamDlg::OnBnClickedChkElecomr()
{
	if (((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->GetCheck())
	{
		((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->SetCheck(0);
		m_nChkElectOmr = 0;
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->SetCheck(1);
		m_nChkElectOmr = 1;
	}
}


void CRecogParamDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	USES_CONVERSION;
	g_strUploadUri = T2A(m_strEzsAddr);

	CDialog::OnOK();
}

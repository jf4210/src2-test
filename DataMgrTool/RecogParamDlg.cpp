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
	, m_nChkSN(1), m_nChkElectOmr(1), m_nChkOmr(1), m_nHandleResult(2), m_nNoRecogVal(0), m_nRecogMode(g_nRecogMode), m_nRecogChkRotation(g_nRecogChkRotation), m_nRecogEasyModel(g_nRecogEasyModel)
{

}

CRecogParamDlg::~CRecogParamDlg()
{
}

void CRecogParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_EZS, m_strEzsAddr);
	DDX_Text(pDX, IDC_EDIT_NoRecog, m_nNoRecogVal);
}


BEGIN_MESSAGE_MAP(CRecogParamDlg, CDialog)
	ON_BN_CLICKED(IDC_CHK_SN, &CRecogParamDlg::OnBnClickedChkSn)
	ON_BN_CLICKED(IDC_CHK_OMR, &CRecogParamDlg::OnBnClickedChkOmr)
	ON_BN_CLICKED(IDC_CHK_ElecOmr, &CRecogParamDlg::OnBnClickedChkElecomr)
	ON_BN_CLICKED(IDOK, &CRecogParamDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_CompressPKG, &CRecogParamDlg::OnBnClickedRadioCompresspkg)
	ON_BN_CLICKED(IDC_RADIO_SendEZS, &CRecogParamDlg::OnBnClickedRadioSendezs)
	ON_BN_CLICKED(IDC_RADIO_NoCompress, &CRecogParamDlg::OnBnClickedRadioNocompress)
	ON_BN_CLICKED(IDC_CHK_RecogMode, &CRecogParamDlg::OnBnClickedChkRecogmode)
	ON_BN_CLICKED(IDC_CHK_RecogChkRotation, &CRecogParamDlg::OnBnClickedChkRecogchkrotation)
	ON_BN_CLICKED(IDC_CHK_RecogEasyModel, &CRecogParamDlg::OnBnClickedChkRecogeasymodel)
END_MESSAGE_MAP()

BOOL CRecogParamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	m_strEzsAddr = A2T(g_strUploadUri.c_str());

	((CButton*)GetDlgItem(IDC_CHK_SN))->SetCheck(m_nChkSN);
	((CButton*)GetDlgItem(IDC_CHK_OMR))->SetCheck(m_nChkSN);
	((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->SetCheck(m_nChkSN);

	if (m_nHandleResult == 0)
	{
		((CButton*)GetDlgItem(IDC_RADIO_CompressPKG))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_SendEZS))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_NoCompress))->SetCheck(0);
		GetDlgItem(IDC_EDIT_EZS)->EnableWindow(FALSE);
	}
	else if (m_nHandleResult == 1)
	{
		((CButton*)GetDlgItem(IDC_RADIO_CompressPKG))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_SendEZS))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_NoCompress))->SetCheck(0);
		GetDlgItem(IDC_EDIT_EZS)->EnableWindow(TRUE);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_CompressPKG))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_SendEZS))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_NoCompress))->SetCheck(1);
		GetDlgItem(IDC_EDIT_EZS)->EnableWindow(TRUE);
	}

	((CButton*)GetDlgItem(IDC_CHK_RecogMode))->SetCheck(m_nRecogMode);
	((CButton*)GetDlgItem(IDC_CHK_RecogChkRotation))->SetCheck(m_nRecogChkRotation);
	((CButton*)GetDlgItem(IDC_CHK_RecogEasyModel))->SetCheck(m_nRecogEasyModel);

	UpdateData(FALSE);

	return TRUE;
}


// CRecogParamDlg 消息处理程序


void CRecogParamDlg::OnBnClickedChkSn()
{
	if (((CButton*)GetDlgItem(IDC_CHK_SN))->GetCheck())
	{
		m_nChkSN = 1;
	}
	else
	{
		m_nChkSN = 0;
	}
}


void CRecogParamDlg::OnBnClickedChkOmr()
{
	if (((CButton*)GetDlgItem(IDC_CHK_OMR))->GetCheck())
	{
		m_nChkOmr = 1;
	}
	else
	{
		m_nChkOmr = 0;
	}
}


void CRecogParamDlg::OnBnClickedChkElecomr()
{
	if (((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->GetCheck())
	{
		m_nChkElectOmr = 1;
	}
	else
	{
		m_nChkElectOmr = 0;
	}
}

void CRecogParamDlg::OnBnClickedChkRecogmode()
{
	if (((CButton*)GetDlgItem(IDC_CHK_RecogMode))->GetCheck())
	{
		m_nRecogMode = 1;
	}
	else
		m_nRecogMode = 0;
}

void CRecogParamDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	USES_CONVERSION;
	g_strUploadUri = T2A(m_strEzsAddr);
	if (m_nHandleResult == 0)
	{
		m_nChkElectOmr = 1;
		m_nChkOmr = 1;
		m_nChkSN = 1;
	}

	CDialog::OnOK();
}


void CRecogParamDlg::OnBnClickedRadioCompresspkg()
{
// 	((CButton*)GetDlgItem(IDC_RADIO_CompressPKG))->SetCheck(1);
// 	((CButton*)GetDlgItem(IDC_RADIO_SendEZS))->SetCheck(0);
	GetDlgItem(IDC_EDIT_EZS)->EnableWindow(FALSE);
	m_nHandleResult = 0;

	m_nChkElectOmr = 1;
	m_nChkOmr = 1;
	m_nChkSN = 1;
	((CButton*)GetDlgItem(IDC_CHK_SN))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHK_OMR))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->SetCheck(1);
}


void CRecogParamDlg::OnBnClickedRadioSendezs()
{
	GetDlgItem(IDC_EDIT_EZS)->EnableWindow(TRUE);
	m_nHandleResult = 1;

	m_nChkElectOmr = 1;
	m_nChkOmr = 1;
	m_nChkSN = 1;
	((CButton*)GetDlgItem(IDC_CHK_SN))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHK_OMR))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->SetCheck(1);
}


void CRecogParamDlg::OnBnClickedRadioNocompress()
{
	GetDlgItem(IDC_EDIT_EZS)->EnableWindow(FALSE);
	m_nHandleResult = 2;

	m_nChkElectOmr = 1;
	m_nChkOmr = 1;
	m_nChkSN = 1;
	((CButton*)GetDlgItem(IDC_CHK_SN))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHK_OMR))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHK_ElecOmr))->SetCheck(1);
}




void CRecogParamDlg::OnBnClickedChkRecogchkrotation()
{
	if (((CButton*)GetDlgItem(IDC_CHK_RecogChkRotation))->GetCheck())
	{
		m_nRecogChkRotation = 1;
	}
	else
		m_nRecogChkRotation = 0;
}


void CRecogParamDlg::OnBnClickedChkRecogeasymodel()
{
	if (((CButton*)GetDlgItem(IDC_CHK_RecogEasyModel))->GetCheck())
		m_nRecogEasyModel = 1;
	else
		m_nRecogEasyModel = 0;
}

// AdvancedSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "AdvancedSetDlg.h"
#include "afxdialogex.h"


// CAdvancedSetDlg 对话框

IMPLEMENT_DYNAMIC(CAdvancedSetDlg, CDialog)

CAdvancedSetDlg::CAdvancedSetDlg(pMODEL	pModel, CWnd* pParent /*=NULL*/)
	: CDialog(CAdvancedSetDlg::IDD, pParent)
	, m_pModel(pModel), m_nScanDpi(200), m_nAutoCut(1)
{

}

CAdvancedSetDlg::~CAdvancedSetDlg()
{
}

void CAdvancedSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_ScanDpi, m_combo_ScanDpi);
	DDX_Control(pDX, IDC_CHK_AutoCut, m_chkAutoCut);
}


BOOL CAdvancedSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitData();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CAdvancedSetDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_ScanDpi, &CAdvancedSetDlg::OnCbnSelchangeComboScandpi)
	ON_BN_CLICKED(IDOK, &CAdvancedSetDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHK_AutoCut, &CAdvancedSetDlg::OnBnClickedChkAutocut)
END_MESSAGE_MAP()


// CAdvancedSetDlg 消息处理程序

void CAdvancedSetDlg::InitData()
{
	m_combo_ScanDpi.ResetContent();
	m_combo_ScanDpi.AddString(_T("150"));
	m_combo_ScanDpi.AddString(_T("200"));

	if (!m_pModel)
	{
		m_combo_ScanDpi.SetCurSel(1);
		m_chkAutoCut.SetCheck(FALSE);
		m_nAutoCut = 0;
		return;
	}

	m_nScanDpi = m_pModel->nScanDpi;

	if (m_pModel->nScanDpi == 150)
		m_combo_ScanDpi.SetCurSel(0);
	else if (m_pModel->nScanDpi == 200)
		m_combo_ScanDpi.SetCurSel(1);
	else
	{
		CString strTmp = _T("");
		strTmp.Format(_T("%d"), m_pModel->nScanDpi);
		m_combo_ScanDpi.AddString(strTmp);
		m_combo_ScanDpi.SetCurSel(2);
	}

	m_nAutoCut = m_pModel->nAutoCut;
	m_chkAutoCut.SetCheck(m_nAutoCut);

	return;
}

void CAdvancedSetDlg::OnCbnSelchangeComboScandpi()
{
	if (m_combo_ScanDpi.GetCurSel() < 0 || m_pModel == NULL)
		return;

	USES_CONVERSION;
	CString strDpi = _T("");
	m_combo_ScanDpi.GetLBText(m_combo_ScanDpi.GetCurSel(), strDpi);
	m_nScanDpi = atoi(T2A(strDpi));
}


void CAdvancedSetDlg::OnBnClickedOk()
{
	CDialog::OnOK();
}

void CAdvancedSetDlg::OnBnClickedChkAutocut()
{
	m_nAutoCut = m_chkAutoCut.GetCheck();
}

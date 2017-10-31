// AdvancedSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanParamSetDlg.h"
#include "afxdialogex.h"
#include "AdanceSetMgrDlg.h"

// CAdvancedSetDlg 对话框

IMPLEMENT_DYNAMIC(CScanParamSetDlg, CDialog)

CScanParamSetDlg::CScanParamSetDlg(CWnd* pParent /*=NULL*/)
: CBaseTabDlg(CScanParamSetDlg::IDD, pParent)
	, m_nScanDpi(200), m_nAutoCut(1), m_nScanPaperSize(1), m_nScanType(2), m_nUseWordAnchorPoint(0)
{

}

CScanParamSetDlg::~CScanParamSetDlg()
{
}

void CScanParamSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseTabDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_ScanDpi, m_combo_ScanDpi);
	DDX_Control(pDX, IDC_COMBO_PaperSize, m_combo_PaperSize);
	DDX_Control(pDX, IDC_CHK_AutoCut, m_chkAutoCut);
	DDX_Control(pDX, IDC_COMBO_ScanType, m_combo_ScanType);
	DDX_Control(pDX, IDC_STATIC_G_Scan, m_groupScanInfo);
	DDX_Control(pDX, IDC_STATIC_G_Model, m_groupModelInfo);
	DDX_Control(pDX, IDC_CHK_UseWordAnchorPoint, m_chkUseWordAnchorPoint);
}


BOOL CScanParamSetDlg::OnInitDialog()
{
	CBaseTabDlg::OnInitDialog();
	
	m_groupScanInfo.SetCatptionTextColor(RGB(0, 0, 0))
		.SetBackgroundColor(RGB(255, 255, 255));
	m_groupModelInfo.SetCatptionTextColor(RGB(0, 0, 0))
		.SetBackgroundColor(RGB(255, 255, 255));

#ifndef USE_TESSERACT
	m_groupModelInfo.ShowWindow(SW_HIDE);
	m_chkUseWordAnchorPoint.ShowWindow(SW_HIDE);
#endif

	return TRUE;
}

BEGIN_MESSAGE_MAP(CScanParamSetDlg, CBaseTabDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_ScanDpi, &CScanParamSetDlg::OnCbnSelchangeComboScandpi)
	ON_BN_CLICKED(IDC_CHK_AutoCut, &CScanParamSetDlg::OnBnClickedChkAutocut)
	ON_CBN_SELCHANGE(IDC_COMBO_PaperSize, &CScanParamSetDlg::OnCbnSelchangeComboPapersize)
	ON_CBN_SELCHANGE(IDC_COMBO_ScanType, &CScanParamSetDlg::OnCbnSelchangeComboScantype)

	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_CHK_UseWordAnchorPoint, &CScanParamSetDlg::OnBnClickedChkUsewordanchorpoint)
END_MESSAGE_MAP()


// CAdvancedSetDlg 消息处理程序

void CScanParamSetDlg::InitData(AdvanceParam& stParam)
{
	m_combo_ScanDpi.ResetContent();
	m_combo_ScanDpi.AddString(_T("150"));
	m_combo_ScanDpi.AddString(_T("200"));

	m_combo_PaperSize.ResetContent();
	m_combo_PaperSize.AddString(_T("A4"));
	m_combo_PaperSize.AddString(_T("A3"));
	m_combo_PaperSize.AddString(_T("定制"));

	m_combo_ScanType.ResetContent();
	m_combo_ScanType.AddString(_T("灰度扫描"));
	m_combo_ScanType.AddString(_T("彩色扫描"));
	
	m_nScanDpi = stParam.nScanDpi;
	if (m_nScanDpi == 150)
		m_combo_ScanDpi.SetCurSel(0);
	else if (m_nScanDpi == 200)
		m_combo_ScanDpi.SetCurSel(1);
	else
	{
		CString strTmp = _T("");
		strTmp.Format(_T("%d"), m_nScanDpi);
		m_combo_ScanDpi.AddString(strTmp);
		m_combo_ScanDpi.SetCurSel(2);
	}

	m_nScanPaperSize = stParam.nScanPaperSize;
	if (stParam.nScanPaperSize == 1)
		m_combo_PaperSize.SetCurSel(0);
	else if (stParam.nScanPaperSize == 2)
		m_combo_PaperSize.SetCurSel(1);
	else
		m_combo_PaperSize.SetCurSel(2);

	if (stParam.nScanType == 1)
	{
		m_combo_ScanType.SetCurSel(0);
		m_nScanType = 1;
	}
	else
	{
		m_combo_ScanType.SetCurSel(1);
		m_nScanType = 2;
	}

	m_nAutoCut = stParam.nAutoCut;
	m_chkAutoCut.SetCheck(m_nAutoCut);

	m_nUseWordAnchorPoint = stParam.nUseWordAnchorPoint;
	m_chkUseWordAnchorPoint.SetCheck(m_nUseWordAnchorPoint);
	return;
}

BOOL CScanParamSetDlg::SaveParamData(AdvanceParam& stParam)
{
	UpdateData(TRUE);
	stParam.nScanDpi	= m_nScanDpi;
	stParam.nScanPaperSize = m_nScanPaperSize;
	stParam.nScanType	= m_nScanType;
	stParam.nAutoCut	= m_nAutoCut;
	stParam.nUseWordAnchorPoint = m_nUseWordAnchorPoint;
	return TRUE;
}

void CScanParamSetDlg::OnCbnSelchangeComboScandpi()
{
	if (m_combo_ScanDpi.GetCurSel() < 0 )
		return;

	USES_CONVERSION;
	CString strDpi = _T("");
	m_combo_ScanDpi.GetLBText(m_combo_ScanDpi.GetCurSel(), strDpi);
	m_nScanDpi = atoi(T2A(strDpi));
}

void CScanParamSetDlg::OnBnClickedChkAutocut()
{
	m_nAutoCut = m_chkAutoCut.GetCheck();
}


void CScanParamSetDlg::OnCbnSelchangeComboPapersize()
{
	if (m_combo_PaperSize.GetCurSel() < 0 )
		return;

	m_nScanPaperSize = m_combo_PaperSize.GetCurSel() + 1;
}


void CScanParamSetDlg::OnCbnSelchangeComboScantype()
{
	if (m_combo_ScanType.GetCurSel() < 0 )
		return;

	m_nScanType = m_combo_ScanType.GetCurSel() + 1;
}

HBRUSH CScanParamSetDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_CHK_AutoCut || CurID == IDC_CHK_UseWordAnchorPoint)
	{
		HBRUSH hMYbr = ::CreateSolidBrush(RGB(255, 255, 255));	//62, 147, 254

		pDC->SetBkMode(TRANSPARENT);

		return hMYbr;
	}
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

BOOL CScanParamSetDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));
	ReleaseDC(pDC);	

	return CBaseTabDlg::OnEraseBkgnd(pDC);
}


void CScanParamSetDlg::OnBnClickedChkUsewordanchorpoint()
{
	m_nUseWordAnchorPoint = m_chkUseWordAnchorPoint.GetCheck();
}

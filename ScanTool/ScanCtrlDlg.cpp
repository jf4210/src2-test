// ScanCtrlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanCtrlDlg.h"
#include "afxdialogex.h"
#include "global.h"

// CScanCtrlDlg 对话框

IMPLEMENT_DYNAMIC(CScanCtrlDlg, CDialog)

CScanCtrlDlg::CScanCtrlDlg(SCANSRCARRY& srcArry, bool bShowPaperSize, CWnd* pParent /*=NULL*/)
: CDialog(CScanCtrlDlg::IDD, pParent)
, m_nStudentNum(0), sourceArry(srcArry), m_nCurrScanSrc(0), m_nCurrDuplex(0), m_bAdvancedSetting(false), m_nCurrPaperSize(0)
, m_bShowPaperSize(bShowPaperSize)
{

}

CScanCtrlDlg::~CScanCtrlDlg()
{
}

void CScanCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DUPLEX, m_comboDuplex);
	DDX_Control(pDX, IDC_COMBO_SCANSRC, m_comboScanSrc);
	DDX_Control(pDX, IDC_COMBO_ModelPaperSize, m_comboModelPaperSize);
	DDX_Text(pDX, IDC_EDIT_TiKaNum, m_nStudentNum);
}

BEGIN_MESSAGE_MAP(CScanCtrlDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_SCANSRC, &CScanCtrlDlg::OnCbnSelchangeComboScansrc)
	ON_CBN_SELCHANGE(IDC_COMBO_DUPLEX, &CScanCtrlDlg::OnCbnSelchangeComboDuplex)
	ON_BN_CLICKED(IDC_BTN_Scan, &CScanCtrlDlg::OnBnClickedBtnScan)
	ON_BN_CLICKED(IDC_BTN_Advanced, &CScanCtrlDlg::OnBnClickedBtnAdvanced)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO_ModelPaperSize, &CScanCtrlDlg::OnCbnSelchangeComboModelpapersize)
END_MESSAGE_MAP()

void CScanCtrlDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nGap = 5;	//控件的间隔
	int nTopGap = 10;	//距离上边边缘的间隔
	int nBottomGap = 5;	//距离下边边缘的间隔
	int nLeftGap = 30;	//距离左边边缘的间隔
	int nRightGap = 30;	//距离右边边缘的间隔
	int nBtnHeigh = 60;
	int	nBtnWidth = 70;

	int nStaticWidth = 65;
	int nStaticHeight = 25;		//校验点类型Static控件高度
	int nCommoboxWidth = cx - nLeftGap - nRightGap - nStaticWidth - nGap;
	int nCommoboxHeight = 25;	//校验点combox的高度
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	if (GetDlgItem(IDC_STATIC_SCANSRC)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_SCANSRC)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nCurrentLeft + nStaticWidth + nGap;
	}
	if (m_comboScanSrc.GetSafeHwnd())
	{
		m_comboScanSrc.MoveWindow(nCurrentLeft, nCurrentTop, nCommoboxWidth, nCommoboxHeight);
		nCurrentLeft = nLeftGap;
		nCurrentTop += nStaticHeight + nGap;
	}

	if (GetDlgItem(IDC_STATIC_DUPLEX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_DUPLEX)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nCurrentLeft + nStaticWidth + nGap;
	}
	if (m_comboDuplex.GetSafeHwnd())
	{
		m_comboDuplex.MoveWindow(nCurrentLeft, nCurrentTop, nCommoboxWidth, nCommoboxHeight);
		nCurrentLeft = nLeftGap;
		nCurrentTop += nStaticHeight + nGap;
	}

	if (m_bShowPaperSize)
	{
		if (GetDlgItem(IDC_STATIC_PaperSize)->GetSafeHwnd())
		{
			GetDlgItem(IDC_STATIC_PaperSize)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
			nCurrentLeft = nCurrentLeft + nStaticWidth + nGap;
		}
		if (m_comboModelPaperSize.GetSafeHwnd())
		{
			m_comboModelPaperSize.MoveWindow(nCurrentLeft, nCurrentTop, nCommoboxWidth, nCommoboxHeight);
			nCurrentLeft = nLeftGap;
			nCurrentTop += nStaticHeight + nGap;
		}
	}	

	if (GetDlgItem(IDC_STATIC_COUNT)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_COUNT)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nCurrentLeft + nStaticWidth + nGap;
	}
	if (GetDlgItem(IDC_EDIT_TiKaNum)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_TiKaNum)->MoveWindow(nCurrentLeft, nCurrentTop, nCommoboxWidth, nStaticHeight);
		nCurrentLeft = nLeftGap;
		nCurrentTop += nStaticHeight + nGap;
	}

	nCurrentLeft = (cx - nGap - nBtnWidth * 2) / 2;
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		int nW = nBtnWidth + 20;
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nBtnHeigh);
		nCurrentLeft = nCurrentLeft + nW + nGap;
	}
	if (GetDlgItem(IDC_BTN_Advanced)->GetSafeHwnd())
	{
		int nW = nBtnWidth - 20;
		GetDlgItem(IDC_BTN_Advanced)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nBtnHeigh);
		nCurrentLeft += nGap;
	}
}

BOOL CScanCtrlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	InitCtrlPosition();
	InitUI();

	return TRUE;
}

bool CScanCtrlDlg::ScanSrcInit()
{
	USES_CONVERSION;
	for (int i = 0; i < sourceArry.GetCount(); i++)
	{
		TW_IDENTITY temp_Source = sourceArry.GetAt(i);
		m_comboScanSrc.AddString(A2T(temp_Source.ProductName));
	}

	if (sourceArry.GetCount() == 0)
	{
		m_comboScanSrc.AddString(_T("没有可用的打印机"));
		return false;
	}
	return true;
}

bool CScanCtrlDlg::InitUI()
{
	ScanSrcInit();

	USES_CONVERSION;
	
#ifdef TO_WHTY
	GetDlgItem(IDC_STATIC_COUNT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_TiKaNum)->ShowWindow(SW_HIDE);
#endif

	m_comboDuplex.AddString(_T("单面扫描"));
	m_comboDuplex.AddString(_T("双面扫描"));

	m_comboModelPaperSize.AddString(_T("A4"));
	m_comboModelPaperSize.AddString(_T("A3"));
	m_comboModelPaperSize.AddString(_T("定制"));

	int sx = 300;
	int sy = 200;
	if (!m_bShowPaperSize)
	{
		GetDlgItem(IDC_STATIC_PaperSize)->ShowWindow(SW_HIDE);
		m_comboModelPaperSize.ShowWindow(SW_HIDE);
	}
	else
	{
		sx = 300;
		sy = 230;
	}
	MoveWindow(0, 0, sx, sy);
	CenterWindow();

	int nSrc = 0;
	int nDuplex = 1;
	char* ret;
	ret = new char[20];
	ret[0] = '\0';
	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanSrc", ret) == 0)
	{
		nSrc = atoi(ret);
	}
	memset(ret, 0, 20);

	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanDuplex", ret) == 0)
	{
		nDuplex = atoi(ret);
	}
	SAFE_RELEASE_ARRY(ret);

	m_comboScanSrc.SetCurSel(nSrc);
	m_comboDuplex.SetCurSel(nDuplex);
	m_comboModelPaperSize.SetCurSel(m_nCurrPaperSize);
	m_nCurrScanSrc = nSrc;
	m_nCurrDuplex = nDuplex;

	UpdateData(FALSE);
	return true;
}


// CScanCtrlDlg 消息处理程序


void CScanCtrlDlg::OnCbnSelchangeComboScansrc()
{
	if (sourceArry.GetCount())
	{
		m_nCurrScanSrc = m_comboScanSrc.GetCurSel();
	}
}

void CScanCtrlDlg::OnCbnSelchangeComboDuplex()
{
	m_nCurrDuplex = m_comboDuplex.GetCurSel();
}

void CScanCtrlDlg::OnBnClickedBtnScan()
{
	UpdateData(TRUE);

	char szRet[20] = { 0 };
	sprintf_s(szRet, "%d", m_nCurrScanSrc);
	WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanSrc", szRet);
	memset(szRet, 0, 20);
	sprintf_s(szRet, "%d", m_nCurrDuplex);
	WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanDuplex", szRet);

	OnOK();
}


void CScanCtrlDlg::OnBnClickedBtnAdvanced()
{
	m_bAdvancedSetting = true;
	OnBnClickedBtnScan();
}


void CScanCtrlDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}


void CScanCtrlDlg::OnCbnSelchangeComboModelpapersize()
{
	m_nCurrPaperSize = m_comboModelPaperSize.GetCurSel();
}

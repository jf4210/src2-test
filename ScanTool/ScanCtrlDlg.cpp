// ScanCtrlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanCtrlDlg.h"
#include "afxdialogex.h"
#include "global.h"

// CScanCtrlDlg 对话框

IMPLEMENT_DYNAMIC(CScanCtrlDlg, CDialog)

CScanCtrlDlg::CScanCtrlDlg(SCANSRCARRY& srcArry, CWnd* pParent /*=NULL*/)
: CDialog(CScanCtrlDlg::IDD, pParent)
, m_nStudentNum(0), sourceArry(srcArry), m_nCurrScanSrc(0), m_nCurrDuplex(0)
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
	DDX_Text(pDX, IDC_EDIT_TiKaNum, m_nStudentNum);
}

BEGIN_MESSAGE_MAP(CScanCtrlDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_SCANSRC, &CScanCtrlDlg::OnCbnSelchangeComboScansrc)
	ON_CBN_SELCHANGE(IDC_COMBO_DUPLEX, &CScanCtrlDlg::OnCbnSelchangeComboDuplex)
	ON_BN_CLICKED(IDC_BTN_Scan, &CScanCtrlDlg::OnBnClickedBtnScan)
END_MESSAGE_MAP()

BOOL CScanCtrlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
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

// OmrInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "OmrInfoDlg.h"
#include "afxdialogex.h"


// COmrInfoDlg 对话框

IMPLEMENT_DYNAMIC(COmrInfoDlg, CDialog)

COmrInfoDlg::COmrInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COmrInfoDlg::IDD, pParent)
{

}

COmrInfoDlg::~COmrInfoDlg()
{
}

void COmrInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PIC_OMR, m_picOmrShow);
	DDX_Control(pDX, IDC_RADIO_TH_H, m_radioTH_H);
	DDX_Control(pDX, IDC_RADIO_TH_V, m_radioTH_V);
	DDX_Control(pDX, IDC_RADIO_XX_H, m_radioXX_H);
	DDX_Control(pDX, IDC_RADIO_XX_V, m_radioXX_V);
	DDX_Control(pDX, IDC_RADIO_Direct_ZX, m_radioDirectZX);
	DDX_Control(pDX, IDC_RADIO_Direct_FX, m_radioDirectFX);
}


BEGIN_MESSAGE_MAP(COmrInfoDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_THEMECHANGED, IDC_RADIO_TH_H, &COmrInfoDlg::OnNMThemeChangedRadioThH)
END_MESSAGE_MAP()

BOOL COmrInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);
	InitCtrlPosition();

	m_picOmrShow.OnInit(0, false);
	CString strPicPath = g_strCurrentPath;
	strPicPath.Append(_T("竖直横向1.png"));

	m_src_img = cv::imread((std::string)(CT2CA)strPicPath);
	m_picOmrShow.ShowImage(m_src_img, 0);
	return TRUE;
}


// COmrInfoDlg 消息处理程序


void COmrInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	InitCtrlPosition();
}

void COmrInfoDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int nGap = 2;	//控件的间隔
	int nTopGap = 2;	//距离上边边缘的间隔
	int nBottomGap = 2;	//距离下边边缘的间隔
	int nLeftGap = 2;	//距离左边边缘的间隔
	int nRightGap = 2;	//距离右边边缘的间隔

	int nCurrentTop = 0;
	int nGroupWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nStaticHeight = 15;		//校验点类型Static控件高度
	int nStaticWidth = (rcClient.Width() - nLeftGap - nRightGap - 2 * nGap) / 3;
	int nRadioWidth = nStaticWidth;

	if (GetDlgItem(IDC_STATIC_TH)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TH)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TH_H)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TH_H)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TH_V)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TH_V)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	if (GetDlgItem(IDC_STATIC_XX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_XX)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_XX_H)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_XX_H)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_XX_V)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_XX_V)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	if (GetDlgItem(IDC_STATIC_Direct)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Direct)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_Direct_ZX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_Direct_ZX)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_Direct_FX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_Direct_FX)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	int nPicOmrWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nPicOmrHeight = rcClient.Height() - nTopGap;
	if (GetDlgItem(IDC_PIC_OMR)->GetSafeHwnd())
	{
		GetDlgItem(IDC_PIC_OMR)->MoveWindow(nLeftGap, nTopGap, nPicOmrWidth, nPicOmrHeight);
	}
}


void COmrInfoDlg::OnNMThemeChangedRadioThH(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 该功能要求使用 Windows XP 或更高版本。
	// 符号 _WIN32_WINNT 必须 >= 0x0501。
	// TODO:  在此添加控件通知处理程序代码
	*pResult = 0;
}

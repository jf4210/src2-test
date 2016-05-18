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
}


BEGIN_MESSAGE_MAP(COmrInfoDlg, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL COmrInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);
	InitCtrlPosition();

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

	int nGap = 5;	//控件的间隔
	int nTopGap = 2;	//距离上边边缘的间隔
	int nBottomGap = 2;	//距离下边边缘的间隔
	int nLeftGap = 2;	//距离左边边缘的间隔
	int nRightGap = 2;	//距离右边边缘的间隔

	int nCurrentTop = 0;
	int nGroupWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nStaticHeight = 20;		//校验点类型Static控件高度
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
}

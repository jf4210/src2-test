// OmrInfoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool.h"
#include "OmrInfoDlg.h"
#include "afxdialogex.h"


// COmrInfoDlg �Ի���

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


// COmrInfoDlg ��Ϣ�������


void COmrInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	InitCtrlPosition();
}

void COmrInfoDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int nGap = 5;	//�ؼ��ļ��
	int nTopGap = 2;	//�����ϱ߱�Ե�ļ��
	int nBottomGap = 2;	//�����±߱�Ե�ļ��
	int nLeftGap = 2;	//������߱�Ե�ļ��
	int nRightGap = 2;	//�����ұ߱�Ե�ļ��

	int nCurrentTop = 0;
	int nGroupWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nStaticHeight = 20;		//У�������Static�ؼ��߶�
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

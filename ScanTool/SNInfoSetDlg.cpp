// SNInfoSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "SNInfoSetDlg.h"
#include "afxdialogex.h"


// CSNInfoSetDlg 对话框

IMPLEMENT_DYNAMIC(CSNInfoSetDlg, CDialog)

CSNInfoSetDlg::CSNInfoSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSNInfoSetDlg::IDD, pParent)
	, m_nSNSel(0), m_nOptionSel(0), m_bShowFist(FALSE), m_nCurrentSNVal(10)
{

}

CSNInfoSetDlg::~CSNInfoSetDlg()
{
}

void CSNInfoSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_SN_H, m_radioSN_H);
	DDX_Control(pDX, IDC_RADIO_SN_V, m_radioSN_V);
	DDX_Control(pDX, IDC_RADIO_Option_1, m_radioADD_Z);
	DDX_Control(pDX, IDC_RADIO_Option_2, m_radioADD_F);
	DDX_Control(pDX, IDC_PIC_SN, m_picSNShow);
}


BEGIN_MESSAGE_MAP(CSNInfoSetDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RADIO_SN_H, &CSNInfoSetDlg::OnBnClickedRadioSnH)
	ON_BN_CLICKED(IDC_RADIO_SN_V, &CSNInfoSetDlg::OnBnClickedRadioSnV)
	ON_BN_CLICKED(IDC_RADIO_Option_1, &CSNInfoSetDlg::OnBnClickedRadioOption1)
	ON_BN_CLICKED(IDC_RADIO_Option_2, &CSNInfoSetDlg::OnBnClickedRadioOption2)
END_MESSAGE_MAP()

BOOL CSNInfoSetDlg::OnInitDialog()
{
	UpdateData(FALSE);
	InitCtrlPosition();

	m_picSNShow.OnInit(0, false);
	InitUI();
	
	ShowSNPic();
	return TRUE;
}

void CSNInfoSetDlg::InitCtrlPosition()
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
	int nStaticWidth = (rcClient.Width() - nLeftGap - nRightGap - 2 * nGap) / 5;
	int nRadioWidth = nStaticWidth * 2;

	if (GetDlgItem(IDC_STATIC_SN))
	{
		GetDlgItem(IDC_STATIC_SN)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_SN_H)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_SN_H)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_SN_V)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_SN_V)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	if (GetDlgItem(IDC_STATIC_Option)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Option)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_Option_1)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_Option_1)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_Option_2)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_Option_2)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, nRadioWidth, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	int nPicSNWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nPicSnHeight = rcClient.Height() - nTopGap;
	if (GetDlgItem(IDC_PIC_SN)->GetSafeHwnd())
	{
		GetDlgItem(IDC_PIC_SN)->MoveWindow(nLeftGap, nTopGap, nPicSNWidth, nPicSnHeight);
	}
}

// CSNInfoSetDlg 消息处理程序
void CSNInfoSetDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
	if (m_picSNShow.GetSafeHwnd())
	{
		if (!m_bShowFist)
		{
			m_bShowFist = TRUE;
			ShowSNPic();
		}
	}
}

void CSNInfoSetDlg::ShowSNPic()
{
	CString strPicPath = _T("");
	if (m_nSNSel == 0)
	{
		if (m_nOptionSel == 0)
		{
			strPicPath = _T(".\\res\\SN1.png");
			m_nCurrentSNVal = 10;
		}
		else
		{
			strPicPath = _T(".\\res\\SN3.png");
			m_nCurrentSNVal = 9;
		}
	}
	else
	{
		if (m_nOptionSel == 0)
		{
			strPicPath = _T(".\\res\\SN4.png");
			m_nCurrentSNVal = 6;
		}
		else
		{
			strPicPath = _T(".\\res\\SN2.png");
			m_nCurrentSNVal = 5;
		}
	}
	m_src_img = cv::imread((std::string)(CT2CA)strPicPath);
	m_picSNShow.ShowImage(m_src_img, 0);
}


void CSNInfoSetDlg::OnBnClickedRadioSnH()
{
	if (m_nSNSel != 0)
	{
		m_nSNSel = 0;
		ShowSNPic();
	}
}


void CSNInfoSetDlg::OnBnClickedRadioSnV()
{
	if (m_nSNSel != 1)
	{
		m_nSNSel = 1;
		ShowSNPic();
	}
}


void CSNInfoSetDlg::OnBnClickedRadioOption1()
{
	if (m_nOptionSel != 0)
	{
		m_nOptionSel = 0;
		ShowSNPic();
	}
}


void CSNInfoSetDlg::OnBnClickedRadioOption2()
{
	if (m_nOptionSel != 1)
	{
		m_nOptionSel = 1;
		ShowSNPic();
	}
}

void CSNInfoSetDlg::ShowUI(int nSnVal)
{
	m_nCurrentSNVal = nSnVal;
	switch (nSnVal)
	{
		case 10:
			m_nSNSel = 0;
			m_nOptionSel = 0;
			break;
		case 9:
			m_nSNSel = 0;
			m_nOptionSel = 1;
			break;
		case 6:
			m_nSNSel = 1;
			m_nOptionSel = 0;
			break;
		case 5:
			m_nSNSel = 1;
			m_nOptionSel = 1;
			break;
	}
	InitUI();
	ShowSNPic();
}

void CSNInfoSetDlg::InitUI()
{
	if (m_nSNSel == 0)
	{
		m_radioSN_H.SetCheck(1);
		m_radioSN_V.SetCheck(0);
		m_radioADD_Z.SetWindowTextW(_T("从上到下"));
		m_radioADD_F.SetWindowTextW(_T("从下到上"));
	}
	else
	{
		m_radioSN_H.SetCheck(0);
		m_radioSN_V.SetCheck(1);
		m_radioADD_Z.SetWindowTextW(_T("从左到右"));
		m_radioADD_F.SetWindowTextW(_T("从右到左"));
	}

	if (m_nOptionSel == 0)
	{
		m_radioADD_Z.SetCheck(1);
		m_radioADD_F.SetCheck(0);
	}
	else
	{
		m_radioADD_Z.SetCheck(0);
		m_radioADD_F.SetCheck(1);
	}
}

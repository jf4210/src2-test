// ScanRecordDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanBmkRecordDlg.h"
#include "afxdialogex.h"


// CScanRecordDlg 对话框

IMPLEMENT_DYNAMIC(CScanBmkRecordDlg, CDialog)

CScanBmkRecordDlg::CScanBmkRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanBmkRecordDlg::IDD, pParent)
	, m_nStatusSize(20)
{

}

CScanBmkRecordDlg::~CScanBmkRecordDlg()
{
}

void CScanBmkRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_GMK_Count, m_strGmkCount);
	DDX_Text(pDX, IDC_STATIC_Scan_Count, m_strScanCount);
	DDX_Text(pDX, IDC_STATIC_UnScan_Count, m_strUnScanCount);
	DDX_Control(pDX, IDC_LIST_Bmk, m_lcBmk);
}


BOOL CScanBmkRecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	return TRUE;
}

BOOL CScanBmkRecordDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CScanBmkRecordDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CScanRecordDlg 消息处理程序

void CScanBmkRecordDlg::InitUI()
{
	SetFontSize(m_nStatusSize);
	UpDateInfo();

	CBitmap bmp;
	bmp.LoadBitmap(IDB_Scrollbar);
	BITMAP bm;
	bmp.GetBitmap(&bm);
	m_bitmap_scrollbar = (HBITMAP)bmp.Detach();
	SkinWndScroll(&m_lcBmk, m_bitmap_scrollbar);
	m_lcBmk.SetExtendedStyle(m_lcBmk.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcBmk.InsertColumn(0, _T("顺序"), LVCFMT_CENTER, 40);
	m_lcBmk.InsertColumn(1, _T("考号"), LVCFMT_CENTER, 110);
	m_lcBmk.InsertColumn(2, _T("姓名"), LVCFMT_CENTER, 110);
	m_lcBmk.InsertColumn(3, _T("扫描状态"), LVCFMT_CENTER, 120);

	InitCtrlPosition();
}

void CScanBmkRecordDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 5;	//上边的间隔
	const int nBottomGap = 35;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	int nCurrLeft = nLeftGap;
	int nCurrTop = nTopGap;
	int nStaticH = 30;

	if (GetDlgItem(IDC_STATIC_1)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.2;
		if (nW > 150) nW = 150;
		GetDlgItem(IDC_STATIC_1)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_GMK_Count)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.1;
		GetDlgItem(IDC_STATIC_GMK_Count)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_2)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.1;
		GetDlgItem(IDC_STATIC_2)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_Scan_Count)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.1;
		GetDlgItem(IDC_STATIC_Scan_Count)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_3)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.1;
		GetDlgItem(IDC_STATIC_3)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_UnScan_Count)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.1;
		GetDlgItem(IDC_STATIC_UnScan_Count)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}

	nCurrLeft = nLeftGap;
	nCurrTop += (nStaticH + nGap);
	if (GetDlgItem(IDC_LIST_Bmk)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 1;
		int nH = cy - nTopGap - nStaticH - nGap - nBottomGap;
		int nXTmp = (cx - nLeftGap - nRightGap) * 0;
		GetDlgItem(IDC_LIST_Bmk)->MoveWindow(nCurrLeft + nXTmp, nCurrTop, nW, nH);
		if (nW > 380)
			m_lcBmk.SetColumnWidth(3, nW - 280);
	}

	if (GetDlgItem(IDC_BTN_ExportScan)->GetSafeHwnd())
	{
		int nW = 100;
		int nH = 25;
		nCurrLeft = cx / 2 - nW / 2;
		nCurrTop = cy - nBottomGap + nGap;
		GetDlgItem(IDC_BTN_ExportScan)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}
}

void CScanBmkRecordDlg::SetFontSize(int nSize)
{
	m_fontStatus.DeleteObject();
	m_fontStatus.CreateFont(nSize, 0, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("Arial"));
	GetDlgItem(IDC_STATIC_1)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_2)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_3)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_GMK_Count)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_Scan_Count)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_UnScan_Count)->SetFont(&m_fontStatus);
}

HBRUSH CScanBmkRecordDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_1 || CurID == IDC_STATIC_2 || CurID == IDC_STATIC_3)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_GMK_Count)
	{
		pDC->SetTextColor(RGB(0, 0, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_Scan_Count)
	{
		pDC->SetTextColor(RGB(0, 255, 0));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_UnScan_Count)
	{
		pDC->SetTextColor(RGB(255, 0, 0));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

BOOL CScanBmkRecordDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CScanBmkRecordDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CScanBmkRecordDlg::UpDateInfo()
{
	//*************************************************
	//			需要根据报名库标识来取值
	//*************************************************
	m_strGmkCount.Format(_T("%d人"), g_lBmkStudent.size());
	m_strScanCount = _T("0人");
	m_strUnScanCount = _T("0人");

	UpdateData(FALSE);
}



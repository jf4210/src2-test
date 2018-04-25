// ScanReminderDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanReminderDlg.h"
#include "afxdialogex.h"


// CScanReminderDlg 对话框

IMPLEMENT_DYNAMIC(CScanReminderDlg, CDialog)

CScanReminderDlg::CScanReminderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanReminderDlg::IDD, pParent)
	, m_nStatusSize(25), m_strShowTips(_T("正在扫描，请稍后...")), m_strScanCount(_T("已扫 0 张")), m_nShowType(1)
{

}

CScanReminderDlg::~CScanReminderDlg()
{
}

void CScanReminderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_ScanCount, m_strScanCount);
	DDX_Text(pDX, IDC_STATIC_Tip, m_strShowTips);
//	DDX_Control(pDX, IDC_STATIC_Tip, m_staticShowTips);
}


BOOL CScanReminderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

//	m_bmpBk.LoadBitmap(IDB_ScanMgr_WaitPic);
	m_bmpBk.LoadBitmap(IDB_ScanMgr_Scanning);

	m_staticShowTips.SubclassDlgItem(IDC_STATIC_Tip, this);
//	m_staticShowTips.SetMultiLine();
	m_staticShowTips.SetCenterAlign();
	m_staticShowTips.SetTextColor(RGB(115, 172, 254));

	InitCtrlPosition();
	SetFontSize(m_nStatusSize);

	return TRUE;
}

BOOL CScanReminderDlg::PreTranslateMessage(MSG* pMsg)
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

BEGIN_MESSAGE_MAP(CScanReminderDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CScanReminderDlg 消息处理程序
void CScanReminderDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 20;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	int nBaseLeft = nLeftGap + (cx - nLeftGap - nRightGap) * 0.3;
	int nCurrLeft = nBaseLeft;
	int nCurrTop = cy * 0.6;

	if (GetDlgItem(IDC_STATIC_Tip)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 1;	//0.4
		int nH = 40;
		if (m_nShowType == 1)
			nH = 30;
		else
			nH = 80;	//可以显示3行字体信息
		GetDlgItem(IDC_STATIC_Tip)->MoveWindow(nLeftGap, nCurrTop, nW, nH);	//nCurrLeft
		nCurrTop += (nH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_1)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.15;
		int nH = 50;
		GetDlgItem(IDC_STATIC_1)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ScanCount)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.1;
		int nH = 50;
		GetDlgItem(IDC_STATIC_ScanCount)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_2)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.1;
		int nH = 50;
		GetDlgItem(IDC_STATIC_2)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrLeft += (nW + nGap);
	}
	Invalidate();
}

void CScanReminderDlg::SetFontSize(int nSize)
{
	m_fontStatus.DeleteObject();
	m_fontStatus.CreateFont(nSize, 12, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("Arial"));
	GetDlgItem(IDC_STATIC_Tip)->SetFont(&m_fontStatus);
	m_staticShowTips.SetShowFont(m_fontStatus);

	m_fontStatus2.DeleteObject();
	m_fontStatus2.CreateFont(nSize + 3, 0, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("Arial"));
	GetDlgItem(IDC_STATIC_1)->SetFont(&m_fontStatus2);
	GetDlgItem(IDC_STATIC_ScanCount)->SetFont(&m_fontStatus2);
	GetDlgItem(IDC_STATIC_2)->SetFont(&m_fontStatus2);
}

void CScanReminderDlg::DrawBorder(CDC *pDC)
{
	CPen *pOldPen = NULL;
	CPen pPen;
	CRect rcClient(0, 0, 0, 0);
	GetClientRect(&rcClient);
	pPen.CreatePen(PS_SOLID, 2, RGB(0, 0, 0));

	pDC->SelectStockObject(NULL_BRUSH);
	pOldPen = pDC->SelectObject(&pPen);
	pDC->Rectangle(&rcClient);
	pDC->SelectObject(pOldPen);
	pPen.Detach();
}

BOOL CScanReminderDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	
	CDialog::OnEraseBkgnd(pDC);

	int iX, iY;
	CDC memDC;
	BITMAP bmp;
#if 1
	iX = iY = 0;

	m_bmpBk.GetBitmap(&bmp);
	iX = rcClient.Width() / 2 - bmp.bmWidth / 2;
	iY = rcClient.Height() / 2 - bmp.bmHeight / 2 - 30;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));

	if (memDC.CreateCompatibleDC(pDC))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_bmpBk);
		m_bmpBk.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, bmp.bmWidth, bmp.bmHeight, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);
	}
	memDC.DeleteDC();
//	ReleaseDC(pDC);
#else
	iX = iY = 0;
	GetClientRect(&rcClient);

	if (memDC.CreateCompatibleDC(pDC))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_bmpBk);
		m_bmpBk.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);
	}
	memDC.DeleteDC();
#endif

	return TRUE;
}


HBRUSH CScanReminderDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_Tip || CurID == IDC_STATIC_1 || CurID == IDC_STATIC_2)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetTextColor(RGB(115, 172, 254));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_ScanCount)
	{
		pDC->SetTextColor(RGB(61, 147, 254));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}


void CScanReminderDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CScanReminderDlg::SetShowTips(CString str, bool bWarn /*= false*/)
{
	m_strShowTips = str;
	if (bWarn)
	{
		m_staticShowTips.SetTextColor(RGB(222, 35, 145));
	}
	else
	{
		m_staticShowTips.SetTextColor(RGB(115, 172, 254));
	}
	UpdateData(FALSE);
//	Invalidate();

	//解决Static控件背景透明时文本覆盖重影
	CRect rtStatic;
	GetDlgItem(IDC_STATIC_Tip)->GetWindowRect(&rtStatic);
	ScreenToClient(&rtStatic);
	InvalidateRect(&rtStatic);
}

void CScanReminderDlg::UpdataScanCount(int nCount)
{
	m_strScanCount.Format(_T("%d"), nCount);
	UpdateData(FALSE);
//	Invalidate();

//解决Static控件背景透明时文本覆盖重影
	CRect rtStatic;
	GetDlgItem(IDC_STATIC_ScanCount)->GetWindowRect(&rtStatic);
	ScreenToClient(&rtStatic);
	InvalidateRect(&rtStatic);
}

void CScanReminderDlg::SetShowScanCount(bool bShow)
{
	if (bShow)
		m_nShowType = 1;
	else
		m_nShowType = 2;
	if (GetDlgItem(IDC_STATIC_1)->GetSafeHwnd())
		GetDlgItem(IDC_STATIC_1)->ShowWindow(bShow);
	if (GetDlgItem(IDC_STATIC_2)->GetSafeHwnd())
		GetDlgItem(IDC_STATIC_2)->ShowWindow(bShow);
	if (GetDlgItem(IDC_STATIC_ScanCount)->GetSafeHwnd())
		GetDlgItem(IDC_STATIC_ScanCount)->ShowWindow(bShow);
	
	InitCtrlPosition();
}


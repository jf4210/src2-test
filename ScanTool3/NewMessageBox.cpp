// TestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "NewMessageBox.h"
#include "afxdialogex.h"

// CTestDlg 对话框

IMPLEMENT_DYNAMIC(CNewMessageBox, CDialog)

CNewMessageBox::CNewMessageBox(CWnd* pParent /*=NULL*/)
: CTipBaseDlg(CNewMessageBox::IDD, pParent)
, m_nShowType(0), m_nBtn(1)
{

}

CNewMessageBox::~CNewMessageBox()
{
}

void CNewMessageBox::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_bmpBtnOK);
	DDX_Control(pDX, IDCANCEL, m_bmpBtnClose);
	DDX_Text(pDX, IDC_STATIC_ShowMsg, m_strMsgShow);
}


BOOL CNewMessageBox::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	InitUI();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CNewMessageBox, CTipBaseDlg)
	ON_WM_SIZE()
	//	ON_WM_NCPAINT()
	//	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CTestDlg 消息处理程序


void CNewMessageBox::OnSize(UINT nType, int cx, int cy)
{
	CTipBaseDlg::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

#if 0
void CNewMessageBox::OnNcPaint()
{
	// TODO:  在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnNcPaint()
	CWindowDC dc(this);
	CRect rcWindow;
	CRect rcClient;
	this->GetClientRect(rcClient);
	this->ClientToScreen(rcClient);
	this->GetWindowRect(rcWindow);
	CPoint point = rcWindow.TopLeft();
	rcClient.OffsetRect(-point);
	rcWindow.OffsetRect(-point);
	int windowWidth = rcWindow.Width();
	int windowHeight = rcWindow.Height();
	HDC hMemDC = ::CreateCompatibleDC(dc.m_hDC);
	HBITMAP hBmp = ::CreateCompatibleBitmap(dc.m_hDC, windowWidth, windowHeight);
	::SelectObject(hMemDC, hBmp);
	// 	Graphics graphics(hMemDC);
	// 	graphics.Clear(Color(255, 255, 255, 255));
	// 	graphics.SetSmoothingMode(SmoothingModeHighQuality);
	//TODO:使用GDI+的DrawImage函数来贴上圆角图片,或者使用RoundRect函数来为对话框画上圆角线
#if 0 /*使用DrawImage来绘制圆角图片*/
	ImageAttributes ia;
	ia.SetWrapMode(WrapModeTileFlipXY);
	graphic.DrawImage(pImg_LTFrame, ……….);
#endif
#if 1 /*使用RoundRect来绘制圆角线*/
	RoundRect(hMemDC, rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom, 10, 10);
#endif
	dc.IntersectClipRect(rcWindow);
	dc.ExcludeClipRect(rcClient);
	::BitBlt(dc.m_hDC, 0, 0, windowWidth, windowHeight, hMemDC, 0, 0, SRCCOPY);
	::DeleteDC(hMemDC);
	::DeleteObject(hBmp);
}


LRESULT CNewMessageBox::OnNcHitTest(CPoint point)
{
	CRect rcWndRect;
	GetWindowRect(rcWndRect);
	//	rcWndRect.bottom = rcWndRect.top + 40;	//40
	if (rcWndRect.PtInRect(point))
		return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}
#endif


void CNewMessageBox::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 10;		//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 10;	//左边的空白间隔
	const int nRightGap = 10;	//右边的空白间隔
	int nGap = 5;

	int nCurrLeft = nLeftGap;
	int nCurrTop = nTopGap;

	if (GetDlgItem(IDC_STATIC_ShowMsg)->GetSafeHwnd())
	{
		int nStaticW = (cx - nLeftGap - nRightGap) * 0.8;
		int nStaticH = 30;
		nCurrLeft = cx / 2 - nStaticW / 2;
		nCurrTop = nTopGap + (cy - nTopGap - nBottomGap) / 2;
		GetDlgItem(IDC_STATIC_ShowMsg)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nStaticH);
	}

	if (m_nBtn == 1)
	{
		int nBtnW = (cx - nLeftGap - nRightGap) * 0.3;
		if (nBtnW < 30) nBtnW = 30;
		int nBtnH = 30;
		if (m_bmpBtnOK.GetSafeHwnd())
		{
			nCurrLeft = cx / 2 - nBtnW / 2;
			m_bmpBtnOK.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		}
	}
	else if (m_nBtn == 2)
	{
		int nBtnW = (cx - nLeftGap - nRightGap) * 0.3;
		if (nBtnW < 30) nBtnW = 30;
		int nBtnH = 30;
		if (m_bmpBtnOK.GetSafeHwnd())
		{
			nCurrLeft = cx / 2 - nBtnW - nGap / 2;
			m_bmpBtnOK.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
			nCurrLeft = cx / 2 + nGap / 2;
		}
		if (m_bmpBtnClose.GetSafeHwnd())
		{
			m_bmpBtnClose.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		}
	}
	Invalidate();
}

void CNewMessageBox::InitUI()
{
	if (m_nShowType == 1)
	{
		m_bmpBk.DeleteObject();
		m_bmpBk.LoadBitmap(IDB_Popup_BK_NoStudent);
		m_nBtn = 2;
	}
	else if (m_nShowType == 2)
	{
		m_bmpBk.DeleteObject();
		m_bmpBk.LoadBitmap(IDB_Popup_BK_NoModel);
		m_nBtn = 1;
	}

	InitCtrlPosition();
}

void CNewMessageBox::setShowInfo(int nType, std::string strMsg)
{
	USES_CONVERSION;
	m_nShowType = nType;
	m_strMsgShow = A2T(strMsg.c_str());

	InitUI();
}

BOOL CNewMessageBox::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	__super::OnEraseBkgnd(pDC);

	int iX, iY;
	CDC memDC;
	BITMAP bmp;

	iX = iY = 0;

	m_bmpBk.GetBitmap(&bmp);
	iX = rcClient.Width() / 2 - bmp.bmWidth / 2;
	iY = rcClient.Height() / 2 - bmp.bmHeight / 2 - 30;
	GetClientRect(&rcClient);

	//	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));

	if (memDC.CreateCompatibleDC(pDC))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_bmpBk);
		m_bmpBk.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, bmp.bmWidth, bmp.bmHeight, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);
	}
	memDC.DeleteDC();

	return TRUE;
}

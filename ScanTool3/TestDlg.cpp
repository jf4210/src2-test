// TestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "TestDlg.h"
#include "afxdialogex.h"

// CTestDlg 对话框

IMPLEMENT_DYNAMIC(CTestDlg, CDialog)

CTestDlg::CTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestDlg::IDD, pParent)
{

}

CTestDlg::~CTestDlg()
{
}

void CTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTestDlg, CDialog)
	ON_WM_SIZE()
//	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


// CTestDlg 消息处理程序


void CTestDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRgn m_rgn;
	CRect rc;
	GetWindowRect(&rc);
	rc -= rc.TopLeft();
	m_rgn.CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, 200, 200);
	SetWindowRgn(m_rgn, TRUE);
}


void CTestDlg::OnNcPaint()
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


LRESULT CTestDlg::OnNcHitTest(CPoint point)
{
	CRect rcWndRect;
	GetWindowRect(rcWndRect);
	//	rcWndRect.bottom = rcWndRect.top + 40;	//40
	if (rcWndRect.PtInRect(point))
		return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}

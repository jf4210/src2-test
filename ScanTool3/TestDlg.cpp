// TestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "TestDlg.h"
#include "afxdialogex.h"

// CTestDlg �Ի���

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


// CTestDlg ��Ϣ�������


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
	// TODO:  �ڴ˴������Ϣ����������
	// ��Ϊ��ͼ��Ϣ���� CDialog::OnNcPaint()
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
	//TODO:ʹ��GDI+��DrawImage����������Բ��ͼƬ,����ʹ��RoundRect������Ϊ�Ի�����Բ����
#if 0 /*ʹ��DrawImage������Բ��ͼƬ*/
	ImageAttributes ia;
	ia.SetWrapMode(WrapModeTileFlipXY);
	graphic.DrawImage(pImg_LTFrame, ������.);
#endif
#if 1 /*ʹ��RoundRect������Բ����*/
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

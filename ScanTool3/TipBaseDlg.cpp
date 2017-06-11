#include "stdafx.h"
#include "TipBaseDlg.h"

COLORREF 		g_colBack;
COLORREF 		g_colLine;

CTipBaseDlg::CTipBaseDlg(UINT nIDTemplate, CWnd* pParent)
: CDialog(nIDTemplate, pParent)
{
}


CTipBaseDlg::~CTipBaseDlg()
{
}

BEGIN_MESSAGE_MAP(CTipBaseDlg, CDialog)
	//{{AFX_MSG_MAP(CBaseDlg)
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CTipBaseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_colBack = RGB(255, 255, 255);		//241, 241, 241
	g_colLine = RGB(0, 0, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CTipBaseDlg::OnNcHitTest(CPoint point)
{
	CRect rcWndRect;
	GetWindowRect(rcWndRect);
//	rcWndRect.bottom = rcWndRect.top + 28;
	if (rcWndRect.PtInRect(point))
		return HTCAPTION;

	return CDialog::OnNcHitTest(point);
}

void CTipBaseDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRgn m_rgn;
	CRect rc;
//	GetWindowRect(&rc);
	GetClientRect(&rc);
//	rc -= rc.TopLeft();
	m_rgn.CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, 20, 20);
	SetWindowRgn(m_rgn, TRUE);
}

BOOL CTipBaseDlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));

// 	CDC dc;
// 	if (dc.CreateCompatibleDC(pDC))
// 	{
// 		HGDIOBJ hOldObject = NULL;
// 		
// 		
// 	}
// 	dc.DeleteDC();

	DrawBorder(pDC, rcClient);

// 	CRect rcClient;
// 	GetClientRect(&rcClient);
// 	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));
// 	ReleaseDC(pDC);

	return TRUE;
}

HBRUSH CTipBaseDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}
void CTipBaseDlg::DrawBorder(CDC* pDC, CRect rect)
{
	CPen pen(PS_SOLID, 2, RGB(108, 178, 254));	//166, 218, 239
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->SelectStockObject(NULL_BRUSH);
//	pDC->Rectangle(&rect);
//	pDC->RoundRect(rect.left + 1, rect.top + 1, rect.right - 2, rect.bottom - 2, 20, 20);
	pDC->RoundRect(rect.left, rect.top, rect.right - 1, rect.bottom - 1, 20, 20);
	pDC->SelectObject(pOldPen);
}



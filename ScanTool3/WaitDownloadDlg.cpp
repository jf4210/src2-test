// WaitDownloadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "WaitDownloadDlg.h"
#include "afxdialogex.h"

#include "Net_Cmd_Protocol.h"
#include "ZipObj.h"
#include "ScanMgrDlg.h"
#include "ScanTool3Dlg.h"

// CWaitDownloadDlg 对话框

IMPLEMENT_DYNAMIC(CWaitDownloadDlg, CDialog)

CWaitDownloadDlg::CWaitDownloadDlg(CWnd* pParent /*=NULL*/)
: CDialog(CWaitDownloadDlg::IDD, pParent)
, m_nStatusSize(30)
{

}

CWaitDownloadDlg::~CWaitDownloadDlg()
{
}

void CWaitDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_WaitInfo, m_strWaitInfo);
}


BOOL CWaitDownloadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_strWaitInfo = _T("正在下载考生库和扫描模板，请稍后......");
	SetFontSize(m_nStatusSize);
//	m_bmpBk.LoadBitmap(IDB_ScanMgr_WaitPic);
	m_bmpBk.LoadBitmap(IDB_ScanMgr_Download);
	InitCtrlPosition();
	UpdateData(FALSE);
	return TRUE;
}

BOOL CWaitDownloadDlg::PreTranslateMessage(MSG* pMsg)
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

BEGIN_MESSAGE_MAP(CWaitDownloadDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CWaitDownloadDlg 消息处理程序
void CWaitDownloadDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nCurrLeft = 0;
	int nCurrTop = rcClient.Height() / 2;
	if (GetDlgItem(IDC_STATIC_WaitInfo)->GetSafeHwnd())
	{
		int nW = cx;
		int nH = rcClient.Height() * 0.3;
		GetDlgItem(IDC_STATIC_WaitInfo)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}
	Invalidate();
}



void CWaitDownloadDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_STATIC_WaitInfo)->SetFont(&m_fontStatus);
}

void CWaitDownloadDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}


HBRUSH CWaitDownloadDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_WaitInfo)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetTextColor(RGB(61, 147, 254));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}


BOOL CWaitDownloadDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

//	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	CDialog::OnEraseBkgnd(pDC);

	int iX, iY;
	CDC memDC;
	BITMAP bmp;

	iX = iY = 0;
	GetClientRect(&rcClient);
#if 1
	m_bmpBk.GetBitmap(&bmp);
	iX = rcClient.Width() / 2 - bmp.bmWidth / 2;
	iY = rcClient.Height() / 2 - bmp.bmHeight / 2 - 20;

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250

	if (memDC.CreateCompatibleDC(pDC))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_bmpBk);
		m_bmpBk.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, bmp.bmWidth, bmp.bmHeight, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);
	}
	memDC.DeleteDC();
#else
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
	ReleaseDC(pDC);

	return TRUE;
}



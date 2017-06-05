// ScanReminderDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanReminderDlg.h"
#include "afxdialogex.h"


// CScanReminderDlg �Ի���

IMPLEMENT_DYNAMIC(CScanReminderDlg, CDialog)

CScanReminderDlg::CScanReminderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanReminderDlg::IDD, pParent)
	, m_nStatusSize(25), m_strShowTips(_T("����ɨ�裬���Ժ�...")), m_strScanCount(_T("��ɨ 0 ��"))
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
}


BOOL CScanReminderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

//	m_bmpBk.LoadBitmap(IDB_ScanMgr_WaitPic);
	m_bmpBk.LoadBitmap(IDB_ScanMgr_Scanning);
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


// CScanReminderDlg ��Ϣ�������
void CScanReminderDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 20;	//�ϱߵļ��
	const int nBottomGap = 10;	//�±ߵļ��
	const int nLeftGap = 20;		//��ߵĿհ׼��
	const int nRightGap = 20;	//�ұߵĿհ׼��
	int nGap = 5;

	int nBaseLeft = nLeftGap + (cx - nLeftGap - nRightGap) * 0.3;
	int nCurrLeft = nBaseLeft;
	int nCurrTop = cy * 0.6;

	if (GetDlgItem(IDC_STATIC_Tip)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.4;
		int nH = 50;
		GetDlgItem(IDC_STATIC_Tip)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
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
	m_fontStatus.CreateFont(nSize, 0, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("Arial"));
	GetDlgItem(IDC_STATIC_Tip)->SetFont(&m_fontStatus);

	m_fontStatus2.DeleteObject();
	m_fontStatus2.CreateFont(nSize + 2, 0, 0, 0,
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
	iY = rcClient.Height() / 2 - bmp.bmHeight / 2;
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

void CScanReminderDlg::SetShowTips(CString str)
{
	m_strShowTips = str;
	UpdateData(FALSE);
	Invalidate();
}

void CScanReminderDlg::UpdataScanCount(int nCount)
{
	m_strScanCount.Format(_T("%d"), nCount);
	UpdateData(FALSE);
	Invalidate();
}


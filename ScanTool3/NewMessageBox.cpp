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
, m_nShowType(1), m_nBtn(1), m_nStatusSize(25), m_nResult(0)
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


BOOL CNewMessageBox::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (m_nBtn > 1 && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		{
			return TRUE;
		}
	}
	return CTipBaseDlg::PreTranslateMessage(pMsg);
}

BOOL CNewMessageBox::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	InitUI();

	m_staticMsgShow.SubclassDlgItem(IDC_STATIC_ShowMsg, this);
	m_staticMsgShow.SetCenterAlign();
	SetFontSize(m_nStatusSize);
	InitCtrlPosition();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CNewMessageBox, CTipBaseDlg)
	ON_WM_SIZE()
	//	ON_WM_NCPAINT()
	//	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CNewMessageBox::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNewMessageBox::OnBnClickedCancel)
END_MESSAGE_MAP()


// CTestDlg 消息处理程序


void CNewMessageBox::OnSize(UINT nType, int cx, int cy)
{
	CTipBaseDlg::OnSize(nType, cx, cy);

	InitCtrlPosition();
}


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
		int nStaticW = (cx - nLeftGap - nRightGap) * 1;
		int nStaticH = 50;
		nCurrLeft = cx / 2 - nStaticW / 2;

		BITMAP bmp;
		if (m_bmpBk.GetSafeHandle())
			m_bmpBk.GetBitmap(&bmp);

		nCurrTop = nTopGap + cy / 2 + bmp.bmHeight / 2 - 35;
		GetDlgItem(IDC_STATIC_ShowMsg)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nStaticH);
	}

	if (m_nBtn == 1)
	{
		int nBtnW = (cx - nLeftGap - nRightGap) * 0.3;
		if (nBtnW < 40) nBtnW = 40;
		int nBtnH = 40;
		if (m_bmpBtnOK.GetSafeHwnd())
		{
			nCurrLeft = cx / 2 - nBtnW / 2;
			nCurrTop = cy - nBottomGap - nBtnH;
			m_bmpBtnOK.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		}
	}
	else if (m_nBtn == 2)
	{
		int nBtnW = (cx - nLeftGap - nRightGap) * 0.3;
		if (nBtnW < 30) nBtnW = 30;
		int nBtnH = 30;
		nCurrTop = cy - nBottomGap - nBtnH;
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

void CNewMessageBox::SetFontSize(int nSize)
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
	m_fontBtn.DeleteObject();
	m_fontBtn.CreateFont(20, 0, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("Arial"));

//	GetDlgItem(IDC_STATIC_ShowMsg)->SetFont(&m_fontStatus);
	m_staticMsgShow.SetShowFont(m_fontStatus);
	m_bmpBtnOK.SetBtnFont(m_fontBtn);
	m_bmpBtnClose.SetBtnFont(m_fontBtn);
}

void CNewMessageBox::InitUI()
{
	if (m_nShowType == 1)
	{
		m_bmpBk.DeleteObject();
		m_bmpBk.LoadBitmap(IDB_Popup_BK_NoStudent);

		if (m_nBtn == 1)
		{
			if (m_bmpBtnClose.GetSafeHwnd()) m_bmpBtnClose.ShowWindow(SW_HIDE);
			if (m_bmpBtnOK.GetSafeHwnd())
				m_bmpBtnOK.SetBmpBtnText(_T("关闭"));
		}
		else if (m_nBtn == 2)
		{
			if (m_bmpBtnClose.GetSafeHwnd())
			{
				m_bmpBtnClose.ShowWindow(SW_SHOW);
				m_bmpBtnClose.SetBmpBtnText(_T("否"));
			}
			if (m_bmpBtnOK.GetSafeHwnd())
				m_bmpBtnOK.SetBmpBtnText(_T("是"));
		}
	}
	else if (m_nShowType == 2)
	{
		m_bmpBk.DeleteObject();
		m_bmpBk.LoadBitmap(IDB_Popup_BK_NoModel);
		
		if (m_nBtn == 1)
		{
			if (m_bmpBtnClose.GetSafeHwnd()) m_bmpBtnClose.ShowWindow(SW_HIDE);
		}
		else if (m_nBtn == 2)
		{
			if (m_bmpBtnClose.GetSafeHwnd()) m_bmpBtnClose.ShowWindow(SW_SHOW);
		}
	}
	else if (m_nShowType == 3)
	{
		m_bmpBk.DeleteObject();
		m_bmpBk.LoadBitmap(IDB_Popup_BK_Success);

		if (m_nBtn == 1)
		{
			if (m_bmpBtnClose.GetSafeHwnd()) m_bmpBtnClose.ShowWindow(SW_HIDE);
			if (m_bmpBtnOK.GetSafeHwnd())
				m_bmpBtnOK.SetBmpBtnText(_T("确定"));
		}
	}

	m_bmpBtnOK.SetStateBitmap(IDB_Popup_Btn_Normal, 0, IDB_Popup_Btn_Hover);
	m_bmpBtnOK.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 0);
	m_bmpBtnClose.SetStateBitmap(IDB_Popup_Btn_Normal, 0, IDB_Popup_Btn_Hover);
	m_bmpBtnClose.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 0);

//	InitCtrlPosition();
}

void CNewMessageBox::setShowInfo(int nType, int nBtns, std::string strMsg)
{
	USES_CONVERSION;
	m_nShowType = nType;
	m_strMsgShow = A2T(strMsg.c_str());
	m_nBtn = nBtns;

	//++判断将显示的行数
// 	int nLines = 1;
// 	int nSingleCount = 0;
// 	LOGFONT	logfont;
//	if (m_fontStatus.GetSafeHandle())
// 	{
// 		CRect client_rect;
// 		if (GetDlgItem(IDC_STATIC_ShowMsg)->GetSafeHwnd())
// 			GetDlgItem(IDC_STATIC_ShowMsg)->GetClientRect(&client_rect);
// 		m_fontStatus.GetLogFont(&logfont);
// 		int nCharW = logfont.lfWidth;
// 		nSingleCount = client_rect.Width() / nCharW - 1;
// 		int nLen = m_strMsgShow.GetLength() * sizeof(TCHAR);
// 		nLines = nLen / nSingleCount + 1;
//	}
	//--

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
	iY = rcClient.Height() / 2 - bmp.bmHeight / 2 - 35;
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


HBRUSH CNewMessageBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CTipBaseDlg::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_ShowMsg)
	{
		pDC->SetTextColor(RGB(91, 77, 77));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	return hbr;
}


void CNewMessageBox::OnBnClickedOk()
{
	m_nResult = IDYES;
	CTipBaseDlg::OnOK();
}


void CNewMessageBox::OnBnClickedCancel()
{
	m_nResult = IDNO;
	CTipBaseDlg::OnCancel();
}

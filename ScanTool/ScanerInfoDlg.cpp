// ScanerInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanerInfoDlg.h"
#include "afxdialogex.h"


// CScanerInfoDlg 对话框

IMPLEMENT_DYNAMIC(CScanerInfoDlg, CDialog)

CScanerInfoDlg::CScanerInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanerInfoDlg::IDD, pParent)
	, m_strUser(_T(""))
	, m_strNickName(_T(""))
	, m_nStatusSize(24)
{

}

CScanerInfoDlg::~CScanerInfoDlg()
{
}

void CScanerInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_UserName, m_strUser);
	DDX_Text(pDX, IDC_EDIT_NickName, m_strNickName);
}


BEGIN_MESSAGE_MAP(CScanerInfoDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CScanerInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_colorStatus = RGB(255, 100, 0);
	SetFontSize(m_nStatusSize);
	InitCtrlPosition();
	UpdateData(FALSE);

	return TRUE;
}

BOOL CScanerInfoDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CScanerInfoDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_EDIT_UserName)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_EDIT_NickName)->SetFont(&m_fontStatus);
}


// CScanerInfoDlg 消息处理程序


HBRUSH CScanerInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();

	if (IDC_EDIT_UserName == CurID)
	{
		pDC->SetTextColor(m_colorStatus);

		return hbr;	// hbrsh;
	}
	if (IDC_EDIT_NickName == CurID)
	{
		pDC->SetTextColor(m_colorStatus);

		return hbr;	// hbrsh;
	}
	return hbr;
}

void CScanerInfoDlg::setShowInfo(CString& strUser, CString& strNickName)
{
	m_strUser		= strUser;
	m_strNickName	= strNickName;
	UpdateData(FALSE);
}

void CScanerInfoDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nLeftGap = 2;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nTopGap = 2;
	const int nGap = 2;			//普通控件的间隔

	int nCurrentTop = nTopGap;
	int nStaticW = 30;
	int nStaticH = 25;
	int nEditW = cx - nLeftGap - nRightGap - nStaticW - nGap;
	if (GetDlgItem(IDC_STATIC_UserName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_UserName)->MoveWindow(nLeftGap, nCurrentTop, nStaticW, nStaticH);
	}
	if (GetDlgItem(IDC_EDIT_UserName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_UserName)->MoveWindow(nLeftGap + nStaticW + nGap, nCurrentTop, nEditW, nStaticH);
		nCurrentTop = nCurrentTop + nStaticH + nGap;
	}
	if (GetDlgItem(IDC_STATIC_NickName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_NickName)->MoveWindow(nLeftGap, nCurrentTop, nStaticW, nStaticH);
	}
	if (GetDlgItem(IDC_EDIT_NickName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_NickName)->MoveWindow(nLeftGap + nStaticW + nGap, nCurrentTop, nEditW, nStaticH);
		nCurrentTop = nCurrentTop + nStaticH + nGap;
	}
}


void CScanerInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

// MultiPageExceptionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "MultiPageExceptionDlg.h"
#include "afxdialogex.h"


// CMultiPageExceptionDlg 对话框

IMPLEMENT_DYNAMIC(CMultiPageExceptionDlg, CDialog)

CMultiPageExceptionDlg::CMultiPageExceptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_MULTIPAGEEXCEPTIONDLG, pParent)
{

}

CMultiPageExceptionDlg::~CMultiPageExceptionDlg()
{
}

void CMultiPageExceptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Paper, m_lcIssuePaper);
	DDX_Control(pDX, IDC_LIST_Pics, m_lcIssuePics);
}

BEGIN_MESSAGE_MAP(CMultiPageExceptionDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CMultiPageExceptionDlg 消息处理程序
BOOL CMultiPageExceptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}

void CMultiPageExceptionDlg::InitUI()
{
	m_lcIssuePaper.SetExtendedStyle(m_lcIssuePaper.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcIssuePaper.InsertColumn(0, _T("考生"), LVCFMT_CENTER, 50);
	m_lcIssuePaper.InsertColumn(1, _T("准考证号"), LVCFMT_CENTER, 110);
	m_lcIssuePaper.InsertColumn(2, _T("备注"), LVCFMT_LEFT, 150);

	HDITEM hditem;
	for (int i = 0; i < m_lcIssuePaper.m_HeaderCtrl.GetItemCount(); i++)
	{
		hditem.mask = HDI_IMAGE | HDI_FORMAT;
		m_lcIssuePaper.m_HeaderCtrl.GetItem(i, &hditem);
		hditem.fmt |= HDF_IMAGE;
		m_lcIssuePaper.m_HeaderCtrl.SetItem(i, &hditem);
	}
	m_lcIssuePaper.EnableToolTips(TRUE);

	m_lcIssuePics.SetExtendedStyle(m_lcIssuePics.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcIssuePics.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 50);
	m_lcIssuePics.InsertColumn(1, _T("图片名"), LVCFMT_CENTER, 110);

	HDITEM hditem;
	for (int i = 0; i < m_lcIssuePics.m_HeaderCtrl.GetItemCount(); i++)
	{
		hditem.mask = HDI_IMAGE | HDI_FORMAT;
		m_lcIssuePics.m_HeaderCtrl.GetItem(i, &hditem);
		hditem.fmt |= HDF_IMAGE;
		m_lcIssuePics.m_HeaderCtrl.SetItem(i, &hditem);
	}
	m_lcIssuePics.EnableToolTips(TRUE);

	InitCtrlPosition();
}

void CMultiPageExceptionDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 2;	//上边的间隔，留给控制栏
	const int nLeftGap = 2;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nStaticTip = 15;		//列表提示static控件高度
	int nListCtrlWidth = cx - nLeftGap - nRightGap;	//图片列表控件宽度
	int nListH = (cy - nTopGap - nBottomGap - nStaticTip * 2 - nGap * 3) * 0.7;
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	if (GetDlgItem(IDC_STATIC_Paper)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Paper)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (m_lcIssuePaper.GetSafeHwnd())
	{
		m_lcIssuePaper.MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nListH);
		nCurrentTop += (nListH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_Pics)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Pics)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (m_lcIssuePics.GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.4;
		int nH = cy - nCurrentTop - nBottomGap;
		m_lcIssuePics.MoveWindow(nCurrentLeft, nCurrentTop, nW, nH);
		nCurrentLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ModelPageIndex)->GetSafeHwnd())
	{

	}
}

BOOL CMultiPageExceptionDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)	//pMsg->wParam == VK_ESCAPE
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CMultiPageExceptionDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}


HBRUSH CMultiPageExceptionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_Paper || CurID == IDC_STATIC_Pics || CurID == IDC_STATIC_ModelPageIndex)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

// PkgRecordDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "PkgRecordDlg.h"
#include "afxdialogex.h"


// CPkgRecordDlg 对话框

IMPLEMENT_DYNAMIC(CPkgRecordDlg, CDialog)

CPkgRecordDlg::CPkgRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPkgRecordDlg::IDD, pParent)
{

}

CPkgRecordDlg::~CPkgRecordDlg()
{
}

void CPkgRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Pkg, m_lcPkg);
}


BEGIN_MESSAGE_MAP(CPkgRecordDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()


BOOL CPkgRecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	return TRUE;
}

BOOL CPkgRecordDlg::PreTranslateMessage(MSG* pMsg)
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

// CPkgRecordDlg 消息处理程序
void CPkgRecordDlg::InitUI()
{
	CBitmap bmp;
	bmp.LoadBitmap(IDB_Scrollbar);
	BITMAP bm;
	bmp.GetBitmap(&bm);
	m_bitmap_scrollbar = (HBITMAP)bmp.Detach();
	SkinWndScroll(&m_lcPkg, m_bitmap_scrollbar);
	m_lcPkg.SetExtendedStyle(m_lcPkg.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcPkg.InsertColumn(0, _T("顺序"), LVCFMT_CENTER, 40);
	m_lcPkg.InsertColumn(1, _T("文件名"), LVCFMT_CENTER, 110);
	m_lcPkg.InsertColumn(2, _T("上传进度"), LVCFMT_CENTER, 110);
	m_lcPkg.InsertColumn(3, _T("上传状态"), LVCFMT_CENTER, 110);

	InitCtrlPosition();
}

void CPkgRecordDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 20;	//上边的间隔
	const int nBottomGap = 30;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	int nCurrLeft = nLeftGap;
	int nCurrTop = nTopGap;
	int nStaticH = 30;

	if (GetDlgItem(IDC_LIST_Pkg)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 1;
		if (nW > 800) nW = 800;
		int nH = cy - nTopGap - nBottomGap;
		nCurrLeft = cx / 2 - nW / 2;
		GetDlgItem(IDC_LIST_Pkg)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		m_lcPkg.SetColumnWidth(1, nW - 280);
	}
	Invalidate();
}


BOOL CPkgRecordDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}


void CPkgRecordDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

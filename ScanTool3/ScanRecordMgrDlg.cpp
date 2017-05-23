// ScanRecordMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanRecordMgrDlg.h"
#include "afxdialogex.h"
#include "ScanMgrDlg.h"


// CScanRecordMgrDlg 对话框

IMPLEMENT_DYNAMIC(CScanRecordMgrDlg, CDialog)

CScanRecordMgrDlg::CScanRecordMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanRecordMgrDlg::IDD, pParent)
	, m_pPkgRecordDlg(NULL), m_pBmkRecordDlg(NULL)
{

}

CScanRecordMgrDlg::~CScanRecordMgrDlg()
{
}

void CScanRecordMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScanRecordMgrDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_BmkRecord, &CScanRecordMgrDlg::OnBnClickedBtnBmkrecord)
	ON_BN_CLICKED(IDC_BTN_PkgRecord, &CScanRecordMgrDlg::OnBnClickedBtnPkgrecord)
	ON_BN_CLICKED(IDC_BTN_ReBackScan, &CScanRecordMgrDlg::OnBnClickedBtnRebackscan)
END_MESSAGE_MAP()


// CScanRecordMgrDlg 消息处理程序

BOOL CScanRecordMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	return TRUE;
}

BOOL CScanRecordMgrDlg::PreTranslateMessage(MSG* pMsg)
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

void CScanRecordMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

BOOL CScanRecordMgrDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CScanRecordMgrDlg::InitUI()
{
	m_pBmkRecordDlg = new CScanBmkRecordDlg(this);
	m_pBmkRecordDlg->Create(CScanBmkRecordDlg::IDD, this);
	m_pBmkRecordDlg->ShowWindow(SW_SHOW);

	m_pPkgRecordDlg = new CPkgRecordDlg(this);
	m_pPkgRecordDlg->Create(CPkgRecordDlg::IDD, this);
	m_pPkgRecordDlg->ShowWindow(SW_HIDE);

	InitCtrlPosition();
}

void CScanRecordMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 20;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 2;

	int nBtnW = 100;
	int nBtnH = 35;

	int nTmpW = cx - nLeftGap - nRightGap;
	if (nTmpW > 800) nTmpW = 800;

	int nCurrLeft = nLeftGap + cx / 2 - nTmpW / 2;
	int nCurrTop = nTopGap;
	if (GetDlgItem(IDC_BTN_BmkRecord)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_BmkRecord)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (GetDlgItem(IDC_BTN_PkgRecord)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_PkgRecord)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (GetDlgItem(IDC_BTN_ReBackScan)->GetSafeHwnd())
	{
		int nW = 80;
		int nX = cx / 2 + nTmpW / 2 - nW;
		GetDlgItem(IDC_BTN_ReBackScan)->MoveWindow(nX, nCurrTop, nW, nBtnH);
	}

	CRect rtChildDlg;
	rtChildDlg.left = nLeftGap + cx / 2 - nTmpW / 2;
	rtChildDlg.top = nTopGap + nBtnH + nGap;
	rtChildDlg.right = rtChildDlg.left + nTmpW;
	rtChildDlg.bottom = cy - nBottomGap;
	
	if (m_pBmkRecordDlg && m_pBmkRecordDlg->GetSafeHwnd())
		m_pBmkRecordDlg->MoveWindow(rtChildDlg);
	if (m_pPkgRecordDlg && m_pPkgRecordDlg->GetSafeHwnd())
		m_pPkgRecordDlg->MoveWindow(rtChildDlg);
	Invalidate();
}

void CScanRecordMgrDlg::ReleaseData()
{
	if (m_pBmkRecordDlg)
	{
		m_pBmkRecordDlg->DestroyWindow();
		SAFE_RELEASE(m_pBmkRecordDlg);
	}
	if (m_pPkgRecordDlg)
	{
		m_pPkgRecordDlg->DestroyWindow();
		SAFE_RELEASE(m_pPkgRecordDlg);
	}
}

void CScanRecordMgrDlg::OnDestroy()
{
	CDialog::OnDestroy();

	ReleaseData();
}

void CScanRecordMgrDlg::OnBnClickedBtnBmkrecord()
{
	m_pBmkRecordDlg->ShowWindow(SW_SHOW);
	m_pPkgRecordDlg->ShowWindow(SW_HIDE);
}


void CScanRecordMgrDlg::OnBnClickedBtnPkgrecord()
{
	m_pBmkRecordDlg->ShowWindow(SW_HIDE);
	m_pPkgRecordDlg->ShowWindow(SW_SHOW);
}


void CScanRecordMgrDlg::OnBnClickedBtnRebackscan()
{
	CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
	pDlg->ShowChildDlg(3);
}

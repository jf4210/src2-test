// ScanMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ScanMgrDlg.h"
#include "afxdialogex.h"


// CScanMgrDlg 对话框

IMPLEMENT_DYNAMIC(CScanMgrDlg, CDialog)

CScanMgrDlg::CScanMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanMgrDlg::IDD, pParent)
	, m_pScanDlg(NULL)
{

}

CScanMgrDlg::~CScanMgrDlg()
{
}

void CScanMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BOOL CScanMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitChildDlg();
	InitCtrlPosition();

	return TRUE;
}

BOOL CScanMgrDlg::PreTranslateMessage(MSG* pMsg)
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

BEGIN_MESSAGE_MAP(CScanMgrDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_Scan, &CScanMgrDlg::OnBnClickedBtnScan)
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CScanMgrDlg 消息处理程序

void CScanMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 100;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	m_rtChildDlg.left = nLeftGap;
	m_rtChildDlg.top = nTopGap;
	m_rtChildDlg.right = cx - nRightGap;
	m_rtChildDlg.bottom = cy - nBottomGap;

	int nCurrLeft = nLeftGap;
	int nCurrTop = 20;
	int nBtnW = cx * 0.1;
	int nBtnH = 50;
	if (nBtnW < 100) nBtnW = 100;
	if (nBtnW > 200) nBtnW = 200;
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (GetDlgItem(IDC_BTN_DataCheck)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_DataCheck)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}

	//child dlg
	if (m_pScanDlg && m_pScanDlg->GetSafeHwnd())
		m_pScanDlg->MoveWindow(m_rtChildDlg);
	Invalidate();
}


void CScanMgrDlg::InitChildDlg()
{
	m_pScanDlg = new CScanDlg(this);
	m_pScanDlg->Create(CScanDlg::IDD, this);
	m_pScanDlg->ShowWindow(SW_SHOW);
}

void CScanMgrDlg::ReleaseDlg()
{
	if (m_pScanDlg)
	{
		m_pScanDlg->DestroyWindow();
		SAFE_RELEASE(m_pScanDlg);
	}
}

void CScanMgrDlg::OnBnClickedBtnScan()
{
	m_pScanDlg->ShowWindow(SW_SHOW);
}


void CScanMgrDlg::OnDestroy()
{
	CDialog::OnDestroy();

	ReleaseDlg();
}


void CScanMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}
void CScanMgrDlg::UpdateInfo()
{
	if (m_pScanDlg)	m_pScanDlg->UpdateInfo();
}


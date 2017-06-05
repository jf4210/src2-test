// ScanRecordMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanRecordMgrDlg.h"
#include "afxdialogex.h"
#include "ScanMgrDlg.h"

#include "ScanTool3Dlg.h"

// CScanRecordMgrDlg 对话框

IMPLEMENT_DYNAMIC(CScanRecordMgrDlg, CDialog)

CScanRecordMgrDlg::CScanRecordMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanRecordMgrDlg::IDD, pParent)
	, m_pPkgRecordDlg(NULL), m_pBmkRecordDlg(NULL), m_nReturnFlag(2)
{

}

CScanRecordMgrDlg::~CScanRecordMgrDlg()
{
}

void CScanRecordMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_BmkRecord, m_bmpBtnBmk);
	DDX_Control(pDX, IDC_BTN_PkgRecord, m_bmpBtnPkg);
	DDX_Control(pDX, IDC_BTN_ReBackScan, m_bmpBtnReturn);
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

	UpdateChildDlg();
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

void CScanRecordMgrDlg::DrawBorder(CDC *pDC)
{
	CPen *pOldPen = NULL;
	CPen pPen;
 	CRect rcClient(0, 0, 0, 0);
// 	GetClientRect(&rcClient);
	rcClient.left = m_rtChildDlg.left - 1;
	rcClient.right = m_rtChildDlg.right + 1;
	rcClient.top = m_rtChildDlg.top - 1;
	rcClient.bottom = m_rtChildDlg.bottom + 1;

	pPen.CreatePen(PS_SOLID, 1, RGB(225, 242, 250));

	pDC->SelectStockObject(NULL_BRUSH);
	pOldPen = pDC->SelectObject(&pPen);
	pDC->Rectangle(&rcClient);
	pDC->SelectObject(pOldPen);
	pPen.Detach();
	//	ReleaseDC(pDC);
}

BOOL CScanRecordMgrDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	DrawBorder(pDC);
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CScanRecordMgrDlg::InitUI()
{
	m_bmpBtnBmk.SetStateBitmap(IDB_Tab_Normal, IDB_Tab_Click, IDB_Tab_Over);
	m_bmpBtnBmk.SetWindowText(_T("考生库详情"));
//	m_bmpBtnBmk.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(116, 116, 116));

	m_bmpBtnPkg.SetStateBitmap(IDB_Tab_Normal, IDB_Tab_Click, IDB_Tab_Over);
	m_bmpBtnPkg.SetWindowText(_T("试卷袋详情"));
//	m_bmpBtnPkg.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(116, 116, 116));

	m_bmpBtnReturn.SetStateBitmap(IDB_RecordDlg_Btn_Hover, 0, IDB_RecordDlg_Btn);
	m_bmpBtnReturn.SetWindowText(_T("返回"));

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
	if (_pCurrExam_ && _pCurrExam_->nModel == 0)	//手阅
	{
		if (GetDlgItem(IDC_BTN_BmkRecord)->GetSafeHwnd())
		{
			// 			GetDlgItem(IDC_BTN_BmkRecord)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
			// 			nCurrLeft += (nBtnW + nGap);

			GetDlgItem(IDC_BTN_BmkRecord)->ShowWindow(SW_HIDE);
		}
		if (GetDlgItem(IDC_BTN_PkgRecord)->GetSafeHwnd())
		{
			GetDlgItem(IDC_BTN_PkgRecord)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
			nCurrLeft += (nBtnW + nGap);
		}
	}
	else	//网阅
	{
		if (GetDlgItem(IDC_BTN_BmkRecord)->GetSafeHwnd())
		{
			GetDlgItem(IDC_BTN_BmkRecord)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
			nCurrLeft += (nBtnW /*+ nGap*/);

			GetDlgItem(IDC_BTN_BmkRecord)->ShowWindow(SW_SHOW);
		}
		if (GetDlgItem(IDC_BTN_PkgRecord)->GetSafeHwnd())
		{
			GetDlgItem(IDC_BTN_PkgRecord)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
			nCurrLeft += (nBtnW + nGap);
		}
	}
	if (GetDlgItem(IDC_BTN_ReBackScan)->GetSafeHwnd())
	{
		int nW = 80;
		int nX = cx / 2 + nTmpW / 2 - nW;
		GetDlgItem(IDC_BTN_ReBackScan)->MoveWindow(nX, nCurrTop, nW, nBtnH);
	}

	m_rtChildDlg.left = nLeftGap + cx / 2 - nTmpW / 2;
	m_rtChildDlg.top = nTopGap + nBtnH + 1 /*nGap*/;
	m_rtChildDlg.right = m_rtChildDlg.left + nTmpW;
	m_rtChildDlg.bottom = cy - 1;
	
	if (m_pBmkRecordDlg && m_pBmkRecordDlg->GetSafeHwnd())
		m_pBmkRecordDlg->MoveWindow(m_rtChildDlg);
	if (m_pPkgRecordDlg && m_pPkgRecordDlg->GetSafeHwnd())
		m_pPkgRecordDlg->MoveWindow(m_rtChildDlg);
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

	m_pBmkRecordDlg->UpDateInfo();
}


void CScanRecordMgrDlg::OnBnClickedBtnPkgrecord()
{
	m_pBmkRecordDlg->ShowWindow(SW_HIDE);
	m_pPkgRecordDlg->ShowWindow(SW_SHOW);

	m_pPkgRecordDlg->UpdateChildDlg();
}


void CScanRecordMgrDlg::OnBnClickedBtnRebackscan()
{
	if (m_nReturnFlag == 1)
	{
		CScanTool3Dlg* pDlg = (CScanTool3Dlg*)AfxGetMainWnd();
		pDlg->SwitchDlg(0);
	}
	else
	{
		CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
		pDlg->ShowChildDlg(3);
	}
}

void CScanRecordMgrDlg::SetReBackDlg(int nFlag)
{
	m_nReturnFlag = nFlag;
}

void CScanRecordMgrDlg::UpdateChildDlg()
{
	m_pBmkRecordDlg->UpDateInfo();
	m_pPkgRecordDlg->UpdateChildDlg();
}


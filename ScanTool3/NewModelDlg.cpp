// NewModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "NewModelDlg.h"
#include "afxdialogex.h"


// CNewModelDlg 对话框

IMPLEMENT_DYNAMIC(CNewModelDlg, CDialog)

CNewModelDlg::CNewModelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewModelDlg::IDD, pParent)
{
	mTreeImageList.Create(16, 16, ILC_COLOR24, 2, 2);

//	mBmpRoot.LoadBitmap(IDB_TREE_R);
	mBmpLeaf.LoadBitmap(IDB_TREE_L);

	mTreeImageList.Add(&mBmpRoot, RGB(0, 0, 0));
	mTreeImageList.Add(&mBmpLeaf, RGB(0, 0, 0));
}

CNewModelDlg::~CNewModelDlg()
{
}

void CNewModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_CheckPoint, m_treeChkPoint);
}


BEGIN_MESSAGE_MAP(CNewModelDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CNewModelDlg 消息处理程序

BOOL CNewModelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	InitCtrlPosition();

	return TRUE;
}

BOOL CNewModelDlg::PreTranslateMessage(MSG* pMsg)
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

void CNewModelDlg::InitTreeData()
{
	m_treeChkPoint.SetImageList(&mTreeImageList, TVSIL_NORMAL);

	HTREEITEM hRootItem = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("全部"), 1, 1, 0, 0, 0, NULL, NULL);
//	HTREEITEM hRootItem = m_treeChkPoint.InsertItem(_T("全部"), 1, 1, NULL, NULL);
	HTREEITEM hFixPoint = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("定点"), 0, 0, 0, 0, 0, NULL, NULL);
// 	HTREEITEM hFixPoint1 = m_treeChkPoint.InsertItem(_T("定点1"), -1, -1, hFixPoint, NULL);
// 	HTREEITEM hFixPoint2 = m_treeChkPoint.InsertItem(_T("定点2"), -1, -1, hFixPoint, NULL);

	HTREEITEM hSubjectPoint = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("科目校验点"), 0, 0, 0, 0, 0, NULL, NULL);
	HTREEITEM hGrayPoint = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("灰度校验点"), 0, 0, 0, 0, 0, NULL, NULL);
	HTREEITEM hQkPoint = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("缺考点"), 0, 0, 0, 0, 0, NULL, NULL);
	HTREEITEM hZkzhPoint = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("考号"), 0, 0, 0, 0, 0, NULL, NULL);
	HTREEITEM hOmrPoint = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("客观题"), 0, 0, 0, 0, 0, NULL, NULL);
	HTREEITEM hElectOmrPoint = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("选做题"), 0, 0, 0, 0, 0, NULL, NULL);

// 	HTREEITEM hRootItem = m_treeChkPoint.InsertItem(TVIF_TEXT, _T("全部"), 0, 0, 0, 0, 0, NULL, NULL);
// 	HTREEITEM hFixPoint = m_treeChkPoint.InsertItem(_T("定点"), 1, 1, hRootItem, NULL);
// 	HTREEITEM hSubjectPoint = m_treeChkPoint.InsertItem(_T("科目校验点"), 1, 1, hRootItem, NULL);
// 	HTREEITEM hGrayPoint = m_treeChkPoint.InsertItem(_T("灰度校验点"), 1, 1, hRootItem, NULL);
// 	HTREEITEM hQkPoint = m_treeChkPoint.InsertItem(_T("缺考点"), 1, 1, hRootItem, NULL);
// 	HTREEITEM hZkzhPoint = m_treeChkPoint.InsertItem(_T("考号"), 1, 1, hRootItem, NULL);
// 	HTREEITEM hOmrPoint = m_treeChkPoint.InsertItem(_T("客观题"), 1, 1, hRootItem, NULL);
// 	HTREEITEM hElectOmrPoint = m_treeChkPoint.InsertItem(_T("选做题"), 1, 1, hRootItem, NULL);

	m_treeChkPoint.Expand(hRootItem,		TVE_EXPAND);
	m_treeChkPoint.Expand(hFixPoint,		TVE_EXPAND);
	m_treeChkPoint.Expand(hSubjectPoint,	TVE_EXPAND);
	m_treeChkPoint.Expand(hGrayPoint,		TVE_EXPAND);
	m_treeChkPoint.Expand(hQkPoint,			TVE_EXPAND);
	m_treeChkPoint.Expand(hZkzhPoint,		TVE_EXPAND);
	m_treeChkPoint.Expand(hOmrPoint,		TVE_EXPAND);
	m_treeChkPoint.Expand(hElectOmrPoint,	TVE_EXPAND);
	m_treeChkPoint.SelectItem(hRootItem);
}

void CNewModelDlg::InitUI()
{
	InitTreeData();
}

void CNewModelDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 10;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	int nBtnW = 80;
	int nBtnH = 40;

	int nCurrLeft = nLeftGap;
	int nCurrTop = nTopGap;
}


void CNewModelDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

BOOL CNewModelDlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 242, 250

	return TRUE;
}

HBRUSH CNewModelDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

// ShowPapersDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DataMgrTool.h"
#include "ShowPapersDlg.h"
#include "afxdialogex.h"


// CShowPapersDlg 对话框

IMPLEMENT_DYNAMIC(CShowPapersDlg, CDialog)

CShowPapersDlg::CShowPapersDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SHOWPAPERSDLG, pParent)
	,m_pShowPicDlg(NULL), m_pPapers(NULL)
{

}

CShowPapersDlg::~CShowPapersDlg()
{
}

void CShowPapersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Papers, m_listPaper);
}


BEGIN_MESSAGE_MAP(CShowPapersDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_MESSAGE(WM_CV_RBTNUP, &CShowPapersDlg::RoiRBtnUp)
	ON_COMMAND(ID_LeftRotate, &CShowPapersDlg::LeftRotate)
	ON_COMMAND(ID_RightRotate, &CShowPapersDlg::RightRotate)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Papers, &CShowPapersDlg::OnNMDblclkListPapers)
END_MESSAGE_MAP()


// CShowPapersDlg 消息处理程序
BOOL CShowPapersDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	ShowPapers(m_pPapers);

	return TRUE;
}

void CShowPapersDlg::InitUI()
{
	m_listPaper.SetExtendedStyle(m_listPaper.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_listPaper.InsertColumn(0, _T("题卡"), LVCFMT_CENTER, 40);
	m_listPaper.InsertColumn(1, _T("考号"), LVCFMT_CENTER, 120);

	if (!m_pShowPicDlg)
	{
		m_pShowPicDlg = new CShowPicDlg();
		m_pShowPicDlg->Create(CShowPicDlg::IDD, this);
		m_pShowPicDlg->ShowWindow(SW_SHOW);
	}
	m_pShowPicDlg->setShowModel(2);

	InitCtrlPosition();
}

void CShowPapersDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 10;	//上边的间隔，留给控制栏
	const int nLeftGap = 10;		//左边的空白间隔
	const int nBottomGap = 40;	//下边的空白间隔
	const int nRightGap = 10;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	int nLeftWidth = 200;
	int nStaticH = 20;
	int nListW = nLeftWidth - nLeftGap;
	if (GetDlgItem(IDC_STATIC_ListTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ListTips)->MoveWindow(nCurrentLeft, nCurrentTop, nListW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	if (m_listPaper.GetSafeHwnd())
	{
		m_listPaper.MoveWindow(nCurrentLeft, nCurrentTop, nListW, cy - nCurrentTop - nBottomGap);
	}

	nCurrentLeft = nLeftWidth + nGap;
	nCurrentTop = nTopGap;
	int nPicShowTabCtrlWidth = cx - nCurrentLeft - nRightGap;
	int nPicShowTabCtrlHigh = cy - nTopGap - nBottomGap;

	if (m_pShowPicDlg && m_pShowPicDlg->GetSafeHwnd())
		m_pShowPicDlg->MoveWindow(nCurrentLeft, nTopGap, nPicShowTabCtrlWidth, nPicShowTabCtrlHigh);
}

void CShowPapersDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (m_pShowPicDlg)
	{
		m_pShowPicDlg->DestroyWindow();
		SAFE_RELEASE(m_pShowPicDlg);
	}
	Invalidate();
}

void CShowPapersDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CShowPapersDlg::setShowPapers(pPAPERSINFO pPapers)
{
	m_pPapers = pPapers;
}

void CShowPapersDlg::ShowPapers(pPAPERSINFO pPapers)
{
	if (!pPapers) return;

	USES_CONVERSION;
	m_listPaper.DeleteAllItems();
	for (auto pPaper : pPapers->lPaper)
	{
		int nCount = m_listPaper.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", pPaper->nIndex);	//nCount + 1
		m_listPaper.InsertItem(nCount, NULL);
		m_listPaper.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_listPaper.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));

		m_listPaper.SetItemData(nCount, (DWORD_PTR)pPaper);
	}
}

void CShowPapersDlg::ShowPaper(pST_PaperInfo pPaper)
{
	m_pShowPicDlg->setShowPaper(pPaper);
}

void CShowPapersDlg::LeftRotate()
{
	if (m_pShowPicDlg) m_pShowPicDlg->setRotate(3);
}

void CShowPapersDlg::RightRotate()
{
	if (m_pShowPicDlg) m_pShowPicDlg->setRotate(2);
}

LRESULT CShowPapersDlg::RoiRBtnUp(WPARAM wParam, LPARAM lParam)
{
	cv::Point pt = *(cv::Point*)(wParam);
	//下面的这段代码, 不单单适应于ListCtrl  
	CMenu menu, *pPopup;
	menu.LoadMenu(IDR_MENU_Rotation);
	pPopup = menu.GetSubMenu(0);
	CPoint myPoint;
	ClientToScreen(&myPoint);
	GetCursorPos(&myPoint); //鼠标位置  
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);//GetParent()

	return TRUE;
}

void CShowPapersDlg::OnNMDblclkListPapers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (pNMItemActivate->iItem < 0)
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_listPaper.GetItemData(pNMItemActivate->iItem);
	m_pShowPicDlg->ShowWindow(SW_SHOW);
	m_pShowPicDlg->setShowPaper(pPaper);
}

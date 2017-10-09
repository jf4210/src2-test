// AnswerShowDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "AnswerShowDlg.h"
#include "afxdialogex.h"


// CAnswerShowDlg 对话框

IMPLEMENT_DYNAMIC(CAnswerShowDlg, CDialogEx)

CAnswerShowDlg::CAnswerShowDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ANSWERSHOWDLG, pParent)
	, m_nCurrentItem(-1)
{

}

CAnswerShowDlg::~CAnswerShowDlg()
{
}

void CAnswerShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Answer, m_lAnswerListCtrl);
}


BEGIN_MESSAGE_MAP(CAnswerShowDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(NM_HOVER, IDC_LIST_Answer, &CAnswerShowDlg::OnNMHoverListAnswer)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Answer, &CAnswerShowDlg::OnNMDblclkListAnswer)
END_MESSAGE_MAP()

// CAnswerShowDlg 消息处理程序
BOOL CAnswerShowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitUI();
	
	return TRUE;
}

void CAnswerShowDlg::InitData(pST_PaperInfo pPaper)
{
	USES_CONVERSION;
	while (m_lAnswerListCtrl.DeleteColumn(1));
	OMRRESULTLIST::iterator it = pPaper->lOmrResult.begin();
	for(int i = 1; it != pPaper->lOmrResult.end(); it++, i++)
	{
		char szTh[10] = { 0 };
		sprintf_s(szTh, "%d", it->nTH);
		m_lAnswerListCtrl.InsertColumn(i, A2T(szTh), LVCFMT_CENTER, 40);

		int nRow = 0;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, _T("题号"));
		++nRow;
		char szType[10] = { 0 };
		if (it->nSingle == 0)
			strcpy_s(szType, "单");
		else if (it->nSingle == 1)
			strcpy_s(szType, "多");
		else
			strcpy_s(szType, "判");
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(szType));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal1.c_str()));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal2.c_str()));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal3.c_str()));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal.c_str()));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		if (it->nDoubt == 1)
			m_lAnswerListCtrl.SetItemText(nRow, i, _T("*"));
	}
}

void CAnswerShowDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nGap = 2;	//控件的间隔
	int nTopGap = 2;	//距离上边边缘的间隔
	int nBottomGap = 2;	//距离下边边缘的间隔
	int nLeftGap = 2;	//距离左边边缘的间隔
	int nRightGap = 2;	//距离右边边缘的间隔
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	if (m_lAnswerListCtrl.GetSafeHwnd())
	{
		int nW = cx - nCurrentLeft - nRightGap;
		int nH = cy - nCurrentTop - nBottomGap;
		m_lAnswerListCtrl.MoveWindow(nCurrentLeft, nCurrentTop, nW, nH);
	}
}

void CAnswerShowDlg::InitUI()
{
	m_lAnswerListCtrl.SetExtendedStyle(m_lAnswerListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES );
	m_lAnswerListCtrl.InsertColumn(0, _T("试卷袋名"), LVCFMT_CENTER, 80);

	int nRow = 0;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("题号"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("单/多"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("识别1"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("识别2"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("识别3"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("最终"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("怀疑"));
	++nRow;

	InitCtrlPosition();
}

void CAnswerShowDlg::SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem)
{
	if (nItem < 0) return;

	lCtrl.GetItemColors(1, nItem, crOldText, crOldBackground);
	for (int i = 0; i < lCtrl.GetItemCount(); i++)							//设置高亮显示(手动设置背景颜色)
		lCtrl.SetItemColors(i, nItem, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255
}

void CAnswerShowDlg::UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem)
{
	if (nItem < 0) return;

	for (int i = 0; i < lCtrl.GetItemCount(); i++)
		if (!lCtrl.GetModified(i, nItem))
			lCtrl.SetItemColors(i, nItem, crOldText, crOldBackground);
		else
			lCtrl.SetItemColors(i, nItem, RGB(255, 0, 0), crOldBackground);
}

void CAnswerShowDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CAnswerShowDlg::OnNMHoverListAnswer(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;		//**********	这里如果不响应，同时返回结果值不为1的话，	****************
						//**********	就会产生产生TRACK SELECT，也就是鼠标悬停	****************
						//**********	一段时间后，所在行自动被选中
}

void CAnswerShowDlg::OnNMDblclkListAnswer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	UnSetListCtrlHighLightShow(m_lAnswerListCtrl, m_nCurrentItem);
	m_nCurrentItem = pNMItemActivate->iSubItem;

	SetListCtrlHighLightShow(m_lAnswerListCtrl, m_nCurrentItem);
}

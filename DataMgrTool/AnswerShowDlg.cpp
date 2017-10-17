// AnswerShowDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DataMgrTool.h"
#include "AnswerShowDlg.h"
#include "afxdialogex.h"


// CAnswerShowDlg 对话框

IMPLEMENT_DYNAMIC(CAnswerShowDlg, CDialogEx)

CAnswerShowDlg::CAnswerShowDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ANSWERSHOWDLG, pParent)
	, m_nCurrentItem(-1), m_pModel(NULL), m_nShowType(6), m_pShowPaper(NULL)
{

}

CAnswerShowDlg::~CAnswerShowDlg()
{
}

void CAnswerShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Answer, m_lAnswerListCtrl);
	DDX_Control(pDX, IDC_CHK_Right, m_btnChkRight);
	DDX_Control(pDX, IDC_CHK_Doubt, m_btnChkDoubt);
	DDX_Control(pDX, IDC_CHK_Null, m_btnChkNull);
}


BEGIN_MESSAGE_MAP(CAnswerShowDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(NM_HOVER, IDC_LIST_Answer, &CAnswerShowDlg::OnNMHoverListAnswer)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Answer, &CAnswerShowDlg::OnNMDblclkListAnswer)
	ON_BN_CLICKED(IDC_CHK_Right, &CAnswerShowDlg::OnBnClickedChkRight)
	ON_BN_CLICKED(IDC_CHK_Doubt, &CAnswerShowDlg::OnBnClickedChkDoubt)
	ON_BN_CLICKED(IDC_CHK_Null, &CAnswerShowDlg::OnBnClickedChkNull)
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
	if (!pPaper)
		return;

	m_pShowPaper = pPaper;

	USES_CONVERSION;
	if (m_pModel)
	{
		OMRRESULTLIST::iterator it = pPaper->lOmrResult.begin();
		for (int i = 1; it != pPaper->lOmrResult.end(); it++, i++)
		{
			int nRow = 0;
			char szType[10] = { 0 };
			if (it->nSingle == 0)
				strcpy_s(szType, "单");
			else if (it->nSingle == 1)
				strcpy_s(szType, "多");
			else
				strcpy_s(szType, "判");
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(szType));
			++nRow;
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal1.c_str()));
			++nRow;
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal2.c_str()));
			++nRow;
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal3.c_str()));
			++nRow;
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal.c_str()));
			++nRow;
			if (it->nDoubt == 1)
				m_lAnswerListCtrl.SetItemText(nRow, i, _T("*"));
		}
	}
	else
	{
		while (m_lAnswerListCtrl.DeleteColumn(1));
		m_lAnswerListCtrl.DeleteAllItems();

		OMRRESULTLIST::iterator it = pPaper->lOmrResult.begin();
		for (int i = 1; it != pPaper->lOmrResult.end(); it++, i++)
		{
			if (m_nShowType == 0 || (m_nShowType == 1 && it->nDoubt != 0) || (m_nShowType == 2 && it->nDoubt != 1) || \
				(m_nShowType == 3 && it->nDoubt == 2) || (m_nShowType == 4 && it->nDoubt != 2) || \
				(m_nShowType == 5 && it->nDoubt == 1) || (m_nShowType == 6 && it->nDoubt == 0))
			{
				i--;
				continue;
			}

			char szTh[10] = { 0 };
			sprintf_s(szTh, "%d", it->nTH);
			m_lAnswerListCtrl.InsertColumn(i, A2T(szTh), LVCFMT_CENTER, 45);
		}
		if (m_lAnswerListCtrl.GetColumns() == 0)
			return;

		int nRow = 0;
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

		it = pPaper->lOmrResult.begin();
		for (int i = 1; it != pPaper->lOmrResult.end(); it++, i++)
		{
			if (m_nShowType == 0 || (m_nShowType == 1 && it->nDoubt != 0) || (m_nShowType == 2 && it->nDoubt != 1) || \
				(m_nShowType == 3 && it->nDoubt == 2) || (m_nShowType == 4 && it->nDoubt != 2) || \
				(m_nShowType == 5 && it->nDoubt == 1) || (m_nShowType == 6 && it->nDoubt == 0))
			{
				i--;
				continue;
			}

			nRow = 0;
			char szType[10] = { 0 };
			if (it->nSingle == 0)
				strcpy_s(szType, "单");
			else if (it->nSingle == 1)
				strcpy_s(szType, "多");
			else
				strcpy_s(szType, "判");
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(szType));
			++nRow;
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal1.c_str()));
			++nRow;
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal2.c_str()));
			++nRow;
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal3.c_str()));
			++nRow;
			m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal.c_str()));
			++nRow;
			if (it->nDoubt == 1)
				m_lAnswerListCtrl.SetItemText(nRow, i, _T("*"));
		}
	}
}

void CAnswerShowDlg::InitModel(pMODEL pModel)
{
	m_pModel = pModel;

	if (m_pModel)
	{
		while (m_lAnswerListCtrl.DeleteColumn(1));
		m_lAnswerListCtrl.DeleteAllItems();

		USES_CONVERSION;
		int nCount = 0;
		for (int i = 0; i < m_pModel->vecPaperModel.size(); i++)
		{
			OMRLIST::iterator it = m_pModel->vecPaperModel[i]->lOMR2.begin();
			for (int j = 1; it != m_pModel->vecPaperModel[i]->lOMR2.end(); it++, j++)
			{
				char szTh[10] = { 0 };
				sprintf_s(szTh, "%d", it->nTH);
				m_lAnswerListCtrl.InsertColumn(j + nCount, A2T(szTh), LVCFMT_CENTER, 45);
			}

			int nRow = 0;
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

			nCount += m_pModel->vecPaperModel[i]->lOMR2.size();
		}
	}
}

void CAnswerShowDlg::SetShowType(int nType)
{
	m_nShowType = nType;
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

	int nChkW = 60;
	int nChkH = 20;
	if (m_btnChkRight.GetSafeHwnd())
	{
		m_btnChkRight.MoveWindow(nCurrentLeft, nCurrentTop, nChkW, nChkH);
		nCurrentLeft += (nChkW + nGap);
	}
	if (m_btnChkDoubt.GetSafeHwnd())
	{
		m_btnChkDoubt.MoveWindow(nCurrentLeft, nCurrentTop, nChkW, nChkH);
		nCurrentLeft += (nChkW + nGap);
	}
	if (m_btnChkNull.GetSafeHwnd())
	{
		m_btnChkNull.MoveWindow(nCurrentLeft, nCurrentTop, nChkW, nChkH);
		nCurrentTop += (nChkH + nGap);
	}

	nCurrentLeft = nLeftGap;
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
	m_lAnswerListCtrl.InsertColumn(0, _T("题号"), LVCFMT_CENTER, 50);

	int nRow = 0;
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

	setCheckStatus(m_nShowType);

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

void CAnswerShowDlg::setCheckStatus(int nStatus)
{
	if (nStatus == 0)
	{
		m_btnChkRight.SetCheck(0);
		m_btnChkDoubt.SetCheck(0);
		m_btnChkNull.SetCheck(0);
	}
	else if (nStatus == 1)
	{
		m_btnChkRight.SetCheck(1);
		m_btnChkDoubt.SetCheck(0);
		m_btnChkNull.SetCheck(0);
	}
	else if (nStatus == 2)
	{
		m_btnChkRight.SetCheck(0);
		m_btnChkDoubt.SetCheck(1);
		m_btnChkNull.SetCheck(0);
	}
	else if (nStatus == 3)
	{
		m_btnChkRight.SetCheck(1);
		m_btnChkDoubt.SetCheck(1);
		m_btnChkNull.SetCheck(0);
	}
	else if (nStatus == 4)
	{
		m_btnChkRight.SetCheck(0);
		m_btnChkDoubt.SetCheck(0);
		m_btnChkNull.SetCheck(1);
	}
	else if (nStatus == 5)
	{
		m_btnChkRight.SetCheck(1);
		m_btnChkDoubt.SetCheck(0);
		m_btnChkNull.SetCheck(1);
	}
	else if (nStatus == 6)
	{
		m_btnChkRight.SetCheck(0);
		m_btnChkDoubt.SetCheck(1);
		m_btnChkNull.SetCheck(1);
	}
	else if (nStatus == 7)
	{
		m_btnChkRight.SetCheck(1);
		m_btnChkDoubt.SetCheck(1);
		m_btnChkNull.SetCheck(1);
	}
}

int CAnswerShowDlg::getCheckStatus()
{
	//空|怀疑|无怀疑 --按位排序
	// 1  1  1  --显示所有
	// 1  1  0  --显示怀疑、空
	int nRight = m_btnChkRight.GetCheck();
	int nDoubt = m_btnChkDoubt.GetCheck();
	int nNull = m_btnChkNull.GetCheck();

	int nResult = 0;
	if (nRight && nDoubt && nNull)	//111
		nResult = 7;
	else if (!nRight && nDoubt && !nNull)	//010
		nResult = 2;
	else if (!nRight && !nDoubt && nNull)	//100
		nResult = 4;
	else if (!nRight && nDoubt && nNull)	//110
		nResult = 6;
	else if (nRight && !nDoubt && !nNull)	//001
		nResult = 1;
	else if (nRight && nDoubt && !nNull)	//011
		nResult = 3;
	else if (nRight && !nDoubt && nNull)	//101
		nResult = 5;


	return nResult;
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

	m_lAnswerListCtrl.Invalidate();
}

void CAnswerShowDlg::OnBnClickedChkRight()
{
	m_nShowType = getCheckStatus();
	InitData(m_pShowPaper);
}

void CAnswerShowDlg::OnBnClickedChkDoubt()
{
	m_nShowType = getCheckStatus();
	InitData(m_pShowPaper);
}

void CAnswerShowDlg::OnBnClickedChkNull()
{
	m_nShowType = getCheckStatus();
	InitData(m_pShowPaper);
}

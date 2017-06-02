// VagueSearchDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "VagueSearchDlg.h"
#include "afxdialogex.h"
#include "ModifyZkzhDlg.h"

// CVagueSearchDlg 对话框

IMPLEMENT_DYNAMIC(CVagueSearchDlg, CDialog)

CVagueSearchDlg::CVagueSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVagueSearchDlg::IDD, pParent)
	, m_nSearchType(2)
{

}

CVagueSearchDlg::~CVagueSearchDlg()
{
}

void CVagueSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ZkzhSearchResult, m_lcBmk);
	DDX_Text(pDX, IDC_EDIT_SearchKey, m_strSearchKey);
	DDX_Control(pDX, IDC_STATIC_Group, m_GroupStatic);
}


BOOL CVagueSearchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CVagueSearchDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_RADIO_SearchName, &CVagueSearchDlg::OnBnClickedRadioSearchname)
	ON_BN_CLICKED(IDC_RADIO_SearchZkzh, &CVagueSearchDlg::OnBnClickedRadioSearchzkzh)
	ON_BN_CLICKED(IDC_BTN_Search, &CVagueSearchDlg::OnBnClickedBtnSearch)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ZkzhSearchResult, &CVagueSearchDlg::OnNMDblclkListZkzhsearchresult)
END_MESSAGE_MAP()


// CVagueSearchDlg 消息处理程序
void CVagueSearchDlg::InitUI()
{
	m_lcBmk.SetExtendedStyle(m_lcBmk.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcBmk.InsertColumn(0, _T("顺序"), LVCFMT_CENTER, 40);
	m_lcBmk.InsertColumn(1, _T("姓名"), LVCFMT_CENTER, 80);
	m_lcBmk.InsertColumn(2, _T("考号"), LVCFMT_CENTER, 120);
	m_lcBmk.InsertColumn(3, _T("班级"), LVCFMT_CENTER, 80);
	m_lcBmk.InsertColumn(4, _T("学校"), LVCFMT_CENTER, 150);

	switch (m_nSearchType)
	{
		case 1:
			((CButton*)GetDlgItem(IDC_RADIO_SearchName))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_SearchZkzh))->SetCheck(0);
			break;
		case 2:
			((CButton*)GetDlgItem(IDC_RADIO_SearchName))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_SearchZkzh))->SetCheck(1);
			break;
	}

//	m_GroupStatic.SetBackgroundColor(RGB(255, 255, 255));

	CBitmap bmp;
	bmp.LoadBitmap(IDB_Scrollbar);
	BITMAP bm;
	bmp.GetBitmap(&bm);
	m_bitmap_scrollbar = (HBITMAP)bmp.Detach();
	SkinWndScroll(&m_lcBmk, m_bitmap_scrollbar);

	InitCtrlPosition();
}

void CVagueSearchDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 5;	//上边的间隔，留给控制栏
	const int nLeftGap = 5;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nStaticTip = 15;		//列表提示static控件高度
	int nRealW = cx - nLeftGap - nRightGap;
	int nBtnH = 30;				//按钮高度
	int nGroupH = 70;			//group控件高度

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	int nTmpTop = nCurrentTop;
	if (GetDlgItem(IDC_STATIC_Group)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Group)->MoveWindow(nCurrentLeft, nCurrentTop, nRealW, nGroupH);
		nCurrentTop += (nStaticTip + nGap);
		nCurrentLeft += nGap;
	}
	if (GetDlgItem(IDC_RADIO_SearchName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_SearchName)->MoveWindow(nCurrentLeft, nCurrentTop, nRealW / 2 - 5, nStaticTip);
		nCurrentLeft += (nRealW / 2 - 5 + nGap);
		//		nCurrentTop += (nStaticTip + nGap);
	}
	if (GetDlgItem(IDC_RADIO_SearchZkzh)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_SearchZkzh)->MoveWindow(nCurrentLeft, nCurrentTop, nRealW / 2 - 5, nStaticTip);
		nCurrentLeft = nLeftGap + nGap + nGap;
		nCurrentTop += (nStaticTip + nGap);
	}
	if (GetDlgItem(IDC_EDIT_SearchKey)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_SearchKey)->MoveWindow(nCurrentLeft, nCurrentTop, nRealW - 70, nBtnH);
		nCurrentLeft += (nRealW - 70 + nGap);
	}
	if (GetDlgItem(IDC_BTN_Search)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Search)->MoveWindow(nCurrentLeft, nCurrentTop, 60, nBtnH);
		nCurrentLeft = nLeftGap + nGap;
		nCurrentTop = nTmpTop + nGap + nGroupH + nGap;
	}
	if (GetDlgItem(IDC_STATIC_DB_Search)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_DB_Search)->MoveWindow(nCurrentLeft, nCurrentTop, nRealW, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (GetDlgItem(IDC_LIST_ZkzhSearchResult)->GetSafeHwnd())
	{
		GetDlgItem(IDC_LIST_ZkzhSearchResult)->MoveWindow(nCurrentLeft, nCurrentTop, nRealW, cy - nCurrentTop - nBottomGap);
	}
	Invalidate();
}

void CVagueSearchDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

BOOL CVagueSearchDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}


HBRUSH CVagueSearchDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_Group || CurID == IDC_STATIC_DB_Search)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_RADIO_SearchName || CurID == IDC_RADIO_SearchZkzh)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	
	return hbr;
}

void CVagueSearchDlg::OnBnClickedRadioSearchname()
{
	if (((CButton*)GetDlgItem(IDC_RADIO_SearchName))->GetCheck())
	{
		m_nSearchType = 1;
		m_strSearchKey = _T("");
		m_lcBmk.DeleteAllItems();
		UpdateData(FALSE);
	}
}

void CVagueSearchDlg::OnBnClickedRadioSearchzkzh()
{
	if (((CButton*)GetDlgItem(IDC_RADIO_SearchZkzh))->GetCheck())
	{
		m_nSearchType = 2;
		m_strSearchKey = _T("");
		m_lcBmk.DeleteAllItems();
		UpdateData(FALSE);
	}
}

void CVagueSearchDlg::OnBnClickedBtnSearch()
{
	UpdateData(TRUE);
	USES_CONVERSION;
	m_lcBmk.DeleteAllItems();
	std::string strKey = T2A(m_strSearchKey);
	STUDENT_LIST lResult;
	std::string strTable = Poco::format("T%d_%d", m_pModel->nExamID, m_pModel->nSubjectID);
	if (m_pStudentMgr && m_pStudentMgr->SearchStudent(strTable, strKey, m_nSearchType, lResult))
	{
		for (auto obj : lResult)
		{
			int nCount = m_lcBmk.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", nCount + 1);
			m_lcBmk.InsertItem(nCount, NULL);

			m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
			m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(obj.strName.c_str()));
			m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(obj.strZkzh.c_str()));
			m_lcBmk.SetItemText(nCount, 3, (LPCTSTR)A2T(obj.strClassroom.c_str()));
			m_lcBmk.SetItemText(nCount, 4, (LPCTSTR)A2T(obj.strSchool.c_str()));
		}
	}
	else
	{
		AfxMessageBox(_T("搜索失败"));
	}
}

void CVagueSearchDlg::setExamInfo(CStudentMgr* pMgr, pMODEL pModel)
{
	m_pStudentMgr	= pMgr;
	m_pModel		= pModel;
}

bool CVagueSearchDlg::vagueSearch(pST_PaperInfo pPaper)
{
	bool bResult = false;
	if (!pPaper) return bResult;

	//将模糊查找字段变成合法的sql字段
	std::string strVagueKey = pPaper->strRecogSN4Search;
	char szVagueKey[50] = { 0 };
	char * p = szVagueKey;
	bool bLastIsSharp = false;	//标识上一个字符是否是#，用于处理连续的#字符，将多个#变成一个
	for (auto p0 : pPaper->strRecogSN4Search)
	{
		if (p0 == '#')
		{
			if (bLastIsSharp)
				continue;
			*p = '%';
			bLastIsSharp = true;
		}
		else
		{
			*p = p0;
			bLastIsSharp = false;
		}
		++p;
	}
	TRACE("进行模糊查找的字符串: %s\n", szVagueKey);
	m_lcBmk.DeleteAllItems();
	STUDENT_LIST lResult;
	std::string strTable = Poco::format("T%d_%d", m_pModel->nExamID, m_pModel->nSubjectID);
	if (m_pStudentMgr && m_pStudentMgr->SearchStudent(strTable, szVagueKey, m_nSearchType, lResult))
	{
		USES_CONVERSION;
		for (auto obj : lResult)
		{
			int nCount = m_lcBmk.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", nCount + 1);
			m_lcBmk.InsertItem(nCount, NULL);

			m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
			m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(obj.strName.c_str()));
			m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(obj.strZkzh.c_str()));
			m_lcBmk.SetItemText(nCount, 3, (LPCTSTR)A2T(obj.strClassroom.c_str()));
			m_lcBmk.SetItemText(nCount, 4, (LPCTSTR)A2T(obj.strSchool.c_str()));
		}
		bResult = true;
	}

	return bResult;
}

void CVagueSearchDlg::OnNMDblclkListZkzhsearchresult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (pNMItemActivate->iItem < 0)
		return;

	CModifyZkzhDlg* pDlg = (CModifyZkzhDlg*)GetParent();

	USES_CONVERSION;
	CString strZkzh = m_lcBmk.GetItemText(pNMItemActivate->iItem, 2);
	pDlg->m_pCurrentShowPaper->strSN = T2A(strZkzh);
	pDlg->m_pCurrentShowPaper->bModifyZKZH = true;

	CheckZkzhInBmk(pDlg->m_pCurrentShowPaper);
	if (pDlg->m_pCurrentShowPaper->nZkzhInBmkStatus == 0)
	{
		SetZkzhScaned(pDlg->m_pCurrentShowPaper->strSN);
		pDlg->m_pCurrentShowPaper->nZkzhInBmkStatus = 1;
	}

	//需要刷新未识别准考证号列表

	COLORREF crText, crBackground;
	pDlg->m_lcZkzh.GetItemColors(pDlg->m_nCurrentSelItem, 1, crText, crBackground);
	pDlg->m_lcZkzh.SetItemText(pDlg->m_nCurrentSelItem, 1, strZkzh, RGB(255, 0, 0), crBackground);
}

void CVagueSearchDlg::SetZkzhScaned(std::string strZkzh)
{
	for (auto obj : g_lBmkStudent)
	{
		if (obj.strZkzh == strZkzh)
		{
			obj.nScaned == 1;
			break;
		}
	}
}

int CVagueSearchDlg::CheckZkzhInBmk(std::string strZkzh)
{
	int nResult = 0;	//0--报名库不存在，1--报名库存在，-1--报名库检测到已经扫描
	for (auto obj : g_lBmkStudent)
	{
		if (obj.strZkzh == strZkzh)
		{
			if (obj.nScaned == 1)
				nResult = -1;
			else
			{
				nResult = 1;
				obj.nScaned = 1;
			}
			break;
		}
	}

	return nResult;
}

void CVagueSearchDlg::CheckZkzhInBmk(pST_PaperInfo pPaper)
{
	if (!pPaper) return;

	int nResult = CheckZkzhInBmk(pPaper->strSN);
	if (nResult == 1)
		pPaper->nZkzhInBmkStatus = 1;
	else if (nResult == -1)
		pPaper->nZkzhInBmkStatus = -1;
	else
		pPaper->nZkzhInBmkStatus = 0;
}

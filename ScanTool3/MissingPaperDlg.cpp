// MissingPaperDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "MissingPaperDlg.h"
#include "afxdialogex.h"


// CMissingPaperDlg 对话框

IMPLEMENT_DYNAMIC(CMissingPaperDlg, CDialog)

CMissingPaperDlg::CMissingPaperDlg(pPAPERSINFO pPapers, pMODEL pModel, CStudentMgr* pMgr, CWnd* pParent /*=NULL*/)
	: CTipBaseDlg(IDD_MISSINGPAPERDLG, pParent)
	, m_strKD(_T("")), m_strKC(_T("")), m_strZW(_T("")), m_pStudentMgr(pMgr), m_pPapers(pPapers), m_pModel(pModel), m_nCurrentSelItem(0)
{

}

CMissingPaperDlg::~CMissingPaperDlg()
{
}

void CMissingPaperDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MissingPaper, m_lcMissingZkzh);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_bmpBtnClose);
	DDX_Text(pDX, IDC_STATIC_KD, m_strKD);
	DDX_Text(pDX, IDC_STATIC_KC, m_strKC);
	DDX_Text(pDX, IDC_STATIC_ZW, m_strZW);
}


BEGIN_MESSAGE_MAP(CMissingPaperDlg, CTipBaseDlg)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CMissingPaperDlg::OnBnClickedBtnClose)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_HOVER, IDC_LIST_MissingPaper, &CMissingPaperDlg::OnNMHoverListMissingpaper)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MissingPaper, &CMissingPaperDlg::OnNMDblclkListMissingpaper)
END_MESSAGE_MAP()


// CMissingPaperDlg 消息处理程序

BOOL CMissingPaperDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	InitUI();
	SetFontSize();
	InitData();

	UpdateData(FALSE);
	return TRUE;
}

void CMissingPaperDlg::InitUI()
{
	m_lcMissingZkzh.SetExtendedStyle(m_lcMissingZkzh.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcMissingZkzh.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 36);
	m_lcMissingZkzh.InsertColumn(1, _T("准考证号"), LVCFMT_CENTER, 100);
	m_lcMissingZkzh.InsertColumn(2, _T("姓名"), LVCFMT_CENTER, 110);

	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);

	InitCtrlPosition();
}

void CMissingPaperDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 5;	//上边的间隔，留给控制栏
	const int nLeftGap = 2;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nStaticTip = 15;		//列表提示static控件高度
	int nRealW = cx - nLeftGap - nRightGap;
	int nListW = nRealW * 0.4;
	int nBtnH = 30;				//按钮高度

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	int nTmpTop = nCurrentTop;

	if (GetDlgItem(IDC_STATIC_1)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_1)->MoveWindow(nCurrentLeft, nCurrentTop, nListW, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (m_lcMissingZkzh.GetSafeHwnd())
	{
		int nListH = cy - nCurrentTop - nBottomGap;
		m_lcMissingZkzh.MoveWindow(nCurrentLeft, nCurrentTop, nListW, nListH);
	}

	nCurrentTop = nTmpTop;
	nCurrentLeft = nLeftGap + nListW + nGap * 5;
	int nStaticW = 20;
	int nStaticH = 35;
	if (GetDlgItem(IDC_STATIC_2)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_2)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_3)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_3)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_4)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_4)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}

	nCurrentTop = nTmpTop;
	nCurrentLeft += nGap;
	nStaticW = cx - nCurrentLeft - nRightGap;
	if (GetDlgItem(IDC_STATIC_KD)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_KD)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_KC)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_KC)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ZW)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ZW)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
}

void CMissingPaperDlg::InitData()
{
	if (!m_pPapers || !m_pModel || !m_pStudentMgr) return;

	std::set<std::string> setKC;	//考场集合，此集合保证数据无重复
	for (auto pPaperItem : m_pPapers->lPaper)
	{
		if (!pPaperItem->strSN.empty())
			setKC.insert(pPaperItem->strSN);
	}

	STUDENT_LIST lAllStudent;
	for (auto itKcCode : setKC)
	{
		STUDENT_LIST lResult;
		std::string strTable = Poco::format("T%d_%d", m_pModel->nExamID, m_pModel->nSubjectID);
		if (m_pStudentMgr && m_pStudentMgr->GetKCStudent(strTable, itKcCode, lResult))
		{
			for (auto itStudent : lResult)
				lAllStudent.push_back(itStudent);
		}
	}

	USES_CONVERSION;
	m_lcMissingZkzh.DeleteAllItems();
	STUDENT_LIST::iterator itStudent = lAllStudent.begin();
	for (; itStudent != lAllStudent.end(); itStudent++)
	{
		bool bFind = false;
		for (auto paperScaned : m_pPapers->lPaper)
		{
			if (paperScaned->strSN == itStudent->strZkzh)
			{
				bFind = true;
				break;
			}
		}
		if (!bFind)
		{
			//添加进试卷列表控件
			int nCount = m_lcMissingZkzh.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", nCount + 1);
			m_lcMissingZkzh.InsertItem(nCount, NULL);

			m_lcMissingZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));	//pPaper->strStudentInfo.c_str()
			m_lcMissingZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(itStudent->strZkzh.c_str()));
			m_lcMissingZkzh.SetItemText(nCount, 2, (LPCTSTR)A2T(itStudent->strName.c_str()));

			m_lcMissingZkzh.SetItemData(nCount, (DWORD_PTR)(&itStudent));
		}
	}

	int nCount = m_lcMissingZkzh.GetItemCount();
	if (nCount > 0)
	{
		m_nCurrentSelItem = 0;
		m_lcMissingZkzh.GetItemColors(m_nCurrentSelItem, 0, crOldText, crOldBackground);

		showStudentInfo(m_nCurrentSelItem);
	}
}

void CMissingPaperDlg::showStudentInfo(int nItem)
{
	ST_STUDENT* pStudent = (ST_STUDENT*)m_lcMissingZkzh.GetItemData(nItem);

	USES_CONVERSION;
	m_strKD = A2T(pStudent->strSchool.c_str());
	m_strKC = A2T(pStudent->strClassroom.c_str());
	m_strZW = "无";
	UpdateData(FALSE);
}

BOOL CMissingPaperDlg::PreTranslateMessage(MSG* pMsg)
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

void CMissingPaperDlg::SetFontSize()
{
	CFont fontStatus;
	fontStatus.CreateFont(20, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("宋体"));
	GetDlgItem(IDC_STATIC_2)->SetFont(&fontStatus);
	GetDlgItem(IDC_STATIC_3)->SetFont(&fontStatus);
	GetDlgItem(IDC_STATIC_4)->SetFont(&fontStatus);

	CFont fontStatus2;
	fontStatus2.CreateFont(25, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("宋体"));
	GetDlgItem(IDC_STATIC_KD)->SetFont(&fontStatus2);
	GetDlgItem(IDC_STATIC_KC)->SetFont(&fontStatus2);
	GetDlgItem(IDC_STATIC_ZW)->SetFont(&fontStatus2);
}

void CMissingPaperDlg::OnBnClickedBtnClose()
{
	OnCancel();
}

HBRUSH CMissingPaperDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CTipBaseDlg::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_2 || CurID == IDC_STATIC_3 || CurID == IDC_STATIC_4)
	{
		pDC->SetTextColor(RGB(100, 100, 100));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	if (CurID == IDC_STATIC_KD || CurID == IDC_STATIC_KC || CurID == IDC_STATIC_ZW)
	{
		pDC->SetTextColor(RGB(250, 50, 20));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	return hbr;
}


void CMissingPaperDlg::OnNMHoverListMissingpaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;
}

void CMissingPaperDlg::OnNMDblclkListMissingpaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem < 0)
		return;

	if (m_nCurrentSelItem < m_lcMissingZkzh.GetItemCount())
	{
		for (int i = 0; i < m_lcMissingZkzh.GetColumns(); i++)
			if (!m_lcMissingZkzh.GetModified(m_nCurrentSelItem, i))
				m_lcMissingZkzh.SetItemColors(m_nCurrentSelItem, i, crOldText, crOldBackground);
			else
				m_lcMissingZkzh.SetItemColors(m_nCurrentSelItem, i, RGB(255, 0, 0), crOldBackground);
	}
	m_nCurrentSelItem = pNMItemActivate->iItem;
	showStudentInfo(m_nCurrentSelItem);

	*pResult = 0;
}

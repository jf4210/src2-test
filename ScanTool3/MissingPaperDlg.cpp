// MissingPaperDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "MissingPaperDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"
#include "ChkIdentityDlg.h"

// CMissingPaperDlg 对话框

IMPLEMENT_DYNAMIC(CMissingPaperDlg, CDialog)

CMissingPaperDlg::CMissingPaperDlg(pPAPERSINFO pPapers, pMODEL pModel, CStudentMgr* pMgr, CWnd* pParent /*=NULL*/)
	: CTipBaseDlg(IDD_MISSINGPAPERDLG, pParent)
	, m_strKD(_T("")), m_strKC(_T("")), m_strZW(_T("")), m_pStudentMgr(pMgr), m_pPapers(pPapers), m_pModel(pModel), m_nCurrentSelItem(0)
	, m_strKcScaned(_T("当前已扫考场试卷:"))
{

}

CMissingPaperDlg::~CMissingPaperDlg()
{
}

void CMissingPaperDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MissingPaper, m_lcMissingZkzh);
	DDX_Control(pDX, IDC_LIST_MissingPaper_Scaned, m_lcMissingZkzh_Scaned);
	DDX_Control(pDX, IDC_LIST_KC, m_lcKC);
	DDX_Control(pDX, IDC_LIST_Scaned, m_lcScaned);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_bmpBtnClose);
	DDX_Control(pDX, IDC_BTN_Submit, m_bmpBtnSubmit);
	DDX_Text(pDX, IDC_STATIC_KD, m_strKD);
	DDX_Text(pDX, IDC_STATIC_KC, m_strKC);
	DDX_Text(pDX, IDC_STATIC_ZW, m_strZW);
	DDX_Text(pDX, IDC_STATIC_8, m_strKcScaned);
}


BEGIN_MESSAGE_MAP(CMissingPaperDlg, CTipBaseDlg)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CMissingPaperDlg::OnBnClickedBtnClose)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_HOVER, IDC_LIST_MissingPaper, &CMissingPaperDlg::OnNMHoverListMissingpaper)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MissingPaper, &CMissingPaperDlg::OnNMDblclkListMissingpaper)
	ON_BN_CLICKED(IDC_BTN_Submit, &CMissingPaperDlg::OnBnClickedBtnSubmit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_KC, &CMissingPaperDlg::OnNMDblclkListKc)
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
	m_lcMissingZkzh.InsertColumn(0, _T("No."), LVCFMT_CENTER, 36);
	m_lcMissingZkzh.InsertColumn(1, _T("准考证号"), LVCFMT_CENTER, 110);
	m_lcMissingZkzh.InsertColumn(2, _T("姓名"), LVCFMT_CENTER, 70);
	m_lcMissingZkzh.InsertColumn(3, _T("考场"), LVCFMT_CENTER, 100);

	m_lcMissingZkzh_Scaned.SetExtendedStyle(m_lcMissingZkzh_Scaned.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcMissingZkzh_Scaned.InsertColumn(0, _T("No."), LVCFMT_CENTER, 36);
	m_lcMissingZkzh_Scaned.InsertColumn(1, _T("准考证号"), LVCFMT_CENTER, 110);
	m_lcMissingZkzh_Scaned.InsertColumn(2, _T("姓名"), LVCFMT_CENTER, 70);
	m_lcMissingZkzh_Scaned.InsertColumn(3, _T("考场"), LVCFMT_CENTER, 100);

	m_lcKC.SetExtendedStyle(m_lcKC.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcKC.InsertColumn(0, _T("考场"), LVCFMT_CENTER, 100);
	m_lcKC.InsertColumn(1, _T("人数"), LVCFMT_CENTER, 36);
	m_lcKC.InsertColumn(2, _T("考点"), LVCFMT_CENTER, 195);

	m_lcScaned.SetExtendedStyle(m_lcScaned.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcScaned.InsertColumn(0, _T("题卡"), LVCFMT_CENTER, 40);
	m_lcScaned.InsertColumn(1, _T("准考证号"), LVCFMT_CENTER, 100);
	m_lcScaned.InsertColumn(2, _T("姓名"), LVCFMT_CENTER, 70);
	m_lcScaned.InsertColumn(3, _T("考场"), LVCFMT_CENTER, 100);

	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);
	m_bmpBtnSubmit.SetStateBitmap(IDB_RecordDlg_Btn, 0, IDB_RecordDlg_Btn_Hover);

	InitCtrlPosition();
}

void CMissingPaperDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 60;	//上边的间隔，留给控制栏
	const int nLeftGap = 20;	//左边的空白间隔
	const int nBottomGap = 80;	//下边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nStaticTip = 15;		//列表提示static控件高度
	int nRealW = cx - nLeftGap - nRightGap;
	int nListW = nRealW * 0.5;

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	int nTmpTop = nCurrentTop;


	if (GetDlgItem(IDC_STATIC_6)->GetSafeHwnd())	//标题
	{
		GetDlgItem(IDC_STATIC_6)->MoveWindow(nLeftGap, nGap * 5, nRealW, nTopGap - nGap * 8);
	}

	if (GetDlgItem(IDC_STATIC_1)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_1)->MoveWindow(nCurrentLeft, nCurrentTop, nListW, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (m_lcMissingZkzh.GetSafeHwnd())
	{
		int nListH = (cy - nCurrentTop - nBottomGap) * 0.65;
		m_lcMissingZkzh.MoveWindow(nCurrentLeft, nCurrentTop, nListW, nListH);
		nCurrentTop += (nListH + nGap * 2);
	}
	if (GetDlgItem(IDC_STATIC_7)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_7)->MoveWindow(nCurrentLeft, nCurrentTop, nListW, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (m_lcMissingZkzh_Scaned.GetSafeHwnd())
	{
		int nListH = cy - nCurrentTop - nBottomGap;
		m_lcMissingZkzh_Scaned.MoveWindow(nCurrentLeft, nCurrentTop, nListW, nListH);
		nCurrentTop += (nListH + nGap);
	}

	nCurrentTop = nTmpTop;
	nCurrentLeft = nLeftGap + nListW + nGap * 5;
	int nStaticW = 45;
	int nStaticH = 50;
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
	nCurrentLeft += (nStaticW + nGap);
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

#if 1
	int nStatusInfoH = 160;
	nCurrentTop = nTopGap + nStatusInfoH;
	nCurrentLeft = nLeftGap + nListW + nGap * 5;
	nStaticW = cx - nCurrentLeft - nRightGap;
	nStaticH = 35;
	if (GetDlgItem(IDC_STATIC_5)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_5)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	int nKCListH = 100;
	if (m_lcKC.GetSafeHwnd())
	{
		m_lcKC.MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nKCListH);
		nCurrentTop += (nKCListH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_8)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_8)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	if (m_lcScaned.GetSafeHwnd())
	{
		int nH = cy - nCurrentTop - nBottomGap;
		m_lcScaned.MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nH);
		nCurrentTop += (nStaticH + nKCListH);
	}
#else
	int nKCListH = 140;
	nTmpTop = nCurrentTop;
	nKCListH = (cy - nBottomGap - nTmpTop) > nKCListH ? nKCListH : (cy - nBottomGap - nTmpTop);
	nCurrentTop = cy - nBottomGap - nKCListH;
	nCurrentLeft = nLeftGap + nListW + nGap * 5;
	nStaticW = cx - nCurrentLeft - nRightGap;
	nStaticH = 35;
	if (GetDlgItem(IDC_STATIC_5)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_5)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	if (m_lcKC.GetSafeHwnd())
	{
		int nH = cy - nCurrentTop - nBottomGap;
		m_lcKC.MoveWindow(nCurrentLeft, nCurrentTop, nStaticW, nH);
	}
#endif
	int nBtnW = 150;
	int nBtnH = nBottomGap / 2;
	nCurrentLeft = (cx - nBtnW) / 2;
	nCurrentTop = cy - nBtnH - nBottomGap / 4;
	if (m_bmpBtnSubmit.GetSafeHwnd())
	{
		m_bmpBtnSubmit.MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
	}
}

void CMissingPaperDlg::InitData()
{
	if (!m_pPapers || !m_pModel || !m_pStudentMgr) return;

	std::stringstream ss;

	USES_CONVERSION;
	std::string strKC;	//utf8
	std::string strTable = Poco::format("T%d_%d", _pModel_->nExamID, _pModel_->nSubjectID);
	std::set<std::string> setKC;	//考场集合，此集合保证数据无重复
	for (auto pPaperItem : m_pPapers->lPaper)
	{
		if (m_pStudentMgr->GetKCFromZkzh(strTable, pPaperItem->strSN, strKC))
			setKC.insert(strKC);	//utf8
	}
	ss << "查询当前试卷袋在报名库中的考场集合数为" << setKC.size() << "个\n";

	m_lAllStudent.clear();
	for (auto itKcCode : setKC)
	{
		STUDENT_LIST lResult;
		std::string strTable = Poco::format("T%d_%d", m_pModel->nExamID, m_pModel->nSubjectID);
		if (m_pStudentMgr && m_pStudentMgr->GetKCStudent(strTable, CMyCodeConvert::Utf8ToGb2312(itKcCode), lResult))
		{
			for (auto itStudent : lResult)
				m_lAllStudent.push_back(itStudent);
			ss << "考场" << CMyCodeConvert::Utf8ToGb2312(itKcCode) << ", 学生" << lResult.size() << "个\n";
		}
		else
			ss << "查询报名库" << strTable << "的考场学生失败，考场:" << CMyCodeConvert::Utf8ToGb2312(itKcCode) << ".\n";

		int nCount = m_lcKC.GetItemCount();
		m_lcKC.InsertItem(nCount, NULL);

		m_lcKC.SetItemText(nCount, 0, (LPCTSTR)A2T(CMyCodeConvert::Utf8ToGb2312(itKcCode).c_str()));	//pPaper->strStudentInfo.c_str()
		std::string strKD;
		if (m_pStudentMgr->GetKDFromKC(strTable, CMyCodeConvert::Utf8ToGb2312(itKcCode), strKD))
		{
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", lResult.size());
			m_lcKC.SetItemText(nCount, 1, (LPCTSTR)A2T(szCount));
			m_lcKC.SetItemText(nCount, 2, (LPCTSTR)A2T(CMyCodeConvert::Utf8ToGb2312(strKD).c_str()));
		}
	}
	ss << "查询到的总学生" << m_lAllStudent.size() << "个\n";

	m_lcMissingZkzh.DeleteAllItems();
	STUDENT_LIST::iterator itStudent = m_lAllStudent.begin();
	for (; itStudent != m_lAllStudent.end(); itStudent++)
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
			if (itStudent->nScaned == 0)
			{
				int nCount = m_lcMissingZkzh.GetItemCount();
				char szCount[10] = { 0 };
				sprintf_s(szCount, "%d", nCount + 1);
				m_lcMissingZkzh.InsertItem(nCount, NULL);

				m_lcMissingZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));	//pPaper->strStudentInfo.c_str()
				m_lcMissingZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(itStudent->strZkzh.c_str()));
				m_lcMissingZkzh.SetItemText(nCount, 2, (LPCTSTR)A2T(itStudent->strName.c_str()));
				m_lcMissingZkzh.SetItemText(nCount, 3, (LPCTSTR)A2T(itStudent->strClassroom.c_str()));

				m_lcMissingZkzh.SetItemData(nCount, (DWORD_PTR)&(*itStudent));
			}
			else
			{
				int nCount = m_lcMissingZkzh_Scaned.GetItemCount();
				char szCount[10] = { 0 };
				sprintf_s(szCount, "%d", nCount + 1);
				m_lcMissingZkzh_Scaned.InsertItem(nCount, NULL);

				m_lcMissingZkzh_Scaned.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));	//pPaper->strStudentInfo.c_str()
				m_lcMissingZkzh_Scaned.SetItemText(nCount, 1, (LPCTSTR)A2T(itStudent->strZkzh.c_str()));
				m_lcMissingZkzh_Scaned.SetItemText(nCount, 2, (LPCTSTR)A2T(itStudent->strName.c_str()));
				m_lcMissingZkzh_Scaned.SetItemText(nCount, 3, (LPCTSTR)A2T(itStudent->strClassroom.c_str()));

				m_lcMissingZkzh_Scaned.SetItemData(nCount, (DWORD_PTR)&(*itStudent));
			}
		}
	}
	ss << "缺失准考证号列表--未扫" << m_lcMissingZkzh.GetItemCount() << "个\n";
	ss << "缺失准考证号列表--已扫" << m_lcMissingZkzh_Scaned.GetItemCount() << "个\n";
	g_pLogger->information(ss.str());

	CString strShow = _T("");
	strShow.Format(_T("缺失准考证号列表--未扫(%d人):"), m_lcMissingZkzh.GetItemCount());
	GetDlgItem(IDC_STATIC_1)->SetWindowTextW(strShow);
	strShow.Format(_T("缺失准考证号列表--已扫(%d人):"), m_lcMissingZkzh_Scaned.GetItemCount());
	GetDlgItem(IDC_STATIC_7)->SetWindowTextW(strShow);

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

	//解决Static控件背景透明时文本覆盖重影
	CRect rtKD, rtKC, rtZW;
	GetDlgItem(IDC_STATIC_KD)->GetWindowRect(&rtKD); 
	GetDlgItem(IDC_STATIC_KC)->GetWindowRect(&rtKC);
	GetDlgItem(IDC_STATIC_ZW)->GetWindowRect(&rtZW);
	ScreenToClient(&rtKD);
	ScreenToClient(&rtKC);
	ScreenToClient(&rtZW);
	InvalidateRect(&rtKD);
	InvalidateRect(&rtKC);
	InvalidateRect(&rtZW);
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
	fontStatus1.CreateFont(15, 0, 0, 0,
						   FW_BOLD, FALSE, FALSE, 0,
						   DEFAULT_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH | FF_SWISS,
						   _T("宋体"));
	GetDlgItem(IDC_STATIC_1)->SetFont(&fontStatus1);
	GetDlgItem(IDC_STATIC_5)->SetFont(&fontStatus1);
	GetDlgItem(IDC_STATIC_7)->SetFont(&fontStatus1);
	GetDlgItem(IDC_STATIC_8)->SetFont(&fontStatus1);

	fontStatus2.CreateFont(18, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("宋体"));
	GetDlgItem(IDC_STATIC_2)->SetFont(&fontStatus2);
	GetDlgItem(IDC_STATIC_3)->SetFont(&fontStatus2);
	GetDlgItem(IDC_STATIC_4)->SetFont(&fontStatus2);

	fontStatus3.CreateFont(25, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("宋体"));
	GetDlgItem(IDC_STATIC_KD)->SetFont(&fontStatus3);
	GetDlgItem(IDC_STATIC_KC)->SetFont(&fontStatus3);
	GetDlgItem(IDC_STATIC_ZW)->SetFont(&fontStatus3);

	fontStatus4.CreateFont(30, 0, 0, 0,
						   FW_BOLD, FALSE, FALSE, 0,
						   DEFAULT_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH | FF_SWISS,
						   _T("宋体"));
	GetDlgItem(IDC_STATIC_6)->SetFont(&fontStatus4);
	
	fontStatus5.CreateFont(20, 0, 0, 0,
						   FW_BOLD, FALSE, FALSE, 0,
						   DEFAULT_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH | FF_SWISS,
						   _T("幼圆"));
	m_bmpBtnSubmit.SetBtnFont(fontStatus5);
}

void CMissingPaperDlg::OnBnClickedBtnClose()
{
	OnCancel();
}

HBRUSH CMissingPaperDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CTipBaseDlg::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_6)
	{
		pDC->SetTextColor(RGB(20, 20, 20));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	if (CurID == IDC_STATIC_1 || CurID == IDC_STATIC_7)
	{
		pDC->SetTextColor(RGB(60, 60, 60));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
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

void CMissingPaperDlg::OnBnClickedBtnSubmit()
{
	std::string strMsg = Poco::format("此试卷袋含%d个考场，扫描数与考场数据不一样,是否强制上传?", m_lcKC.GetItemCount());
	CNewMessageBox	dlg;
	dlg.setShowInfo(2, 2, strMsg);
	dlg.DoModal();
	if (dlg.m_nResult != IDYES)
		return;

	CChkIdentityDlg dlg2;
	if (dlg2.DoModal() != IDOK)
		return;

	OnOK();
}

void CMissingPaperDlg::OnNMDblclkListKc(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	USES_CONVERSION;
	CString strKCVal = m_lcKC.GetItemText(pNMItemActivate->iItem, 0);

	CString strShowInfo = _T("");
	strShowInfo.Format(_T("考场[%s]的已扫描试卷如下:"), strKCVal);
// 	GetDlgItem(IDC_STATIC_8)->SetWindowTextW(strShowInfo);
	m_strKcScaned = strShowInfo;
	m_lcScaned.DeleteAllItems();
	for (auto pPaperItem : m_pPapers->lPaper)
	{
		for (auto pBmkItem : m_lAllStudent)
		{
			if (pPaperItem->strSN == pBmkItem.strZkzh)
			{
				if (T2A(strKCVal) == pBmkItem.strClassroom)
				{
					int nCount = m_lcScaned.GetItemCount();
					m_lcScaned.InsertItem(nCount, NULL);

					m_lcScaned.SetItemText(nCount, 0, (LPCTSTR)A2T(pPaperItem->strStudentInfo.c_str()));	//pPaper->strStudentInfo.c_str()
					m_lcScaned.SetItemText(nCount, 1, (LPCTSTR)A2T(pBmkItem.strZkzh.c_str()));
					m_lcScaned.SetItemText(nCount, 2, (LPCTSTR)A2T(pBmkItem.strName.c_str()));
					m_lcScaned.SetItemText(nCount, 3, (LPCTSTR)A2T(pBmkItem.strClassroom.c_str()));
				}
			}
		}
	}
	UpdateData(FALSE);
	Invalidate();
	*pResult = 0;
}

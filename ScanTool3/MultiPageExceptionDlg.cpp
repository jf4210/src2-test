// MultiPageExceptionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "MultiPageExceptionDlg.h"
#include "afxdialogex.h"
#include "MultiPageMgr.h"
#include "NewMessageBox.h"

// CMultiPageExceptionDlg 对话框

IMPLEMENT_DYNAMIC(CMultiPageExceptionDlg, CDialog)

CMultiPageExceptionDlg::CMultiPageExceptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_MULTIPAGEEXCEPTIONDLG, pParent)
, m_pModel(NULL), m_pPapers(NULL), m_nPicPagination(0), m_nCurrentPaperID(0), m_nCurrentPicID(0), m_strPicZKZH(_T("")), m_pCurrentShowPaper(NULL)
, m_pVagueSearchDlg(NULL), m_pShowPicDlg(NULL)
{

}

CMultiPageExceptionDlg::~CMultiPageExceptionDlg()
{
}

void CMultiPageExceptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Paper, m_lcIssuePaper);
	DDX_Control(pDX, IDC_LIST_Pics, m_lcIssuePics);
	DDX_Text(pDX, IDC_EDIT_ModelPageIndex, m_nPicPagination);
	DDX_Text(pDX, IDC_EDIT_BelongsZKZH, m_strPicZKZH);
	DDX_Control(pDX, IDC_BTN_Apply, m_bmpBtnApply);
}

BEGIN_MESSAGE_MAP(CMultiPageExceptionDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Paper, &CMultiPageExceptionDlg::OnNMDblclkListPaper)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Pics, &CMultiPageExceptionDlg::OnNMDblclkListPics)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Paper, &CMultiPageExceptionDlg::OnNMHoverListPaper)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Pics, &CMultiPageExceptionDlg::OnNMHoverListPics)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_Apply, &CMultiPageExceptionDlg::OnBnClickedBtnApply)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_Paper, &CMultiPageExceptionDlg::OnNMRClickListPaper)
	ON_COMMAND(ID_MultiPage_Del_ExceptionPaper, &CMultiPageExceptionDlg::OnMultipageDelExceptionpaper)
END_MESSAGE_MAP()

// CMultiPageExceptionDlg 消息处理程序
BOOL CMultiPageExceptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	InitData();

	return TRUE;
}

void CMultiPageExceptionDlg::InitUI()
{
	m_lcIssuePaper.SetExtendedStyle(m_lcIssuePaper.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcIssuePaper.InsertColumn(0, _T("考生"), LVCFMT_CENTER, 50);
	m_lcIssuePaper.InsertColumn(1, _T("准考证号"), LVCFMT_CENTER, 110);
	m_lcIssuePaper.InsertColumn(2, _T("备注"), LVCFMT_LEFT, 150);

	HDITEM hditem;
	for (int i = 0; i < m_lcIssuePaper.m_HeaderCtrl.GetItemCount(); i++)
	{
		hditem.mask = HDI_IMAGE | HDI_FORMAT;
		m_lcIssuePaper.m_HeaderCtrl.GetItem(i, &hditem);
		hditem.fmt |= HDF_IMAGE;
		m_lcIssuePaper.m_HeaderCtrl.SetItem(i, &hditem);
	}
	m_lcIssuePaper.EnableToolTips(TRUE);

	m_lcIssuePics.SetExtendedStyle(m_lcIssuePics.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcIssuePics.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 50);
	m_lcIssuePics.InsertColumn(1, _T("图片名"), LVCFMT_CENTER, 110);

	HDITEM hditem2;
	for (int i = 0; i < m_lcIssuePics.m_HeaderCtrl.GetItemCount(); i++)
	{
		hditem2.mask = HDI_IMAGE | HDI_FORMAT;
		m_lcIssuePics.m_HeaderCtrl.GetItem(i, &hditem2);
		hditem2.fmt |= HDF_IMAGE;
		m_lcIssuePics.m_HeaderCtrl.SetItem(i, &hditem2);
	}
	m_lcIssuePics.EnableToolTips(TRUE);

	m_bmpBtnApply.SetStateBitmap(IDB_ScanMgr_BtnSave, 0, IDB_ScanMgr_BtnSave_Hover);
	m_bmpBtnApply.SetWindowText(_T("应用"));
	m_bmpBtnApply.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(116, 116, 116));

	InitCtrlPosition();
}

void CMultiPageExceptionDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 2;	//上边的间隔，留给控制栏
	const int nLeftGap = 2;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nStaticTip = 17;		//列表提示static控件高度
	int nListCtrlWidth = cx - nLeftGap - nRightGap;	//图片列表控件宽度
	int nListH = (cy - nTopGap - nBottomGap - nStaticTip * 2 - nGap * 3) * 0.5;
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	if (GetDlgItem(IDC_STATIC_Paper)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Paper)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (m_lcIssuePaper.GetSafeHwnd())
	{
		m_lcIssuePaper.MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nListH);
		nCurrentTop += (nListH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_Pics)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Pics)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (m_lcIssuePics.GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		int nH = cy - nCurrentTop - nBottomGap;
		m_lcIssuePics.MoveWindow(nCurrentLeft, nCurrentTop, nW, nH);
		nCurrentLeft += (nW + nGap);
	}
	int nTmpLeft = nCurrentLeft;
	int nNewStaticTipH = 20;
	if (GetDlgItem(IDC_STATIC_BelongsZKZH)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		nW = (nW - nGap) * 0.4;
		GetDlgItem(IDC_STATIC_BelongsZKZH)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nNewStaticTipH);
		nCurrentLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_EDIT_BelongsZKZH)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		nW = (nW - nGap) * 0.6;
		GetDlgItem(IDC_EDIT_BelongsZKZH)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nNewStaticTipH);
		nCurrentLeft = nTmpLeft;
		nCurrentTop += (nNewStaticTipH + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ModelPageIndex)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		nW = (nW - nGap * 2) * 0.4;
		GetDlgItem(IDC_STATIC_ModelPageIndex)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nNewStaticTipH);
		nCurrentLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_EDIT_ModelPageIndex)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		nW = (nW - nGap * 2) * 0.4;
		GetDlgItem(IDC_EDIT_ModelPageIndex)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nNewStaticTipH);
		nCurrentLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ModelPageIndex2)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		nW = (nW - nGap * 2) * 0.2;
		GetDlgItem(IDC_STATIC_ModelPageIndex2)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nNewStaticTipH);
		nCurrentTop += (nNewStaticTipH + nGap * 2);
		nCurrentLeft = nTmpLeft;
	}
	int nBtnH = 30;
	if (GetDlgItem(IDC_BTN_Apply)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		int nBtnW = nW * 0.5;
		GetDlgItem(IDC_BTN_Apply)->MoveWindow(nCurrentLeft + nW * 0.25, nCurrentTop, nBtnW, nBtnH);
		nCurrentTop += (nBtnH + nGap);
	}
}

void CMultiPageExceptionDlg::ShowPaperDetail(pST_PaperInfo pPaper)
{
	if (!pPaper) return;

	//m_lcIssuePaper.GetItemColors(m_nCurrentPaperID, 0, crPaperOldText, crPaperOldBackground);
	for (int i = 0; i < m_lcIssuePaper.GetColumns(); i++)							//设置高亮显示(手动设置背景颜色)
		m_lcIssuePaper.SetItemColors(m_nCurrentPaperID, i, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255

	USES_CONVERSION;
	m_lcIssuePics.DeleteAllItems();
	for (auto pPic : pPaper->lPic)
	{
		int nCount = m_lcIssuePics.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);	//nCount + 1
		m_lcIssuePics.InsertItem(nCount, NULL);
		m_lcIssuePics.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));	//pPaper->strStudentInfo.c_str()
		std::string strPicName = pPic->strPicName;
		strPicName = strPicName.substr(0, strPicName.find('.'));
		m_lcIssuePics.SetItemText(nCount, 1, (LPCTSTR)A2T(strPicName.c_str()));
		m_lcIssuePics.SetItemData(nCount, (DWORD_PTR)pPic);
	}
	m_nCurrentPicID = 0;
	m_lcIssuePics.GetItemColors(m_nCurrentPicID, 0, crPicOldText, crPicOldBackground);

	pST_PicInfo pPic = (pST_PicInfo)m_lcIssuePics.GetItemData(m_nCurrentPicID);
	ShowPicDetail(pPic);
	if(m_pShowPicDlg) ShowPaperByItem(m_nCurrentPaperID);
	if (m_pVagueSearchDlg) m_pVagueSearchDlg->setNotifyDlg(NULL);	//多页异常处理窗口不进行模糊搜索查询设置
}

void CMultiPageExceptionDlg::ShowPicDetail(pST_PicInfo pPic, bool bShowPic /*= false*/)
{
	if (!pPic) return;

	//m_lcIssuePics.GetItemColors(m_nCurrentPicID, 0, crPicOldText, crPicOldBackground);
	for (int i = 0; i < m_lcIssuePics.GetColumns(); i++)							//设置高亮显示(手动设置背景颜色)
		m_lcIssuePics.SetItemColors(m_nCurrentPicID, i, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255

	m_strPicZKZH = (static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.c_str();
	m_nPicPagination = pPic->nPicModelIndex + 1;

	if(m_pShowPicDlg && bShowPic) m_pShowPicDlg->setShowPaper(m_pCurrentShowPaper, m_nCurrentPicID);

 	UpdateData(FALSE);
 	m_lcIssuePics.Invalidate();
}

void CMultiPageExceptionDlg::SetPicInfo(pST_PicInfo pPic)
{
	UpdateData(TRUE);
	
	//如果是未识别到页码的，先根据填写的页码标识进行重识别
	if ((static_cast<pST_PaperInfo>(pPic->pPaper))->nPaginationStatus == 0)
	{
		pRECOGTASK pTask = new RECOGTASK;
		pTask->pPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
		g_lRecogTask.push_back(pTask);



	}

	USES_CONVERSION;
	std::string strCurrZkzh = T2A(m_strPicZKZH);
	CMultiPageMgr multiPageObj(_pModel_);
	if(multiPageObj.ModifyPic(pPic, m_pPapers, m_nPicPagination, strCurrZkzh))
		ReInitData(m_pModel, m_pPapers);
	else
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "设置失败，可能参数非法！");
		dlg.DoModal();
	}
}

void CMultiPageExceptionDlg::ShowPaperByItem(int nItem)
{
	if (nItem < 0)
		return;
	if (nItem >= m_lcIssuePaper.GetItemCount())
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcIssuePaper.GetItemData(nItem);

	m_pCurrentShowPaper = pPaper;
	//m_lcIssuePaper.GetItemColors(nItem, 0, crPaperOldText, crPaperOldBackground);
	for (int i = 0; i < m_lcIssuePaper.GetColumns(); i++)							//设置高亮显示(手动设置背景颜色)
		m_lcIssuePaper.SetItemColors(nItem, i, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255

	m_pShowPicDlg->ReInitUI(pPaper);
	m_pShowPicDlg->setShowPaper(pPaper);

	UpdateData(FALSE);
	m_lcIssuePaper.Invalidate();
}

BOOL CMultiPageExceptionDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)	//pMsg->wParam == VK_ESCAPE
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CMultiPageExceptionDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

HBRUSH CMultiPageExceptionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_Paper || CurID == IDC_STATIC_Pics || CurID == IDC_STATIC_ModelPageIndex || CurID == IDC_STATIC_ModelPageIndex ||\
		CurID == IDC_STATIC_ModelPageIndex2 || CurID == IDC_STATIC_BelongsZKZH)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

void CMultiPageExceptionDlg::SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg)
{
	m_pShowPicDlg = pShowDlg;
	m_pVagueSearchDlg = pSearchDlg;
}

void CMultiPageExceptionDlg::ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo)
{
	m_pModel = pModel;
	m_pPapers = pPapersInfo;

	m_nPicPagination = 0;
	m_strPicZKZH = "";
	m_nCurrentPaperID = 0;
	m_nCurrentPicID = 0;

	InitData();
	UpdateData(FALSE);
}

void CMultiPageExceptionDlg::InitData()
{
	if (!m_pPapers) return;

	m_lcIssuePics.DeleteAllItems();
	m_lcIssuePaper.DeleteAllItems();

	USES_CONVERSION;
	for (auto pPaper : m_pPapers->lPaper)
	{
		if (pPaper->nPaginationStatus != 2 && !pPaper->bReScan)
		{
			//添加进试卷列表控件
			int nCount = m_lcIssuePaper.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", pPaper->nIndex);	//nCount + 1
			m_lcIssuePaper.InsertItem(nCount, NULL);
			m_lcIssuePaper.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
			m_lcIssuePaper.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
			//显示备注信息，为何出现在此列表
			std::string strDetailInfo;
			switch (pPaper->nPaginationStatus)
			{
				case 0: strDetailInfo = "页码未识别，请设置页码"; break;
				case 1: strDetailInfo = "考号未识别，请设置考号"; break;
				case 3: strDetailInfo = "图片数与模板不一致"; break;
				case 4: strDetailInfo = "页码有重复"; break;
				default: strDetailInfo = "未知错误";
			}
			m_lcIssuePaper.SetItemText(nCount, 2, (LPCTSTR)A2T(strDetailInfo.c_str()));
			m_lcIssuePaper.SetItemData(nCount, (DWORD_PTR)pPaper);

			CString strTips = _T("双击显示此试卷信息");
			m_lcIssuePaper.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
			m_lcIssuePaper.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
			m_lcIssuePaper.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
		}
	}
	if (m_lcIssuePaper.GetItemCount() > 0)
	{
		m_nCurrentPaperID = 0;
		m_lcIssuePaper.GetItemColors(m_nCurrentPaperID, 0, crPaperOldText, crPaperOldBackground);

		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcIssuePaper.GetItemData(m_nCurrentPaperID);
		ShowPaperDetail(pPaper);
	}
}

void CMultiPageExceptionDlg::OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem < 0)
		return;

	if (m_nCurrentPaperID < m_lcIssuePaper.GetItemCount())
	{
		for (int i = 0; i < m_lcIssuePaper.GetColumns(); i++)
			if (!m_lcIssuePaper.GetModified(m_nCurrentPaperID, i))
				m_lcIssuePaper.SetItemColors(m_nCurrentPaperID, i, crPaperOldText, crPaperOldBackground);
			else
				m_lcIssuePaper.SetItemColors(m_nCurrentPaperID, i, RGB(255, 0, 0), crPaperOldBackground);
	}
	m_nCurrentPaperID = pNMItemActivate->iItem;
	
	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcIssuePaper.GetItemData(m_nCurrentPaperID);
	ShowPaperDetail(pPaper);
	*pResult = 0;
}

void CMultiPageExceptionDlg::OnNMDblclkListPics(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	if (m_nCurrentPicID < m_lcIssuePics.GetItemCount())
	{
		for (int i = 0; i < m_lcIssuePics.GetColumns(); i++)
			if (!m_lcIssuePics.GetModified(m_nCurrentPicID, i))
				m_lcIssuePics.SetItemColors(m_nCurrentPicID, i, crPicOldText, crPicOldBackground);
			else
				m_lcIssuePics.SetItemColors(m_nCurrentPicID, i, RGB(255, 0, 0), crPicOldBackground);
	}
	m_nCurrentPicID = pNMItemActivate->iItem;
 
	pST_PicInfo pPic = (pST_PicInfo)m_lcIssuePics.GetItemData(m_nCurrentPicID);
	ShowPicDetail(pPic, true);

	*pResult = 0;
}

void CMultiPageExceptionDlg::OnNMHoverListPaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;
}

void CMultiPageExceptionDlg::OnNMHoverListPics(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;
}

void CMultiPageExceptionDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CMultiPageExceptionDlg::OnBnClickedBtnApply()
{
	pST_PicInfo pPic = (pST_PicInfo)m_lcIssuePics.GetItemData(m_nCurrentPicID);
	SetPicInfo(pPic);
}


void CMultiPageExceptionDlg::OnNMRClickListPaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (m_lcIssuePaper.GetSelectedCount() <= 0) return;

	m_lcIssuePaper.SetItemState(m_nCurrentPaperID, 0, LVIS_DROPHILITED);		// 取消高亮显示
	m_nCurrentPaperID = pNMItemActivate->iItem;
	m_lcIssuePaper.SetItemState(m_nCurrentPaperID, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行，失去焦点后也一直显示
	
	//下面的这段代码, 不单单适应于ListCtrl  
	CMenu menu, *pPopup;
	menu.LoadMenu(IDR_MENU_MultiPageException);
	pPopup = menu.GetSubMenu(0);
	CPoint myPoint;
	ClientToScreen(&myPoint);
	GetCursorPos(&myPoint); //鼠标位置  
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);//GetParent()
}


void CMultiPageExceptionDlg::OnMultipageDelExceptionpaper()
{
	CNewMessageBox	dlg;
	dlg.setShowInfo(2, 2, "是否确定删除此考生试卷？");
	dlg.DoModal();
	if (dlg.m_nResult != IDYES)
		return ;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcIssuePaper.GetItemData(m_nCurrentPaperID);
	//pPaper->nPagination_Del_Paper = 1;
	pPaper->bReScan = true;

	PAPER_LIST::iterator itPaper = m_pPapers->lPaper.begin();
	for (; itPaper != m_pPapers->lPaper.end();)
	{
		pST_PaperInfo pItemPaper = *itPaper;
		if (pPaper == pItemPaper)
		{
			itPaper = m_pPapers->lPaper.erase(itPaper);
			m_pPapers->lIssue.push_back(pPaper);
			break;
		}
		itPaper++;
	}

	ReInitData(m_pModel, m_pPapers);
}

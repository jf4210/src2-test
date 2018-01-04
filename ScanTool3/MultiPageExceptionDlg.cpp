// MultiPageExceptionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "MultiPageExceptionDlg.h"
#include "afxdialogex.h"
#include "MultiPageMgr.h"

// CMultiPageExceptionDlg 对话框

IMPLEMENT_DYNAMIC(CMultiPageExceptionDlg, CDialog)

CMultiPageExceptionDlg::CMultiPageExceptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_MULTIPAGEEXCEPTIONDLG, pParent)
, m_pModel(NULL), m_pPapers(NULL), m_nPicPagination(0), m_nCurrentPaperID(0), m_nCurrentPicID(0), m_strPicZKZH(_T(""))
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
}

BEGIN_MESSAGE_MAP(CMultiPageExceptionDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Paper, &CMultiPageExceptionDlg::OnNMDblclkListPaper)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Pics, &CMultiPageExceptionDlg::OnNMDblclkListPics)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Paper, &CMultiPageExceptionDlg::OnNMHoverListPaper)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Pics, &CMultiPageExceptionDlg::OnNMHoverListPics)
END_MESSAGE_MAP()

// CMultiPageExceptionDlg 消息处理程序
BOOL CMultiPageExceptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();

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

	int nStaticTip = 15;		//列表提示static控件高度
	int nListCtrlWidth = cx - nLeftGap - nRightGap;	//图片列表控件宽度
	int nListH = (cy - nTopGap - nBottomGap - nStaticTip * 2 - nGap * 3) * 0.7;
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
	if (GetDlgItem(IDC_STATIC_BelongsZKZH)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		GetDlgItem(IDC_STATIC_BelongsZKZH)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nStaticTip);
		nCurrentLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_EDIT_BelongsZKZH)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		GetDlgItem(IDC_EDIT_BelongsZKZH)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nStaticTip);
		nCurrentLeft = nTmpLeft;
		nCurrentTop += (nStaticTip + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ModelPageIndex)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		nW = (nW - nGap * 2) * 0.6;
		GetDlgItem(IDC_STATIC_ModelPageIndex)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nStaticTip);
		nCurrentLeft = (nW + nGap);
	}
	if (GetDlgItem(IDC_EDIT_ModelPageIndex)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		nW = (nW - nGap * 2) * 0.3;
		GetDlgItem(IDC_EDIT_ModelPageIndex)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nStaticTip);
		nCurrentLeft = (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ModelPageIndex2)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap - nGap) * 0.5;
		nW = (nW - nGap * 2) * 0.1;
		GetDlgItem(IDC_STATIC_ModelPageIndex2)->MoveWindow(nCurrentLeft, nCurrentTop, nW, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
}

void CMultiPageExceptionDlg::ShowPaperDetail(pST_PaperInfo pPaper)
{
	if (!pPaper) return;

	m_lcIssuePaper.GetItemColors(m_nCurrentPaperID, 0, crPaperOldText, crPaperOldBackground);
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
	pST_PicInfo pPic = (pST_PicInfo)m_lcIssuePics.GetItemData(m_nCurrentPicID);
	ShowPicDetail(pPic);
}

void CMultiPageExceptionDlg::ShowPicDetail(pST_PicInfo pPic)
{
	if (!pPic) return;

	m_lcIssuePics.GetItemColors(m_nCurrentPicID, 0, crPicOldText, crPicOldBackground);
	for (int i = 0; i < m_lcIssuePics.GetColumns(); i++)							//设置高亮显示(手动设置背景颜色)
		m_lcIssuePics.SetItemColors(m_nCurrentPicID, i, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255

	m_strPicZKZH = (static_cast<pST_PaperInfo>(pPic->pPaper))->strSN.c_str();
	m_nPicPagination = pPic->nPicModelIndex + 1;
	UpdateData(FALSE);
}

void CMultiPageExceptionDlg::SetPicInfo(pST_PicInfo pPic)
{
	UpdateData(TRUE);
	//pPic->nPicModelIndex = m_nPicPagination - 1;

	USES_CONVERSION;
	std::string strCurrZkzh = T2A(m_strPicZKZH);
	if (pPic->nPicModelIndex != m_nPicPagination - 1)
	{
		CMultiPageMgr multiPageObj;
		multiPageObj.ModifyPicPagination(pPic, m_nPicPagination);

		//修改选择题和选择题的页码ID信息

 		if (strCurrZkzh == static_cast<pST_PaperInfo>(pPic->pPaper)->strSN)
 		{
			static_cast<pST_PaperInfo>(pPic->pPaper)->lPic.sort([](pST_PicInfo pPic1, pST_PicInfo pPic2)
			{return pPic1->nPicModelIndex < pPic2->nPicModelIndex; });
 		}
 		else
 		{
 			//考号与试卷不一致，重新划分到对应的试卷中去
 			if (static_cast<pST_SCAN_PAPER>(pPic->pSrcScanPic->pParentScanPaper)->bDoubleScan)
 			{
 				pST_PaperInfo pCurrentPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
 				if (pCurrentPaper->lPic.size() <= 2)		//如果图片只有2张，又是双面扫描，则不需要重新构建试卷
 				{
 					bool bFind = false;		//是否在现有试卷袋中发现相同准考证号
 					PAPER_LIST::iterator itPaper = m_pPapers->lPaper.begin();
 					for (; itPaper != m_pPapers->lPaper.end(); )
 					{
 						pST_PaperInfo pNewPaper = *itPaper;
 						if (pNewPaper == pCurrentPaper)
 						{
 							itPaper = m_pPapers->lPaper.erase(itPaper);
 							continue;
 						}
 						if (strCurrZkzh == pNewPaper->strSN)
 						{
 							bFind = true;

 							//试卷合并
 							if (!pNewPaper->bIssuePaper)	pNewPaper->bIssuePaper = pCurrentPaper->bIssuePaper;
 							if (!pNewPaper->bModifyZKZH)	pNewPaper->bModifyZKZH = pCurrentPaper->bModifyZKZH;
 							if (!pNewPaper->bReScan)		pNewPaper->bReScan = pCurrentPaper->bReScan;
 							if (!pNewPaper->bRecogCourse)	pNewPaper->bRecogCourse = pCurrentPaper->bRecogCourse;
 							pNewPaper->nPicsExchange += pCurrentPaper->nPicsExchange;
 							pNewPaper->nPaginationStatus = 2;
 							if (pNewPaper->nQKFlag == 0)	pNewPaper->nQKFlag = pCurrentPaper->nQKFlag;
 							if (pNewPaper->nWJFlag == 0)	pNewPaper->nWJFlag = pCurrentPaper->nWJFlag;
 							pNewPaper->nZkzhInBmkStatus = pCurrentPaper->nZkzhInBmkStatus;
 							//pPaper->strRecogSN4Search
 							//pPaper->lSnResult
 
							SCAN_PAPER_LIST::iterator itSrcScanPaper = pCurrentPaper->lSrcScanPaper.begin();
 							for (; itSrcScanPaper != pCurrentPaper->lSrcScanPaper.end();)
 							{
								pST_SCAN_PAPER pScanPaper = *itSrcScanPaper;
 								pNewPaper->lSrcScanPaper.push_back(pScanPaper);
 								itSrcScanPaper = pCurrentPaper->lSrcScanPaper.erase(itSrcScanPaper);
 							}
 							for (auto omrObj : pCurrentPaper->lOmrResult)
 								pNewPaper->lOmrResult.push_back(omrObj);
 							for (auto electOmrObj : pCurrentPaper->lElectOmrResult)
 								pNewPaper->lElectOmrResult.push_back(electOmrObj);
 
 							PIC_LIST::iterator itPic = pCurrentPaper->lPic.begin();
 							for (; itPic != pCurrentPaper->lPic.end(); )
 							{
 								pST_PicInfo pPic = *itPic;
 								itPic = pCurrentPaper->lPic.erase(itPic);
 
 								pPic->pPaper = pNewPaper;
 								bool bInsert = false;
 								PIC_LIST::iterator itNewPic = pNewPaper->lPic.begin();
 								for (; itNewPic != pNewPaper->lPic.end(); itNewPic++)
 								{
 									if ((*itNewPic)->nPicModelIndex > pPic->nPicModelIndex)
 									{
 										bInsert = true;
 										pNewPaper->lPic.insert(itNewPic, pPic);
 										break;
 									}
 								}
 								if (!bInsert)
 									pNewPaper->lPic.push_back(pPic);
 							}
 							if (pCurrentPaper->strSN.empty())
 								m_pPapers->nSnNull--;

							break;
 						} 
 						itPaper++;
 					}
					if(bFind) SAFE_RELEASE(pCurrentPaper);
 				}
 			}
 		}
	}
	else
	{
		if (strCurrZkzh != static_cast<pST_PaperInfo>(pPic->pPaper)->strSN)
		{

		}
	}

	
	//双面扫描时，设置当前页面所属试卷的另一页试卷的页码
	pST_PaperInfo pPaper = static_cast<pST_PaperInfo>(pPic->pPaper);
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{

	}
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

void CMultiPageExceptionDlg::InitData()
{
	if (!m_pPapers) return;

	m_lcIssuePics.DeleteAllItems();
	m_lcIssuePaper.DeleteAllItems();

	USES_CONVERSION;
	for (auto pPaper : m_pPapers->lPaper)
	{
		if (pPaper->nPaginationStatus != 2)
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

	m_nCurrentPaperID = 0;
	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcIssuePaper.GetItemData(m_nCurrentPaperID);
	ShowPaperDetail(pPaper);
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
	ShowPicDetail(pPic);

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

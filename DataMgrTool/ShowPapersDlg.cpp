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
	,m_pShowPicDlg(NULL), m_pAnswerShowDlg(NULL), m_pPapers(NULL), m_nCurrItemPaperList(-1), m_strMsg(_T("")), m_nPaperShowType(7)
{

}

CShowPapersDlg::~CShowPapersDlg()
{
}

void CShowPapersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Papers, m_listPaper);
	DDX_Control(pDX, IDC_EDIT_OmrRecogINfo, m_edit_OmrRecogInfo);
	DDX_Text(pDX, IDC_EDIT_OmrRecogINfo, m_strMsg);
	DDX_Control(pDX, IDC_CHK_Normal, m_btnChkNormal);
	DDX_Control(pDX, IDC_CHK_Doubt, m_btnChkDoubt);
	DDX_Control(pDX, IDC_CHK_Null, m_btnChkNull);
}


BEGIN_MESSAGE_MAP(CShowPapersDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_MESSAGE(WM_CV_RBTNUP, &CShowPapersDlg::RoiRBtnUp)
	ON_COMMAND(ID_LeftRotate, &CShowPapersDlg::LeftRotate)
	ON_COMMAND(ID_RightRotate, &CShowPapersDlg::RightRotate)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Papers, &CShowPapersDlg::OnNMDblclkListPapers)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_Papers, &CShowPapersDlg::OnLvnKeydownListPapers)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Papers, &CShowPapersDlg::OnNMHoverListPapers)
	ON_BN_CLICKED(IDC_CHK_Normal, &CShowPapersDlg::OnBnClickedChkNormal)
	ON_BN_CLICKED(IDC_CHK_Doubt, &CShowPapersDlg::OnBnClickedChkDoubt)
	ON_BN_CLICKED(IDC_CHK_Null, &CShowPapersDlg::OnBnClickedChkNull)
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

	if (!m_pAnswerShowDlg)
	{
		m_pAnswerShowDlg = new CAnswerShowDlg();
		m_pAnswerShowDlg->Create(IDD_ANSWERSHOWDLG, this);
		m_pAnswerShowDlg->ShowWindow(SW_SHOW);
	}

	setCheckStatus(m_nPaperShowType);

	CRect rc;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	int sx = rc.Width();
	int sy = rc.Height();
	if (sx > 1024)
		sx = 1024;
	if (sy > 768)
		sy = 768;
	MoveWindow(0, 0, sx, sy);
	CenterWindow();

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
	int nEditH = (cy - nTopGap - nBottomGap) * 0.27 - nGap;
	if (GetDlgItem(IDC_STATIC_ListTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ListTips)->MoveWindow(nCurrentLeft, nCurrentTop, nListW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	if (m_listPaper.GetSafeHwnd())
	{
		int nH = cy - nCurrentTop - nBottomGap - nStaticH - nGap - nEditH - nGap;
		m_listPaper.MoveWindow(nCurrentLeft, nCurrentTop, nListW, nH);
		nCurrentTop += (nH + nGap);
	}
	int nChkW = (nListW - 2 * nGap) / 3;
	if (GetDlgItem(IDC_CHK_Normal)->GetSafeHwnd())
	{
		GetDlgItem(IDC_CHK_Normal)->MoveWindow(nCurrentLeft, nCurrentTop, nChkW, nStaticH);
		nCurrentLeft += (nChkW + nGap);
	}
	if (GetDlgItem(IDC_CHK_Doubt)->GetSafeHwnd())
	{
		GetDlgItem(IDC_CHK_Doubt)->MoveWindow(nCurrentLeft, nCurrentTop, nChkW, nStaticH);
		nCurrentLeft += (nChkW + nGap);
	}
	if (GetDlgItem(IDC_CHK_Null)->GetSafeHwnd())
	{
		GetDlgItem(IDC_CHK_Null)->MoveWindow(nCurrentLeft, nCurrentTop, nChkW, nStaticH);
		nCurrentTop += (nStaticH + nGap);
	}
	nCurrentLeft = nLeftGap;
	if (m_edit_OmrRecogInfo.GetSafeHwnd())
	{
		m_edit_OmrRecogInfo.MoveWindow(nCurrentLeft, nCurrentTop, nListW, nEditH);
	}

	nCurrentLeft = nLeftWidth + nGap;
	nCurrentTop = nTopGap;
	int nPicShowTabCtrlWidth = cx - nCurrentLeft - nRightGap;
	int nPicShowTabCtrlHigh = (cy - nTopGap - nBottomGap) * 0.73;

	if (m_pShowPicDlg && m_pShowPicDlg->GetSafeHwnd())
	{
		m_pShowPicDlg->MoveWindow(nCurrentLeft, nCurrentTop, nPicShowTabCtrlWidth, nPicShowTabCtrlHigh);
		nCurrentTop += (nPicShowTabCtrlHigh + nGap);
	}
	if (m_pAnswerShowDlg && m_pAnswerShowDlg->GetSafeHwnd())
	{
		int nH = cy - nCurrentTop - nBottomGap;
		m_pAnswerShowDlg->MoveWindow(nCurrentLeft, nCurrentTop, nPicShowTabCtrlWidth, nH);
	}

	if (GetDlgItem(IDOK)->GetSafeHwnd())
	{
		int nBtnW = 80;
		int nBtnH = 35;
		GetDlgItem(IDOK)->MoveWindow(cx / 2 - nBtnW / 2, cy - nBottomGap + nGap, nBtnW, nBtnH);
	}
}

void CShowPapersDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (m_pShowPicDlg)
	{
		m_pShowPicDlg->DestroyWindow();
		SAFE_RELEASE(m_pShowPicDlg);
	}
	if (m_pAnswerShowDlg)
	{
		m_pAnswerShowDlg->DestroyWindow();
		SAFE_RELEASE(m_pAnswerShowDlg);
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
	if (m_listPaper.GetItemCount() > 0)
	{
		m_nCurrItemPaperList = 0;
		pST_PaperInfo pPaper = (pST_PaperInfo)m_listPaper.GetItemData(m_nCurrItemPaperList);
		m_pShowPicDlg->setShowPaper(pPaper);
		m_pAnswerShowDlg->InitData(pPaper);
	}

	m_listPaper.GetItemColors(m_nCurrItemPaperList, 0, crOldText, crOldBackground);
}

void CShowPapersDlg::ShowPaper(pST_PaperInfo pPaper)
{
	m_pShowPicDlg->setShowPaper(pPaper);
	m_pAnswerShowDlg->InitData(pPaper);
}

void CShowPapersDlg::ShowOmrTh(pST_PaperInfo pPaper, int nTh)
{
	m_pShowPicDlg->showPaperOmrTh(pPaper, nTh);

	//+++++++++++++++++++++++++++++++++++++++++
	//下面计算改题选中的阀值
	//+++++++++++++++++++++++++++++++++++++++++
	OMRRESULTLIST::iterator itOmr = pPaper->lOmrResult.begin();
	for (; itOmr != pPaper->lOmrResult.end(); itOmr++)
	{
		if (itOmr->nTH == nTh)
		{
			break;
		}
		
	}

	std::string strShowInfo = "\r\n============\r\n";
	if (itOmr != pPaper->lOmrResult.end())
	{
		RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
		for (; itRect != itOmr->lSelAnswer.end(); itRect++)
		{
			char szTmp[200] = { 0 };
			sprintf_s(szTmp, "%c,密度=%.3f(%.3f/%.3f), ", itRect->nAnswer + 65, \
					  itRect->fRealDensity / itRect->fStandardDensity, itRect->fRealDensity, itRect->fStandardDensity);
			strShowInfo.append(szTmp);
		}
		strShowInfo.append("\r\n");
		RECTLIST::iterator itRect2 = itOmr->lSelAnswer.begin();
		for (; itRect2 != itOmr->lSelAnswer.end(); itRect2++)
		{
			char szTmp[200] = { 0 };
			sprintf_s(szTmp, "%c,灰度=%.3f(%.3f-%.3f), ", itRect2->nAnswer + 65, \
					  itRect2->fRealMeanGray - itRect2->fStandardMeanGray, itRect2->fRealMeanGray, itRect2->fStandardMeanGray);
			strShowInfo.append(szTmp);
		}

		//------------------------------
		std::vector<pRECTINFO> vecItemsDensityDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemDensityDiff;
		RECTLIST::iterator itItem = itOmr->lSelAnswer.begin();
		for (; itItem != itOmr->lSelAnswer.end(); itItem++)
		{
			vecItemsDensityDesc.push_back(&(*itItem));
		}
		std::sort(vecItemsDensityDesc.begin(), vecItemsDensityDesc.end(), SortByItemDensity);

		for (int i = 0; i < vecItemsDensityDesc.size(); i++)
		{
			int j = i + 1;
			if (j < vecItemsDensityDesc.size())
			{
				ST_ITEM_DIFF stDiff;
				sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsDensityDesc[i]->nAnswer + 65), (char)(vecItemsDensityDesc[j]->nAnswer + 65));
				stDiff.fDiff = vecItemsDensityDesc[i]->fRealValuePercent - vecItemsDensityDesc[j]->fRealValuePercent;
				stDiff.fFirst = vecItemsDensityDesc[i]->fRealValuePercent;
				stDiff.fSecond = vecItemsDensityDesc[j]->fRealValuePercent;
				vecOmrItemDensityDiff.push_back(stDiff);
			}
		}
		//------------------------------
		strShowInfo.append("\r\n[");
		for (int i = 0; i < vecOmrItemDensityDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.5f ", vecOmrItemDensityDiff[i].szVal, vecOmrItemDensityDiff[i].fDiff);
			strShowInfo.append(szTmp);
		}
		strShowInfo.append("]");

		float fDensityMeanPer = 0.0;
		float fDensityMeanPer2 = 0.0;
		for (int i = 0; i < vecItemsDensityDesc.size(); i++)
		{
			fDensityMeanPer += vecItemsDensityDesc[i]->fRealValuePercent;
		}
		fDensityMeanPer = fDensityMeanPer / vecItemsDensityDesc.size();

		for (int i = 0; i < vecOmrItemDensityDiff.size(); i++)
			fDensityMeanPer2 += vecOmrItemDensityDiff[i].fDiff;
		fDensityMeanPer2 = fDensityMeanPer2 / vecOmrItemDensityDiff.size();

		char szTmp2[100] = { 0 };
		sprintf_s(szTmp2, "密度平均值:%.3f, 密度差平均值值:%.3f, ", fDensityMeanPer, fDensityMeanPer2);
		strShowInfo.append(szTmp2);

		float fDensityThreshold2 = 0.0;
		strShowInfo.append("\r\n密度选中:[");
		for (int i = 0; i < vecOmrItemDensityDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			float fGrayThresholdGray = fDensityMeanPer2 - vecOmrItemDensityDiff[i].fDiff;
			float fGrayMean1 = vecItemsDensityDesc[i]->fRealValuePercent - fDensityMeanPer;
			float fGrayMean12 = -(vecItemsDensityDesc[i + 1]->fRealValuePercent - fDensityMeanPer);
			sprintf_s(szTmp, "%s:%.5f ", vecOmrItemDensityDiff[i].szVal, _dDiffThread_Fix_ + vecOmrItemDensityDiff[i].fDiff * 0.5 + fGrayThresholdGray * 0.5 + fDensityThreshold2);	//_dDiffThread_Fix_ + fGrayThresholdGray + fDensityThreshold2
			strShowInfo.append(szTmp);

			fDensityThreshold2 += (_dDiffThread_Fix_ + vecOmrItemDensityDiff[i].fDiff * 0.5 + fGrayThresholdGray * 0.5 + fDensityThreshold2) / 2;	//_dDiffThread_Fix_ + fGrayThresholdGray + fDensityThreshold2
		}
		strShowInfo.append("]\r\n");
		//---------------------------------------------


		//-----------------------------------	灰度
		std::vector<pRECTINFO> vecItemsGrayDesc;
		std::vector<ST_ITEM_DIFF> vecOmrItemGrayDiff;
		RECTLIST::iterator itItem2 = itOmr->lSelAnswer.begin();
		for (; itItem2 != itOmr->lSelAnswer.end(); itItem2++)
		{
			vecItemsGrayDesc.push_back(&(*itItem2));
		}
		std::sort(vecItemsGrayDesc.begin(), vecItemsGrayDesc.end(), SortByItemGray);

		for (int i = 0; i < vecItemsGrayDesc.size(); i++)
		{
			int j = i + 1;
			if (j < vecItemsGrayDesc.size())
			{
				ST_ITEM_DIFF stDiff;
				sprintf_s(stDiff.szVal, "%c%c", (char)(vecItemsGrayDesc[i]->nAnswer + 65), (char)(vecItemsGrayDesc[j]->nAnswer + 65));
				stDiff.fDiff = abs(vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[j]->fRealMeanGray);
				stDiff.fFirst = vecItemsGrayDesc[i]->fRealMeanGray;
				stDiff.fSecond = vecItemsGrayDesc[j]->fRealMeanGray;
				vecOmrItemGrayDiff.push_back(stDiff);
			}
		}
		//-----------------------------------

		strShowInfo.append("[");
		for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
		{
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.3f ", vecOmrItemGrayDiff[i].szVal, vecOmrItemGrayDiff[i].fDiff);
			strShowInfo.append(szTmp);
		}
		strShowInfo.append("]");
		float fMeanGrayDiff = 0.0;
		for (int i = 0; i < vecItemsGrayDesc.size(); i++)
		{
			fMeanGrayDiff += (vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray);
		}
		fMeanGrayDiff = fMeanGrayDiff / vecItemsGrayDesc.size();
		char szTmp1[40] = { 0 };
		sprintf_s(szTmp1, "平均灰度差:%.3f, ", fMeanGrayDiff);
		strShowInfo.append(szTmp1);
		strShowInfo.append("\r\n灰度选中[");
		float fThreld = 0.0;
		float fGrayDiffLast = 0.0;		//对上一次判断选中的选项对下一个选项选中判断的增益
		for (int i = 0; i < vecOmrItemGrayDiff.size(); i++)
		{
			float fGrayThresholdGray = vecItemsGrayDesc[i]->fRealMeanGray - vecItemsGrayDesc[i]->fStandardMeanGray - fMeanGrayDiff;
			char szTmp[40] = { 0 };
			sprintf_s(szTmp, "%s:%.3f ", vecOmrItemGrayDiff[i].szVal, _dDiffThread_3_ + fGrayThresholdGray + fGrayDiffLast);
			strShowInfo.append(szTmp);
			fGrayDiffLast += abs(fGrayThresholdGray) / 2;
		}
		strShowInfo.append("]\r\n");
	}

	USES_CONVERSION;
	if (m_strMsg.GetLength() > 10000)
		m_strMsg.Empty();
	m_strMsg.Append(A2T(strShowInfo.c_str()));

	m_edit_OmrRecogInfo.SetWindowTextW(m_strMsg);


	int nLineCount = m_edit_OmrRecogInfo.GetLineCount();
	m_edit_OmrRecogInfo.LineScroll(nLineCount);
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

void CShowPapersDlg::SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem)
{
	if (nItem < 0) return;

	lCtrl.GetItemColors(nItem, 0, crOldText, crOldBackground);
	for (int i = 0; i < lCtrl.GetColumns(); i++)							//设置高亮显示(手动设置背景颜色)
		lCtrl.SetItemColors(nItem, i, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255
}

void CShowPapersDlg::UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem)
{
	if (nItem < 0) return;

	for (int i = 0; i < lCtrl.GetColumns(); i++)
		if (!lCtrl.GetModified(nItem, i))
			lCtrl.SetItemColors(nItem, i, crOldText, crOldBackground);
		else
			lCtrl.SetItemColors(nItem, i, RGB(255, 0, 0), crOldBackground);
}

void CShowPapersDlg::ShowPaperListByType(int nType)
{
	m_nCurrItemPaperList = 0;
	m_listPaper.DeleteAllItems();
	//0-不显示，1-无怀疑, 2-显示怀疑，3-显示无怀疑和怀疑，4-显示空，5-显示无怀疑和空，6-显示怀疑和空, 7-显示所有

	USES_CONVERSION;
	for (auto pPaper : m_pPapers->lPaper)
	{
		bool bHasDoubt = false;
		bool bHasNull = false;

		for (auto itemOmr : pPaper->lOmrResult)
		{
			if (itemOmr.nDoubt == 1)
			{
				bHasDoubt = true;
				break;
			}
			if (itemOmr.nDoubt == 2)
			{
				bHasNull = true;
				break;
			}
		}

		if(nType == 0)
			break;
		else if (nType == 1)
		{
			if(bHasDoubt || bHasNull)
				continue;
		}
		else if (nType == 2)
		{
			if(!bHasDoubt)
				continue;
		}
		else if (nType == 3)
		{
			if (bHasNull)
				continue;
		}
		else if (nType == 4)
		{
			if (!bHasNull)
				continue;
		}
		else if (nType == 5)
		{
			if (bHasDoubt)
				continue;
		}
		else if (nType == 6)
		{
			if (!bHasDoubt && !bHasNull)
				continue;
		}
		else if (nType == 7)
		{
		}

		int nCount = m_listPaper.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", pPaper->nIndex);	//nCount + 1
		m_listPaper.InsertItem(nCount, NULL);
		m_listPaper.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_listPaper.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));

		m_listPaper.SetItemData(nCount, (DWORD_PTR)pPaper);
	}
	m_listPaper.GetItemColors(m_nCurrItemPaperList, 0, crOldText, crOldBackground);
}

void CShowPapersDlg::setCheckStatus(int nStatus)
{
	if (nStatus == 0)
	{
		m_btnChkNormal.SetCheck(0);
		m_btnChkDoubt.SetCheck(0);
		m_btnChkNull.SetCheck(0);
	}
	else if (nStatus == 1)
	{
		m_btnChkNormal.SetCheck(1);
		m_btnChkDoubt.SetCheck(0);
		m_btnChkNull.SetCheck(0);
	}
	else if (nStatus == 2)
	{
		m_btnChkNormal.SetCheck(0);
		m_btnChkDoubt.SetCheck(1);
		m_btnChkNull.SetCheck(0);
	}
	else if (nStatus == 3)
	{
		m_btnChkNormal.SetCheck(1);
		m_btnChkDoubt.SetCheck(1);
		m_btnChkNull.SetCheck(0);
	}
	else if (nStatus == 4)
	{
		m_btnChkNormal.SetCheck(0);
		m_btnChkDoubt.SetCheck(0);
		m_btnChkNull.SetCheck(1);
	}
	else if (nStatus == 5)
	{
		m_btnChkNormal.SetCheck(1);
		m_btnChkDoubt.SetCheck(0);
		m_btnChkNull.SetCheck(1);
	}
	else if (nStatus == 6)
	{
		m_btnChkNormal.SetCheck(0);
		m_btnChkDoubt.SetCheck(1);
		m_btnChkNull.SetCheck(1);
	}
	else if (nStatus == 7)
	{
		m_btnChkNormal.SetCheck(1);
		m_btnChkDoubt.SetCheck(1);
		m_btnChkNull.SetCheck(1);
	}
}

int CShowPapersDlg::getCheckStatus()
{
	//空|怀疑|无怀疑 --按位排序
	// 1  1  1  --显示所有
	// 1  1  0  --显示怀疑、空
	int nNormal = m_btnChkNormal.GetCheck();
	int nDoubt = m_btnChkDoubt.GetCheck();
	int nNull = m_btnChkNull.GetCheck();

	int nResult = 0;
	if (nNormal && nDoubt && nNull)	//111
		nResult = 7;
	else if (!nNormal && nDoubt && !nNull)	//010
		nResult = 2;
	else if (!nNormal && !nDoubt && nNull)	//100
		nResult = 4;
	else if (!nNormal && nDoubt && nNull)	//110
		nResult = 6;
	else if (nNormal && !nDoubt && !nNull)	//001
		nResult = 1;
	else if (nNormal && nDoubt && !nNull)	//011
		nResult = 3;
	else if (nNormal && !nDoubt && nNull)	//101
		nResult = 5;

	return nResult;
}

void CShowPapersDlg::OnNMDblclkListPapers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (pNMItemActivate->iItem < 0)
		return;

	UnSetListCtrlHighLightShow(m_listPaper, m_nCurrItemPaperList);

	m_nCurrItemPaperList = pNMItemActivate->iItem;

	SetListCtrlHighLightShow(m_listPaper, m_nCurrItemPaperList);
	m_listPaper.Invalidate();

	pST_PaperInfo pPaper = (pST_PaperInfo)m_listPaper.GetItemData(pNMItemActivate->iItem);
	m_pShowPicDlg->setShowPaper(pPaper);
	m_pAnswerShowDlg->InitData(pPaper);
}


void CShowPapersDlg::OnLvnKeydownListPapers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	*pResult = 0;

	if (m_nCurrItemPaperList < 0) return;

	if (pLVKeyDow->wVKey == VK_UP)
	{
		UnSetListCtrlHighLightShow(m_listPaper, m_nCurrItemPaperList);

		m_nCurrItemPaperList--;
		if (m_nCurrItemPaperList <= 0)
			m_nCurrItemPaperList = 0;

		SetListCtrlHighLightShow(m_listPaper, m_nCurrItemPaperList);

		pST_PaperInfo pPaper = (pST_PaperInfo)m_listPaper.GetItemData(m_nCurrItemPaperList);
		m_pShowPicDlg->setShowPaper(pPaper);
		m_pAnswerShowDlg->InitData(pPaper);
	}
	else if (pLVKeyDow->wVKey == VK_DOWN)
	{
		UnSetListCtrlHighLightShow(m_listPaper, m_nCurrItemPaperList);

		m_nCurrItemPaperList++;
		if (m_nCurrItemPaperList >= m_listPaper.GetItemCount() - 1)
			m_nCurrItemPaperList = m_listPaper.GetItemCount() - 1;

		SetListCtrlHighLightShow(m_listPaper, m_nCurrItemPaperList);

		pST_PaperInfo pPaper = (pST_PaperInfo)m_listPaper.GetItemData(m_nCurrItemPaperList);
		m_pShowPicDlg->setShowPaper(pPaper);
		m_pAnswerShowDlg->InitData(pPaper);
	}
}


void CShowPapersDlg::OnNMHoverListPapers(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;		//**********	这里如果不响应，同时返回结果值不为1的话，	****************
						//**********	就会产生产生TRACK SELECT，也就是鼠标悬停	****************
						//**********	一段时间后，所在行自动被选中
}


void CShowPapersDlg::OnBnClickedChkNormal()
{
	m_nPaperShowType = getCheckStatus();
	ShowPaperListByType(m_nPaperShowType);
}

void CShowPapersDlg::OnBnClickedChkDoubt()
{
	m_nPaperShowType = getCheckStatus();
	ShowPaperListByType(m_nPaperShowType);	
}

void CShowPapersDlg::OnBnClickedChkNull()
{
	m_nPaperShowType = getCheckStatus();
	ShowPaperListByType(m_nPaperShowType);	
}

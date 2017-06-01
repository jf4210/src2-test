// ScanProcessDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanProcessDlg.h"
#include "afxdialogex.h"
#include "ScanMgrDlg.h"
#include "ModifyZkzhDlg.h"

// CScanProcessDlg �Ի���

IMPLEMENT_DYNAMIC(CScanProcessDlg, CDialog)

CScanProcessDlg::CScanProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanProcessDlg::IDD, pParent)
	, m_nCurrentScanCount(0), m_pReminderDlg(NULL), m_pShowPicDlg(NULL), m_pStudentMgr(NULL)
{

}

CScanProcessDlg::~CScanProcessDlg()
{
}

void CScanProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Paper, m_lcPicture);
	DDX_Control(pDX, IDC_BTN_ScanProcess, m_bmpBtnScanProcess);
	DDX_Control(pDX, IDC_BTN_ScanAgain, m_bmpBtnScanAgain);
	DDX_Control(pDX, IDC_BTN_Save, m_bmpBtnSave);
}


BOOL CScanProcessDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI(); 
	SetFontSize();

	USES_CONVERSION;
	char szPicTmpPath[MAX_PATH] = { 0 };
	sprintf_s(szPicTmpPath, "%sPaper\\Tmp", T2A(g_strCurrentPath));

	m_strCurrPicSavePath = szPicTmpPath;

	return TRUE;
}

BOOL CScanProcessDlg::PreTranslateMessage(MSG* pMsg)
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

BEGIN_MESSAGE_MAP(CScanProcessDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_ScanAgain, &CScanProcessDlg::OnBnClickedBtnScanagain)
	ON_BN_CLICKED(IDC_BTN_Save, &CScanProcessDlg::OnBnClickedBtnSave)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Paper, &CScanProcessDlg::OnNMDblclkListPaper)
	ON_BN_CLICKED(IDC_BTN_ScanProcess, &CScanProcessDlg::OnBnClickedBtnScanprocess)
	ON_MESSAGE(MSG_ZKZH_RECOG, &CScanProcessDlg::MsgZkzhRecog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CScanProcessDlg ��Ϣ�������

void CScanProcessDlg::InitUI()
{
	m_bmpBtnScanProcess.SetStateBitmap(IDB_Exam_ScanProcessBtn_Hover, 0, IDB_Exam_ScanProcessBtn);
	m_bmpBtnScanProcess.SetWindowText(_T("  ɨ����Ȳ�ѯ"));
	m_bmpBtnScanProcess.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(116, 116, 116));
	m_bmpBtnScanAgain.SetStateBitmap(IDB_ScanMgr_BtnScanAgain, 0, IDB_ScanMgr_BtnScanAgain_Hover);
	m_bmpBtnScanAgain.SetWindowText(_T("  ����ɨ��"));
	m_bmpBtnScanAgain.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(116, 116, 116));
	m_bmpBtnSave.SetStateBitmap(IDB_ScanMgr_BtnSave, 0, IDB_ScanMgr_BtnSave_Hover);
	m_bmpBtnSave.SetWindowText(_T("  �����ϴ�"));
	m_bmpBtnSave.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(116, 116, 116));

	CBitmap bmp;
	bmp.LoadBitmap(IDB_Scrollbar);
	BITMAP bm;
	bmp.GetBitmap(&bm);
	m_bitmap_scrollbar = (HBITMAP)bmp.Detach();
//	SkinWndScroll(&m_lcPicture, m_bitmap_scrollbar);
#if 0
	int nListColWidth[7] = { 40, 120};
	m_lcPicture.DeleteAllItems();
	DWORD dwStyle = ::GetWindowLong(m_lcPicture.m_hWnd, GWL_STYLE);
	dwStyle &= ~(LVS_TYPEMASK);
	dwStyle &= ~(LVS_EDITLABELS);    
	::SetWindowLong(m_lcPicture.m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT | LVS_NOLABELWRAP | LVS_SHOWSELALWAYS);
	DWORD styles = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyleEx(m_lcPicture.m_hWnd, styles, styles);
	TCHAR rgtsz[2][10] = { _T("�⿨"), _T("����")};
	LV_COLUMN lvcolumn;
	CRect rect;
	m_lcPicture.GetWindowRect(&rect);
	for (int i = 0; i < 2; i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = rgtsz[i];
		lvcolumn.iSubItem = i;
		lvcolumn.iOrder = i;
		lvcolumn.cx = nListColWidth[i];
		m_lcPicture.InsertColumn(i, &lvcolumn);
	}
#else
	m_lcPicture.SetExtendedStyle(m_lcPicture.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcPicture.InsertColumn(0, _T("�⿨"), LVCFMT_CENTER, 40);
	m_lcPicture.InsertColumn(1, _T("����"), LVCFMT_CENTER, 120);
//	m_lcPicture.InsertColumn(2, _T("*"), LVCFMT_CENTER, 20);
#endif

	m_pReminderDlg = new CScanReminderDlg(this);
	m_pReminderDlg->Create(CScanReminderDlg::IDD, this);
	m_pReminderDlg->ShowWindow(SW_SHOW);

	m_pShowPicDlg = new CShowPicDlg(this);
	m_pShowPicDlg->Create(CShowPicDlg::IDD, this);
	m_pShowPicDlg->ShowWindow(SW_HIDE);

	InitCtrlPosition();
}

void CScanProcessDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 10;	//�ϱߵļ��
	const int nBottomGap = 10;	//�±ߵļ��
	const int nLeftGap = 20;		//��ߵĿհ׼��
	const int nRightGap = 20;	//�ұߵĿհ׼��
	int nGap = 5;
	
	int nCurrLeft = nLeftGap;
	int nCurrTop = nTopGap;
	int nLeftW = cx * 0.3;
	if (nLeftW < 160) nLeftW = 160;
	if (nLeftW > 200) nLeftW = 200;

	int nBtnH = (cy - nTopGap - nBottomGap) * 0.1;
	if (nBtnH < 40) nBtnH = 40;
	if (nBtnH > 50) nBtnH = 50;
	if (m_lcPicture.GetSafeHwnd())
	{
		int nH = cy - nTopGap - nBottomGap - nBtnH - nGap - nBtnH - nGap;
		m_lcPicture.MoveWindow(nCurrLeft, nCurrTop, nLeftW, nH);
		nCurrTop += (nH + nGap);

		m_lcPicture.SetColumnWidth(1, nLeftW - 40 - 20);
	}
	if (GetDlgItem(IDC_BTN_Save)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Save)->MoveWindow(nCurrLeft, nCurrTop, nLeftW, nBtnH);
		nCurrTop += (nBtnH + nGap);
	}
	if (GetDlgItem(IDC_BTN_ScanAgain)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ScanAgain)->MoveWindow(nCurrLeft, nCurrTop, nLeftW, nBtnH);
		nCurrTop += (nBtnH + nGap);
	}


	m_rtChildDlg.left = nLeftGap + nLeftW + nGap;
	m_rtChildDlg.top = nTopGap;
	m_rtChildDlg.right = cx - nRightGap;
	m_rtChildDlg.bottom = cy - nBottomGap - 40 - nGap;

	if (m_pReminderDlg && m_pReminderDlg->GetSafeHwnd())
	{
		m_pReminderDlg->MoveWindow(m_rtChildDlg);
	}
	if (m_pShowPicDlg && m_pShowPicDlg->GetSafeHwnd())
	{
		m_pShowPicDlg->MoveWindow(m_rtChildDlg);
	}

	//btn
	if (GetDlgItem(IDC_BTN_ScanProcess)->GetSafeHwnd())
	{
		int nW = 160;
		int nH = 40;
		nCurrLeft = cx - nRightGap - nW;
		nCurrTop = cy - nBottomGap - nH;
		GetDlgItem(IDC_BTN_ScanProcess)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}
	Invalidate();
}

BOOL CScanProcessDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
//	DrawBorder(pDC);
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CScanProcessDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

// 	if (m_lcPicture.GetSafeHwnd())
// 		UnskinWndScroll(&m_lcPicture);
	InitCtrlPosition();
// 	if (m_lcPicture.GetSafeHwnd())
// 		SkinWndScroll(&m_lcPicture, m_bitmap_scrollbar);
}

void CScanProcessDlg::AddPaper(int nID, pST_PaperInfo pPaper)
{
	if (!pPaper) return;

	USES_CONVERSION;
	int nCount = m_lcPicture.GetItemCount();
	char szCount[10] = { 0 };
	sprintf_s(szCount, "%d", nID);
	m_lcPicture.InsertItem(nCount, NULL);

	m_lcPicture.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
	m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
	m_lcPicture.SetItemData(nCount, (DWORD_PTR)pPaper);
//	m_lcPicture.EnsureVisible(nCount, FALSE);	//�������һ��
}

void CScanProcessDlg::ResetPicList()
{
	m_lcPicture.DeleteAllItems();
}

void CScanProcessDlg::InitShow()
{
	switch (_nScanStatus_)
	{
		case 1:
			EnableBtn(FALSE);
// 			m_pReminderDlg->ShowWindow(SW_SHOW);
// 			m_pShowPicDlg->ShowWindow(SW_HIDE);
			break;
		case 2:
			EnableBtn(TRUE);
// 			m_pReminderDlg->ShowWindow(SW_HIDE);
// 			m_pShowPicDlg->ShowWindow(SW_SHOW);
			break;
		default:
			EnableBtn(TRUE);
// 			m_pReminderDlg->ShowWindow(SW_HIDE);
// 			m_pShowPicDlg->ShowWindow(SW_SHOW);
	}
	m_pReminderDlg->ShowWindow(SW_SHOW);
	m_pShowPicDlg->ShowWindow(SW_HIDE);
	m_pReminderDlg->UpdataScanCount(_nScanCount_);		//����ɨ������
	m_pShowPicDlg->UpdateUI();
}

void CScanProcessDlg::UpdateChildInfo(bool bScanDone /*= false*/)
{
	if (bScanDone)
		m_pReminderDlg->SetShowTips(_T("������ɨ�����"));
	else
		m_pReminderDlg->SetShowTips(_T("����ɨ�裬���Ժ�..."));
	m_pReminderDlg->UpdataScanCount(_nScanCount_);		//����ɨ������
}

void CScanProcessDlg::ScanCompleted()
{
	//��ʾ����ʶ����ɵ�׼��֤��
	if (g_nOperatingMode == 1 || g_bModifySN)
	{
		USES_CONVERSION;
		int nCount = m_lcPicture.GetItemCount();
		for (int i = 0; i < nCount; i++)
		{
			pST_PaperInfo pItemPaper = (pST_PaperInfo)(DWORD_PTR)m_lcPicture.GetItemData(i);
			if (pItemPaper)
			{
				if (!pItemPaper->strSN.empty())
					m_lcPicture.SetItemText(i, 1, (LPCTSTR)A2T(pItemPaper->strSN.c_str()));
				else
				{
					m_lcPicture.SetItemText(i, 1, _T("δʶ��"));
					m_lcPicture.SetItemColors(i, 1, RGB(255, 0, 0), RGB(255, 255, 255));
				}
			}
		}
		SetTimer(TIMER_CheckRecogComplete, 100, NULL);
	}
}

void CScanProcessDlg::EnableBtn(BOOL bEnable)
{
	if (GetDlgItem(IDC_BTN_Save)->GetSafeHwnd())
		GetDlgItem(IDC_BTN_Save)->EnableWindow(bEnable);
	if (GetDlgItem(IDC_BTN_ScanAgain)->GetSafeHwnd())
		GetDlgItem(IDC_BTN_ScanAgain)->EnableWindow(bEnable);
}

void CScanProcessDlg::WriteJsonFile()
{

	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itNomarlPaper = _pCurrPapersInfo_->lPaper.begin();
	for (int i = 0; itNomarlPaper != _pCurrPapersInfo_->lPaper.end(); itNomarlPaper++, i++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itNomarlPaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itNomarlPaper)->strSN);
		jsnPaper.set("qk", (*itNomarlPaper)->nQKFlag);

		int nIssueFlag = 0;			//0 - �����Ծ���ȫ����ʶ�������ģ����˹���Ԥ��1 - �����Ծ�ɨ��Ա�ֶ��޸Ĺ���2-׼��֤��Ϊ�գ�ɨ��Աû���޸ģ�3-ɨ��Ա��ʶ����Ҫ��ɨ���Ծ�
		if ((*itNomarlPaper)->strSN.empty() && !(*itNomarlPaper)->bModifyZKZH)
			nIssueFlag = 2;
		if ((*itNomarlPaper)->bModifyZKZH)
			nIssueFlag = 1;
		jsnPaper.set("issueFlag", nIssueFlag);
		//++���ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
		jsnPaper.set("modify", (*itNomarlPaper)->bModifyZKZH);	//׼��֤���޸ı�ʶ
		jsnPaper.set("reScan", (*itNomarlPaper)->bReScan);		//��ɨ��ʶ
		jsnPaper.set("IssueList", 0);		//��ʶ�˿������������б����ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
		//--

		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itNomarlPaper)->lSnResult.begin();
		for (; itSn != (*itNomarlPaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				jsnSnPosition.set("x", itRect->rt.x);
				jsnSnPosition.set("y", itRect->rt.y);
				jsnSnPosition.set("w", itRect->rt.width);
				jsnSnPosition.set("h", itRect->rt.height);
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaper.set("snDetail", jsnSnDetailArry);

		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itNomarlPaper)->lOmrResult.begin();
		for (; itOmr != (*itNomarlPaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value1", itOmr->strRecogVal1);
			jsnOmr.set("value2", itOmr->strRecogVal2);
			jsnOmr.set("value3", itOmr->strRecogVal3);
			jsnOmr.set("doubt", itOmr->nDoubt);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);

		Poco::JSON::Array jsnElectOmrArry;
		ELECTOMR_LIST::iterator itElectOmr = (*itNomarlPaper)->lElectOmrResult.begin();
		for (; itElectOmr != (*itNomarlPaper)->lElectOmrResult.end(); itElectOmr++)
		{
			Poco::JSON::Object jsnElectOmr;
			jsnElectOmr.set("paperId", i + 1);
			jsnElectOmr.set("doubt", itElectOmr->nDoubt);
			jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnElectOmr.set("value", itElectOmr->strRecogResult);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
			for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnElectOmr.set("position", jsnPositionArry);
			jsnElectOmrArry.add(jsnElectOmr);
		}
		jsnPaper.set("electOmr", jsnElectOmrArry);		//ѡ������
		jsnPaperArry.add(jsnPaper);
	}

	if (g_nOperatingMode == 1)		//��ģʽʱ���쳣�Ծ�Ҳһ���ϴ����������ʶ
	{
		PAPER_LIST::iterator itIssuePaper = _pCurrPapersInfo_->lIssue.begin();
		int nNomarlCount = _pCurrPapersInfo_->lPaper.size();
		for (int j = nNomarlCount; itIssuePaper != _pCurrPapersInfo_->lIssue.end(); itIssuePaper++, j++)
		{
			Poco::JSON::Object jsnPaper;
			jsnPaper.set("name", (*itIssuePaper)->strStudentInfo);
			jsnPaper.set("zkzh", (*itIssuePaper)->strSN);
			jsnPaper.set("qk", (*itIssuePaper)->nQKFlag);

			int nIssueFlag = 0;			//0 - �����Ծ���ȫ����ʶ�������ģ����˹���Ԥ��1 - �����Ծ�ɨ��Ա�ֶ��޸Ĺ���2-׼��֤��Ϊ�գ�ɨ��Աû���޸ģ�3-ɨ��Ա��ʶ����Ҫ��ɨ���Ծ�
			if ((*itIssuePaper)->strSN.empty())
				nIssueFlag = 2;
			if ((*itIssuePaper)->bReScan)		//������ɨȨ�޸��󣬷ź�������
				nIssueFlag = 3;
			jsnPaper.set("issueFlag", nIssueFlag);
			//++���ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
			jsnPaper.set("modify", (*itIssuePaper)->bModifyZKZH);	//׼��֤���޸ı�ʶ
			jsnPaper.set("reScan", (*itIssuePaper)->bReScan);		//��ɨ��ʶ
			jsnPaper.set("IssueList", 1);		//��ʶ�˿������������б����ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
			//--

			Poco::JSON::Array jsnSnDetailArry;
			SNLIST::iterator itSn = (*itIssuePaper)->lSnResult.begin();
			for (; itSn != (*itIssuePaper)->lSnResult.end(); itSn++)
			{
				Poco::JSON::Object jsnSnItem;
				jsnSnItem.set("sn", (*itSn)->nItem);
				jsnSnItem.set("val", (*itSn)->nRecogVal);

				Poco::JSON::Object jsnSnPosition;
				RECTLIST::iterator itRect = (*itSn)->lSN.begin();
				for (; itRect != (*itSn)->lSN.end(); itRect++)
				{
					jsnSnPosition.set("x", itRect->rt.x);
					jsnSnPosition.set("y", itRect->rt.y);
					jsnSnPosition.set("w", itRect->rt.width);
					jsnSnPosition.set("h", itRect->rt.height);
				}
				jsnSnItem.set("position", jsnSnPosition);
				jsnSnDetailArry.add(jsnSnItem);
			}
			jsnPaper.set("snDetail", jsnSnDetailArry);

			Poco::JSON::Array jsnOmrArry;
			OMRRESULTLIST::iterator itOmr = (*itIssuePaper)->lOmrResult.begin();
			for (; itOmr != (*itIssuePaper)->lOmrResult.end(); itOmr++)
			{
				Poco::JSON::Object jsnOmr;
				jsnOmr.set("th", itOmr->nTH);
				jsnOmr.set("type", itOmr->nSingle + 1);
				jsnOmr.set("value", itOmr->strRecogVal);
				jsnOmr.set("value1", itOmr->strRecogVal1);
				jsnOmr.set("value2", itOmr->strRecogVal2);
				jsnOmr.set("value3", itOmr->strRecogVal3);
				jsnOmr.set("doubt", itOmr->nDoubt);
				Poco::JSON::Array jsnPositionArry;
				RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
				for (; itRect != itOmr->lSelAnswer.end(); itRect++)
				{
					Poco::JSON::Object jsnItem;
					char szVal[5] = { 0 };
					sprintf_s(szVal, "%c", itRect->nAnswer + 65);
					jsnItem.set("val", szVal);
					jsnItem.set("x", itRect->rt.x);
					jsnItem.set("y", itRect->rt.y);
					jsnItem.set("w", itRect->rt.width);
					jsnItem.set("h", itRect->rt.height);
					jsnPositionArry.add(jsnItem);
				}
				jsnOmr.set("position", jsnPositionArry);
				jsnOmrArry.add(jsnOmr);
			}
			jsnPaper.set("omr", jsnOmrArry);

			Poco::JSON::Array jsnElectOmrArry;
			ELECTOMR_LIST::iterator itElectOmr = (*itIssuePaper)->lElectOmrResult.begin();
			for (; itElectOmr != (*itIssuePaper)->lElectOmrResult.end(); itElectOmr++)
			{
				Poco::JSON::Object jsnElectOmr;
				jsnElectOmr.set("paperId", j + 1);
				jsnElectOmr.set("doubt", itElectOmr->nDoubt);
				jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
				jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
				jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
				jsnElectOmr.set("value", itElectOmr->strRecogResult);
				Poco::JSON::Array jsnPositionArry;
				RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
				for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
				{
					Poco::JSON::Object jsnItem;
					char szVal[5] = { 0 };
					sprintf_s(szVal, "%c", itRect->nAnswer + 65);
					jsnItem.set("val", szVal);
					jsnItem.set("x", itRect->rt.x);
					jsnItem.set("y", itRect->rt.y);
					jsnItem.set("w", itRect->rt.width);
					jsnItem.set("h", itRect->rt.height);
					jsnPositionArry.add(jsnItem);
				}
				jsnElectOmr.set("position", jsnPositionArry);
				jsnElectOmrArry.add(jsnElectOmr);
			}
			jsnPaper.set("electOmr", jsnElectOmrArry);		//ѡ������
			jsnPaperArry.add(jsnPaper);						//�����Ծ�Ҳ�����б���
		}
	}

	//д�Ծ����Ϣ���ļ�
	std::string strUploader = CMyCodeConvert::Gb2312ToUtf8(_strUserName_);
	std::string sEzs = _strEzs_;
	Poco::JSON::Object jsnFileData;

	jsnFileData.set("examId", _pModel_->nExamID);
	jsnFileData.set("subjectId", _pModel_->nSubjectID);
	jsnFileData.set("uploader", strUploader);
	jsnFileData.set("ezs", sEzs);
	jsnFileData.set("nTeacherId", _nTeacherId_);
	jsnFileData.set("nUserId", _nUserId_);
	jsnFileData.set("scanNum", _pCurrPapersInfo_->nPaperCount);		//ɨ���ѧ������
	jsnFileData.set("detail", jsnPaperArry);
	jsnFileData.set("desc", CMyCodeConvert::Gb2312ToUtf8(_pCurrPapersInfo_->strPapersDesc));

	jsnFileData.set("nOmrDoubt", _pCurrPapersInfo_->nOmrDoubt);
	jsnFileData.set("nOmrNull", _pCurrPapersInfo_->nOmrNull);
	jsnFileData.set("nSnNull", _pCurrPapersInfo_->nSnNull);
	jsnFileData.set("RecogMode", g_nOperatingMode);			//ʶ��ģʽ��1-��ģʽ(��������У��㲻ֹͣʶ��)��2-�ϸ�ģʽ
	std::stringstream jsnString;
	jsnFileData.stringify(jsnString, 0);

	std::string strFileData;
	if (!encString(jsnString.str(), strFileData))
		strFileData = jsnString.str();

	char szExamInfoPath[MAX_PATH] = { 0 };
	sprintf_s(szExamInfoPath, "%s\\papersInfo.dat", m_strCurrPicSavePath.c_str());
	ofstream out(szExamInfoPath);
	out << strFileData.c_str();
	out.close();
}

void CScanProcessDlg::SetFontSize()
{
	m_fontBtn1.DeleteObject();
	m_fontBtn1.CreateFont(12, 0, 0, 0,
						 FW_BOLD, FALSE, FALSE, 0,
						 DEFAULT_CHARSET,
						 OUT_DEFAULT_PRECIS,
						 CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY,
						 DEFAULT_PITCH | FF_SWISS,
						 _T("��Բ"));
	m_bmpBtnScanProcess.SetBtnFont(m_fontBtn1);
	m_fontBtn2.DeleteObject();
	m_fontBtn2.CreateFont(20, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("��Բ"));
	m_bmpBtnSave.SetBtnFont(m_fontBtn2);
	m_bmpBtnScanAgain.SetBtnFont(m_fontBtn2);
}

void CScanProcessDlg::ShowPapers(pPAPERSINFO pPapers)
{
	//��ʾ����ʶ����ɵ�׼��֤��
	USES_CONVERSION;
	//������ʾ�ϸ��Ծ�
	m_lcPicture.DeleteAllItems();
	for (auto pPaper : pPapers->lPaper)
	{
		int nCount = m_lcPicture.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lcPicture.InsertItem(nCount, NULL);
		m_lcPicture.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
		if (pPaper->bModifyZKZH)
		{
//			m_lcPicture.SetItemText(nCount, 2, _T("*"));
			m_lcPicture.SetItemColors(nCount, 1, RGB(0, 0, 255), RGB(255, 255, 255));
		}
		m_lcPicture.SetItemData(nCount, (DWORD_PTR)pPaper);
	}
	
	//�쳣�Ծ�ŵ������Ծ������ʾ
	for (auto pPaper : pPapers->lIssue)
	{
		int nCount = m_lcPicture.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lcPicture.InsertItem(nCount, NULL);
		m_lcPicture.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
//		m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
		CString strErrInfo = _T("");
		if (pPaper->strSN.empty())	strErrInfo = _T("����Ϊ��");
		if (pPaper->bReScan)	strErrInfo = _T("��ɨ");
		m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)strErrInfo);	//2
		m_lcPicture.SetItemData(nCount, (DWORD_PTR)pPaper);
		m_lcPicture.SetItemColors(nCount, 1, RGB(255, 0, 0), RGB(255, 255, 255));

		CString strTips = _T("�쳣�Ծ��������ϴ���Ҳ����������� ��Ҫ�����ҳ������浥��ɨ��");
		m_lcPicture.SetItemToolTipText(nCount, 0, strTips);
		m_lcPicture.SetItemToolTipText(nCount, 1, strTips);
//		m_lcPicture.SetItemToolTipText(nCount, 2, strTips);
	}
}

LRESULT CScanProcessDlg::MsgZkzhRecog(WPARAM wParam, LPARAM lParam)
{
	pST_PaperInfo pPaper = (pST_PaperInfo)wParam;
	pPAPERSINFO   pPapers = (pPAPERSINFO)lParam;
	if (g_nOperatingMode != 1 && !g_bModifySN)
		return FALSE;
	if (_pCurrExam_->nModel)
		return FALSE;

	USES_CONVERSION;
	int nCount = m_lcPicture.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pItemPaper = (pST_PaperInfo)(DWORD_PTR)m_lcPicture.GetItemData(i);
		if (pItemPaper == pPaper)
		{
			if (!pPaper->strSN.empty())
				m_lcPicture.SetItemText(i, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
			else
			{
				m_lcPicture.SetItemText(i, 1, _T("δʶ��"));
				m_lcPicture.SetItemColors(i, 1, RGB(255, 0, 0), RGB(255, 255, 255));
			}
			break;
		}
	}
	return TRUE;
}

void CScanProcessDlg::OnBnClickedBtnScanagain()
{
	bool bDelCurrPapers = true;
	if (_pCurrPapersInfo_)
	{
		int nCount = _pCurrPapersInfo_->lPaper.size() + _pCurrPapersInfo_->lIssue.size();
		if (nCount > 0)
		{
			CString strMsg = _T("");
			strMsg.Format(_T("��ǰ�Ծ����%d���Ծ�δ���棬�Ƿ�ɾ��?"), nCount);
			if (MessageBox(strMsg, _T("��ʾ"), MB_YESNO) != IDYES)
			{
				bDelCurrPapers = false;
			//	return;
			}
		}		
	}

	if (!_pModel_)
	{
		AfxMessageBox(_T("��ǰɨ��ģ��Ϊ��"));
		return;
	}

	int nSrc = 0;
	int nRegDuplex = 1;
	char* ret;
	ret = new char[20];
	ret[0] = '\0';
	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanSrc", ret) == 0)
	{
		nSrc = atoi(ret);
	}
	memset(ret, 0, 20);

	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanDuplex", ret) == 0)
	{
		nRegDuplex = atoi(ret);
	}
	SAFE_RELEASE_ARRY(ret);

	TW_INT16      index = nSrc;

	_nScanStatus_ = 0;

	USES_CONVERSION;
	char szPicTmpPath[MAX_PATH] = { 0 };
	sprintf_s(szPicTmpPath, "%sPaper\\Tmp", T2A(g_strCurrentPath));

	std::string strUtfPath = CMyCodeConvert::Gb2312ToUtf8(szPicTmpPath);
	if (bDelCurrPapers)
	{
		try
		{
			Poco::File tmpPath(strUtfPath);
			if (tmpPath.exists())
				tmpPath.remove(true);

			Poco::File tmpPath1(strUtfPath);
			tmpPath1.createDirectories();
		}
		catch (Poco::Exception& exc)
		{
			std::string strLog = "ɾ����ʱ�ļ���ʧ��(" + exc.message() + "): ";
			strLog.append(szPicTmpPath);
			g_pLogger->information(strLog);
		}
	}	

	m_strCurrPicSavePath = szPicTmpPath;

	//��ȡɨ�����
	int nScanSize = 1;				//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3, TWSS_NONE-�Զ���
	int nScanType = 2;				//0-�ڰף�1-�Ҷȣ�2-��ɫ
	int nScanDpi = 200;				//dpi: 72, 150, 200, 300
	int nAutoCut = 1;
	nScanSize = _pModel_->nScanSize;
	nScanType = _pModel_->nScanType;
	nScanDpi = _pModel_->nScanDpi;
	nAutoCut = _pModel_->nAutoCut;

//	m_nModelPicNums = _pModel_->nPicNum;

	bool bShowScanSrcUI = g_bShowScanSrcUI;			//��ʾ�߼�ɨ�����

	int nDuplex = nRegDuplex;		//��˫��,0-����,1-˫��
	int nSize = TWSS_NONE;							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3, TWSS_NONE-�Զ���
	if (nScanSize == 1)
		nSize = TWSS_A4LETTER;
	else if (nScanSize == 2)
		nSize = TWSS_A3;
	else
		nSize = TWSS_NONE;

	int nNum = 0;
	if (nDuplex == 0)
	{
		nNum = m_nCurrentScanCount * _pModel_->nPicNum;
	}
	else
	{
		int nModelPics = _pModel_->nPicNum;
		if (nModelPics % 2)
			nModelPics++;

		nNum = m_nCurrentScanCount * nModelPics;
	}
	if (nNum == 0)
		nNum = -1;

	if (bDelCurrPapers)
	{
		SAFE_RELEASE(_pCurrPapersInfo_);
		_pCurrPapersInfo_ = new PAPERSINFO();
	}

	CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
	pTW_IDENTITY pID = NULL;
	pID = pDlg->GetScanSrc(index);
	if (NULL != pID)
	{
		_nScanStatus_ = 1;
		pST_SCANCTRL pScanCtrl = new ST_SCANCTRL();
		pScanCtrl->nScannerId = pID->Id;
		pScanCtrl->nScanCount = nNum;			//nNum
		pScanCtrl->nScanDuplexenable = nDuplex;
		pScanCtrl->nScanPixelType = nScanType;
		pScanCtrl->nScanResolution = nScanDpi;
		pScanCtrl->nScanSize = nSize;
		pScanCtrl->bShowUI = bShowScanSrcUI;	//bShowScanSrcUI;

		pDlg->m_scanThread.setNotifyDlg(pDlg);
		pDlg->m_scanThread.setModelInfo(_pModel_->nPicNum, m_strCurrPicSavePath);
		if (bDelCurrPapers)
		{
			pDlg->m_scanThread.resetData();
			pDlg->ResetChildDlg();
		}
		pDlg->m_scanThread.PostThreadMessage(MSG_START_SCAN, pID->Id, (LPARAM)pScanCtrl);
	}	
	
	InitShow();
	UpdateChildInfo();
}

void CScanProcessDlg::OnBnClickedBtnSave()
{
	if (!_pCurrPapersInfo_)
	{
		AfxMessageBox(_T("û���Ծ����Ϣ"));
		return;
	}

	if (_pCurrPapersInfo_->nPapersType == 1)
	{
		AfxMessageBox(_T("�����Ѿ���������Ծ���������ٴδ���ϴ�"));
		return;
	}

	bool bRecogComplete = true;
	for (auto p : _pCurrPapersInfo_->lPaper)
	{
		if (!p->bRecogComplete)
		{
			bRecogComplete = false;
			break;
		}
	}
	if (!bRecogComplete)
	{
		AfxMessageBox(_T("���Ժ�ͼ������ʶ��"));
		return;
	}

	std::string strUser;
	std::string strEzs;
	int nTeacherId = -1;
	int nUserId = -1;

	strUser = _strUserName_;
	strEzs = _strEzs_;
	nTeacherId = _nTeacherId_;
	nUserId = _nUserId_;

	USES_CONVERSION;
	if (_pCurrPapersInfo_->lIssue.size() > 0)
	{
		if (g_nOperatingMode == 2)
		{
			AfxMessageBox(_T("����ʶ���쳣�Ծ������ϴ������ȴ����쳣�Ծ�"));
			return;
		}
		else
		{
			CString strMsg = _T("");
			strMsg.Format(_T("����%d�������Ծ���Щ�Ծ���Ҫ�����ҳ�ɨ�裬�˴ν����ϴ���%d���Ծ��Ƿ�ȷ���ϴ�?"), _pCurrPapersInfo_->lIssue.size(), _pCurrPapersInfo_->lIssue.size());
			if (MessageBox(strMsg, _T("����"), MB_YESNO) != IDYES)
			{
				return;
			}
			_pCurrPapersInfo_->nPaperCount = _pCurrPapersInfo_->lPaper.size();		//�޸�ɨ���������������Ծ�ɾ�������㵽ɨ���Ծ��С�
		}
	}

	WriteJsonFile();

	//�Ծ��ѹ��
	char szPapersSavePath[500] = { 0 };
	char szZipName[210] = { 0 };
	char szZipBaseName[200] = { 0 };

	Poco::LocalDateTime now;
	char szTime[50] = { 0 };
	sprintf_s(szTime, "%d%02d%02d%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

	if (_pCurrExam_->nModel == 0)
	{
		sprintf_s(szPapersSavePath, "%sPaper\\%s_%d-%d_%s_%d", T2A(g_strCurrentPath), _strUserName_.c_str(), _pModel_->nExamID, _pModel_->nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount);
		sprintf_s(szZipBaseName, "%s_%d-%d_%s_%d", _strUserName_.c_str(), _pModel_->nExamID, _pModel_->nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount);
		sprintf_s(szZipName, "%s_%d-%d_%s_%d%s", _strUserName_.c_str(), _pModel_->nExamID, _pModel_->nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount, T2A(PAPERS_EXT_NAME));
	}
	else
	{
		std::string strExamID = _pCurrExam_->strExamID;
		sprintf_s(szPapersSavePath, "%sPaper\\%s_%s_%d_%s_%d", T2A(g_strCurrentPath), _strUserName_.c_str(), strExamID.c_str(), _pModel_->nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount);
		sprintf_s(szZipBaseName, "%s_%s_%d_%s_%d", _strUserName_.c_str(), strExamID.c_str(), _pModel_->nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount);
		sprintf_s(szZipName, "%s_%s_%d_%s_%d%s", _strUserName_.c_str(), strExamID.c_str(), _pModel_->nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount, T2A(PAPERS_EXT_NAME));
	}

	//��ʱĿ¼�������Ա�ѹ��ʱ����ɨ��
	std::string strSrcPicDirPath;
	try
	{
		Poco::File tmpPath(CMyCodeConvert::Gb2312ToUtf8(m_strCurrPicSavePath));

		char szCompressDirPath[500] = { 0 };
		sprintf_s(szCompressDirPath, "%sPaper\\%s_ToCompress", T2A(g_strCurrentPath), szZipBaseName);
		strSrcPicDirPath = szCompressDirPath;
		std::string strUtf8NewPath = CMyCodeConvert::Gb2312ToUtf8(strSrcPicDirPath);

		tmpPath.renameTo(strUtf8NewPath);
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "��ʱ�ļ���������ʧ��(" + exc.message() + "): ";
		strLog.append(m_strCurrPicSavePath);
		g_pLogger->information(strLog);
		strSrcPicDirPath = m_strCurrPicSavePath;

		//******************	ע��	*******************
		//*************************************************
		//*************************************************
		//���ﱣ�������⣬�ᷢ������
		//*************************************************
	}

	pCOMPRESSTASK pTask = new COMPRESSTASK;
	pTask->strCompressFileName = szZipName;
	pTask->strExtName = T2A(PAPERS_EXT_NAME);
	pTask->strSavePath = szPapersSavePath;
	pTask->strSrcFilePath = strSrcPicDirPath;
	pTask->pPapersInfo = _pCurrPapersInfo_;
	g_fmCompressLock.lock();
	g_lCompressTask.push_back(pTask);
	g_fmCompressLock.unlock();

// 	CString strInfo;
// 	bool bWarn = false;
// 	strInfo.Format(_T("���ڱ���%s..."), A2T(szZipName));
// 	SetStatusShowInfo(strInfo, bWarn);

	_pCurrPapersInfo_ = NULL;
	ResetPicList();
}

void CScanProcessDlg::OnDestroy()
{
	CDialog::OnDestroy();

	SAFE_RELEASE(m_pStudentMgr);
	if (m_pReminderDlg)
	{
		m_pReminderDlg->DestroyWindow();
		SAFE_RELEASE(m_pReminderDlg);
	}
	if (m_pShowPicDlg)
	{
		m_pShowPicDlg->DestroyWindow();
		SAFE_RELEASE(m_pShowPicDlg);
	}
}

void CScanProcessDlg::OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (_nScanStatus_ == 1)
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcPicture.GetItemData(pNMItemActivate->iItem);
	m_pReminderDlg->ShowWindow(SW_HIDE);
	m_pShowPicDlg->ShowWindow(SW_SHOW);
	m_pShowPicDlg->setShowPaper(pPaper);

	pST_PaperInfo pItemPaper = (pST_PaperInfo)(DWORD_PTR)m_lcPicture.GetItemData(pNMItemActivate->iItem);
	if ((/*g_nOperatingMode == 1 ||*/ g_bModifySN) && _pModel_ && pItemPaper && (pItemPaper->strSN.empty() || pItemPaper->bModifyZKZH || pItemPaper->bReScan))
	{
		if (!m_pStudentMgr)
		{
			USES_CONVERSION;
			m_pStudentMgr = new CStudentMgr();
			std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
			bool bResult = m_pStudentMgr->InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
		}
		CModifyZkzhDlg zkzhDlg(_pModel_, _pCurrPapersInfo_, m_pStudentMgr, pItemPaper);
		zkzhDlg.DoModal();

		ShowPapers(_pCurrPapersInfo_);
	}
}

void CScanProcessDlg::OnBnClickedBtnScanprocess()
{
	CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
	pDlg->ShowChildDlg(4);
}


void CScanProcessDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_CheckRecogComplete)
	{
		if (!_pCurrPapersInfo_)
		{
			KillTimer(TIMER_CheckRecogComplete);
			return;
		}

		bool bRecogComplete = true;
		bool bNeedShowZkzhDlg = false;
		for (auto p : _pCurrPapersInfo_->lPaper)
		{
			if (!p->bRecogComplete)
			{
				bRecogComplete = false;
				break;
			}
			if (p->strSN.empty())
				bNeedShowZkzhDlg = true;
		}
		if (bRecogComplete)
		{
			USES_CONVERSION;
			if (_nScanStatus_ == 2 && (/*g_nOperatingMode == 1 ||*/ g_bModifySN) && bNeedShowZkzhDlg)
			{
				KillTimer(TIMER_CheckRecogComplete);
				if (!m_pStudentMgr)
				{
					USES_CONVERSION;
					m_pStudentMgr = new CStudentMgr();
					std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
					bool bResult = m_pStudentMgr->InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
				}
				CModifyZkzhDlg zkzhDlg(_pModel_, _pCurrPapersInfo_, m_pStudentMgr);
				zkzhDlg.DoModal();

				ShowPapers(_pCurrPapersInfo_);
			}
			else
				KillTimer(TIMER_CheckRecogComplete);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

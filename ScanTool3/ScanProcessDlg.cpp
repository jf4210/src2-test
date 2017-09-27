// ScanProcessDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanProcessDlg.h"
#include "afxdialogex.h"
#include "ScanMgrDlg.h"
//#include "ModifyZkzhDlg.h"
#include "NewMessageBox.h"
#include "ScanTool3Dlg.h"
// CScanProcessDlg �Ի���

IMPLEMENT_DYNAMIC(CScanProcessDlg, CDialog)

CScanProcessDlg::CScanProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanProcessDlg::IDD, pParent)
	, m_nCurrentScanCount(0), m_pReminderDlg(NULL), m_pShowPicDlg(NULL), m_pStudentMgr(NULL)/*, m_pModifyZkzhDlg(NULL)*/
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
	m_vecCHzkzh.clear();

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
	m_pShowPicDlg->setShowModel(2);

#ifdef TEST_MODIFY_ZKZH_CHIld
// 	CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
// 	m_pModifyZkzhDlg = new CModifyZkzhDlg(_pModel_, _pCurrPapersInfo_, m_pStudentMgr);
// 	m_pModifyZkzhDlg->Create(CModifyZkzhDlg::IDD, this);
// 	m_pModifyZkzhDlg->ShowWindow(SW_HIDE);
#endif

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
// 	if (m_pModifyZkzhDlg && m_pModifyZkzhDlg->GetSafeHwnd())
// 	{
// 		m_pModifyZkzhDlg->MoveWindow(m_rtChildDlg);
// 	}
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
	if (_pCurrExam_->nModel == 1)
		m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
	else
		m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
	m_lcPicture.SetItemData(nCount, (DWORD_PTR)pPaper);
//	m_lcPicture.EnsureVisible(nCount, FALSE);	//�������һ��

	if (_pCurrExam_->nModel == 0)	//����ģʽ
	{
		m_pReminderDlg->ShowWindow(SW_SHOW);
		m_pShowPicDlg->ShowWindow(SW_HIDE);
	}
	else //����ģʽ
	{
		m_pReminderDlg->ShowWindow(SW_HIDE);
		m_pShowPicDlg->ShowWindow(SW_SHOW);
//		m_pShowPicDlg->setShowPaper(pPaper);
	}
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
	Invalidate();
	TRACE("=============>>> InitShow() %d\n", _nScanStatus_);
}

void CScanProcessDlg::UpdateChildInfo(bool bScanDone /*= false*/)
{
	if (bScanDone)
	{
//		m_pReminderDlg->SetShowTips(_T("������ɨ�����"));
		SetStatusShow(1, _T("������ɨ�����"));
	}
	else
	{
//		m_pReminderDlg->SetShowTips(_T("����ɨ�裬���Ժ�..."));
		SetStatusShow(1, _T("����ɨ�裬���Ժ�..."));
	}
	m_pReminderDlg->UpdataScanCount(_nScanCount_);		//����ɨ������
}

void CScanProcessDlg::ScanCompleted()
{
	if (_pCurrExam_->nModel != 1 && !_pModel_)
		return;

	int nModelPics = 0;
	if (_pCurrExam_->nModel == 1)
		nModelPics = _nPicNum4Ty_;
	else
	{
		nModelPics = _pModel_->nPicNum;
		//ɨ���������ʱ��ģ��ͼ����ֵ1
		if (_nScanAnswerModel_ == 2) nModelPics = 1;
	}

	_pCurrPapersInfo_->nPaperCount = _nScanCount_ / nModelPics;	//����ɨ���Ծ�����
	
	if (_pCurrExam_->nModel == 1)
		return;

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
//					m_lcPicture.SetItemColors(i, 1, RGB(255, 0, 0), RGB(255, 255, 255));
				}
			}
		}
		if(_nScanAnswerModel_ == 0)		//ɨ�������⡢�͹����ʱ������׼��֤���쳣�������
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

bool CScanProcessDlg::WriteJsonFile()
{
	clock_t start_pic, end_pic;
	start_pic = clock();
	TRACE("-------------- 11\n");
	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itNomarlPaper = _pCurrPapersInfo_->lPaper.begin();
	for (int i = 0; itNomarlPaper != _pCurrPapersInfo_->lPaper.end(); itNomarlPaper++, i++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itNomarlPaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itNomarlPaper)->strSN);
		jsnPaper.set("qk", (*itNomarlPaper)->nQKFlag);
		jsnPaper.set("wj", (*itNomarlPaper)->nWJFlag);
		jsnPaper.set("standardAnswer", _nScanAnswerModel_);		//0-�����Ծ�1-Omr���2-��������

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
		jsnPaper.set("picExchangeTimes", (*itNomarlPaper)->nPicsExchange);	//���ѧ�����Ծ�����ͼ��Ĵ���������1ҳ���2ҳ����˳��Ĵ�����ż�����൱��û�ж��������ξͱ�������

		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itNomarlPaper)->lSnResult.begin();
		for (; itSn != (*itNomarlPaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Array jsnSnValPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				Poco::JSON::Object jsnSnPosition;
				jsnSnPosition.set("x", itRect->rt.x);
				jsnSnPosition.set("y", itRect->rt.y);
				jsnSnPosition.set("w", itRect->rt.width);
				jsnSnPosition.set("h", itRect->rt.height);
				jsnSnValPosition.add(jsnSnPosition);
			}
			jsnSnItem.set("position", jsnSnValPosition);
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
		if (jsnElectOmrArry.size() > 0)
			jsnPaper.set("electOmr", jsnElectOmrArry);		//ѡ������

		//-------------------------------------	��¼����ȱ��У������Ϣ���ڻָ��鿴�Ծ��ʱ���ã�����������;
		Poco::JSON::Array jsnPaperChkPointArry;
		for (auto itPic : (*itNomarlPaper)->lPic)
		{
			Poco::JSON::Object jsnPicPoint;

			Poco::JSON::Array jsnPaperFixArry;
			RECTLIST::iterator itFix = itPic->lFix.begin();
			for (int j = 0; itFix != itPic->lFix.end(); itFix++, j++)
			{
				Poco::JSON::Object jsnItem;
				jsnItem.set("x", itFix->rt.x);
				jsnItem.set("y", itFix->rt.y);
				jsnItem.set("w", itFix->rt.width);
				jsnItem.set("h", itFix->rt.height);
				jsnPaperFixArry.add(jsnItem);
			}
			Poco::JSON::Array jsnPaperModelCharAncharPointArry;
			RECTLIST::iterator itModelFix = itPic->lFix.begin();
			for (int j = 0; itModelFix != itPic->lFix.end(); itModelFix++, j++)
			{
				Poco::JSON::Object jsnItem;
				jsnItem.set("x", itModelFix->rt.x);
				jsnItem.set("y", itModelFix->rt.y);
				jsnItem.set("w", itModelFix->rt.width);
				jsnItem.set("h", itModelFix->rt.height);
				jsnPaperModelCharAncharPointArry.add(jsnItem);
			}
			Poco::JSON::Array jsnPaperCharAncharAreaArry;
			CHARACTER_ANCHOR_AREA_LIST::iterator itCharAnchorArea = itPic->lCharacterAnchorArea.begin();
			for (; itCharAnchorArea != itPic->lCharacterAnchorArea.end(); itCharAnchorArea++)
			{
				Poco::JSON::Object jsnItem;
				jsnItem.set("x", (*itCharAnchorArea)->rt.x);
				jsnItem.set("y", (*itCharAnchorArea)->rt.y);
				jsnItem.set("w", (*itCharAnchorArea)->rt.width);
				jsnItem.set("h", (*itCharAnchorArea)->rt.height);

				Poco::JSON::Array jsnPaperCharAncharPointArry;
				for (int j = 0; j < (*itCharAnchorArea)->vecCharacterRt.size(); j++)	//�������ʶ������ʶ�𵽵Ķ���
				{
					Poco::JSON::Object jsnItem2;
					jsnItem2.set("x", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.x);
					jsnItem2.set("y", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.y);
					jsnItem2.set("w", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.width);
					jsnItem2.set("h", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.height);
					jsnPaperCharAncharPointArry.add(jsnItem2);
				}
				jsnItem.set("AP", jsnPaperCharAncharPointArry);

				jsnPaperCharAncharAreaArry.add(jsnItem);
			}
			jsnPicPoint.set("FL", jsnPaperFixArry);	//�����б�
			jsnPicPoint.set("MFL", jsnPaperModelCharAncharPointArry);	//ģ�嶨���б����������ֶ����Ӧ�����ֵ�
			jsnPicPoint.set("CL", jsnPaperCharAncharAreaArry);	//����ʶ�����б�

			jsnPaperChkPointArry.add(jsnPicPoint);
		}
		jsnPaper.set("ChkPoint", jsnPaperChkPointArry);
		//-------------------------------------

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
			jsnPaper.set("wj", (*itIssuePaper)->nWJFlag);
			jsnPaper.set("standardAnswer", _nScanAnswerModel_);		//0-�����Ծ�1-Omr���2-��������

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
			jsnPaper.set("picExchangeTimes", (*itIssuePaper)->nPicsExchange);	//���ѧ�����Ծ�����ͼ��Ĵ���������1ҳ���2ҳ����˳��Ĵ�����ż�����൱��û�ж��������ξͱ�������

			Poco::JSON::Array jsnSnDetailArry;
			SNLIST::iterator itSn = (*itIssuePaper)->lSnResult.begin();
			for (; itSn != (*itIssuePaper)->lSnResult.end(); itSn++)
			{
				Poco::JSON::Object jsnSnItem;
				jsnSnItem.set("sn", (*itSn)->nItem);
				jsnSnItem.set("val", (*itSn)->nRecogVal);

				Poco::JSON::Array jsnSnValPosition;
				RECTLIST::iterator itRect = (*itSn)->lSN.begin();
				for (; itRect != (*itSn)->lSN.end(); itRect++)
				{
					Poco::JSON::Object jsnSnPosition;
					jsnSnPosition.set("x", itRect->rt.x);
					jsnSnPosition.set("y", itRect->rt.y);
					jsnSnPosition.set("w", itRect->rt.width);
					jsnSnPosition.set("h", itRect->rt.height);
					jsnSnValPosition.add(jsnSnPosition);
				}
				jsnSnItem.set("position", jsnSnValPosition);
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

			//-------------------------------------	��¼����ȱ��У������Ϣ���ڻָ��鿴�Ծ��ʱ���ã�����������;
			Poco::JSON::Array jsnPaperChkPointArry;
			for (auto itPic : (*itIssuePaper)->lPic)
			{
				Poco::JSON::Object jsnPicPoint;

				Poco::JSON::Array jsnPaperFixArry;
				RECTLIST::iterator itFix = itPic->lFix.begin();
				for (int j = 0; itFix != itPic->lFix.end(); itFix++, j++)
				{
					Poco::JSON::Object jsnItem;
					jsnItem.set("x", itFix->rt.x);
					jsnItem.set("y", itFix->rt.y);
					jsnItem.set("w", itFix->rt.width);
					jsnItem.set("h", itFix->rt.height);
					jsnPaperFixArry.add(jsnItem);
				}
				Poco::JSON::Array jsnPaperModelCharAncharPointArry;
				RECTLIST::iterator itModelFix = itPic->lFix.begin();
				for (int j = 0; itModelFix != itPic->lFix.end(); itModelFix++, j++)
				{
					Poco::JSON::Object jsnItem;
					jsnItem.set("x", itModelFix->rt.x);
					jsnItem.set("y", itModelFix->rt.y);
					jsnItem.set("w", itModelFix->rt.width);
					jsnItem.set("h", itModelFix->rt.height);
					jsnPaperModelCharAncharPointArry.add(jsnItem);
				}
				Poco::JSON::Array jsnPaperCharAncharAreaArry;
				CHARACTER_ANCHOR_AREA_LIST::iterator itCharAnchorArea = itPic->lCharacterAnchorArea.begin();
				for (; itCharAnchorArea != itPic->lCharacterAnchorArea.end(); itCharAnchorArea++)
				{
					Poco::JSON::Object jsnItem;
					jsnItem.set("x", (*itCharAnchorArea)->rt.x);
					jsnItem.set("y", (*itCharAnchorArea)->rt.y);
					jsnItem.set("w", (*itCharAnchorArea)->rt.width);
					jsnItem.set("h", (*itCharAnchorArea)->rt.height);

					Poco::JSON::Array jsnPaperCharAncharPointArry;
					for (int j = 0; j < (*itCharAnchorArea)->vecCharacterRt.size(); j++)	//�������ʶ������ʶ�𵽵Ķ���
					{
						Poco::JSON::Object jsnItem2;
						jsnItem2.set("x", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.x);
						jsnItem2.set("y", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.y);
						jsnItem2.set("w", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.width);
						jsnItem2.set("h", (*itCharAnchorArea)->vecCharacterRt[j]->rc.rt.height);
						jsnPaperCharAncharPointArry.add(jsnItem2);
					}
					jsnItem.set("AP", jsnPaperCharAncharPointArry);

					jsnPaperCharAncharAreaArry.add(jsnItem);
				}
				jsnPicPoint.set("FL", jsnPaperFixArry);	//�����б�
				jsnPicPoint.set("MFL", jsnPaperModelCharAncharPointArry);	//ģ�嶨���б����������ֶ����Ӧ�����ֵ�
				jsnPicPoint.set("CL", jsnPaperCharAncharAreaArry);	//����ʶ�����б�

				jsnPaperChkPointArry.add(jsnPicPoint);
			}
			jsnPaper.set("ChkPoint", jsnPaperChkPointArry);
			//-------------------------------------

			jsnPaperArry.add(jsnPaper);						//�����Ծ�Ҳ�����б���
		}
	}
	
	//д�Ծ����Ϣ���ļ�
	std::string strUploader;
	if (_pCurrExam_->nModel == 0)
		strUploader = CMyCodeConvert::Gb2312ToUtf8(_strUserName_);
	else
		strUploader = CMyCodeConvert::Gb2312ToUtf8(_pCurrExam_->strPersonID);
	std::string sEzs = _strEzs_;
	Poco::JSON::Object jsnFileData;

	if (_pCurrExam_->nModel == 0)		//����ģʽ(�������汾)���������յ�ֱ��������zip�ļ�������Ҫʶ�����е���Ϣ�ļ�
	{
		jsnFileData.set("examId", _pModel_->nExamID);
		jsnFileData.set("subjectId", _pModel_->nSubjectID);
	}
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

	end_pic = clock();
	TRACE("-------------- 12 --> time: %d\n", end_pic - start_pic);
	std::string strFileData;
	if (!encString(jsnString.str(), strFileData))
		strFileData = jsnString.str();

	TRACE("-------------- 13\n");
	char szExamInfoPath[MAX_PATH] = { 0 };
	sprintf_s(szExamInfoPath, "%s\\papersInfo.dat", m_strCurrPicSavePath.c_str());
	ofstream out(szExamInfoPath);
	out << strFileData.c_str();
	out.close();

	bool bResult = false;
	//���Ŀ��Լ��paperд�����
	try
	{
		Poco::File jsonFile(CMyCodeConvert::Gb2312ToUtf8(szExamInfoPath));
		if (jsonFile.exists())
			bResult = true;
	}
	catch (Poco::Exception& exc)
	{
		std::string strErr = "�����Ծ����Ϣ���ļ�[" + std::string(szExamInfoPath) + "]ʧ��" + exc.message();
		g_pLogger->information(strErr);
	}
	return bResult;
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

void CScanProcessDlg::SetStatusShow(int nType, CString strShowInfo, bool bWarn /*= false*/)
{
	//1--ɨ��״̬��Ϣ��2--�����Ծ����Ϣ
	if (nType == 2)
	{
		m_pReminderDlg->SetShowScanCount(false);
		m_pReminderDlg->SetShowTips(strShowInfo, bWarn);

// 		if (_pCurrExam_->nModel == 0)
// 		{
			m_pReminderDlg->ShowWindow(SW_SHOW);
			m_pShowPicDlg->ShowWindow(SW_HIDE);
//		}
	}
	else
	{
		m_pReminderDlg->SetShowScanCount(true);
		m_pReminderDlg->SetShowTips(strShowInfo, bWarn);
	}
}

void CScanProcessDlg::UpdateExamBmk()
{
	if (g_lBmkStudent.size() <= 0)
		return;

	if (!_pCurrExam_ || !_pCurrSub_)
		return;

	//++�����Ծ���м���ڱ������б�ʶ���޸ĵ�ǰ��Ŀ�ܱ�����ɨ���ʶ
	for (auto pPaper : _pCurrPapersInfo_->lPaper)
	{
		for (auto& subjectStudent : g_lBmkStudent)
		{
			if (subjectStudent.strZkzh == pPaper->strSN)
			{
				if (pPaper->nZkzhInBmkStatus != 0)
				{
					subjectStudent.nScaned = 1;
				}
				break;
			}
		}
	}
	//--

	EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
	if (itFindExam == g_mapBmkMgr.end())
		return;

	for (auto subjectStudent : g_lBmkStudent)
	{
		for (auto& examStudent : itFindExam->second)
		{
			if (examStudent.strZkzh == subjectStudent.strZkzh)
			{
				for (auto& sujectItem : examStudent.lSubjectScanStatus)
				{
					if (sujectItem.nSubjectID == _pCurrSub_->nSubjID)
					{
						sujectItem.nScaned = subjectStudent.nScaned;
						break;
					}
				}
				break;
			}
		}
	}
}

void CScanProcessDlg::ReShowCurrPapers()
{
	ShowPapers(_pCurrPapersInfo_);
}

void CScanProcessDlg::ShowPapers(pPAPERSINFO pPapers)
{
	if (!pPapers) return;

	//��ʾ����ʶ����ɵ�׼��֤��
	USES_CONVERSION;
	//������ʾ�ϸ��Ծ�
	m_lcPicture.DeleteAllItems();
	for (auto pPaper : pPapers->lPaper)
	{
		int nCount = m_lcPicture.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", pPaper->nIndex);	//nCount + 1
		m_lcPicture.InsertItem(nCount, NULL);
		m_lcPicture.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
		if (pPaper->bModifyZKZH)
			m_lcPicture.SetItemColors(nCount, 1, RGB(0, 0, 255), RGB(255, 255, 255));
		if (pPaper->nZkzhInBmkStatus != 1 && _bGetBmk_)	//���ڱ������С��غ�
			m_lcPicture.SetItemColors(nCount, 1, RGB(0, 255, 0), RGB(255, 255, 255));
		if (pPaper->nPicsExchange != 0)	//�Ծ�����˳��
			m_lcPicture.SetItemColors(nCount, 1, RGB(0, 255, 255), RGB(255, 255, 255));

		m_lcPicture.SetItemData(nCount, (DWORD_PTR)pPaper);
	}
	
	//�쳣�Ծ�ŵ������Ծ������ʾ
	for (auto pPaper : pPapers->lIssue)
	{
		int nCount = m_lcPicture.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", pPaper->nIndex);
		m_lcPicture.InsertItem(nCount, NULL);
		m_lcPicture.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
//		m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
		CString strErrInfo = _T("");
		if (pPaper->strSN.empty())	strErrInfo = _T("����Ϊ��");
		if (pPaper->bReScan)	strErrInfo = _T("ɾ��");	//��ɨ
		m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)strErrInfo);	//2
		m_lcPicture.SetItemData(nCount, (DWORD_PTR)pPaper);
		m_lcPicture.SetItemColors(nCount, 1, RGB(255, 0, 0), RGB(255, 255, 255));

		CString strTips = _T("�쳣�Ծ��������ϴ���Ҳ����������� ��Ҫ�����ҳ������浥��ɨ��");
		m_lcPicture.SetItemToolTipText(nCount, 0, strTips);
		m_lcPicture.SetItemToolTipText(nCount, 1, strTips);
//		m_lcPicture.SetItemToolTipText(nCount, 2, strTips);
	}
	m_lcPicture.Invalidate();
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
			m_lcPicture.EnsureVisible(i, FALSE);

			if (!pPaper->strSN.empty())
			{
				m_lcPicture.SetItemText(i, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
				CheckZkzhInBmk(pPaper);

				if (pPaper->nZkzhInBmkStatus == -1)
				{
					bool bFind = false;
					for (auto sn : m_vecCHzkzh)
					{
						if (sn == pPaper->strSN)
						{
							bFind = true;
							break;
						}
					}

					if(!bFind) m_vecCHzkzh.push_back(pPaper->strSN);	//�غŵĿ��ŷ��������У���Ҫȥ��
				}

// 				if (_bGetBmk_ && pPaper->nZkzhInBmkStatus != 1)
// 					m_lcPicture.SetItemColors(i, 1, RGB(0, 255, 0), RGB(255, 255, 255));
			}
			else
			{
				if (pPaper->strRecogSN4Search.empty())
				{
					m_lcPicture.SetItemText(i, 1, _T("����ʶ��Ϊ��"));
//					m_lcPicture.SetItemColors(i, 1, RGB(255, 0, 0), RGB(255, 255, 255));
				}
				else
				{
					m_lcPicture.SetItemText(i, 1, _T("����ʶ����ȫ"));
//					m_lcPicture.SetItemColors(i, 1, RGB(255, 0, 0), RGB(255, 255, 255));
				}
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
			std::string strMsg = Poco::format("��ǰ�Ծ����%d���Ծ�δ���棬�Ƿ�ɾ��?", nCount);

			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 2, strMsg);
			dlg.DoModal();
			if (dlg.m_nResult != IDYES)
			{
				bDelCurrPapers = false;
				return;
			}
		}		
	}

	if (_pCurrExam_->nModel == 0 && !_pModel_)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "��ǰɨ��ģ��Ϊ��");
		dlg.DoModal();
		return;
	}

	if (_nScanAnswerModel_ == 1)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(1, 1, "��ǰ����ɨ�� �͹���(Omr) ��ģʽ��");
		dlg.DoModal();
	}
	else if (_nScanAnswerModel_ == 2)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(1, 1, "��ǰ����ɨ�� ������ ��ģʽ��");
		dlg.DoModal();
	}

	m_vecCHzkzh.clear();
	InitTmpSubjectBmk();

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
	int _nModelPicNums = 0;

	//��ȡɨ�����
	int nScanSize = 1;				//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3, TWSS_NONE-�Զ���
	int nScanType = 2;				//0-�ڰף�1-�Ҷȣ�2-��ɫ
	int nScanDpi = 200;				//dpi: 72, 150, 200, 300
	int nAutoCut = 1;
	if (_pModel_)
	{
		nScanSize = _pModel_->nScanSize;
		nScanType = _pModel_->nScanType;
		nScanDpi = _pModel_->nScanDpi;
		nAutoCut = _pModel_->nAutoCut;

		_nModelPicNums = _pModel_->nPicNum;

		//ɨ���������ʱ��ģ��ͼ����ֵ1
		if (_nScanAnswerModel_ == 2) _nModelPicNums = 1;
	}
	else  //����
	{
		_nModelPicNums = _nPicNum4Ty_;
	}

//	m_nModelPicNums = _pModel_->nPicNum;

	bool bShowScanSrcUI = g_bShowScanSrcUI;			//��ʾ�߼�ɨ�����

	_nDoubleScan_ = nRegDuplex;
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
		nNum = m_nCurrentScanCount * _nModelPicNums;
	}
	else
	{
		int nModelPics = _nModelPicNums;
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

#ifdef Test_Data
	TestData(bDelCurrPapers);
	InitShow();
	UpdateChildInfo();
	return;
#endif

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
		pScanCtrl->nAutoCut = nAutoCut;

		pDlg->m_scanThread.ResetGlobalVal();
		pDlg->m_scanThread.setNotifyDlg(pDlg);
		pDlg->m_scanThread.setModelInfo(_nModelPicNums, m_strCurrPicSavePath);
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
#ifdef TEST_TIP_SHOW
//	CString str = _T("����478bsdffsfsdfsdfsfsfdsfsdfsf73500c2940239e3622d7e2fc59e6_63-129_20170622151957_1.pkg�ɹ�");
	CString str = _T("����f91f021afcbc479ba4a77cb4d3f70a52_402-590_20170531111240_10.pkg�ɹ�");
//	CString str = _T("����һ�����������߰˾�ʮһ�����������߰˾�ʮһ�����������߰˾�ʮ�ɹ�");
	SetStatusShow(2, str, 1);
	return;
#endif
	if (!_pCurrPapersInfo_)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "û���Ծ����Ϣ");
		dlg.DoModal();
		return;
	}
	int nCount = _pCurrPapersInfo_->lPaper.size() + _pCurrPapersInfo_->lIssue.size();
	if (nCount == 0)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "���Ծ���ڣ�");
		dlg.DoModal();
		return;
	}

	if (_pCurrPapersInfo_->nPapersType == 1)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "�����Ѿ���������Ծ���������ٴδ���ϴ�");
		dlg.DoModal();
		return;
	}

	if (_pCurrExam_->nModel == 0 && _nScanAnswerModel_ != 2)	//����ģʽ
	{
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
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "���Ժ�ͼ������ʶ��");
			dlg.DoModal();
			return;
		}
	}

	std::string strUser;
	std::string strEzs;
	int nTeacherId = -1;
	int nUserId = -1;

	strUser = _strUserName_;
	if (_pCurrExam_->nModel == 1)
		strUser = _pCurrExam_->strPersonID;
	strEzs = _strEzs_;
	nTeacherId = _nTeacherId_;
	nUserId = _nUserId_;

	USES_CONVERSION;
	if (_pCurrPapersInfo_->lIssue.size() > 0)
	{
		if (g_nOperatingMode == 2)
		{
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "����ʶ���쳣�Ծ������ϴ������ȴ����쳣�Ծ�");
			dlg.DoModal();
			return;
		}
		else
		{
			std::string strMsg = Poco::format("����%d���������Щ�Ծ���Ҫ�����ҳ�ɨ�裬�Ƿ�ȷ���ϴ�?", (int)_pCurrPapersInfo_->lIssue.size());
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 2, strMsg);
			dlg.DoModal();
			if (dlg.m_nResult != IDYES)
				return;
			_pCurrPapersInfo_->nPaperCount = _pCurrPapersInfo_->lPaper.size();		//�޸�ɨ���������������Ծ�ɾ�������㵽ɨ���Ծ��С�
		}
	}

	int nSubjectID = 0;
	if (_pCurrExam_->nModel == 0)
		nSubjectID = _pModel_->nSubjectID;

	EnableBtn(FALSE);

	TRACE("------------------- 1\n");
	bool bResult = WriteJsonFile();
	if (!bResult)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "�����Ծ����Ϣ���ļ�ʧ�ܣ������ԣ�");
		dlg.DoModal();
		return;
	}

	TRACE("------------------- 2\n");
	//�Ծ��ѹ��
	char szPapersSavePath[500] = { 0 };
	char szZipName[210] = { 0 };
	char szZipBaseName[200] = { 0 };

	Poco::LocalDateTime now;
	char szTime[50] = { 0 };
	sprintf_s(szTime, "%d%02d%02d%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

	CString strExtName = _T("");
	if (_pCurrExam_->nModel == 1)
		strExtName = PAPERS_EXT_NAME_4TY;
	else
		strExtName = PAPERS_EXT_NAME;
	if (_pCurrExam_->nModel == 0)
	{
		sprintf_s(szPapersSavePath, "%sPaper\\%s_%d-%d_%s_%d", T2A(g_strCurrentPath), strUser.c_str(), _pModel_->nExamID, nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount);
		sprintf_s(szZipBaseName, "%s_%d-%d_%s_%d", strUser.c_str(), _pModel_->nExamID, nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount);
		sprintf_s(szZipName, "%s_%d-%d_%s_%d%s", strUser.c_str(), _pModel_->nExamID, nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount, T2A(strExtName));
	}
	else
	{
		std::string strExamID = _pCurrExam_->strExamID;
		sprintf_s(szPapersSavePath, "%sPaper\\%s_%s_%d_%s_%d", T2A(g_strCurrentPath), strUser.c_str(), strExamID.c_str(), nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount);
		sprintf_s(szZipBaseName, "%s_%s_%d_%s_%d", strUser.c_str(), strExamID.c_str(), nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount);
		sprintf_s(szZipName, "%s_%s_%d_%s_%d%s", strUser.c_str(), strExamID.c_str(), nSubjectID, szTime, _pCurrPapersInfo_->nPaperCount, T2A(strExtName));
	}

	//��ʱĿ¼�������Ա�ѹ��ʱ����ɨ��
	std::string strSrcPicDirPath;
	try
	{
		Poco::File tmpPath(CMyCodeConvert::Gb2312ToUtf8(m_strCurrPicSavePath));

		char szCompressDirPath[500] = { 0 };
		if (_pCurrExam_->nModel == 1)	//���Ĳ�������
			sprintf_s(szCompressDirPath, "%sPaper\\%s_ToCompress_UnPwd", T2A(g_strCurrentPath), szZipBaseName);
		else
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

	TRACE("------------------- 3\n");

	pCOMPRESSTASK pTask = new COMPRESSTASK;
	pTask->strCompressFileName = szZipName;
	pTask->strExtName = T2A(strExtName);	//T2A(PAPERS_EXT_NAME);
	pTask->strSavePath = szPapersSavePath;
	pTask->strSrcFilePath = strSrcPicDirPath;
	pTask->pPapersInfo = _pCurrPapersInfo_;
	g_fmCompressLock.lock();
	g_lCompressTask.push_back(pTask);
	g_fmCompressLock.unlock();
	
	//��¼��ǰ�ܹ�ɨ�������
	_nScanPaperCount_ += _pCurrPapersInfo_->nPaperCount;

	TRACE("------------------- 4\n");
	CString strStatus = _T("���ڱ���");
	strStatus.Format(_T("���ڱ���%s"), A2T(szZipName));
	SetStatusShow(2, strStatus);

	UpdateExamBmk();

	TRACE("------------------- 5\n");
	_pCurrPapersInfo_ = NULL;
	ResetPicList();
	EnableBtn(TRUE);
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
// 	if (m_pModifyZkzhDlg)
// 	{
// 		m_pModifyZkzhDlg->DestroyWindow();
// 		SAFE_RELEASE(m_pModifyZkzhDlg);
// 	}
}

void CScanProcessDlg::OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (pNMItemActivate->iItem < 0)
		return;

	if (_nScanStatus_ == 1)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "����ɨ�裬���Ժ󡣡���");
		dlg.DoModal();
		return;
	}

	if (_pCurrExam_->nModel == 0 && _nScanAnswerModel_ != 2)
	{
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
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "��ȴ�ʶ����ɡ�����");
			dlg.DoModal();
			return;
		}
	}	

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcPicture.GetItemData(pNMItemActivate->iItem);
	m_pReminderDlg->ShowWindow(SW_HIDE);
	m_pShowPicDlg->ShowWindow(SW_SHOW);
	m_pShowPicDlg->setShowPaper(pPaper);

	if (_pCurrExam_->nModel == 1 || _nScanAnswerModel_ != 0)	//ɨ��͹���omr����������ʱ��������׼��֤���޸Ĵ���
		return;

	pST_PaperInfo pItemPaper = (pST_PaperInfo)(DWORD_PTR)m_lcPicture.GetItemData(pNMItemActivate->iItem);
	//***	ע�⣺������ڱ������е�ͬʱ�����ⲻ�յ�ҲҪ�����޸�	********	2017.6.4
	if ((/*g_nOperatingMode == 1 ||*/ g_bModifySN) && _pModel_ && pItemPaper && \
		(pItemPaper->strSN.empty() || pItemPaper->bModifyZKZH || pItemPaper->bReScan || (_bGetBmk_ && pItemPaper->nZkzhInBmkStatus != 1) || pItemPaper->nPicsExchange != 0 || \
		 !pItemPaper->bRecogCourse))
	{
		if (!m_pStudentMgr)
		{
			USES_CONVERSION;
			m_pStudentMgr = new CStudentMgr();
			std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
			bool bResult = m_pStudentMgr->InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
		}
	#ifdef TEST_MODIFY_ZKZH_CHIld
		CScanTool3Dlg* pDlg = (CScanTool3Dlg*)AfxGetMainWnd();
		pDlg->SwitchModifyZkzkDlg(_pModel_, _pCurrPapersInfo_, m_pStudentMgr, pItemPaper);
//		ShowPapers(_pCurrPapersInfo_);
	#else
		CModifyZkzhDlg zkzhDlg(_pModel_, _pCurrPapersInfo_, m_pStudentMgr, pItemPaper);
		zkzhDlg.DoModal();
		ShowPapers(_pCurrPapersInfo_);
	#endif
	}
}

void CScanProcessDlg::OnBnClickedBtnScanprocess()
{
	CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
	pDlg->SetReturnDlg(2);
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
			if (p->strSN.empty() || (p->nZkzhInBmkStatus != 1 && _bGetBmk_) || !p->bRecogCourse)	//�������б����ʱ�����׼��֤���Ƿ��ڱ������б�����
				bNeedShowZkzhDlg = true;
		}

		if (bRecogComplete)
		{
			USES_CONVERSION;
			//++����ˢ��һ���б�
			int nCount = m_lcPicture.GetItemCount();
			for (int i = 0; i < nCount; i++)
			{
				pST_PaperInfo pItemPaper = (pST_PaperInfo)(DWORD_PTR)m_lcPicture.GetItemData(i);
				if (pItemPaper)
				{
					if (!pItemPaper->strSN.empty())
					{
						m_lcPicture.SetItemText(i, 1, (LPCTSTR)A2T(pItemPaper->strSN.c_str()));
						if (_bGetBmk_ && pItemPaper->nZkzhInBmkStatus != 1)
							m_lcPicture.SetItemColors(i, 1, RGB(0, 255, 0), RGB(255, 255, 255));
						else if (_bGetBmk_ && pItemPaper->nZkzhInBmkStatus == 1)	//���������������û�����غ��б��еģ���ֹ���ֵ�1���Ծ�������������غŵģ���ǰ���غŵ��Ծ��޷��������⣩
						{
							if (m_vecCHzkzh.size() > 0)
							{
								for (auto sn : m_vecCHzkzh)
								{
									if (sn == pItemPaper->strSN)
									{
										pItemPaper->nZkzhInBmkStatus = -1;		//�غ�
										break;
									}
								}
							}
						}
					}
					else
					{
						if (pItemPaper->strRecogSN4Search.empty())
						{
							m_lcPicture.SetItemText(i, 1, _T("����ʶ��Ϊ��"));
							m_lcPicture.SetItemColors(i, 1, RGB(255, 0, 0), RGB(255, 255, 255));
						}
						else
						{
							m_lcPicture.SetItemText(i, 1, _T("����ʶ����ȫ"));
							m_lcPicture.SetItemColors(i, 1, RGB(255, 0, 0), RGB(255, 255, 255));
						}
					}
				}
			}
			//--
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
			#ifdef TEST_MODIFY_ZKZH_CHIld
				CScanTool3Dlg* pDlg = (CScanTool3Dlg*)AfxGetMainWnd();
				pDlg->SwitchModifyZkzkDlg(_pModel_, _pCurrPapersInfo_, m_pStudentMgr);
			#else
				CModifyZkzhDlg zkzhDlg(_pModel_, _pCurrPapersInfo_, m_pStudentMgr);
				zkzhDlg.DoModal();
				ShowPapers(_pCurrPapersInfo_);
			#endif
			}
			else
				KillTimer(TIMER_CheckRecogComplete);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

int CScanProcessDlg::CheckZkzhInBmk(std::string strZkzh)
{
	int nResult = 0;	//0--�����ⲻ���ڣ�1--��������ڣ�-1--�������⵽�Ѿ�ɨ��
//	for (auto& obj : g_lBmkStudent)
	for (auto& obj : m_lBmkStudent)
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

void CScanProcessDlg::CheckZkzhInBmk(pST_PaperInfo pPaper)
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

void CScanProcessDlg::TestData(bool bReset)
{
	_nScanStatus_ = 1;
	pST_SCANCTRL pScanCtrl = new ST_SCANCTRL();
	
	int _nModelPicNums = 0;
	if (_pModel_)
		_nModelPicNums = _pModel_->nPicNum;
	else
		_nModelPicNums = _nPicNum4Ty_;

	CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
	pDlg->m_scanThread.ResetGlobalVal();
	pDlg->m_scanThread.setNotifyDlg(pDlg);
	pDlg->m_scanThread.setModelInfo(_nModelPicNums, m_strCurrPicSavePath);
	if (bReset)
	{
		pDlg->m_scanThread.resetData();
		pDlg->ResetChildDlg();
	}
	pDlg->m_scanThread.PostThreadMessage(MSG_START_SCAN, 0, (LPARAM)pScanCtrl);
}

void CScanProcessDlg::ShowSinglePic(cv::Mat& matPic)
{
	m_pShowPicDlg->showTmpPic(matPic);
}

void CScanProcessDlg::InitTmpSubjectBmk()
{
	if (_bGetBmk_)
	{
		m_lBmkStudent.clear();
		m_lBmkStudent = g_lBmkStudent;
	}
}


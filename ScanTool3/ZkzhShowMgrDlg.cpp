// ZkzhShowMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ZkzhShowMgrDlg.h"
#include "afxdialogex.h"


// CZkzhShowMgrDlg 对话框

IMPLEMENT_DYNAMIC(CZkzhShowMgrDlg, CDialog)

CZkzhShowMgrDlg::CZkzhShowMgrDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ZKZHSHOWMGRDLG, pParent)
	, m_pModel(pModel), m_pPapers(pPapersInfo), m_pDefShowPaper(pShowPaper)
	, m_pVagueSearchDlg(NULL), m_pShowPicDlg(NULL), m_pStudentMgr(pStuMgr)
	, m_pMultiPageExceptionDlg(NULL), m_pZkzhExceptionDlg(NULL), m_pLostCornerDlg(NULL)
{

}

CZkzhShowMgrDlg::~CZkzhShowMgrDlg()
{
}

void CZkzhShowMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CZkzhShowMgrDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CZkzhShowMgrDlg 消息处理程序
BOOL CZkzhShowMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();

	return TRUE;
}

void CZkzhShowMgrDlg::InitUI()
{
	USES_CONVERSION;
	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		CButton* pBtn = m_vecBtn[i];
		SAFE_RELEASE(pBtn);
		m_vecBtn[i] = NULL;
	}
	m_vecBtn.clear();

	char szBtnName[20] = { 0 };
	sprintf_s(szBtnName, "考号异常");

	CBmpButton* pNewButton = new CBmpButton();// 也可以定义为类的成员变量。
	pNewButton->SetStateBitmap(IDB_RecordDlg_Btn_Over, IDB_RecordDlg_Btn, IDB_RecordDlg_Btn_Hover, 0, IDB_RecordDlg_Btn);
	CRect rcButton(10, 10, 60, 30); // 按钮在对话框中的位置。
	pNewButton->Create(A2T(szBtnName), 0, rcButton, this, 201);	//设置索引从201开始
	pNewButton->ShowWindow(SW_SHOW);
	m_vecBtn.push_back(pNewButton);
	pNewButton->CheckBtn(TRUE);
	
	if (!m_pZkzhExceptionDlg)
	{
		m_pZkzhExceptionDlg = new CZkzhExceptionDlg(m_pModel, m_pPapers, m_pStudentMgr, m_pDefShowPaper, this);
		m_pZkzhExceptionDlg->Create(IDD_ZKZHEXCEPTIONDLG, this);
		m_pZkzhExceptionDlg->ShowWindow(SW_SHOW);
	}

	if (m_pModel && m_pModel->nUsePagination)
	{
		//先检查试卷袋中是否存在多页模式的问题卷，不存在则不显示此页面
		bool bNeedShowPage = false;
		if (m_pPapers)
		{
			for (auto paper : m_pPapers->lPaper)
			{
				if (paper->nPaginationStatus != 2)
				{
					bNeedShowPage = true;
					break;
				}
			}
		}
		if (bNeedShowPage)
		{
			char szBtnName[20] = { 0 };
			sprintf_s(szBtnName, "页码异常");

			CBmpButton* pNewButton = new CBmpButton();// 也可以定义为类的成员变量。
			pNewButton->SetStateBitmap(IDB_RecordDlg_Btn_Over, IDB_RecordDlg_Btn, IDB_RecordDlg_Btn_Hover, 0, IDB_RecordDlg_Btn);
			CRect rcButton(10, 10, 60, 30); // 按钮在对话框中的位置。
			pNewButton->Create(A2T(szBtnName), 0, rcButton, this, 202);	//设置索引从201开始
			pNewButton->ShowWindow(SW_SHOW);
			m_vecBtn.push_back(pNewButton);

			if (!m_pMultiPageExceptionDlg)
			{
				m_pMultiPageExceptionDlg = new CMultiPageExceptionDlg();
				m_pMultiPageExceptionDlg->Create(IDD_MULTIPAGEEXCEPTIONDLG, this);
				m_pMultiPageExceptionDlg->ShowWindow(SW_HIDE);
			}
		}
	}
	bool bShowLostCornerDlg = true;
// 	for (auto pPaper : m_pPapers->lPaper)
// 	{
// 		for (auto pPic : pPaper->lPic)
// 			if (pPic->lLostCorner.size())
// 			{
// 				bShowLostCornerDlg = true;
// 				break;
// 			}
// 		if(bShowLostCornerDlg) break;
// 	}
	if (bShowLostCornerDlg)
	{
		char szBtnName[20] = { 0 };
		sprintf_s(szBtnName, "折角检测");

		CBmpButton* pNewButton = new CBmpButton();// 也可以定义为类的成员变量。
		pNewButton->SetStateBitmap(IDB_RecordDlg_Btn_Over, IDB_RecordDlg_Btn, IDB_RecordDlg_Btn_Hover, 0, IDB_RecordDlg_Btn);
		CRect rcButton(10, 10, 60, 30); // 按钮在对话框中的位置。
		pNewButton->Create(A2T(szBtnName), 0, rcButton, this, 203);	//设置索引从201开始
		pNewButton->ShowWindow(SW_SHOW);
		m_vecBtn.push_back(pNewButton);

		if (!m_pLostCornerDlg)
		{
			m_pLostCornerDlg = new CLostCornerDlg();
			m_pLostCornerDlg->Create(IDD_LOSTCORNERDLG, this);
			m_pLostCornerDlg->ShowWindow(SW_HIDE);
		}
	}
	if (m_vecBtn.size() <= 1)
	{
		for (auto btn : m_vecBtn)
			btn->ShowWindow(SW_HIDE);
	}
	InitCtrlPosition();
}

void CZkzhShowMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 1;	//上边的间隔，留给控制栏
	const int nLeftGap = 5;		//左边的空白间隔
	const int nBottomGap = 1;	//下边的空白间隔
	const int nRightGap = 5;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nBtnW = 60;
	int nBtnH = 30;

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	int nDlgW = cx - nLeftGap - nRightGap;
	int nDlgH;
	if (m_vecBtn.size() == 1)
		nDlgH = cy - nTopGap - nBottomGap;
	else
		nDlgH = cy - nTopGap - nBtnH - nGap - nBottomGap;

	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		nCurrentLeft = nLeftGap + i * (nBtnW + 1);
		if (m_vecBtn[i]->GetSafeHwnd())
		{
			m_vecBtn[i]->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		}
	}
	nCurrentLeft = nLeftGap;
	if (m_vecBtn.size() > 1)
		nCurrentTop += (nBtnH + nGap);

	if (m_pZkzhExceptionDlg && m_pZkzhExceptionDlg->GetSafeHwnd())
	{
		m_pZkzhExceptionDlg->MoveWindow(nCurrentLeft, nCurrentTop, nDlgW, nDlgH);
	}
	if (m_pMultiPageExceptionDlg && m_pMultiPageExceptionDlg->GetSafeHwnd())
	{
		m_pMultiPageExceptionDlg->MoveWindow(nCurrentLeft, nCurrentTop, nDlgW, nDlgH);
	}
	if (m_pLostCornerDlg && m_pLostCornerDlg->GetSafeHwnd())
	{
		m_pLostCornerDlg->MoveWindow(nCurrentLeft, nCurrentTop, nDlgW, nDlgH);
	}
	Invalidate();
}

void CZkzhShowMgrDlg::ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper /*= NULL*/)
{
	m_pModel = pModel;
	m_pPapers = pPapersInfo;
	m_pStudentMgr = pStuMgr;
	m_pDefShowPaper = pShowPaper;

	InitUI();
	m_pZkzhExceptionDlg->ReInitData(m_pModel, m_pPapers, m_pStudentMgr, m_pDefShowPaper);
	if (m_pMultiPageExceptionDlg) m_pMultiPageExceptionDlg->ReInitData(m_pModel, m_pPapers);
	if (m_pLostCornerDlg) m_pLostCornerDlg->ReInitData(m_pModel, m_pPapers);
}

void CZkzhShowMgrDlg::ReInitDataFromChildDlg(pMODEL pModel, pPAPERSINFO pPapersInfo)
{
	m_pZkzhExceptionDlg->ReInitData(m_pModel, m_pPapers, m_pStudentMgr, m_pDefShowPaper);
	if (m_pMultiPageExceptionDlg) m_pMultiPageExceptionDlg->ReInitData(m_pModel, m_pPapers);
	if (m_pLostCornerDlg) m_pLostCornerDlg->ReInitData(m_pModel, m_pPapers);
}

void CZkzhShowMgrDlg::SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg)
{
	m_pZkzhExceptionDlg->SetDlgInfo(pShowDlg, pSearchDlg);
	if (m_pMultiPageExceptionDlg) m_pMultiPageExceptionDlg->SetDlgInfo(pShowDlg, pSearchDlg);
	if (m_pLostCornerDlg) m_pLostCornerDlg->SetDlgInfo(pShowDlg, pSearchDlg);
}

void CZkzhShowMgrDlg::InitData()
{

}

bool CZkzhShowMgrDlg::ReleaseData()
{
	if (m_pZkzhExceptionDlg)
	{
		if (!m_pZkzhExceptionDlg->ReleaseData())
			return false;
		m_pZkzhExceptionDlg->DestroyWindow();
		SAFE_RELEASE(m_pZkzhExceptionDlg);
	}
	if (m_pMultiPageExceptionDlg)
	{
		m_pMultiPageExceptionDlg->DestroyWindow();
		SAFE_RELEASE(m_pMultiPageExceptionDlg);
	}
	if (m_pLostCornerDlg)
	{
		m_pLostCornerDlg->DestroyWindow();
		SAFE_RELEASE(m_pLostCornerDlg);
	}
	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		CButton* pBtn = m_vecBtn[i];
		SAFE_RELEASE(pBtn);
		m_vecBtn[i] = NULL;
	}
	m_vecBtn.clear();

	return true;
}

BOOL CZkzhShowMgrDlg::PreTranslateMessage(MSG* pMsg)
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

LRESULT CZkzhShowMgrDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_COMMAND == message)
	{
		USES_CONVERSION;
		WORD wID = LOWORD(wParam);
		if (wID == 201)		//一定存在
		{
			m_vecBtn[0]->CheckBtn(TRUE);
			if (m_vecBtn.size() > 1) m_vecBtn[1]->CheckBtn(FALSE);
			if (m_vecBtn.size() > 2) m_vecBtn[2]->CheckBtn(FALSE);
			m_pZkzhExceptionDlg->ShowWindow(SW_SHOW);
			m_pZkzhExceptionDlg->ReInitData(m_pModel, m_pPapers, m_pStudentMgr, m_pDefShowPaper);
			if (m_pMultiPageExceptionDlg) m_pMultiPageExceptionDlg->ShowWindow(SW_HIDE);
			if (m_pLostCornerDlg)	m_pLostCornerDlg->ShowWindow(SW_HIDE);
		}
		else if(wID == 202)		//可能存在
		{
			m_vecBtn[0]->CheckBtn(FALSE);
			m_vecBtn[1]->CheckBtn(TRUE);
			if (m_vecBtn.size() > 2) m_vecBtn[2]->CheckBtn(FALSE);
			m_pZkzhExceptionDlg->ShowWindow(SW_HIDE);
			if (m_pMultiPageExceptionDlg)
			{
				m_pMultiPageExceptionDlg->ShowWindow(SW_SHOW);
				m_pMultiPageExceptionDlg->ReInitData(m_pModel, m_pPapers);
			}
			if (m_pLostCornerDlg)	m_pLostCornerDlg->ShowWindow(SW_HIDE);
		}
		else if (wID == 203)	//可能存在
		{
			m_vecBtn[0]->CheckBtn(FALSE);
			if (m_pMultiPageExceptionDlg)
			{
				m_vecBtn[1]->CheckBtn(FALSE);
				m_vecBtn[2]->CheckBtn(TRUE);
			}
			else
				m_vecBtn[1]->CheckBtn(TRUE);
			m_pZkzhExceptionDlg->ShowWindow(SW_HIDE);
			if (m_pMultiPageExceptionDlg)	m_pMultiPageExceptionDlg->ShowWindow(SW_HIDE);
			if (m_pLostCornerDlg)
			{
				m_pLostCornerDlg->ShowWindow(SW_SHOW);
				m_pLostCornerDlg->ReInitData(m_pModel, m_pPapers);
			}
		}
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CZkzhShowMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

BOOL CZkzhShowMgrDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CZkzhShowMgrDlg::OnDestroy()
{
	if (!ReleaseData())
		return;

	CDialog::OnDestroy();
}

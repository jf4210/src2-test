// ZkzhShowMgrDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ZkzhShowMgrDlg.h"
#include "afxdialogex.h"


// CZkzhShowMgrDlg �Ի���

IMPLEMENT_DYNAMIC(CZkzhShowMgrDlg, CDialog)

CZkzhShowMgrDlg::CZkzhShowMgrDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ZKZHSHOWMGRDLG, pParent)
	, m_pModel(pModel), m_pPapers(pPapersInfo), m_pDefShowPaper(pShowPaper)
	, m_pVagueSearchDlg(NULL), m_pShowPicDlg(NULL), m_pStudentMgr(pStuMgr)
	, m_pMultiPageExceptionDlg(NULL), m_pZkzhExceptionDlg(NULL)
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


// CZkzhShowMgrDlg ��Ϣ�������
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
	sprintf_s(szBtnName, "�����쳣");

	CBmpButton* pNewButton = new CBmpButton();// Ҳ���Զ���Ϊ��ĳ�Ա������
	pNewButton->SetStateBitmap(IDB_RecordDlg_Btn_Over, IDB_RecordDlg_Btn, IDB_RecordDlg_Btn_Hover, 0, IDB_RecordDlg_Btn);
	CRect rcButton(10, 10, 60, 30); // ��ť�ڶԻ����е�λ�á�
	pNewButton->Create(A2T(szBtnName), 0, rcButton, this, 201);	//����������201��ʼ
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
		char szBtnName[20] = { 0 };
		sprintf_s(szBtnName, "ҳ���쳣");

		CBmpButton* pNewButton = new CBmpButton();// Ҳ���Զ���Ϊ��ĳ�Ա������
		pNewButton->SetStateBitmap(IDB_RecordDlg_Btn_Over, IDB_RecordDlg_Btn, IDB_RecordDlg_Btn_Hover, 0, IDB_RecordDlg_Btn);
		CRect rcButton(10, 10, 60, 30); // ��ť�ڶԻ����е�λ�á�
		pNewButton->Create(A2T(szBtnName), 0, rcButton, this, 202);	//����������201��ʼ
		pNewButton->ShowWindow(SW_SHOW);
		m_vecBtn.push_back(pNewButton);

		if (!m_pMultiPageExceptionDlg)
		{
			m_pMultiPageExceptionDlg = new CMultiPageExceptionDlg();
			m_pMultiPageExceptionDlg->Create(IDD_MULTIPAGEEXCEPTIONDLG, this);
			m_pMultiPageExceptionDlg->ShowWindow(SW_HIDE);
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

	int nTopGap = 1;	//�ϱߵļ��������������
	const int nLeftGap = 5;		//��ߵĿհ׼��
	const int nBottomGap = 1;	//�±ߵĿհ׼��
	const int nRightGap = 5;	//�ұߵĿհ׼��
	const int nGap = 2;			//��ͨ�ؼ��ļ��

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
}

void CZkzhShowMgrDlg::ReInitDataFromChildDlg(pMODEL pModel, pPAPERSINFO pPapersInfo)
{
	m_pZkzhExceptionDlg->ReInitData(m_pModel, m_pPapers, m_pStudentMgr, m_pDefShowPaper);
	if (m_pMultiPageExceptionDlg) m_pMultiPageExceptionDlg->ReInitData(m_pModel, m_pPapers);
}

void CZkzhShowMgrDlg::SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg)
{
	m_pZkzhExceptionDlg->SetDlgInfo(pShowDlg, pSearchDlg);
	if(m_pMultiPageExceptionDlg) m_pMultiPageExceptionDlg->SetDlgInfo(pShowDlg, pSearchDlg);
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
		if (wID == 201)
		{
			m_vecBtn[0]->CheckBtn(TRUE);
			m_vecBtn[1]->CheckBtn(FALSE);
			m_pZkzhExceptionDlg->ShowWindow(SW_SHOW);
			m_pZkzhExceptionDlg->ReInitData(m_pModel, m_pPapers, m_pStudentMgr, m_pDefShowPaper);
			if (m_pMultiPageExceptionDlg) m_pMultiPageExceptionDlg->ShowWindow(SW_HIDE);
		}
		else if(wID == 202)
		{
			m_vecBtn[0]->CheckBtn(FALSE);
			m_vecBtn[1]->CheckBtn(TRUE);
			m_pZkzhExceptionDlg->ShowWindow(SW_HIDE);
			if (m_pMultiPageExceptionDlg)
			{
				m_pMultiPageExceptionDlg->ShowWindow(SW_SHOW);
				m_pMultiPageExceptionDlg->ReInitData(m_pModel, m_pPapers);
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

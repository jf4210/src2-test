// LostCornerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "LostCornerDlg.h"
#include "afxdialogex.h"


// CLostCornerDlg �Ի���

IMPLEMENT_DYNAMIC(CLostCornerDlg, CDialog)

CLostCornerDlg::CLostCornerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_LOSTCORNERDLG, pParent)
	,m_pPapers(NULL) ,m_pModel(NULL)
{

}

CLostCornerDlg::~CLostCornerDlg()
{
}

void CLostCornerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LostCorner, m_lcLostCornerPaper);
}

BEGIN_MESSAGE_MAP(CLostCornerDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_LostCorner, &CLostCornerDlg::OnNMDblclkListLostcorner)
	ON_NOTIFY(NM_HOVER, IDC_LIST_LostCorner, &CLostCornerDlg::OnNMHoverListLostcorner)
END_MESSAGE_MAP()

// CLostCornerDlg ��Ϣ�������
BOOL CLostCornerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	InitData();

	return TRUE;
}

void CLostCornerDlg::SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg)
{
	m_pShowPicDlg = pShowDlg;
	m_pVagueSearchDlg = pSearchDlg;
}

void CLostCornerDlg::ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo)
{
	m_pModel = pModel;
	m_pPapers = pPapersInfo;

	m_nCurrentPaperID = 0;
	m_nCurrentPicID = 0;

	InitData();
	UpdateData(FALSE);
}

void CLostCornerDlg::InitData()
{
	if (!m_pPapers) return;

	m_lcLostCornerPaper.DeleteAllItems();
	m_lcLostCornerPaper.DeleteAllItems();

	USES_CONVERSION;
	for (auto pPaper : m_pPapers->lPaper)
	{
		bool bInsert = false;
		for (auto pPic : pPaper->lPic)
		{
			if (pPic->lLostCorner.size())
			{
				if (!bInsert)
				{
					bInsert = true;
					//��ӽ��Ծ��б�ؼ�
					int nCount = m_lcLostCornerPaper.GetItemCount();
					char szCount[10] = { 0 };
					sprintf_s(szCount, "%d", pPaper->nIndex);	//nCount + 1
					m_lcLostCornerPaper.InsertItem(nCount, NULL);
					m_lcLostCornerPaper.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
					m_lcLostCornerPaper.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
					//��ʾ��ע��Ϣ��Ϊ�γ����ڴ��б�
					std::string strDetailInfo;
					strDetailInfo = Poco::format("�۽�%d��", (int)pPic->lLostCorner.size());
					m_lcLostCornerPaper.SetItemText(nCount, 2, (LPCTSTR)A2T(strDetailInfo.c_str()));
					m_lcLostCornerPaper.SetItemData(nCount, (DWORD_PTR)pPaper);

					CString strTips = _T("˫����ʾ���Ծ���Ϣ");
					m_lcLostCornerPaper.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
					m_lcLostCornerPaper.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
					m_lcLostCornerPaper.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
				}
			}
		}		
	}
	if (m_lcLostCornerPaper.GetItemCount() > 0)
	{
		m_nCurrentPaperID = 0;
		m_lcLostCornerPaper.GetItemColors(m_nCurrentPaperID, 0, crPaperOldText, crPaperOldBackground);

		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcLostCornerPaper.GetItemData(m_nCurrentPaperID);
		ShowPaperDetail(pPaper);
	}
}

void CLostCornerDlg::InitUI()
{
	m_lcLostCornerPaper.SetExtendedStyle(m_lcLostCornerPaper.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcLostCornerPaper.InsertColumn(0, _T("����"), LVCFMT_CENTER, 50);
	m_lcLostCornerPaper.InsertColumn(1, _T("׼��֤��"), LVCFMT_CENTER, 110);
	m_lcLostCornerPaper.InsertColumn(2, _T("��ע"), LVCFMT_LEFT, 150);

	InitCtrlPosition();
}

void CLostCornerDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 2;	//�ϱߵļ��������������
	const int nLeftGap = 2;		//��ߵĿհ׼��
	const int nBottomGap = 2;	//�±ߵĿհ׼��
	const int nRightGap = 2;	//�ұߵĿհ׼��
	const int nGap = 2;			//��ͨ�ؼ��ļ��

	int nStaticTip = 20;		//�б���ʾstatic�ؼ��߶�
	int nListCtrlWidth = cx - nLeftGap - nRightGap;	//ͼƬ�б�ؼ����
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	if (GetDlgItem(IDC_STATIC)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC)->MoveWindow(nCurrentTop, nCurrentLeft, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (m_lcLostCornerPaper.GetSafeHwnd())
	{
		int nListH = cy - nCurrentTop - nBottomGap;
		m_lcLostCornerPaper.MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nListH);
	}
}

void CLostCornerDlg::ShowPaperDetail(pST_PaperInfo pPaper)
{
	if (!pPaper) return;

	for (int i = 0; i < m_lcLostCornerPaper.GetColumns(); i++)							//���ø�����ʾ(�ֶ����ñ�����ɫ)
		m_lcLostCornerPaper.SetItemColors(m_nCurrentPaperID, i, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255
	
	if (m_pShowPicDlg)
	{
		m_pShowPicDlg->ReInitUI(pPaper);
		m_pShowPicDlg->setShowPaper(pPaper);
	}
	
	if (m_pVagueSearchDlg) m_pVagueSearchDlg->setNotifyDlg(NULL);	//�˴��ڲ�����ģ��������ѯ����
}

BOOL CLostCornerDlg::PreTranslateMessage(MSG* pMsg)
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

HBRUSH CLostCornerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

BOOL CLostCornerDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}


void CLostCornerDlg::OnNMDblclkListLostcorner(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem < 0)
		return;

	if (m_nCurrentPaperID < m_lcLostCornerPaper.GetItemCount())
	{
		for (int i = 0; i < m_lcLostCornerPaper.GetColumns(); i++)
			if (!m_lcLostCornerPaper.GetModified(m_nCurrentPaperID, i))
				m_lcLostCornerPaper.SetItemColors(m_nCurrentPaperID, i, crPaperOldText, crPaperOldBackground);
			else
				m_lcLostCornerPaper.SetItemColors(m_nCurrentPaperID, i, RGB(255, 0, 0), crPaperOldBackground);
	}
	m_nCurrentPaperID = pNMItemActivate->iItem;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcLostCornerPaper.GetItemData(m_nCurrentPaperID);
	ShowPaperDetail(pPaper);
	*pResult = 0;
}

void CLostCornerDlg::OnNMHoverListLostcorner(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;
}

// ZkzhExceptionDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ZkzhExceptionDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"


// CZkzhExceptionDlg �Ի���

IMPLEMENT_DYNAMIC(CZkzhExceptionDlg, CDialog)

CZkzhExceptionDlg::CZkzhExceptionDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ZKZHEXCEPTIONDLG, pParent)
	, m_pModel(pModel), m_pPapers(pPapersInfo), m_pCurrentShowPaper(NULL), m_nCurrentSelItem(0), m_pDefShowPaper(pShowPaper)
	, m_pVagueSearchDlg(NULL), m_pShowPicDlg(NULL), m_pStudentMgr(pStuMgr)
{

}

CZkzhExceptionDlg::~CZkzhExceptionDlg()
{
}

void CZkzhExceptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ZkzhExcDlg, m_lcZkzh);
}


BEGIN_MESSAGE_MAP(CZkzhExceptionDlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ZkzhExcDlg, &CZkzhExceptionDlg::OnNMDblclkListZkzhexception)
	ON_NOTIFY(NM_HOVER, IDC_LIST_ZkzhExcDlg, &CZkzhExceptionDlg::OnNMHoverListZkzhexception)
	ON_REGISTERED_MESSAGE(WM_XLISTCTRL_EDIT_END, OnEditEnd)
	ON_REGISTERED_MESSAGE(WM_XLISTCTRL_LBUTTONDOWN_EDIT_CLICKED, OnLBtnDownEdit)
	ON_MESSAGE(MSG_VAGUESEARCH_ZKZH, &CZkzhExceptionDlg::MsgVagueSearchResult)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CLICK, IDC_LIST_ZkzhExcDlg, &CZkzhExceptionDlg::OnNMClickListZkzhexcdlg)
END_MESSAGE_MAP()


// CZkzhExceptionDlg ��Ϣ�������
BOOL CZkzhExceptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	InitData();

	return TRUE;
}

void CZkzhExceptionDlg::InitUI()
{
	m_lcZkzh.SetExtendedStyle(m_lcZkzh.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcZkzh.InsertColumn(0, _T("����"), LVCFMT_CENTER, 50);
	m_lcZkzh.InsertColumn(1, _T("׼��֤��(�ɱ༭)"), LVCFMT_CENTER, 110);
	if(g_nHighSevereMode)
		m_lcZkzh.InsertColumn(2, _T("��ע"), LVCFMT_LEFT, 150);
	else
	{
		m_lcZkzh.InsertColumn(2, _T("ɾ����ɨ"), LVCFMT_CENTER, 80);	//��ɨ��ʶ
		m_lcZkzh.InsertColumn(3, _T("��ע"), LVCFMT_LEFT, 150);
	}
	HDITEM hditem;
	for (int i = 0; i < m_lcZkzh.m_HeaderCtrl.GetItemCount(); i++)
	{
		hditem.mask = HDI_IMAGE | HDI_FORMAT;
		m_lcZkzh.m_HeaderCtrl.GetItem(i, &hditem);
		hditem.fmt |= HDF_IMAGE;
		if (i == 2 && g_nHighSevereMode == 0)
			hditem.iImage = XHEADERCTRL_UNCHECKED_IMAGE;
		m_lcZkzh.m_HeaderCtrl.SetItem(i, &hditem);
	}
	m_lcZkzh.EnableToolTips(TRUE);

	InitCtrlPosition();
}

void CZkzhExceptionDlg::InitCtrlPosition()
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

	int nListCtrlWidth = cx - nLeftGap - nRightGap;	//ͼƬ�б�ؼ����
	int nStaticTip = 15;							//�б���ʾstatic�ؼ��߶�
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	if (GetDlgItem(IDC_STATIC_IssueZkzh)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_IssueZkzh)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}

	int nZkzhLCH = cy - nCurrentTop - nBottomGap;
	if (m_lcZkzh.GetSafeHwnd())
	{
		m_lcZkzh.MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nZkzhLCH);
		nCurrentTop += (nZkzhLCH + nGap);
	}
	Invalidate();
}

void CZkzhExceptionDlg::InitData()
{
	if (NULL == m_pPapers)
		return;

	bool bFindFirstShow = false;
	m_lcZkzh.DeleteAllItems();

	USES_CONVERSION;
	for (auto pPaper : m_pPapers->lPaper)
	{
		if (pPaper->strSN.empty() || pPaper->bModifyZKZH || (pPaper->nZkzhInBmkStatus != 1 && _bGetBmk_) || pPaper->nPicsExchange != 0)
		{
			//��ӽ��Ծ��б�ؼ�
			int nCount = m_lcZkzh.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", pPaper->nIndex);	//nCount + 1
			m_lcZkzh.InsertItem(nCount, NULL);

			m_lcZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));	//pPaper->strStudentInfo.c_str()
			m_lcZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));

			m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
			m_lcZkzh.SetEdit(nCount, 1);

			//��ʾ��ע��Ϣ��Ϊ�γ����ڴ��б�
			std::string strDetailInfo = GetDetailInfo(pPaper);

			m_lcZkzh.SetItemToolTipText(nCount, 0, _T("˫����ʾ�˿����Ծ�"));
			m_lcZkzh.SetItemToolTipText(nCount, 1, _T("����޸�׼��֤��"));
			if (g_nHighSevereMode)
			{
				m_lcZkzh.SetItemText(nCount, 2, (LPCTSTR)A2T(strDetailInfo.c_str()));
				m_lcZkzh.SetItemToolTipText(nCount, 2, _T("��ѡ�������Ծ���Ҫ����ɨ��"));
			}
			else
			{
				int nReScan = 0;
				if (pPaper->bReScan) nReScan = 1;
				m_lcZkzh.SetItemText(nCount, 2, _T(""));	//��ɨ
				m_lcZkzh.SetCheckbox(nCount, 2, nReScan);
				m_lcZkzh.SetItemText(nCount, 3, (LPCTSTR)A2T(strDetailInfo.c_str()));
				m_lcZkzh.SetItemToolTipText(nCount, 3, _T("˫����ʾ�˿����Ծ�"));
			}

			if (pPaper == m_pDefShowPaper)
			{
				bFindFirstShow = true;
				m_nCurrentSelItem = nCount;
			}
// 			CString strTips = _T("˫����ʾ�˿����Ծ�");
// 			m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
// 			strTips = _T("����޸�׼��֤��");
// 			m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
// 			strTips = _T("��ѡ�������Ծ���Ҫ����ɨ��");
		}
	}
	for (auto pPaper : m_pPapers->lIssue)
	{
		int nCount = m_lcZkzh.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", pPaper->nIndex);
		m_lcZkzh.InsertItem(nCount, NULL);

		m_lcZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));	//pPaper->strStudentInfo.c_str()
		m_lcZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));

		m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
		m_lcZkzh.SetEdit(nCount, 1);

		//��ʾ��ע��Ϣ��Ϊ�γ����ڴ��б�
		std::string strDetailInfo = GetDetailInfo(pPaper);
		
		m_lcZkzh.SetItemToolTipText(nCount, 0, _T("˫����ʾ�˿����Ծ�"));
		m_lcZkzh.SetItemToolTipText(nCount, 1, _T("����޸�׼��֤��"));
		if (g_nHighSevereMode)
		{
			m_lcZkzh.SetItemText(nCount, 2, (LPCTSTR)A2T(strDetailInfo.c_str()));
			m_lcZkzh.SetItemToolTipText(nCount, 2, _T("��ѡ�������Ծ���Ҫ����ɨ��"));
		}
		else
		{
			int nReScan = 0;
			if (pPaper->bReScan) nReScan = 1;
			m_lcZkzh.SetItemText(nCount, 2, _T(""));	//��ɨ
			m_lcZkzh.SetCheckbox(nCount, 2, nReScan);
			m_lcZkzh.SetItemText(nCount, 3, (LPCTSTR)A2T(strDetailInfo.c_str()));
			m_lcZkzh.SetItemToolTipText(nCount, 3, _T("˫����ʾ�˿����Ծ�"));
		}

		if (pPaper == m_pDefShowPaper)
		{
			bFindFirstShow = true;
			m_nCurrentSelItem = nCount;
		}
	}
	int nCount = m_lcZkzh.GetItemCount();
	if (nCount > 0)
	{
		if (!bFindFirstShow)
			m_nCurrentSelItem = 0;

		m_lcZkzh.GetItemColors(m_nCurrentSelItem, 0, crOldText, crOldBackground);
		ShowPaperByItem(m_nCurrentSelItem);
		VagueSearch(m_nCurrentSelItem);	//ģ����������
	}
}

bool CZkzhExceptionDlg::ReleaseData()
{
	if (!g_nExitFlag)
	{
		int nCount = m_lcZkzh.GetItemCount();
		if (g_nZkzhNull2Issue == 1)
		{
			for (int i = 0; i < nCount; i++)
			{
				pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
				if (pPaper->strSN.empty())
				{
					CNewMessageBox	dlg;
					if (g_nHighSevereMode)
					{
						dlg.setShowInfo(2, 1, "��⵽����׼��֤��Ϊ�յĿ����������ÿ�����׼��֤��!");
						dlg.DoModal();
						return false;
					}
					dlg.setShowInfo(2, 2, "���ڿ���Ϊ�յĿ����������޸ģ���Ӱ��μӺ�����������ԣ�");
					dlg.DoModal();
					if (dlg.m_nResult != IDYES)
						return false;
					break;
				}
			}
		}
		if (g_nHighSevereMode == 0)		//������ģʽʱ��û��ɾ���Ծ�Ĺ��ܣ�CListCtrl����ʾCheckBoxѡ���
		{
			for (int i = 0; i < nCount; i++)
			{
				pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
				if (m_lcZkzh.GetCheckbox(i, 2))
					pPaper->bReScan = true;			//���ô��Ծ���Ҫ����ɨ��
				else
					pPaper->bReScan = false;
			}
		}

		//������Ծ��Ѿ����޸��������������б�ɾ��
		if (m_pPapers)
		{
			PAPER_LIST::iterator itIssue = m_pPapers->lIssue.begin();
			for (; itIssue != m_pPapers->lIssue.end();)
			{
				pST_PaperInfo pPaper = *itIssue;
				if ((g_nZkzhNull2Issue == 1 && !pPaper->strSN.empty() || g_nZkzhNull2Issue == 0) && !pPaper->bReScan)		//���Ų��գ��Ҳ�����ɨ������Ϊ���������Ծ����������б��У����ԭ���������б����ƶ��������б�
				{
					itIssue = m_pPapers->lIssue.erase(itIssue);
					m_pPapers->lPaper.push_back(pPaper);
					continue;
				}
				itIssue++;
			}

			//��Ҫ��ɨ���Ծ���������Ծ��б�
			PAPER_LIST::iterator itPaper = m_pPapers->lPaper.begin();
			for (; itPaper != m_pPapers->lPaper.end();)
			{
				pST_PaperInfo pPaper = *itPaper;
				if ((g_nZkzhNull2Issue == 1 && pPaper->strSN.empty()) || pPaper->bReScan)
				{
					itPaper = m_pPapers->lPaper.erase(itPaper);
					m_pPapers->lIssue.push_back(pPaper);
					continue;
				}
				itPaper++;
			}
		}
	}

	SetZkzhStatus();
	m_lcZkzh.DeleteAllItems();
	return true;
}

BOOL CZkzhExceptionDlg::PreTranslateMessage(MSG* pMsg)
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

std::string CZkzhExceptionDlg::GetDetailInfo(pST_PaperInfo pPaper)
{
	std::string strDetailInfo;
	if (pPaper->nPicsExchange == 0)
	{
		if (pPaper->strSN.empty())
			strDetailInfo = "����Ϊ��";
		if (pPaper->bModifyZKZH)
			strDetailInfo = "�й��޸�";
		if (_bGetBmk_)
		{
			if (pPaper->nZkzhInBmkStatus == -1)
				strDetailInfo = "�����غ�";
			if (pPaper->nZkzhInBmkStatus == 0)
				strDetailInfo = "���Ų��ڱ�����";
		}
		if (!pPaper->bRecogCourse)
			strDetailInfo = "��ĿУ��ʧ��";
		if (pPaper->bReScan)
			strDetailInfo = "����Ծ�ɾ��";
	}
	else
	{
		if (pPaper->strSN.empty())
			strDetailInfo = "ͼ���й�����������Ϊ��";
		else
			strDetailInfo = "ͼ���й�����";
		if (pPaper->bModifyZKZH)
			strDetailInfo = "ͼ���й��������й��޸�";
		if (_bGetBmk_)
		{
			if (pPaper->nZkzhInBmkStatus == -1)
				strDetailInfo = "ͼ���й������������غ�";
			if (pPaper->nZkzhInBmkStatus == 0)
				strDetailInfo = "ͼ���й����������Ų��ڱ�����";
		}
		if (!pPaper->bRecogCourse)
			strDetailInfo = "��ĿУ��ʧ��";
		if (pPaper->bReScan)
			strDetailInfo = "ͼ���й�����������Ծ�ɾ��";
	}
	return strDetailInfo;
}

int CZkzhExceptionDlg::CheckZkzhInBmk(std::string strZkzh)
{
	int nResult = 0;	//0--�����ⲻ���ڣ�1--��������ڣ�-1--�������⵽�Ѿ�ɨ��
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

void CZkzhExceptionDlg::CheckZkzhInBmk(pST_PaperInfo pPaper)
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

void CZkzhExceptionDlg::ShowPaperByItem(int nItem)
{
	if (nItem < 0)
		return;
	if (nItem >= m_lcZkzh.GetItemCount())
		return;
	if (!m_pShowPicDlg)
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);

	m_pCurrentShowPaper = pPaper;
	m_lcZkzh.GetItemColors(nItem, 0, crOldText, crOldBackground);
	for (int i = 0; i < m_lcZkzh.GetColumns(); i++)							//���ø�����ʾ(�ֶ����ñ�����ɫ)
		m_lcZkzh.SetItemColors(nItem, i, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255

	m_pShowPicDlg->ReInitUI(pPaper);
	m_pShowPicDlg->setShowPaper(pPaper);

	UpdateData(FALSE);
	m_lcZkzh.Invalidate();
}

bool CZkzhExceptionDlg::VagueSearch(int nItem)
{
	bool bResult = false;
	if (nItem < 0)
		return bResult;
	if (nItem >= m_lcZkzh.GetItemCount())
		return bResult;
	if (!m_pVagueSearchDlg)
		return bResult;

	m_pVagueSearchDlg->setNotifyDlg(this);

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);
	bResult = m_pVagueSearchDlg->vagueSearch(pPaper);
	return bResult;
}

void CZkzhExceptionDlg::SetZkzhStatus()
{
	if (g_nExitFlag)	//ȫ���˳���ʶ���ر���������������ã���ֹ�����ڴ治��Ĵ����б���Ծ���Ϣ�����Ѿ�����
		return;
	std::vector<std::string> vecCHzkzh;		//����غŵ�׼��֤�ţ�����2�α�����
											//��ֹ���ֵ�һ��������������������غź�ǰ���һ�ε��Ծ��״̬������������������
	int nCount = m_lcZkzh.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
		CheckZkzhInBmk(pPaper);
		if (pPaper->nZkzhInBmkStatus == -1)
		{
			bool bFind = false;
			for (auto sn : vecCHzkzh)
			{
				if (sn == pPaper->strSN)
				{
					bFind = true;
					break;
				}
			}

			if (!bFind) vecCHzkzh.push_back(pPaper->strSN);	//�غŵĿ��ŷ��������У���Ҫȥ��
		}
	}
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
		if (pPaper->nZkzhInBmkStatus == 1)		//ֻ��Ҫ��������Ծ���׼��֤���ǲ������غŵ�׼��֤���б���
		{
			if (vecCHzkzh.size() > 0)
			{
				for (auto sn : vecCHzkzh)
				{
					if (sn == pPaper->strSN)
					{
						pPaper->nZkzhInBmkStatus = -1;		//�غ�
						break;
					}
				}
				continue;
			}
		}
		// 		if (pPaper->nZkzhInBmkStatus == -1)		//�غŵ��Ծ���Ҫ����ǲ���Ŀǰ�����غţ���ֹ֮ǰ�����غ��ˣ�����û���ϴ�������Ĺ�����
		// 		{
		// 		}
	}
}

void CZkzhExceptionDlg::SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg)
{
	m_pShowPicDlg = pShowDlg;
	m_pVagueSearchDlg = pSearchDlg;
}

void CZkzhExceptionDlg::ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper /*= NULL*/)
{
	m_pModel = pModel;
	m_pPapers = pPapersInfo;
	m_pStudentMgr = pStuMgr;
	m_pCurrentShowPaper = pShowPaper;
	m_pDefShowPaper = pShowPaper;

	if (_bGetBmk_)
	{
		m_lBmkStudent.clear();
		m_lBmkStudent = g_lBmkStudent;
	}
	InitData();
	InitCtrlPosition();
}

void CZkzhExceptionDlg::OnNMDblclkListZkzhexception(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem < 0)
		return;

	if (m_nCurrentSelItem < m_lcZkzh.GetItemCount())
	{
		for (int i = 0; i < m_lcZkzh.GetColumns(); i++)
			if (!m_lcZkzh.GetModified(m_nCurrentSelItem, i))
				m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, crOldText, crOldBackground);
			else
				m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, RGB(255, 0, 0), crOldBackground);
	}
	m_nCurrentSelItem = pNMItemActivate->iItem;
	ShowPaperByItem(m_nCurrentSelItem);
	VagueSearch(m_nCurrentSelItem);	//ģ����������
	*pResult = 0;
}

void CZkzhExceptionDlg::OnNMHoverListZkzhexception(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;
}

void CZkzhExceptionDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

HBRUSH CZkzhExceptionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_LIST_ZkzhException)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

BOOL CZkzhExceptionDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

LRESULT CZkzhExceptionDlg::OnEditEnd(WPARAM nItem, LPARAM nSubItem)
{
	if (nItem >= 0 && nSubItem >= 0)
	{
		USES_CONVERSION;
		CString strText = m_lcZkzh.GetItemText(nItem, nSubItem);
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);
		pPaper->strSN = T2A(strText);

		for (int i = 0; i < m_lcZkzh.GetColumns(); i++)						//ȡ��������ʾ(�ֶ����ñ�����ɫ)
			if (!m_lcZkzh.GetModified(m_nCurrentSelItem, i))
				m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, crOldText, crOldBackground);
			else
				m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, RGB(255, 0, 0), crOldBackground);

		COLORREF crText, crBackground;
		m_lcZkzh.GetItemColors(nItem, nSubItem, crText, crBackground);
		if (m_lcZkzh.GetModified(nItem, nSubItem))
		{
			m_lcZkzh.SetItemText(nItem, nSubItem, strText, RGB(255, 0, 0), crBackground);
			pPaper->bModifyZKZH = true;

#if 0	//test
			if (m_pStudentMgr && m_pStudentMgr->UpdateStudentStatus("student", pPaper->strSN, 1))
			{
				TRACE("���¿���%sɨ��״̬���\n", pPaper->strStudentInfo.c_str());
			}
#endif
		}
		else
		{
			m_lcZkzh.SetItemText(nItem, nSubItem, strText, RGB(0, 0, 0), crBackground);
		}
		m_nCurrentSelItem = nItem;
		m_pCurrentShowPaper = pPaper;
		if (m_pShowPicDlg)
			m_pShowPicDlg->setShowPaper(pPaper);

		UpdateData(FALSE);
		m_lcZkzh.Invalidate();
	}
	return 0;
}

LRESULT CZkzhExceptionDlg::OnLBtnDownEdit(WPARAM nItem, LPARAM nSubItem)
{
	if (nItem >= 0 && nSubItem >= 0)
	{
		USES_CONVERSION;
		CString strText = m_lcZkzh.GetItemText(nItem, nSubItem);
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);

		for (int i = 0; i < m_lcZkzh.GetColumns(); i++)						//ȡ��������ʾ(�ֶ����ñ�����ɫ)
			if (!m_lcZkzh.GetModified(m_nCurrentSelItem, i))
				m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, crOldText, crOldBackground);
			else
				m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, RGB(255, 0, 0), crOldBackground);

		COLORREF crText, crBackground;
		m_lcZkzh.GetItemColors(nItem, nSubItem, crText, crBackground);
		if (m_lcZkzh.GetModified(nItem, nSubItem))
		{
			m_lcZkzh.SetItemText(nItem, nSubItem, strText, RGB(255, 0, 0), crBackground);
		}
		else
		{
			m_lcZkzh.SetItemText(nItem, nSubItem, strText, RGB(0, 0, 0), crBackground);
		}
		m_nCurrentSelItem = nItem;
		m_pCurrentShowPaper = pPaper;
		if (m_pShowPicDlg)
			m_pShowPicDlg->setShowPaper(pPaper);

		UpdateData(FALSE);
		m_lcZkzh.Invalidate();
	}
	return 0;
}

LRESULT CZkzhExceptionDlg::MsgVagueSearchResult(WPARAM wParam, LPARAM lParam)
{
	CString strZkzh = (LPCTSTR)wParam;
	USES_CONVERSION;
	m_pCurrentShowPaper->strSN = T2A(strZkzh);
	m_pCurrentShowPaper->bModifyZKZH = true;

	COLORREF crText, crBackground;
	m_lcZkzh.GetItemColors(m_nCurrentSelItem, 1, crText, crBackground);
	m_lcZkzh.SetItemText(m_nCurrentSelItem, 1, strZkzh, RGB(255, 0, 0), crBackground);
	return 1;
}

void CZkzhExceptionDlg::OnDestroy()
{
	if (!ReleaseData())
		return;

	CDialog::OnDestroy();
}


void CZkzhExceptionDlg::OnNMClickListZkzhexcdlg(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (pNMItemActivate->iItem < 0)
		return;

	if (g_nHighSevereMode)		//������ģʽʱ��û��ɾ���Ծ�Ĺ��ܣ�CListCtrl����ʾCheckBoxѡ���
		return;

	if (pNMItemActivate->iSubItem != 2)
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(pNMItemActivate->iItem);
	if (m_lcZkzh.GetCheckbox(pNMItemActivate->iItem, 2))
		pPaper->bReScan = true;			//���ô��Ծ���Ҫ����ɨ��
	else
		pPaper->bReScan = false;

	if (m_pPapers)
	{
		if (!pPaper->bReScan)
		{
			//������Ծ��Ѿ����޸��������������б�ɾ��
			PAPER_LIST::iterator itIssue = m_pPapers->lIssue.begin();
			for (; itIssue != m_pPapers->lIssue.end();)
			{
				pST_PaperInfo pItemPaper = *itIssue;
				if (pItemPaper == pPaper && !pItemPaper->bReScan)		//������ɨ������Ϊ���������Ծ����������б��У����ԭ���������б����ƶ��������б�
				{
					itIssue = m_pPapers->lIssue.erase(itIssue);
					m_pPapers->lPaper.push_back(pItemPaper);
					break;
				}
				itIssue++;
			}
		}
		else
		{
			//��Ҫ��ɨ���Ծ���������Ծ��б�
			PAPER_LIST::iterator itPaper = m_pPapers->lPaper.begin();
			for (; itPaper != m_pPapers->lPaper.end();)
			{
				pST_PaperInfo pItemPaper = *itPaper;
				if (pItemPaper == pPaper && pItemPaper->bReScan)
				{
					itPaper = m_pPapers->lPaper.erase(itPaper);
					m_pPapers->lIssue.push_back(pItemPaper);
					break;
				}
				itPaper++;
			}
		}		
	}
}

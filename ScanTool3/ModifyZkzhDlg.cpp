// ModifyZkzhDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ModifyZkzhDlg.h"
#include "afxdialogex.h"


// CModifyZkzhDlg �Ի���

IMPLEMENT_DYNAMIC(CModifyZkzhDlg, CDialog)

CModifyZkzhDlg::CModifyZkzhDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent /*=NULL*/)
	: CDialog(CModifyZkzhDlg::IDD, pParent)
	, m_pCurrentPicShow(NULL), m_pModel(pModel), m_pPapers(pPapersInfo), m_nModelPicNums(1), m_nCurrTabSel(0), m_pCurrentShowPaper(NULL), m_pVagueSearchDlg(NULL)
	, m_nCurrentSelItem(0), m_pStudentMgr(pStuMgr), m_pShowPaper(pShowPaper), m_nSearchType(2)
{

}

CModifyZkzhDlg::~CModifyZkzhDlg()
{
	std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
	for (; itPic != m_vecPicShow.end();)
	{
		CPicShow* pModelPicShow = *itPic;
		SAFE_RELEASE(pModelPicShow);
		itPic = m_vecPicShow.erase(itPic);
	}
}

void CModifyZkzhDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Zkzh, m_lcZkzh);
	DDX_Control(pDX, IDC_TAB_ZkzhPic, m_tabPicShowCtrl);
	DDX_Text(pDX, IDC_EDIT_Zkzh, m_strCurZkzh);
	DDX_Control(pDX, IDC_LIST_ZkzhSearchResult, m_lcBmk);
	DDX_Text(pDX, IDC_EDIT_SearchKey, m_strSearchKey);
}


BOOL CModifyZkzhDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (NULL != m_pModel)
	{
		m_nModelPicNums = m_pModel->nPicNum;
	}
	InitUI();
	InitData();
	ShowPaperByItem(m_nCurrentSelItem);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CModifyZkzhDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_ZkzhPic, &CModifyZkzhDlg::OnTcnSelchangeTabZkzhpic)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Zkzh, &CModifyZkzhDlg::OnNMDblclkListZkzh)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Zkzh, &CModifyZkzhDlg::OnNMHoverListZkzh)
	ON_REGISTERED_MESSAGE(WM_XLISTCTRL_EDIT_END, OnEditEnd)
	ON_REGISTERED_MESSAGE(WM_XLISTCTRL_LBUTTONDOWN_EDIT_CLICKED, OnLBtnDownEdit)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CModifyZkzhDlg::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_RADIO_SearchZkzh, &CModifyZkzhDlg::OnBnClickedRadioSearchzkzh)
	ON_BN_CLICKED(IDC_RADIO_SearchName, &CModifyZkzhDlg::OnBnClickedRadioSearchname)
	ON_BN_CLICKED(IDC_BTN_Search, &CModifyZkzhDlg::OnBnClickedBtnSearch)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ZkzhSearchResult, &CModifyZkzhDlg::OnNMDblclkListZkzhsearchresult)
	ON_MESSAGE(WM_CV_RBTNUP, &CModifyZkzhDlg::RoiRBtnUp)
	ON_COMMAND(ID_LeftRotate, &CModifyZkzhDlg::LeftRotate)
	ON_COMMAND(ID_RightRotate, &CModifyZkzhDlg::RightRotate)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CModifyZkzhDlg ��Ϣ�������

void CModifyZkzhDlg::InitUI()
{
#ifdef NewListModelTest
	m_lcZkzh.SetExtendedStyle(m_lcZkzh.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
//	m_lcZkzh.InsertColumn(0, _T("˳��"), LVCFMT_CENTER, 40);
	m_lcZkzh.InsertColumn(0, _T("����"), LVCFMT_CENTER, 110);
	m_lcZkzh.InsertColumn(1, _T("׼��֤��(�ɱ༭)"), LVCFMT_CENTER, 110);
	m_lcZkzh.InsertColumn(2, _T("ɾ����ɨ"), LVCFMT_CENTER, 80);	//��ɨ��ʶ
	m_lcZkzh.InsertColumn(3, _T("��ע"), LVCFMT_CENTER, 150);

	HDITEM hditem;
	for (int i = 0; i < m_lcZkzh.m_HeaderCtrl.GetItemCount(); i++)
	{
		hditem.mask = HDI_IMAGE | HDI_FORMAT;
		m_lcZkzh.m_HeaderCtrl.GetItem(i, &hditem);
		hditem.fmt |= HDF_IMAGE;
		if (i == 2)
			hditem.iImage = XHEADERCTRL_UNCHECKED_IMAGE;
		m_lcZkzh.m_HeaderCtrl.SetItem(i, &hditem);
	}
	// call EnableToolTips to enable tooltip display
	m_lcZkzh.EnableToolTips(TRUE);
#else
	m_lcZkzh.SetExtendedStyle(m_lcZkzh.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcZkzh.InsertColumn(0, _T("˳��"), LVCFMT_CENTER, 40);
	m_lcZkzh.InsertColumn(1, _T("ѧ����ʶ"), LVCFMT_CENTER, 110);
	m_lcZkzh.InsertColumn(2, _T("׼��֤��(�ɱ༭)"), LVCFMT_CENTER, 110);
	m_lcZkzh.InsertColumn(3, _T("ɾ����ɨ"), LVCFMT_CENTER, 80);	//��ɨ��ʶ

	HDITEM hditem;
	for(int i = 0; i < m_lcZkzh.m_HeaderCtrl.GetItemCount(); i++)
	{
		hditem.mask = HDI_IMAGE | HDI_FORMAT;
		m_lcZkzh.m_HeaderCtrl.GetItem(i, &hditem);
		hditem.fmt |= HDF_IMAGE;
		if (i == 3)
			hditem.iImage = XHEADERCTRL_UNCHECKED_IMAGE;
		m_lcZkzh.m_HeaderCtrl.SetItem(i, &hditem);
	}
	// call EnableToolTips to enable tooltip display
	m_lcZkzh.EnableToolTips(TRUE);
#endif

	m_lcBmk.SetExtendedStyle(m_lcBmk.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcBmk.InsertColumn(0, _T("˳��"), LVCFMT_CENTER, 40);
	m_lcBmk.InsertColumn(1, _T("����"), LVCFMT_CENTER, 80);
	m_lcBmk.InsertColumn(2, _T("׼��֤��"), LVCFMT_CENTER, 120);
	m_lcBmk.InsertColumn(3, _T("�༶"), LVCFMT_CENTER, 80);
	m_lcBmk.InsertColumn(4, _T("ѧУ"), LVCFMT_CENTER, 150);

	switch (m_nSearchType)
	{
		case 1:
			((CButton*)GetDlgItem(IDC_RADIO_SearchName))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_SearchZkzh))->SetCheck(0);
			break;
		case 2:
			((CButton*)GetDlgItem(IDC_RADIO_SearchName))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_SearchZkzh))->SetCheck(1);
			break;
	}

	InitTab();

	m_pVagueSearchDlg = new CVagueSearchDlg();
	m_pVagueSearchDlg->Create(CVagueSearchDlg::IDD, this);
	m_pVagueSearchDlg->ShowWindow(SW_SHOW);
	m_pVagueSearchDlg->setExamInfo(m_pStudentMgr, m_pModel);

	MoveWindow(0, 0, 1024, 650);
	CenterWindow();
	InitCtrlPosition();
}

void CModifyZkzhDlg::InitTab()
{
	if (m_pModel)
	{
		std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
		for (; itPic != m_vecPicShow.end();)
//		for (auto itPic : m_vecPicShow)
		{
			CPicShow* pModelPicShow = *itPic;
			if (pModelPicShow)
			{
				delete pModelPicShow;
				pModelPicShow = NULL;
			}
			itPic = m_vecPicShow.erase(itPic);
		}
	}
	m_tabPicShowCtrl.DeleteAllItems();

	USES_CONVERSION;
	CRect rtTab;
	m_tabPicShowCtrl.GetClientRect(&rtTab);
	for (int i = 0; i < m_nModelPicNums; i++)
	{
		char szTabHeadName[20] = { 0 };
		sprintf_s(szTabHeadName, "��%dҳ", i + 1);

		m_tabPicShowCtrl.InsertItem(i, A2T(szTabHeadName));

		CPicShow* pPicShow = new CPicShow(this);
		pPicShow->Create(CPicShow::IDD, &m_tabPicShowCtrl);
		pPicShow->ShowWindow(SW_HIDE);
		pPicShow->MoveWindow(&rtTab);
		m_vecPicShow.push_back(pPicShow);
	}
	m_tabPicShowCtrl.SetCurSel(0);
	if (m_vecPicShow.size())
	{
		m_vecPicShow[0]->ShowWindow(SW_SHOW);
		m_pCurrentPicShow = m_vecPicShow[0];
	}

	if (m_tabPicShowCtrl.GetSafeHwnd())
	{
		CRect rtTab;
		m_tabPicShowCtrl.GetClientRect(&rtTab);
		int nTabHead_H = 24;		//tab�ؼ�ͷ�ĸ߶�
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
}

void CModifyZkzhDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 5;	//�ϱߵļ��������������
	const int nLeftGap = 5;		//��ߵĿհ׼��
	const int nBottomGap = 2;	//�±ߵĿհ׼��
	const int nRightGap = 2;	//�ұߵĿհ׼��
	const int nGap = 2;			//��ͨ�ؼ��ļ��

	int nGroupH = 70;			//group�ؼ��߶�
	int nListCtrlWidth = 350;	//ͼƬ�б�ؼ����
	int nStaticTip = 15;		//�б���ʾstatic�ؼ��߶�
	int nBtnH = 30;				//��ť�߶�
	int nBtnW = (nListCtrlWidth - nGap) * 0.3;
	int nBottomH = 10;			//���²��ֵĸ߶ȣ����ڷ��ð�ť��
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	if (GetDlgItem(IDC_STATIC_Zkzh_S1)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Zkzh_S1)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
//	int nZkzhLCH = cy - nTopGap - nStaticTip - nGap - nStaticTip - nGap - nBtnH - nBottomH - nBottomGap;
	int nZkzhLCH = cy - nTopGap - nStaticTip - nGap - nGap - nGroupH - nGap - nStaticTip - nGap - 150 - nBottomH - nBottomGap;
	if (m_lcZkzh.GetSafeHwnd())
	{
		m_lcZkzh.MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nZkzhLCH);
//		nCurrentLeft += (nListCtrlWidth + nGap);
		nCurrentTop += (nZkzhLCH + nGap);
	}
	//�������ѯ
#if 1
	if (m_pVagueSearchDlg && m_pVagueSearchDlg->GetSafeHwnd())
	{
		int nH = cy - nBottomGap - nCurrentTop;
		m_pVagueSearchDlg->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nH);
	}
#else
	int nTmpTop = nCurrentTop;
	if (GetDlgItem(IDC_STATIC_Group)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Group)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nGroupH);
		nCurrentTop += (nStaticTip + nGap);
		nCurrentLeft += nGap;
	}
	if (GetDlgItem(IDC_RADIO_SearchName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_SearchName)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth / 2 - 5, nStaticTip);
		nCurrentLeft += (nListCtrlWidth / 2 - 5 + nGap);
//		nCurrentTop += (nStaticTip + nGap);
	}
	if (GetDlgItem(IDC_RADIO_SearchZkzh)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_SearchZkzh)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth / 2 - 5, nStaticTip);
		nCurrentLeft = nLeftGap + nGap + nGap;
		nCurrentTop += (nStaticTip + nGap);
	}
	if (GetDlgItem(IDC_EDIT_SearchKey)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_SearchKey)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth - 70, nBtnH);
		nCurrentLeft += (nListCtrlWidth - 70 + nGap);
	}
	if (GetDlgItem(IDC_BTN_Search)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Search)->MoveWindow(nCurrentLeft, nCurrentTop, 60, nBtnH);
		nCurrentLeft = nLeftGap + nGap;
		nCurrentTop = nTmpTop + nGap + nGroupH + nGap;
	}
	if (GetDlgItem(IDC_STATIC_DB_Search)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_DB_Search)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (GetDlgItem(IDC_LIST_ZkzhSearchResult)->GetSafeHwnd())
	{
		GetDlgItem(IDC_LIST_ZkzhSearchResult)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, 150);
		nCurrentTop += (nStaticTip + nGap);
	}
#endif
	//------------------------------------------------���ڿ�ɾ�����Ѿ�����
	if (GetDlgItem(IDC_STATIC_Zkzh_S2)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Zkzh_S2)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop += (nStaticTip + nGap);
	}
	if (GetDlgItem(IDC_STATIC_Zkzh_S3)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Zkzh_S3)->MoveWindow(nCurrentLeft, nCurrentTop, 35, nBtnH);
		nCurrentLeft += (35 + nGap);
	}
	if (GetDlgItem(IDC_EDIT_Zkzh)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_Zkzh)->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth - nGap - 35 - nGap - nBtnW, nBtnH);
		nCurrentLeft += (nListCtrlWidth - nGap - 35 - nGap - nBtnW + nGap);
	}
	if (GetDlgItem(IDC_BTN_SAVE)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_SAVE)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nLeftGap + nListCtrlWidth + nGap;
	}
	//-------------------------------------------------

	//tab
	nCurrentTop = nTopGap;
	int nPicShowTabCtrlWidth = cx - nLeftGap - nRightGap - nListCtrlWidth - nGap - nGap;
	int nPicShowTabCtrlHigh = cy - nTopGap - nBottomH - nBottomGap;
	if (m_tabPicShowCtrl.GetSafeHwnd())
	{
		m_tabPicShowCtrl.MoveWindow(nCurrentLeft, nTopGap, nPicShowTabCtrlWidth, nPicShowTabCtrlHigh);

		CRect rtTab;
		m_tabPicShowCtrl.GetClientRect(&rtTab);
		int nTabHead_H = 24;		//tab�ؼ�ͷ�ĸ߶�
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
}

void CModifyZkzhDlg::InitData()
{
	if (NULL == m_pPapers)
		return;

	USES_CONVERSION;
	for (auto pPaper : m_pPapers->lPaper)
	{
		if (pPaper->strSN.empty() || pPaper->bModifyZKZH || (pPaper->nZkzhInBmkStatus != 1 && g_lBmkStudent.size() > 0))
		{
			//��ӽ��Ծ��б�ؼ�
			int nCount = m_lcZkzh.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", nCount + 1);
			m_lcZkzh.InsertItem(nCount, NULL);
#ifdef NewListModelTest
			m_lcZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
			m_lcZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));

			int nReScan = 0;
			if (pPaper->bReScan) nReScan = 1;
			m_lcZkzh.SetItemText(nCount, 2, _T(""));	//��ɨ
			m_lcZkzh.SetCheckbox(nCount, 2, nReScan);
			m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
			m_lcZkzh.SetEdit(nCount, 1);

			//��ʾ��ע��Ϣ��Ϊ�γ����ڴ��б�
			std::string strDetailInfo;
			if (pPaper->bModifyZKZH)
				strDetailInfo = "���޸�";
			if (pPaper->nZkzhInBmkStatus == -1 && g_lBmkStudent.size() > 0)
				strDetailInfo = "��⵽���ŷ����غţ�����";
			if (pPaper->bReScan)
				strDetailInfo = "����Ծ�ɾ��";

			if (pPaper == m_pShowPaper)
				m_nCurrentSelItem = nCount;

			CString strTips = _T("˫����ʾ�˿����Ծ�");
			m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
			m_lcZkzh.SetItemToolTipText(nCount, 3, (LPCTSTR)strTips);
			strTips = _T("����޸�׼��֤��");
			m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
			strTips = _T("��ѡ�������Ծ���Ҫ����ɨ��");
			m_lcZkzh.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
#else
			m_lcZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
			m_lcZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
			m_lcZkzh.SetItemText(nCount, 2, (LPCTSTR)A2T(pPaper->strSN.c_str()));

			int nReScan = 0;
			if (pPaper->bReScan) nReScan = 1;
			m_lcZkzh.SetItemText(nCount, 3, _T(""));	//��ɨ
			m_lcZkzh.SetCheckbox(nCount, 3, nReScan);
			m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
			m_lcZkzh.SetEdit(nCount, 2);

			if (pPaper == m_pShowPaper)
				m_nCurrentSelItem = nCount;

			CString strTips = _T("˫����ʾ�˿����Ծ�");
			m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
			m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
			strTips = _T("����޸�׼��֤��");
			m_lcZkzh.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
			strTips = _T("��ѡ�������Ծ���Ҫ����ɨ��");
			m_lcZkzh.SetItemToolTipText(nCount, 3, (LPCTSTR)strTips);
#endif
		}
	}
	for (auto pPaper : m_pPapers->lIssue)
	{
#ifdef NewListModelTest
		int nCount = m_lcZkzh.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lcZkzh.InsertItem(nCount, NULL);

		m_lcZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
		m_lcZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));

		int nReScan = 0;
		if (pPaper->bReScan) nReScan = 1;
		m_lcZkzh.SetItemText(nCount, 2, _T(""));	//��ɨ
		m_lcZkzh.SetCheckbox(nCount, 2, nReScan);
		m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
		m_lcZkzh.SetEdit(nCount, 1);

		if (pPaper == m_pShowPaper)
			m_nCurrentSelItem = nCount;

		CString strTips = _T("˫����ʾ�˿����Ծ�");
		m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
		m_lcZkzh.SetItemToolTipText(nCount, 3, (LPCTSTR)strTips);
		strTips = _T("����޸�׼��֤��");
		m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
		strTips = _T("��ѡ�������Ծ���Ҫ����ɨ��");
		m_lcZkzh.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
#else
		int nCount = m_lcZkzh.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lcZkzh.InsertItem(nCount, NULL);

		m_lcZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lcZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
		m_lcZkzh.SetItemText(nCount, 2, (LPCTSTR)A2T(pPaper->strSN.c_str()));

		int nReScan = 0;
		if (pPaper->bReScan) nReScan = 1;
		m_lcZkzh.SetItemText(nCount, 3, _T(""));	//��ɨ
		m_lcZkzh.SetCheckbox(nCount, 3, nReScan);
		m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
		m_lcZkzh.SetEdit(nCount, 2);

		if (pPaper == m_pShowPaper)
			m_nCurrentSelItem = nCount;

		CString strTips = _T("˫����ʾ�˿����Ծ�");
		m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
		m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
		strTips = _T("����޸�׼��֤��");
		m_lcZkzh.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
		strTips = _T("��ѡ�������Ծ���Ҫ����ɨ��");
		m_lcZkzh.SetItemToolTipText(nCount, 3, (LPCTSTR)strTips);
#endif
	}
}

LRESULT CModifyZkzhDlg::RoiRBtnUp(WPARAM wParam, LPARAM lParam)
{
	cv::Point pt = *(cv::Point*)(wParam);
	//�������δ���, ��������Ӧ��ListCtrl  
	CMenu menu, *pPopup;
	menu.LoadMenu(IDR_MENU_Rotation);
	pPopup = menu.GetSubMenu(0);
	CPoint myPoint;
	ClientToScreen(&myPoint);
	GetCursorPos(&myPoint); //���λ��  
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);//GetParent()
// 	m_ptRBtnUp = pt;
	return TRUE;
}

void CModifyZkzhDlg::LeftRotate()
{
	m_vecPicShow[m_nCurrTabSel]->SetRotateDir(3);
}

void CModifyZkzhDlg::RightRotate()
{
	m_vecPicShow[m_nCurrTabSel]->SetRotateDir(2);
}

void CModifyZkzhDlg::ShowPaperByItem(int nItem)
{
	if (nItem < 0)
		return;
	if (nItem >= m_lcZkzh.GetItemCount())
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);

	m_pCurrentShowPaper = pPaper;
	ShowPaperZkzhPosition(pPaper);
// 	m_lcZkzh.SetItemState(nItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
// 	m_lcZkzh.SetSelectionMark(nItem);
// 	m_lcZkzh.SetFocus();
// 	m_lcZkzh.SetItemState(nItem, LVIS_DROPHILITED, LVIS_DROPHILITED);		//������ʾһ�У�ʧȥ�����Ҳһֱ��ʾ
	
	m_lcZkzh.GetItemColors(nItem, 0, crOldText, crOldBackground);
	for (int i = 0; i < m_lcZkzh.GetColumns(); i++)							//���ø�����ʾ(�ֶ����ñ�����ɫ)
		m_lcZkzh.SetItemColors(nItem, i, RGB(0, 0, 0), RGB(70, 70, 255));

	USES_CONVERSION;
	m_nCurrTabSel = 0;
	m_strCurZkzh = pPaper->strSN.c_str();
	GetDlgItem(IDC_STATIC_Zkzh_S3)->SetWindowText(A2T(std::string(pPaper->strStudentInfo + ":").c_str()));
	GetDlgItem(IDC_EDIT_Zkzh)->SetFocus();

	m_tabPicShowCtrl.SetCurSel(0);
	m_pCurrentPicShow = m_vecPicShow[0];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != 0)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
	UpdateData(FALSE);
	m_lcZkzh.Invalidate();
}

void CModifyZkzhDlg::ShowPaperZkzhPosition(pST_PaperInfo pPaper)
{
	if (NULL == pPaper || NULL == m_pModel)
		return;

	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		cv::Mat matSrc = cv::imread((*itPic)->strPicPath);
#ifdef PIC_RECTIFY_TEST
		cv::Mat dst;
		cv::Mat rotMat;
		PicRectify(matSrc, dst, rotMat);
		cv::Mat matImg;
		if (dst.channels() == 1)
			cvtColor(dst, matImg, CV_GRAY2BGR);
		else
			matImg = dst;
		// #ifdef WarpAffine_TEST
		// 		cv::Mat	inverseMat(2, 3, CV_32FC1);
		// 		PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
		// #endif
#else
		cv::Mat matImg = matSrc;
#endif

// #ifdef WarpAffine_TEST
// 		cv::Mat	inverseMat(2, 3, CV_32FC1);
// 		if (pPaper->pModel)
// 			PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
// #endif
// 
// #ifdef Test_ShowOriPosition
// 		cv::Mat	inverseMat(2, 3, CV_32FC1);
// 		GetInverseMat((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
// #endif

		PrintRecogRect(i, pPaper, *itPic, matImg);

		cv::Point pt(0, 0);
		if (pPaper->pModel)
			pt = pPaper->pModel->vecPaperModel[i]->rcSNTracker.rt.tl() - cv::Point(300, 300);

		m_vecPicShow[i]->ShowPic(matImg, pt);
	}
}

void CModifyZkzhDlg::PrintRecogRect(int nPic, pST_PaperInfo pPaper, pST_PicInfo pPic, cv::Mat& matImg)
{

#ifdef WarpAffine_TEST
	cv::Mat	inverseMat(2, 3, CV_32FC1);
	if (pPaper->pModel)
		PicTransfer(nPic, matImg, pPic->lFix, pPaper->pModel->vecPaperModel[nPic]->lFix, inverseMat);
#endif

#ifdef Test_ShowOriPosition
	cv::Mat	inverseMat(2, 3, CV_32FC1);
	GetInverseMat(pPic->lFix, pPaper->pModel->vecPaperModel[nPic]->lFix, inverseMat);
#endif
	cv::Mat tmp = matImg;
	cv::Mat tmp2 = matImg.clone();
	if (pPaper->pModel)
	{
		RECTLIST::iterator itHTracker = pPaper->pModel->vecPaperModel[nPic]->lSelHTracker.begin();
		for (int j = 0; itHTracker != pPaper->pModel->vecPaperModel[nPic]->lSelHTracker.end(); itHTracker++, j++)
		{
			cv::Rect rt = (*itHTracker).rt;

			rectangle(tmp, rt, CV_RGB(25, 200, 20), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
		RECTLIST::iterator itVTracker = pPaper->pModel->vecPaperModel[nPic]->lSelVTracker.begin();
		for (int j = 0; itVTracker != pPaper->pModel->vecPaperModel[nPic]->lSelVTracker.end(); itVTracker++, j++)
		{
			cv::Rect rt = (*itVTracker).rt;

			rectangle(tmp, rt, CV_RGB(25, 200, 20), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
	}

	if (pPaper->pModel)
	{
		RECTLIST::iterator itSelRoi = pPaper->pModel->vecPaperModel[nPic]->lSelFixRoi.begin();													//��ʾʶ�𶨵��ѡ����
		for (int j = 0; itSelRoi != pPaper->pModel->vecPaperModel[nPic]->lSelFixRoi.end(); itSelRoi++, j++)
		{
			cv::Rect rt = (*itSelRoi).rt;

			char szCP[20] = { 0 };
			rectangle(tmp, rt, CV_RGB(0, 0, 255), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
	}


	RECTLIST::iterator itPicFix = pPic->lFix.begin();														//��ʾʶ������Ķ���
	for (int j = 0; itPicFix != pPic->lFix.end(); itPicFix++, j++)
	{
		cv::Rect rt = (*itPicFix).rt;

		char szCP[20] = { 0 };
		sprintf_s(szCP, "R_F%d", j);
		putText(tmp, szCP, cv::Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
		rectangle(tmp, rt, CV_RGB(0, 255, 0), 2);
		rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
	}
	if (pPaper->pModel)
	{
		RECTLIST::iterator itFixRect = pPaper->pModel->vecPaperModel[nPic]->lFix.begin();								//��ʾģ���ϵĶ����Ӧ�����Ծ��ϵ��¶���
		for (int j = 0; itFixRect != pPaper->pModel->vecPaperModel[nPic]->lFix.end(); itFixRect++, j++)
		{
			cv::Rect rt = (*itFixRect).rt;

			char szCP[20] = { 0 };
			sprintf_s(szCP, "M_F%d", j);
			putText(tmp, szCP, cv::Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
	}

	RECTLIST::iterator itNormal = pPic->lNormalRect.begin();													//��ʾʶ�������ĵ�
	for (int j = 0; itNormal != pPic->lNormalRect.end(); itNormal++, j++)
	{
		cv::Rect rt = (*itNormal).rt;

		char szCP[20] = { 0 };
		if (itNormal->eCPType == SN || itNormal->eCPType == OMR)
		{
			rectangle(tmp, rt, CV_RGB(0, 255, 0), 2);
			rectangle(tmp2, rt, CV_RGB(155, 233, 70), -1);
		}
		else
		{
			rectangle(tmp, rt, CV_RGB(50, 255, 55), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
	}
	RECTLIST::iterator itIssue = pPic->lIssueRect.begin();													//��ʾʶ���쳣�ĵ�
	for (int j = 0; itIssue != pPic->lIssueRect.end(); itIssue++, j++)
	{
		cv::Rect rt = (*itIssue).rt;
		
		char szCP[20] = { 0 };
		sprintf_s(szCP, "E%d", j);
		putText(tmp, szCP, cv::Point(rt.x, rt.y - 5), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255));	//CV_FONT_HERSHEY_COMPLEX
		rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
		rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
	}

	addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
}

BOOL CModifyZkzhDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)	//pMsg->wParam == VK_ESCAPE
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_ESCAPE)
		{
			if (!ReleaseData())
				return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CModifyZkzhDlg::OnTcnSelchangeTabZkzhpic(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	int nIndex = m_tabPicShowCtrl.GetCurSel();
	m_nCurrTabSel = nIndex;

	m_pCurrentPicShow = m_vecPicShow[nIndex];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != nIndex)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}


void CModifyZkzhDlg::OnNMDblclkListZkzh(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem < 0)
		return;

	for (int i = 0; i < m_lcZkzh.GetColumns(); i++)
		if (!m_lcZkzh.GetModified(m_nCurrentSelItem, i))
			m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, crOldText, crOldBackground);
		else
			m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, RGB(255, 0, 0), crOldBackground);

//	m_lcZkzh.SetItemState(m_nCurrentSelItem, 0, LVIS_DROPHILITED);
	m_nCurrentSelItem = pNMItemActivate->iItem;
	ShowPaperByItem(pNMItemActivate->iItem);

	VagueSearch(pNMItemActivate->iItem);	//ģ����������

	*pResult = 0;
}


void CModifyZkzhDlg::OnNMHoverListZkzh(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;		//**********	�����������Ӧ��ͬʱ���ؽ��ֵ��Ϊ1�Ļ���	****************
						//**********	�ͻ��������TRACK SELECT��Ҳ���������ͣ	****************
						//**********	һ��ʱ����������Զ���ѡ��
}

LRESULT CModifyZkzhDlg::OnEditEnd(WPARAM nItem, LPARAM nSubItem)
{
	if (nItem >= 0 && nSubItem >= 0)
	{
		USES_CONVERSION;
		CString strText = m_lcZkzh.GetItemText(nItem, nSubItem);
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);
		pPaper->strSN = T2A(strText);
		m_strCurZkzh = strText;
		GetDlgItem(IDC_STATIC_Zkzh_S3)->SetWindowText(A2T(std::string(pPaper->strStudentInfo + ":").c_str()));

//		m_lcZkzh.SetItemState(nItem, 0, LVIS_DROPHILITED);
		for (int i = 0; i < m_lcZkzh.GetColumns(); i++)						//ȡ��������ʾ(�ֶ����ñ�����ɫ)
			if(!m_lcZkzh.GetModified(m_nCurrentSelItem, i))
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
		ShowPaperZkzhPosition(pPaper);

		UpdateData(FALSE);
		m_lcZkzh.Invalidate();
// 		POINT pt;
// 		pt.x = 0;
// 		pt.y = 0;
// 		if (nItem < m_lcZkzh.GetItemCount() - 1)	//������һ��
// 		{
// 			m_lcZkzh.SetItemState(nItem, 0, LVIS_DROPHILITED);
// 			m_nCurrentSelItem = nItem + 1;
// 			ShowPaperByItem(m_nCurrentSelItem);
// 		}
	}
	return 0;
}



LRESULT CModifyZkzhDlg::OnLBtnDownEdit(WPARAM nItem, LPARAM nSubItem)
{
	if (nItem >= 0 && nSubItem >= 0)
	{
		USES_CONVERSION;
		CString strText = m_lcZkzh.GetItemText(nItem, nSubItem);
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);
		m_strCurZkzh = strText;
		GetDlgItem(IDC_STATIC_Zkzh_S3)->SetWindowText(A2T(std::string(pPaper->strStudentInfo + ":").c_str()));

		//		m_lcZkzh.SetItemState(nItem, 0, LVIS_DROPHILITED);
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
		ShowPaperZkzhPosition(pPaper);

		UpdateData(FALSE);
		m_lcZkzh.Invalidate();
	}
	return 0;
}

void CModifyZkzhDlg::OnBnClickedBtnSave()
{
	UpdateData(TRUE);
	USES_CONVERSION;
	std::string strOldVal = m_pCurrentShowPaper->strSN;
	m_pCurrentShowPaper->strSN = T2A(m_strCurZkzh);
	CString strText = A2T(m_pCurrentShowPaper->strSN.c_str());

//	m_lcZkzh.SetItemState(m_nCurrentSelItem, 0, LVIS_DROPHILITED);
	for (int i = 0; i < m_lcZkzh.GetColumns(); i++)						//ȡ��������ʾ(�ֶ����ñ�����ɫ)
		if (!m_lcZkzh.GetModified(m_nCurrentSelItem, i))
			m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, crOldText, crOldBackground);
		else
			m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, RGB(255, 0, 0), crOldBackground);
#ifdef NewListModelTest
		COLORREF crText, crBackground;
		m_lcZkzh.GetItemColors(m_nCurrentSelItem, 1, crText, crBackground);
		if (strOldVal != m_pCurrentShowPaper->strSN)
		{
			m_lcZkzh.SetItemText(m_nCurrentSelItem, 1, strText, RGB(255, 0, 0), crBackground);
			m_lcZkzh.SetModified(m_nCurrentSelItem, 1, TRUE);
			m_pCurrentShowPaper->bModifyZKZH = true;
		}
		else
		{
			m_lcZkzh.SetItemText(m_nCurrentSelItem, 1, strText, RGB(0, 0, 0), crBackground);
		}
#else
	COLORREF crText, crBackground;
	m_lcZkzh.GetItemColors(m_nCurrentSelItem, 2, crText, crBackground);
	if (strOldVal != m_pCurrentShowPaper->strSN)
	{
		m_lcZkzh.SetItemText(m_nCurrentSelItem, 2, strText, RGB(255, 0, 0), crBackground);
		m_lcZkzh.SetModified(m_nCurrentSelItem, 2, TRUE);
		m_pCurrentShowPaper->bModifyZKZH = true;
	}
	else
	{
		m_lcZkzh.SetItemText(m_nCurrentSelItem, 2, strText, RGB(0, 0, 0), crBackground);
	}
#endif

	if (m_nCurrentSelItem < m_lcZkzh.GetColumns())
		m_nCurrentSelItem = m_nCurrentSelItem + 1;
	else
		m_nCurrentSelItem = 0;
	ShowPaperByItem(m_nCurrentSelItem);
}


void CModifyZkzhDlg::OnBnClickedRadioSearchzkzh()
{
	if (((CButton*)GetDlgItem(IDC_RADIO_SearchZkzh))->GetCheck())
	{
		m_nSearchType = 2;
		m_strSearchKey = _T("");
		m_lcBmk.DeleteAllItems();
		UpdateData(FALSE);
	}
}


void CModifyZkzhDlg::OnBnClickedRadioSearchname()
{
	if (((CButton*)GetDlgItem(IDC_RADIO_SearchName))->GetCheck())
	{
		m_nSearchType = 1;
		m_strSearchKey = _T("");
		m_lcBmk.DeleteAllItems();
		UpdateData(FALSE);
	}
}


void CModifyZkzhDlg::OnBnClickedBtnSearch()
{
	UpdateData(TRUE);
	USES_CONVERSION;
	m_lcBmk.DeleteAllItems();
	std::string strKey = T2A(m_strSearchKey);
	STUDENT_LIST lResult;
	std::string strTable = Poco::format("T%d_%d", m_pModel->nExamID, m_pModel->nSubjectID);
	if (m_pStudentMgr && m_pStudentMgr->SearchStudent(strTable, strKey, m_nSearchType, lResult))
	{
		for (auto obj : lResult)
		{
			int nCount = m_lcBmk.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", nCount + 1);
			m_lcBmk.InsertItem(nCount, NULL);

			m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
			m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(obj.strName.c_str()));
			m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(obj.strZkzh.c_str()));
			m_lcBmk.SetItemText(nCount, 3, (LPCTSTR)A2T(obj.strClassroom.c_str()));
			m_lcBmk.SetItemText(nCount, 4, (LPCTSTR)A2T(obj.strSchool.c_str()));
		}
	}
	else
	{
		AfxMessageBox(_T("����ʧ��"));
	}
}


void CModifyZkzhDlg::OnNMDblclkListZkzhsearchresult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	USES_CONVERSION;
	CString strZkzh = m_lcBmk.GetItemText(pNMItemActivate->iItem, 2);
	m_pCurrentShowPaper->strSN = T2A(strZkzh);
	m_pCurrentShowPaper->bModifyZKZH = true;
	//��Ҫˢ��δʶ��׼��֤���б�
	

	COLORREF crText, crBackground;
	m_lcZkzh.GetItemColors(m_nCurrentSelItem, 2, crText, crBackground);
	m_lcZkzh.SetItemText(m_nCurrentSelItem, 2, strZkzh, RGB(255, 0, 0), crBackground);
	
}

bool CModifyZkzhDlg::ReleaseData()
{
	if (m_pVagueSearchDlg)
	{
		m_pVagueSearchDlg->DestroyWindow();
		SAFE_RELEASE(m_pVagueSearchDlg);
	}
	
	int nCount = m_lcZkzh.GetItemCount();
	if (g_nZkzhNull2Issue == 1)
	{
		for (int i = 0; i < nCount; i++)
		{
			pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
			if (pPaper->strSN.empty())
			{
				if (MessageBox(_T("����׼��֤��Ϊ�յĿ�����������޸ģ���Ӱ��˿����������������Ƿ���ԣ�"), _T("����"), MB_YESNO) != IDYES)
					return false;
				break;
			}
		}
	}
#ifdef NewListModelTest
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
		if (m_lcZkzh.GetCheckbox(i, 2))
			pPaper->bReScan = true;			//���ô��Ծ���Ҫ����ɨ��
		else
			pPaper->bReScan = false;
	}
#else
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
		if (m_lcZkzh.GetCheckbox(i, 3))
			pPaper->bReScan = true;			//���ô��Ծ���Ҫ����ɨ��
		else
			pPaper->bReScan = false;
	}
#endif

	//������Ծ��Ѿ����޸��������������б�ɾ��
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

	return true;
}

bool CModifyZkzhDlg::VagueSearch(int nItem)
{
	bool bResult = false;
	if (nItem < 0)
		return bResult;
	if (nItem >= m_lcZkzh.GetItemCount())
		return bResult;
	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);
	if (pPaper->strRecogSN4Search.empty())
		return bResult;

#if 1
	bResult = m_pVagueSearchDlg->vagueSearch(pPaper);
#else
	//��ģ�������ֶα�ɺϷ���sql�ֶ�
	std::string strVagueKey = pPaper->strRecogSN4Search;
	char szVagueKey[50] = { 0 };
	char * p = szVagueKey;
	bool bLastIsSharp = false;	//��ʶ��һ���ַ��Ƿ���#�����ڴ���������#�ַ��������#���һ��
	for (auto p0 : pPaper->strRecogSN4Search)
	{
		if (p0 == '#')
		{
			if (bLastIsSharp)
				continue;
			*p = '%';
			bLastIsSharp = true;
		}
		else
		{
			*p = p0;
			bLastIsSharp = false;
		}
		++p;
	}
	TRACE("����ģ�����ҵ��ַ���: %s\n", szVagueKey);
	m_lcBmk.DeleteAllItems();
	STUDENT_LIST lResult;
	std::string strTable = Poco::format("T%d_%d", m_pModel->nExamID, m_pModel->nSubjectID);
	if (m_pStudentMgr && m_pStudentMgr->SearchStudent(strTable, szVagueKey, m_nSearchType, lResult))
	{
		USES_CONVERSION;
		for (auto obj : lResult)
		{
			int nCount = m_lcBmk.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", nCount + 1);
			m_lcBmk.InsertItem(nCount, NULL);

			m_lcBmk.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
			m_lcBmk.SetItemText(nCount, 1, (LPCTSTR)A2T(obj.strName.c_str()));
			m_lcBmk.SetItemText(nCount, 2, (LPCTSTR)A2T(obj.strZkzh.c_str()));
			m_lcBmk.SetItemText(nCount, 3, (LPCTSTR)A2T(obj.strClassroom.c_str()));
			m_lcBmk.SetItemText(nCount, 4, (LPCTSTR)A2T(obj.strSchool.c_str()));
		}
		bResult = true;
	}
#endif
	return bResult;
}

void CModifyZkzhDlg::OnClose()
{
	if (!ReleaseData())
		return;

	CDialog::OnClose();
}


BOOL CModifyZkzhDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}


HBRUSH CModifyZkzhDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_Zkzh_S1)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
// 	if (CurID == IDC_STATIC_Group || CurID == IDC_RADIO_SearchName || CurID == IDC_RADIO_SearchZkzh || CurID == IDC_STATIC_DB_Search)
// 	{
// 		//		pDC->SetBkColor(RGB(255, 255, 255));
// 		pDC->SetBkMode(TRANSPARENT);
// 		return (HBRUSH)GetStockObject(NULL_BRUSH);
// 	}
	return hbr;
}

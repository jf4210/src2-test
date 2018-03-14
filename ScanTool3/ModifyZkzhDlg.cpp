// ModifyZkzhDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ModifyZkzhDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"
#include "ScanTool3Dlg.h"
#include "OmrRecog.h"
// CModifyZkzhDlg �Ի���

IMPLEMENT_DYNAMIC(CModifyZkzhDlg, CDialog)

CModifyZkzhDlg::CModifyZkzhDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent /*=NULL*/)
: CDialog(CModifyZkzhDlg::IDD, pParent)
	, m_pModel(pModel), m_pPapers(pPapersInfo), m_pCurrentShowPaper(NULL), m_pVagueSearchDlg(NULL), m_pShowPicDlg(NULL)
	, m_nCurrentSelItem(0), m_pStudentMgr(pStuMgr), m_pDefShowPaper(pShowPaper)
#ifdef TEST_EXCEPTION_DLG
	, m_pZkzhShowMgrDlg(NULL)
#endif
{

}

CModifyZkzhDlg::~CModifyZkzhDlg()
{
}

void CModifyZkzhDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Zkzh, m_lcZkzh);
	//DDX_Text(pDX, IDC_EDIT_Zkzh, m_strCurZkzh);
	DDX_Control(pDX, IDC_BTN_Back, m_bmpBtnReturn);
}


BOOL CModifyZkzhDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	InitData();
	ShowPaperByItem(m_nCurrentSelItem);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CModifyZkzhDlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Zkzh, &CModifyZkzhDlg::OnNMDblclkListZkzh)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Zkzh, &CModifyZkzhDlg::OnNMHoverListZkzh)
	ON_REGISTERED_MESSAGE(WM_XLISTCTRL_EDIT_END, OnEditEnd)
	ON_REGISTERED_MESSAGE(WM_XLISTCTRL_LBUTTONDOWN_EDIT_CLICKED, OnLBtnDownEdit)
//	ON_BN_CLICKED(IDC_BTN_SAVE_ModifiSnDlg, &CModifyZkzhDlg::OnBnClickedBtnSave)
	ON_MESSAGE(WM_CV_RBTNUP, &CModifyZkzhDlg::RoiRBtnUp)
	ON_COMMAND(ID_LeftRotate, &CModifyZkzhDlg::LeftRotate)
	ON_COMMAND(ID_RightRotate, &CModifyZkzhDlg::RightRotate)
	ON_COMMAND(ID_PicsExchange, &CModifyZkzhDlg::PicsExchange)
	ON_MESSAGE(MSG_ZKZH_RECOG, &CModifyZkzhDlg::MsgZkzhRecog)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_Back, &CModifyZkzhDlg::OnBnClickedBtnBack)
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CModifyZkzhDlg ��Ϣ�������
void CModifyZkzhDlg::ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper /*= NULL*/)
{
	m_pModel			= pModel;
	m_pPapers			= pPapersInfo;
	m_pStudentMgr		= pStuMgr;
	m_pCurrentShowPaper = pShowPaper;
	m_pDefShowPaper		= pShowPaper;

	if (_bGetBmk_)
	{
		m_lBmkStudent.clear();
		m_lBmkStudent = g_lBmkStudent;
	}

	if (!m_pVagueSearchDlg)
	{
		m_pVagueSearchDlg = new CVagueSearchDlg();
		m_pVagueSearchDlg->Create(CVagueSearchDlg::IDD, this);
		m_pVagueSearchDlg->ShowWindow(SW_SHOW);
		TRACE("***************===========	new CVagueSearchDlg()2 ===========*************************\n");
	}
	if (!m_pShowPicDlg)
	{
		m_pShowPicDlg = new CShowPicDlg();
		m_pShowPicDlg->Create(CShowPicDlg::IDD, this);
		m_pShowPicDlg->ShowWindow(SW_SHOW);
	}
	m_pShowPicDlg->setShowModel(2);
	m_pVagueSearchDlg->setExamInfo(m_pStudentMgr, m_pModel);

#ifdef TEST_EXCEPTION_DLG
	if (!m_pZkzhShowMgrDlg)
	{
		m_pZkzhShowMgrDlg = new CZkzhShowMgrDlg(m_pModel, m_pPapers, m_pStudentMgr, m_pDefShowPaper);
		m_pZkzhShowMgrDlg->Create(IDD_ZKZHSHOWMGRDLG, this);
		m_pZkzhShowMgrDlg->ShowWindow(SW_SHOW);
	}
	m_pZkzhShowMgrDlg->ReInitData(pModel, pPapersInfo, pStuMgr, pShowPaper);
	m_pZkzhShowMgrDlg->SetDlgInfo(m_pShowPicDlg, m_pVagueSearchDlg);
#endif

	InitData();
	ShowPaperByItem(m_nCurrentSelItem);
	InitCtrlPosition();
}

void CModifyZkzhDlg::InitUI()
{
	m_lcZkzh.SetExtendedStyle(m_lcZkzh.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcZkzh.InsertColumn(0, _T("����"), LVCFMT_CENTER, 50);
	m_lcZkzh.InsertColumn(1, _T("׼��֤��(�ɱ༭)"), LVCFMT_CENTER, 110);
	m_lcZkzh.InsertColumn(2, _T("ɾ����ɨ"), LVCFMT_CENTER, 80);	//��ɨ��ʶ
	m_lcZkzh.InsertColumn(3, _T("��ע"), LVCFMT_LEFT, 150);

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
	m_lcZkzh.EnableToolTips(TRUE);

	m_bmpBtnReturn.SetStateBitmap(IDB_RecordDlg_Btn, 0, IDB_RecordDlg_Btn_Hover);
	m_bmpBtnReturn.SetWindowText(_T("����"));

	if (!m_pShowPicDlg)
	{
		m_pShowPicDlg = new CShowPicDlg();
		m_pShowPicDlg->Create(CShowPicDlg::IDD, this);
		m_pShowPicDlg->ShowWindow(SW_SHOW);
	}
	m_pShowPicDlg->setShowModel(2);
	if (!m_pVagueSearchDlg)
	{
		m_pVagueSearchDlg = new CVagueSearchDlg();
		m_pVagueSearchDlg->Create(CVagueSearchDlg::IDD, this);
		m_pVagueSearchDlg->ShowWindow(SW_SHOW);
		TRACE("***************===========	new CVagueSearchDlg()1 ===========*************************\n");
	}
	m_pVagueSearchDlg->setExamInfo(m_pStudentMgr, m_pModel);

#ifdef TEST_EXCEPTION_DLG
	if (!m_pZkzhShowMgrDlg)
	{
		m_pZkzhShowMgrDlg = new CZkzhShowMgrDlg(m_pModel, m_pPapers, m_pStudentMgr, m_pDefShowPaper);
		m_pZkzhShowMgrDlg->Create(IDD_ZKZHSHOWMGRDLG, this);
		m_pZkzhShowMgrDlg->ShowWindow(SW_SHOW);
	}
	m_pZkzhShowMgrDlg->SetDlgInfo(m_pShowPicDlg, m_pVagueSearchDlg);

	m_lcZkzh.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_Zkzh_S1)->ShowWindow(SW_HIDE);
#endif

	InitCtrlPosition();
}

void CModifyZkzhDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = 30;	//�ϱߵļ��������������
	const int nLeftGap = 20;		//��ߵĿհ׼��
	const int nBottomGap = 20;	//�±ߵĿհ׼��
	const int nRightGap = 20;	//�ұߵĿհ׼��
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
	int nVagueSearchDlgH = (cy - nTopGap - nBottomH - nBottomGap) * 0.4;
	if (nVagueSearchDlgH < 250)
		nVagueSearchDlgH = 250;
	if (nVagueSearchDlgH > 300)
		nVagueSearchDlgH = 300;

//	int nZkzhLCH = cy - nTopGap - nStaticTip - nGap - nGap - nGroupH - nGap - nStaticTip - nGap - 150 - nBottomH - nBottomGap;
	int nZkzhLCH = cy - nTopGap - nStaticTip - nGap - nVagueSearchDlgH - nBottomH - nBottomGap;
	if (m_lcZkzh.GetSafeHwnd())
	{
		m_lcZkzh.MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nZkzhLCH);
//		nCurrentLeft += (nListCtrlWidth + nGap);
		nCurrentTop += (nZkzhLCH + nGap);
	}
	//�������ѯ
	if (m_pVagueSearchDlg && m_pVagueSearchDlg->GetSafeHwnd())
	{
//		int nH = cy - nBottomGap - nCurrentTop;
		m_pVagueSearchDlg->MoveWindow(nCurrentLeft, nCurrentTop, nListCtrlWidth, nVagueSearchDlgH);
	}

#ifdef TEST_EXCEPTION_DLG
	if (m_pZkzhShowMgrDlg && m_pZkzhShowMgrDlg->GetSafeHwnd())
	{
		int nH = cy - nTopGap - nGap - nVagueSearchDlgH - nBottomH - nBottomGap;
		m_pZkzhShowMgrDlg->MoveWindow(nCurrentLeft, nTopGap, nListCtrlWidth, nH);
	}
#endif

	//tab
	nCurrentLeft = nLeftGap + nListCtrlWidth + nGap;
	nCurrentTop = nTopGap;
	int nPicShowTabCtrlWidth = cx - nLeftGap - nRightGap - nListCtrlWidth - nGap - nGap;
	int nPicShowTabCtrlHigh = cy - nTopGap - nBottomH - nBottomGap;

	if (m_pShowPicDlg && m_pShowPicDlg->GetSafeHwnd())
		m_pShowPicDlg->MoveWindow(nCurrentLeft, nTopGap, nPicShowTabCtrlWidth, nPicShowTabCtrlHigh);

	if (GetDlgItem(IDC_BTN_Back)->GetSafeHwnd())
	{
		int nBtnW = 40;
		int nBtnH = nTopGap - 5;
		GetDlgItem(IDC_BTN_Back)->MoveWindow(cx - nRightGap - nBtnW, 5, nBtnW, nBtnH);
	}
	Invalidate();
}

void CModifyZkzhDlg::InitData()
{
	if (NULL == m_pPapers)
		return;

	bool bFindFirstShow = false;
	m_lcZkzh.DeleteAllItems();

	USES_CONVERSION;
// 	m_vecCHzkzh.clear();
// 	//------------------------����ȡ�غŵĿ���
// 	for (auto pPaper : m_pPapers->lPaper)
// 	{
// 		if (pPaper->nZkzhInBmkStatus == -1)
// 		{
// 			bool bFind = false;
// 			for (auto sn : m_vecCHzkzh)
// 			{
// 				if (sn == pPaper->strSN)
// 				{
// 					bFind = true;
// 					break;
// 				}
// 			}
// 			if (!bFind) m_vecCHzkzh.push_back(pPaper->strSN);	//�غŵĿ��ŷ��������У���Ҫȥ��
// 		}
// 	}
// 	for (auto pPaper : m_pPapers->lIssue)
// 	{
// 		if (pPaper->nZkzhInBmkStatus == -1)
// 		{
// 			bool bFind = false;
// 			for (auto sn : m_vecCHzkzh)
// 			{
// 				if (sn == pPaper->strSN)
// 				{
// 					bFind = true;
// 					break;
// 				}
// 			}
// 			if (!bFind) m_vecCHzkzh.push_back(pPaper->strSN);	//�غŵĿ��ŷ��������У���Ҫȥ��
// 		}
// 	}
// 	//------------------------

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

			int nReScan = 0;
			if (pPaper->bReScan) nReScan = 1;
			m_lcZkzh.SetItemText(nCount, 2, _T(""));	//��ɨ
			m_lcZkzh.SetCheckbox(nCount, 2, nReScan);
			m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
			m_lcZkzh.SetEdit(nCount, 1);

			//��ʾ��ע��Ϣ��Ϊ�γ����ڴ��б�
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
			m_lcZkzh.SetItemText(nCount, 3, (LPCTSTR)A2T(strDetailInfo.c_str()));

			if (pPaper == m_pDefShowPaper)
			{
				bFindFirstShow = true;
				m_nCurrentSelItem = nCount;
			}

			CString strTips = _T("˫����ʾ�˿����Ծ�");
			m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
			m_lcZkzh.SetItemToolTipText(nCount, 3, (LPCTSTR)strTips);
			strTips = _T("����޸�׼��֤��");
			m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
			strTips = _T("��ѡ�������Ծ���Ҫ����ɨ��");
			m_lcZkzh.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
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

		int nReScan = 0;
		if (pPaper->bReScan) nReScan = 1;
		m_lcZkzh.SetItemText(nCount, 2, _T(""));	//��ɨ
		m_lcZkzh.SetCheckbox(nCount, 2, nReScan);
		m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
		m_lcZkzh.SetEdit(nCount, 1);

		//��ʾ��ע��Ϣ��Ϊ�γ����ڴ��б�
		std::string strDetailInfo;
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

		m_lcZkzh.SetItemText(nCount, 3, (LPCTSTR)A2T(strDetailInfo.c_str()));

		if (pPaper == m_pDefShowPaper)
		{
			bFindFirstShow = true;
			m_nCurrentSelItem = nCount;
		}

		CString strTips = _T("˫����ʾ�˿����Ծ�");
		m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
		m_lcZkzh.SetItemToolTipText(nCount, 3, (LPCTSTR)strTips);
		strTips = _T("����޸�׼��֤��");
		m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
		strTips = _T("��ѡ�������Ծ���Ҫ����ɨ��");
		m_lcZkzh.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
	}

	if (!bFindFirstShow)
		m_nCurrentSelItem = 0;
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
	if (m_pShowPicDlg) m_pShowPicDlg->setRotate(3);
}

void CModifyZkzhDlg::RightRotate()
{
	if (m_pShowPicDlg) m_pShowPicDlg->setRotate(2);
}

void CModifyZkzhDlg::PicsExchange()
{
	if (!m_pModel)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "ģ�岻����");
		dlg.DoModal();		
		return;
	}

	if (m_pModel->nPicNum != 2)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "ֻ�����һ��ѧ��2��ͼƬʱ�ſɵ���");
		dlg.DoModal();
		return;
	}

	if (!m_pCurrentShowPaper)
		return;

	//ͼ��������
	bool bPicsExchangeSucc = true;
	pST_PicInfo pPic1 = *(m_pCurrentShowPaper->lPic.begin());
	pST_PicInfo pPic2 = *(m_pCurrentShowPaper->lPic.rbegin());
	try
	{
// 		std::string strBasePath = pPic1->strPicPath.substr(0, pPic1->strPicPath.rfind("\\") + 1);
// 		std::string strTmpName = pPic1->strPicName;
		Poco::File fPic1(pPic1->strPicPath);
		fPic1.renameTo(pPic1->strPicPath + "_tmp");

		Poco::File fPic2(pPic2->strPicPath);
		fPic2.renameTo(pPic1->strPicPath);

		fPic1.renameTo(pPic2->strPicPath);
	}
	catch (Poco::Exception &e)
	{
		bPicsExchangeSucc = false;
		std::string strErr = e.displayText();

		TRACE("ͼ�����������ʧ��: %s\n", strErr.c_str());
	}

	if (!bPicsExchangeSucc)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "2��ͼƬ����ʧ�ܣ���ɾ����ɨ");
		dlg.DoModal();
		return;
	}

	//��������
	cv::Mat matSrc1 = cv::imread(pPic1->strPicPath);
	COmrRecog omrObj;
	int nResult1 = omrObj.GetRightPicOrientation(matSrc1, 0, _nDoubleScan_ == 0 ? false : true);
	std::string strPicPath1 = pPic1->strPicPath;
	if (nResult1 >= 2 && nResult1 <= 4)
		imwrite(strPicPath1, matSrc1);

	cv::Mat matSrc2 = cv::imread(pPic2->strPicPath);
	int nResult2 = omrObj.GetRightPicOrientation(matSrc2, 1, _nDoubleScan_ == 0 ? false : true);
	std::string strPicPath2 = pPic2->strPicPath;
	if (nResult2 >= 2 && nResult2 <= 4)
		imwrite(strPicPath2, matSrc2);

// 	m_pCurrentShowPaper->lPic.clear();
// 	m_pCurrentShowPaper->lPic.push_back(pPic2);
// 	m_pCurrentShowPaper->lPic.push_back(pPic1);

	m_pCurrentShowPaper->nPicsExchange++;		//��¼��ǰ�Ծ���������
	m_pCurrentShowPaper->pSrcDlg = this;
	m_pCurrentShowPaper->bIssuePaper = false;
	m_pCurrentShowPaper->bRecogCourse = true;
	m_pCurrentShowPaper->nQKFlag = 0;
	m_pCurrentShowPaper->nWJFlag = 0;
	m_pCurrentShowPaper->bRecogComplete = false;
	m_pCurrentShowPaper->strSN = "";
	m_pCurrentShowPaper->strRecogSN4Search = "";
	m_pCurrentShowPaper->lOmrResult.clear();
	m_pCurrentShowPaper->lElectOmrResult.clear();
	for (auto itSn : m_pCurrentShowPaper->lSnResult)
	{
		pSN_ITEM pSNItem = itSn;
		SAFE_RELEASE(pSNItem);
	}
	m_pCurrentShowPaper->lSnResult.clear();


	pPic1->bFindIssue = false;
	pPic1->nRecoged = 0;
	pPic1->bRecogCourse = true;
	pPic1->nQKFlag = 0;
	pPic1->nWJFlag = 0;
	pPic1->lFix.clear();
	pPic1->lIssueRect.clear();
	pPic1->lNormalRect.clear();
	pPic1->lOmrResult.clear();
	pPic1->lElectOmrResult.clear();

	pPic2->bFindIssue = false;
	pPic2->nRecoged = 0;
	pPic2->bRecogCourse = true;
	pPic2->nQKFlag = 0;
	pPic2->nWJFlag = 0;
	pPic2->lFix.clear();
	pPic2->lIssueRect.clear();
	pPic2->lNormalRect.clear();
	pPic2->lOmrResult.clear();
	pPic2->lElectOmrResult.clear();

	pRECOGTASK pTask = new RECOGTASK;
	pTask->pPaper = m_pCurrentShowPaper;
	g_lRecogTask.push_back(pTask);
}

void CModifyZkzhDlg::ShowPaperByItem(int nItem)
{
	if (nItem < 0)
		return;
	if (m_lcZkzh.GetItemCount() <= 0 || nItem >= m_lcZkzh.GetItemCount())
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(nItem);
	if (pPaper == NULL)
		return;
#if 1
	m_pCurrentShowPaper = pPaper;
	m_lcZkzh.GetItemColors(nItem, 0, crOldText, crOldBackground);
	for (int i = 0; i < m_lcZkzh.GetColumns(); i++)							//���ø�����ʾ(�ֶ����ñ�����ɫ)
		m_lcZkzh.SetItemColors(nItem, i, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255

	//USES_CONVERSION;
	//m_strCurZkzh = pPaper->strSN.c_str();
	//GetDlgItem(IDC_STATIC_Zkzh_S3)->SetWindowText(A2T(std::string(pPaper->strStudentInfo + ":").c_str()));
	//GetDlgItem(IDC_EDIT_Zkzh)->SetFocus();

	m_pShowPicDlg->setShowPaper(pPaper);
#else
	m_pCurrentShowPaper = pPaper;
	ShowPaperZkzhPosition(pPaper);
// 	m_lcZkzh.SetItemState(nItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
// 	m_lcZkzh.SetSelectionMark(nItem);
// 	m_lcZkzh.SetFocus();
// 	m_lcZkzh.SetItemState(nItem, LVIS_DROPHILITED, LVIS_DROPHILITED);		//������ʾһ�У�ʧȥ�����Ҳһֱ��ʾ
	
	m_lcZkzh.GetItemColors(nItem, 0, crOldText, crOldBackground);
	for (int i = 0; i < m_lcZkzh.GetColumns(); i++)							//���ø�����ʾ(�ֶ����ñ�����ɫ)
		m_lcZkzh.SetItemColors(nItem, i, RGB(0, 0, 0), RGB(70, 70, 255));

// 	USES_CONVERSION;
// 	m_strCurZkzh = pPaper->strSN.c_str();
// 	GetDlgItem(IDC_STATIC_Zkzh_S3)->SetWindowText(A2T(std::string(pPaper->strStudentInfo + ":").c_str()));
// 	GetDlgItem(IDC_EDIT_Zkzh)->SetFocus();
#endif
	UpdateData(FALSE);
	m_lcZkzh.Invalidate();
}

void CModifyZkzhDlg::SetZkzhStatus()
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
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CModifyZkzhDlg::OnNMDblclkListZkzh(NMHDR *pNMHDR, LRESULT *pResult)
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
//	m_lcZkzh.SetItemState(m_nCurrentSelItem, 0, LVIS_DROPHILITED);
	m_nCurrentSelItem = pNMItemActivate->iItem;
	ShowPaperByItem(pNMItemActivate->iItem);

	VagueSearch(pNMItemActivate->iItem);	//ģ����������

#if 0	//test
	POINT pt;
	GetCursorPos(&pt);
	CRect rcClient;
	rcClient.left = pt.x;
	rcClient.top = pt.y;
	rcClient.right = rcClient.left + 350;
	rcClient.bottom = rcClient.top + 350;
	m_pVagueSearchDlg->MoveWindow(rcClient);
#endif

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
// 		m_strCurZkzh = strText;
// 		GetDlgItem(IDC_STATIC_Zkzh_S3)->SetWindowText(A2T(std::string(pPaper->strStudentInfo + ":").c_str()));

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
		if (m_pShowPicDlg)
			m_pShowPicDlg->setShowPaper(pPaper);

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
		if (pPaper == NULL)
			return 0;
// 		m_strCurZkzh = strText;
// 		GetDlgItem(IDC_STATIC_Zkzh_S3)->SetWindowText(A2T(std::string(pPaper->strStudentInfo + ":").c_str()));

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
		if (m_pShowPicDlg)
			m_pShowPicDlg->setShowPaper(pPaper);

		UpdateData(FALSE);
		m_lcZkzh.Invalidate();
	}
	return 0;
}

bool CModifyZkzhDlg::ReleaseData()
{
#ifndef TEST_EXCEPTION_DLG
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
					dlg.setShowInfo(2, 2, "���ڿ���Ϊ�յĿ����������޸ģ���Ӱ��μӺ�����������ԣ�");
					dlg.DoModal();
					if (dlg.m_nResult != IDYES)
						return false;
					break;
				}
			}
		}

		for (int i = 0; i < nCount; i++)
		{
			pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
			if (m_lcZkzh.GetCheckbox(i, 2))
				pPaper->bReScan = true;			//���ô��Ծ���Ҫ����ɨ��
			else
				pPaper->bReScan = false;
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
#endif
#ifdef TEST_EXCEPTION_DLG
	if (m_pZkzhShowMgrDlg)
	{
		if (!m_pZkzhShowMgrDlg->ReleaseData())
			return false;
		m_pZkzhShowMgrDlg->DestroyWindow();
		SAFE_RELEASE(m_pZkzhShowMgrDlg);
	}
#endif
	if (m_pVagueSearchDlg)
	{
		m_pVagueSearchDlg->DestroyWindow();
		SAFE_RELEASE(m_pVagueSearchDlg);
	}
	if (m_pShowPicDlg)
	{
		m_pShowPicDlg->DestroyWindow();
		SAFE_RELEASE(m_pShowPicDlg);
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
// 	if (pPaper->strRecogSN4Search.empty())
// 		return bResult;

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

LRESULT CModifyZkzhDlg::MsgZkzhRecog(WPARAM wParam, LPARAM lParam)
{
	pST_PaperInfo pPaper = (pST_PaperInfo)wParam;
	pPAPERSINFO   pPapers = (pPAPERSINFO)lParam;
	if (g_nOperatingMode != 1 && !g_bModifySN)
		return FALSE;
	if (_pCurrExam_->nModel)
		return FALSE;

	USES_CONVERSION;
	int nCount = m_lcZkzh.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pItemPaper = (pST_PaperInfo)(DWORD_PTR)m_lcZkzh.GetItemData(i);
		if (pItemPaper == pPaper)
		{
			if (!pPaper->strSN.empty())
			{
				m_lcZkzh.SetItemText(i, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));
				CheckZkzhInBmk(pPaper);

				//��ʾ��ע��Ϣ��Ϊ�γ����ڴ��б�
				std::string strDetailInfo;
				if (pPaper->strSN.empty())
					strDetailInfo = "ͼ�񱻵���������Ϊ��";
				else
					strDetailInfo = "ͼ�񱻵���";
				if (pPaper->bModifyZKZH)
					strDetailInfo = "ͼ�񱻵������й��޸�";
				if (_bGetBmk_)
				{
					if (pPaper->nZkzhInBmkStatus == -1)
						strDetailInfo = "ͼ�񱻵����������غ�";
					if (pPaper->nZkzhInBmkStatus == 0)
						strDetailInfo = "ͼ�񱻵��������Ų��ڱ�����";
				}
				if (pPaper->bReScan)
					strDetailInfo = "ͼ�񱻵���������Ծ�ɾ��";

				m_lcZkzh.SetItemText(i, 3, (LPCTSTR)A2T(strDetailInfo.c_str()));
			}
			else
			{
				if (pPaper->strRecogSN4Search.empty())
				{
					m_lcZkzh.SetItemText(i, 1, _T(" "));
					m_lcZkzh.SetItemText(i, 3, _T("ͼ�񱻵���������Ϊ��"));
				}
				else
				{
					m_lcZkzh.SetItemText(i, 1, _T(" "));
					m_lcZkzh.SetItemText(i, 3, _T("ͼ�񱻵���������ʶ����ȫ"));
				}
			}
			break;
		}
	}

	CNewMessageBox	dlg;
	dlg.setShowInfo(1, 1, "ͼ��������");
	dlg.DoModal();

	if (m_nCurrentSelItem < m_lcZkzh.GetItemCount())
	{
		for (int i = 0; i < m_lcZkzh.GetColumns(); i++)
		if (!m_lcZkzh.GetModified(m_nCurrentSelItem, i))
			m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, crOldText, crOldBackground);
		else
			m_lcZkzh.SetItemColors(m_nCurrentSelItem, i, RGB(255, 0, 0), crOldBackground);
	}
	ShowPaperByItem(m_nCurrentSelItem);
	return TRUE;
}

void CModifyZkzhDlg::OnClose()
{
	CDialog::OnClose();
}

void CModifyZkzhDlg::OnDestroy()
{
	if (!ReleaseData())
		return;

	CDialog::OnDestroy();
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


void CModifyZkzhDlg::OnBnClickedBtnBack()
{
	if (!ReleaseData())
		return;
	ShowWindow(SW_HIDE);
	CScanTool3Dlg* pDlg = (CScanTool3Dlg*)GetParent();
	pDlg->SwitchDlg(2);
}


int CModifyZkzhDlg::CheckZkzhInBmk(std::string strZkzh)
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

void CModifyZkzhDlg::CheckZkzhInBmk(pST_PaperInfo pPaper)
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

void CModifyZkzhDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

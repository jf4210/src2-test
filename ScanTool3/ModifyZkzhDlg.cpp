// ModifyZkzhDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ModifyZkzhDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"
#include "ScanTool3Dlg.h"
#include "OmrRecog.h"
// CModifyZkzhDlg 对话框

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


// CModifyZkzhDlg 消息处理程序
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
	m_lcZkzh.InsertColumn(0, _T("考生"), LVCFMT_CENTER, 50);
	m_lcZkzh.InsertColumn(1, _T("准考证号(可编辑)"), LVCFMT_CENTER, 110);
	m_lcZkzh.InsertColumn(2, _T("删除重扫"), LVCFMT_CENTER, 80);	//重扫标识
	m_lcZkzh.InsertColumn(3, _T("备注"), LVCFMT_LEFT, 150);

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
	m_bmpBtnReturn.SetWindowText(_T("返回"));

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

	int nTopGap = 30;	//上边的间隔，留给控制栏
	const int nLeftGap = 20;		//左边的空白间隔
	const int nBottomGap = 20;	//下边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nGroupH = 70;			//group控件高度
	int nListCtrlWidth = 350;	//图片列表控件宽度
	int nStaticTip = 15;		//列表提示static控件高度
	int nBtnH = 30;				//按钮高度
	int nBtnW = (nListCtrlWidth - nGap) * 0.3;
	int nBottomH = 10;			//底下部分的高度，用于放置按钮等
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
	//报名库查询
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
// 	//------------------------先提取重号的考号
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
// 			if (!bFind) m_vecCHzkzh.push_back(pPaper->strSN);	//重号的考号放入容器中，需要去重
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
// 			if (!bFind) m_vecCHzkzh.push_back(pPaper->strSN);	//重号的考号放入容器中，需要去重
// 		}
// 	}
// 	//------------------------

	for (auto pPaper : m_pPapers->lPaper)
	{
		if (pPaper->strSN.empty() || pPaper->bModifyZKZH || (pPaper->nZkzhInBmkStatus != 1 && _bGetBmk_) || pPaper->nPicsExchange != 0)
		{
			//添加进试卷列表控件
			int nCount = m_lcZkzh.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", pPaper->nIndex);	//nCount + 1
			m_lcZkzh.InsertItem(nCount, NULL);

			m_lcZkzh.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));	//pPaper->strStudentInfo.c_str()
			m_lcZkzh.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strSN.c_str()));

			int nReScan = 0;
			if (pPaper->bReScan) nReScan = 1;
			m_lcZkzh.SetItemText(nCount, 2, _T(""));	//重扫
			m_lcZkzh.SetCheckbox(nCount, 2, nReScan);
			m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
			m_lcZkzh.SetEdit(nCount, 1);

			//显示备注信息，为何出现在此列表
			std::string strDetailInfo;
			if (pPaper->nPicsExchange == 0)
			{
				if (pPaper->strSN.empty())
					strDetailInfo = "考号为空";
				if (pPaper->bModifyZKZH)
					strDetailInfo = "有过修改";
				if (_bGetBmk_)
				{
					if (pPaper->nZkzhInBmkStatus == -1)
						strDetailInfo = "发生重号";
					if (pPaper->nZkzhInBmkStatus == 0)
						strDetailInfo = "考号不在报名库";
				}
				if (pPaper->bReScan)
					strDetailInfo = "这份试卷将删除";
			}
			else
			{
				if (pPaper->strSN.empty())
					strDetailInfo = "图像有过调换，考号为空";
				else
					strDetailInfo = "图像有过调换";
				if (pPaper->bModifyZKZH)
					strDetailInfo = "图像有过调换，有过修改";
				if (_bGetBmk_)
				{
					if (pPaper->nZkzhInBmkStatus == -1)
						strDetailInfo = "图像有过调换，发生重号";
					if (pPaper->nZkzhInBmkStatus == 0)
						strDetailInfo = "图像有过调换，考号不在报名库";
				}
				if (!pPaper->bRecogCourse)
					strDetailInfo = "科目校验失败";
				if (pPaper->bReScan)
					strDetailInfo = "图像有过调换，这份试卷将删除";
			}
			m_lcZkzh.SetItemText(nCount, 3, (LPCTSTR)A2T(strDetailInfo.c_str()));

			if (pPaper == m_pDefShowPaper)
			{
				bFindFirstShow = true;
				m_nCurrentSelItem = nCount;
			}

			CString strTips = _T("双击显示此考生试卷");
			m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
			m_lcZkzh.SetItemToolTipText(nCount, 3, (LPCTSTR)strTips);
			strTips = _T("点击修改准考证号");
			m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
			strTips = _T("勾选此项，这份试卷将需要重新扫描");
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
		m_lcZkzh.SetItemText(nCount, 2, _T(""));	//重扫
		m_lcZkzh.SetCheckbox(nCount, 2, nReScan);
		m_lcZkzh.SetItemData(nCount, (DWORD_PTR)pPaper);
		m_lcZkzh.SetEdit(nCount, 1);

		//显示备注信息，为何出现在此列表
		std::string strDetailInfo;
		if (pPaper->bModifyZKZH)
			strDetailInfo = "有过修改";
		if (_bGetBmk_)
		{
			if (pPaper->nZkzhInBmkStatus == -1)
				strDetailInfo = "发生重号";
			if (pPaper->nZkzhInBmkStatus == 0)
				strDetailInfo = "考号不在报名库";
		}
		if (!pPaper->bRecogCourse)
			strDetailInfo = "科目校验失败";
		if (pPaper->bReScan)
			strDetailInfo = "这份试卷将删除";

		m_lcZkzh.SetItemText(nCount, 3, (LPCTSTR)A2T(strDetailInfo.c_str()));

		if (pPaper == m_pDefShowPaper)
		{
			bFindFirstShow = true;
			m_nCurrentSelItem = nCount;
		}

		CString strTips = _T("双击显示此考生试卷");
		m_lcZkzh.SetItemToolTipText(nCount, 0, (LPCTSTR)strTips);
		m_lcZkzh.SetItemToolTipText(nCount, 3, (LPCTSTR)strTips);
		strTips = _T("点击修改准考证号");
		m_lcZkzh.SetItemToolTipText(nCount, 1, (LPCTSTR)strTips);
		strTips = _T("勾选此项，这份试卷将需要重新扫描");
		m_lcZkzh.SetItemToolTipText(nCount, 2, (LPCTSTR)strTips);
	}

	if (!bFindFirstShow)
		m_nCurrentSelItem = 0;
}

LRESULT CModifyZkzhDlg::RoiRBtnUp(WPARAM wParam, LPARAM lParam)
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
		dlg.setShowInfo(2, 1, "模板不存在");
		dlg.DoModal();		
		return;
	}

	if (m_pModel->nPicNum != 2)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "只有针对一个学生2张图片时才可调换");
		dlg.DoModal();
		return;
	}

	if (!m_pCurrentShowPaper)
		return;

	//图像重命名
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

		TRACE("图像调换重命名失败: %s\n", strErr.c_str());
	}

	if (!bPicsExchangeSucc)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "2张图片调换失败，请删除重扫");
		dlg.DoModal();
		return;
	}

	//调整方向
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

	m_pCurrentShowPaper->nPicsExchange++;		//记录当前试卷被调换次数
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
	for (int i = 0; i < m_lcZkzh.GetColumns(); i++)							//设置高亮显示(手动设置背景颜色)
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
// 	m_lcZkzh.SetItemState(nItem, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行，失去焦点后也一直显示
	
	m_lcZkzh.GetItemColors(nItem, 0, crOldText, crOldBackground);
	for (int i = 0; i < m_lcZkzh.GetColumns(); i++)							//设置高亮显示(手动设置背景颜色)
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
	if (g_nExitFlag)	//全局退出标识，关闭软件，不进行设置，防止出现内存不足的错误，列表的试卷信息对象已经无用
		return;
	std::vector<std::string> vecCHzkzh;		//检查重号的准考证号，进行2次遍历，
											//防止出现第一次属于正常，后面出现重号后，前面第一次的试卷的状态还是属于正常的问题
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

			if (!bFind) vecCHzkzh.push_back(pPaper->strSN);	//重号的考号放入容器中，需要去重
		}
	}
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pPaper = (pST_PaperInfo)m_lcZkzh.GetItemData(i);
		if (pPaper->nZkzhInBmkStatus == 1)		//只需要检查正常试卷，看准考证号是不是在重号的准考证号列表中
		{
			if (vecCHzkzh.size() > 0)
			{
				for (auto sn : vecCHzkzh)
				{
					if (sn == pPaper->strSN)
					{
						pPaper->nZkzhInBmkStatus = -1;		//重号
						break;
					}
				}
				continue;
			}
		}
// 		if (pPaper->nZkzhInBmkStatus == -1)		//重号的试卷需要检查是不是目前还是重号，防止之前设置重号了，但是没有上传，后面改过来了
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

	VagueSearch(pNMItemActivate->iItem);	//模糊搜索考号

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
	*pResult = 1;		//**********	这里如果不响应，同时返回结果值不为1的话，	****************
						//**********	就会产生产生TRACK SELECT，也就是鼠标悬停	****************
						//**********	一段时间后，所在行自动被选中
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
		for (int i = 0; i < m_lcZkzh.GetColumns(); i++)						//取消高亮显示(手动设置背景颜色)
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
				TRACE("更新考生%s扫描状态完成\n", pPaper->strStudentInfo.c_str());
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
// 		if (nItem < m_lcZkzh.GetItemCount() - 1)	//跳到下一行
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
		for (int i = 0; i < m_lcZkzh.GetColumns(); i++)						//取消高亮显示(手动设置背景颜色)
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
					dlg.setShowInfo(2, 2, "存在考号为空的考生，若不修改，将影响参加后面的评卷，忽略？");
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
				pPaper->bReScan = true;			//设置此试卷需要重新扫描
			else
				pPaper->bReScan = false;
		}

		//如果此试卷已经被修改正常，从问题列表删除
		if (m_pPapers)
		{
			PAPER_LIST::iterator itIssue = m_pPapers->lIssue.begin();
			for (; itIssue != m_pPapers->lIssue.end();)
			{
				pST_PaperInfo pPaper = *itIssue;
				if ((g_nZkzhNull2Issue == 1 && !pPaper->strSN.empty() || g_nZkzhNull2Issue == 0) && !pPaper->bReScan)		//考号不空，且不用重扫，则认为属于正常试卷，放入正常列表中，如果原来在问题列表，则移动到正常列表
				{
					itIssue = m_pPapers->lIssue.erase(itIssue);
					m_pPapers->lPaper.push_back(pPaper);
					continue;
				}
				itIssue++;
			}

			//需要重扫的试卷放入问题试卷列表
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
	//将模糊查找字段变成合法的sql字段
	std::string strVagueKey = pPaper->strRecogSN4Search;
	char szVagueKey[50] = { 0 };
	char * p = szVagueKey;
	bool bLastIsSharp = false;	//标识上一个字符是否是#，用于处理连续的#字符，将多个#变成一个
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
	TRACE("进行模糊查找的字符串: %s\n", szVagueKey);
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

				//显示备注信息，为何出现在此列表
				std::string strDetailInfo;
				if (pPaper->strSN.empty())
					strDetailInfo = "图像被调换，考号为空";
				else
					strDetailInfo = "图像被调换";
				if (pPaper->bModifyZKZH)
					strDetailInfo = "图像被调换，有过修改";
				if (_bGetBmk_)
				{
					if (pPaper->nZkzhInBmkStatus == -1)
						strDetailInfo = "图像被调换，发生重号";
					if (pPaper->nZkzhInBmkStatus == 0)
						strDetailInfo = "图像被调换，考号不在报名库";
				}
				if (pPaper->bReScan)
					strDetailInfo = "图像被调换，这份试卷将删除";

				m_lcZkzh.SetItemText(i, 3, (LPCTSTR)A2T(strDetailInfo.c_str()));
			}
			else
			{
				if (pPaper->strRecogSN4Search.empty())
				{
					m_lcZkzh.SetItemText(i, 1, _T(" "));
					m_lcZkzh.SetItemText(i, 3, _T("图像被调换，考号为空"));
				}
				else
				{
					m_lcZkzh.SetItemText(i, 1, _T(" "));
					m_lcZkzh.SetItemText(i, 3, _T("图像被调换，考号识别不完全"));
				}
			}
			break;
		}
	}

	CNewMessageBox	dlg;
	dlg.setShowInfo(1, 1, "图像调换完成");
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
	int nResult = 0;	//0--报名库不存在，1--报名库存在，-1--报名库检测到已经扫描
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

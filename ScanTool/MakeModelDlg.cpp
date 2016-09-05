// MakeModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanToolDlg.h"
#include "GuideDlg.h"
#include "MakeModelDlg.h"
#include "afxdialogex.h"
#include "ModelSaveDlg.h"
#include <afxinet.h>
#include "ModelInfoDlg.h"
#include "THSetDlg.h"
#include "ScanCtrlDlg.h"
#include "Net_Cmd_Protocol.h"
#include "./pdf2jpg/MuPDFConvert.h"

using namespace std;
using namespace cv;
// CMakeModelDlg 对话框

IMPLEMENT_DYNAMIC(CMakeModelDlg, CDialog)

CMakeModelDlg::CMakeModelDlg(pMODEL pModel /*= NULL*/, CWnd* pParent /*=NULL*/)
	: CDialog(CMakeModelDlg::IDD, pParent)
	, m_pModelPicShow(NULL), m_nGaussKernel(5), m_nSharpKernel(5), m_nThresholdKernel(150), m_nCannyKernel(90), m_nDelateKernel(6), m_nErodeKernel(2)
	, m_pModel(pModel), m_bNewModelFlag(false), m_nModelPicNums(2), m_nCurrTabSel(0), m_bSavedModelFlag(false), m_ncomboCurrentSel(0), m_eCurCPType(UNKNOWN)
	, m_nCurListCtrlSel(0), m_nStartTH(0)
	, m_nWhiteVal(225), m_nHeadVal(150), m_nABModelVal(150), m_nCourseVal(150), m_nQK_CPVal(150), m_nGrayVal(150), m_nFixVal(150), m_nOMR(230), m_nSN(200)
	, m_fHeadThresholdPercent(0.75), m_fABModelThresholdPercent(0.75), m_fCourseThresholdPercent(0.75), m_fQK_CPThresholdPercent(0.75), m_fFixThresholdPercent(0.80)
	, m_fGrayThresholdPercent(0.75), m_fWhiteThresholdPercent(0.75), m_fOMRThresholdPercent(1.5), m_fSNThresholdPercent(1.5)
	, m_pCurRectInfo(NULL), m_ptFixCP(0,0)
	, m_bFistHTracker(true), m_bFistVTracker(true), m_bFistSNTracker(true)
	, m_pRecogInfoDlg(NULL), m_pOmrInfoDlg(NULL), m_pSNInfoDlg(NULL)
	, m_bShiftKeyDown(false)
{
	InitParam();
}

CMakeModelDlg::~CMakeModelDlg()
{
	SAFE_RELEASE(m_pRecogInfoDlg);
	SAFE_RELEASE(m_pOmrInfoDlg);
	SAFE_RELEASE(m_pSNInfoDlg);

	if (m_bNewModelFlag && !m_bSavedModelFlag && m_pModel != NULL)
		SAFE_RELEASE(m_pModel);

	std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
	for (; itPic != m_vecPicShow.end();)
	{
		CPicShow* pModelPicShow = *itPic;
		if (pModelPicShow)
		{
			delete pModelPicShow;
			pModelPicShow = NULL;
		}
		itPic = m_vecPicShow.erase(itPic);
	}

	std::vector<pPaperModelInfo>::iterator itPaperModelInfo = m_vecPaperModelInfo.begin();
	for (; itPaperModelInfo != m_vecPaperModelInfo.end();)
	{
		pPaperModelInfo pPaperModel = *itPaperModelInfo;
		if (pPaperModel)
		{
			delete pPaperModel;
			pPaperModel = NULL;
		}
		itPaperModelInfo = m_vecPaperModelInfo.erase(itPaperModelInfo);
	}
}

void CMakeModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_ModelPic, m_tabModelPicCtrl);
	DDX_Control(pDX, IDC_COMBO_CPType, m_comboCheckPointType);
	DDX_Control(pDX, IDC_LIST_CheckPoint, m_cpListCtrl);
}

BEGIN_MESSAGE_MAP(CMakeModelDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_SelPic, &CMakeModelDlg::OnBnClickedBtnSelpic)
	ON_BN_CLICKED(IDC_BTN_RESET, &CMakeModelDlg::OnBnClickedBtnReset)
	ON_MESSAGE(WM_CV_LBTNUP, &CMakeModelDlg::RoiLBtnUp)
	ON_MESSAGE(WM_CV_LBTNDOWN, &CMakeModelDlg::RoiLBtnDown)
	ON_MESSAGE(WM_CV_RBTNUP, &CMakeModelDlg::RoiRBtnUp)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CMakeModelDlg::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_ExitModelDlg, &CMakeModelDlg::OnBnClickedBtnExitmodeldlg)
	ON_BN_CLICKED(IDC_BTN_New, &CMakeModelDlg::OnBnClickedBtnNew)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_ModelPic, &CMakeModelDlg::OnTcnSelchangeTabModelpic)
	ON_CBN_SELCHANGE(IDC_COMBO_CPType, &CMakeModelDlg::OnCbnSelchangeComboCptype)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CheckPoint, &CMakeModelDlg::OnNMRClickListCheckpoint)
	ON_COMMAND(ID_DelRecognition, &CMakeModelDlg::DeleteRectInfoOnList)
	ON_COMMAND(ID_DelPicRectRecog, &CMakeModelDlg::DelRectInfoOnPic)
	ON_COMMAND(ID_TrackerRecognize, &CMakeModelDlg::RecognizeRectTracker)
	ON_COMMAND(ID_AddRecog, &CMakeModelDlg::AddRecogRectToList)
	ON_COMMAND(ID_RecogSN, &CMakeModelDlg::AddRecogSN)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CheckPoint, &CMakeModelDlg::OnNMDblclkListCheckpoint)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_CheckPoint, &CMakeModelDlg::OnLvnKeydownListCheckpoint)
//	ON_BN_CLICKED(IDC_BTN_SaveRecogInfo, &CMakeModelDlg::OnBnClickedBtnSaverecoginfo)
	ON_MESSAGE(WM_CV_HTrackerChange, &CMakeModelDlg::HTrackerChange)
	ON_MESSAGE(WM_CV_VTrackerChange, &CMakeModelDlg::VTrackerChange)
	ON_MESSAGE(WM_CV_SNTrackerChange, &CMakeModelDlg::SNTrackerChange)
	ON_BN_CLICKED(IDC_BTN_uploadModel, &CMakeModelDlg::OnBnClickedBtnuploadmodel)
	ON_BN_CLICKED(IDC_BTN_ScanModel, &CMakeModelDlg::OnBnClickedBtnScanmodel)
	ON_MESSAGE(WM_CV_ShiftDown, &CMakeModelDlg::ShiftKeyDown)
	ON_MESSAGE(WM_CV_ShiftUp, &CMakeModelDlg::ShiftKeyUp)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_HOVER, IDC_LIST_CheckPoint, &CMakeModelDlg::OnNMHoverListCheckpoint)
END_MESSAGE_MAP()

// CMakeModelDlg 消息处理程序
BOOL CMakeModelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	InitConf();
	if (m_pModel)
	{
		m_vecPaperModelInfo.clear();

		for (int i = 0; i < m_pModel->nPicNum; i++)
		{
			CString strPicPath = g_strCurrentPath + _T("Model\\") + m_pModel->strModelName + _T("\\") + m_pModel->vecPaperModel[i]->strModelPicName;

			pPaperModelInfo pPaperModel = new PaperModelInfo;
			m_vecPaperModelInfo.push_back(pPaperModel);
			pPaperModel->nPaper = i;
			pPaperModel->strModelPicPath = strPicPath;
			pPaperModel->strModelPicName = m_pModel->vecPaperModel[i]->strModelPicName;

#ifdef PIC_RECTIFY_TEST
			Mat matSrc = imread((std::string)(CT2CA)strPicPath);
			Mat dst;
			Mat rotMat;
			PicRectify(matSrc, dst, rotMat);
			Mat matImg;
			if (dst.channels() == 1)
				cvtColor(dst, matImg, CV_GRAY2BGR);
			else
				matImg = dst;

			pPaperModel->matSrcImg = matImg;
			pPaperModel->matDstImg = pPaperModel->matSrcImg;
#else
			pPaperModel->matSrcImg = imread((std::string)(CT2CA)strPicPath);
			pPaperModel->matDstImg = pPaperModel->matSrcImg;
#endif

			Mat src_img;
			src_img = m_vecPaperModelInfo[i]->matDstImg;
			m_vecPicShow[i]->ShowPic(src_img);

			pPaperModel->nPicW = src_img.cols;
			pPaperModel->nPicH = src_img.rows;
			
			pPaperModel->bFirstH = false;
			pPaperModel->bFirstV = false;
			pPaperModel->rtHTracker = m_pModel->vecPaperModel[i]->rtHTracker;
			pPaperModel->rtVTracker = m_pModel->vecPaperModel[i]->rtVTracker;
			pPaperModel->rtSNTracker = m_pModel->vecPaperModel[i]->rtSNTracker;

			RECTLIST::iterator itSelHTracker = m_pModel->vecPaperModel[i]->lSelHTracker.begin();
			for (; itSelHTracker != m_pModel->vecPaperModel[i]->lSelHTracker.end(); itSelHTracker++)
			{
				pPaperModel->vecHTracker.push_back(*itSelHTracker);
			}
			RECTLIST::iterator itSelVTracker = m_pModel->vecPaperModel[i]->lSelVTracker.begin();
			for (; itSelVTracker != m_pModel->vecPaperModel[i]->lSelVTracker.end(); itSelVTracker++)
			{
				pPaperModel->vecVTracker.push_back(*itSelVTracker);
			}
			RECTLIST::iterator itSelRoi = m_pModel->vecPaperModel[i]->lSelFixRoi.begin();
			for (; itSelRoi != m_pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++)
			{
				pPaperModel->vecRtSel.push_back(*itSelRoi);
			}
			OMRLIST::iterator itOmr2 = m_pModel->vecPaperModel[i]->lOMR2.begin();
			for (; itOmr2 != m_pModel->vecPaperModel[i]->lOMR2.end(); itOmr2++)
			{
				pPaperModel->vecOmr2.push_back(*itOmr2);
			}
			RECTLIST::iterator itFix = m_pModel->vecPaperModel[i]->lFix.begin();
			for (; itFix != m_pModel->vecPaperModel[i]->lFix.end(); itFix++)
			{
				pPaperModel->vecRtFix.push_back(*itFix);
			}
			RECTLIST::iterator itHHead = m_pModel->vecPaperModel[i]->lH_Head.begin();
			for (; itHHead != m_pModel->vecPaperModel[i]->lH_Head.end(); itHHead++)
			{
				pPaperModel->vecH_Head.push_back(*itHHead);
			}
			RECTLIST::iterator itVHead = m_pModel->vecPaperModel[i]->lV_Head.begin();
			for (; itVHead != m_pModel->vecPaperModel[i]->lV_Head.end(); itVHead++)
			{
				pPaperModel->vecV_Head.push_back(*itVHead);
			}
			RECTLIST::iterator itABModel = m_pModel->vecPaperModel[i]->lABModel.begin();
			for (; itABModel != m_pModel->vecPaperModel[i]->lABModel.end(); itABModel++)
			{
				pPaperModel->vecABModel.push_back(*itABModel);
			}
			RECTLIST::iterator itCourse = m_pModel->vecPaperModel[i]->lCourse.begin();
			for (; itCourse != m_pModel->vecPaperModel[i]->lCourse.end(); itCourse++)
			{
				pPaperModel->vecCourse.push_back(*itCourse);
			}
			RECTLIST::iterator itQK = m_pModel->vecPaperModel[i]->lQK_CP.begin();
			for (; itQK != m_pModel->vecPaperModel[i]->lQK_CP.end(); itQK++)
			{
				pPaperModel->vecQK_CP.push_back(*itQK);
			}
			RECTLIST::iterator itGray = m_pModel->vecPaperModel[i]->lGray.begin();
			for (; itGray != m_pModel->vecPaperModel[i]->lGray.end(); itGray++)
			{
				pPaperModel->vecGray.push_back(*itGray);
			}
			RECTLIST::iterator itWhite = m_pModel->vecPaperModel[i]->lWhite.begin();
			for (; itWhite != m_pModel->vecPaperModel[i]->lWhite.end(); itWhite++)
			{
				pPaperModel->vecWhite.push_back(*itWhite);
			}
			SNLIST::iterator itSn = m_pModel->vecPaperModel[i]->lSNInfo.begin();
			for (; itSn != m_pModel->vecPaperModel[i]->lSNInfo.end(); itSn++)
			{
				pSN_ITEM pSnItem = new SN_ITEM;
				pSnItem->nItem = (*itSn)->nItem;
				pSnItem->nRecogVal = (*itSn)->nRecogVal;
				RECTLIST::iterator itRc = (*itSn)->lSN.begin();
				for (; itRc != (*itSn)->lSN.end(); itRc++)
				{
					RECTINFO rc = *itRc;
					pSnItem->lSN.push_back(rc);
				}
				pPaperModel->lSN.push_back(pSnItem);
			}
			

			ShowRectByCPType(m_eCurCPType);
			UpdataCPList();
		}

		CString strTitle = _T("");
		strTitle.Format(_T("模板名称: %s"), m_pModel->strModelName);
		SetWindowText(strTitle);
	}
	else
	{
		CString strTitle = _T("未保存模板");
		SetWindowText(strTitle);
	}

	return TRUE;
}
void CMakeModelDlg::OnTcnSelchangeTabModelpic(NMHDR *pNMHDR, LRESULT *pResult)
{	
	*pResult = 0;
	int nIndex		= m_tabModelPicCtrl.GetCurSel();
	m_nCurrTabSel	= nIndex;
	m_pModelPicShow = m_vecPicShow[nIndex];
	m_pModelPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != nIndex)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
	m_comboCheckPointType.SetCurSel(0);
	m_cpListCtrl.DeleteAllItems();

	m_ncomboCurrentSel = 0;
	m_eCurCPType = GetComboSelCpType();
	UpdataCPList();
	ShowRectByCPType(m_eCurCPType);	
	Invalidate();
}

void CMakeModelDlg::InitTab()
{
	std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
	for (; itPic != m_vecPicShow.end();)
	{
		CPicShow* pModelPicShow = *itPic;
		if (pModelPicShow)
		{
			delete pModelPicShow;
			pModelPicShow = NULL;
		}
		itPic = m_vecPicShow.erase(itPic);
	}
	m_tabModelPicCtrl.DeleteAllItems();

	USES_CONVERSION;
	CRect rtTab;
	m_tabModelPicCtrl.GetClientRect(&rtTab);
	for (int i = 0; i < m_nModelPicNums; i++)
	{
		char szTabHeadName[20] = { 0 };
		sprintf_s(szTabHeadName, "第%d页", i + 1);

		m_tabModelPicCtrl.InsertItem(i, A2T(szTabHeadName));

		CPicShow* pPicShow = new CPicShow(this);
		pPicShow->Create(CPicShow::IDD, &m_tabModelPicCtrl);
		pPicShow->ShowWindow(SW_HIDE);
		pPicShow->MoveWindow(&rtTab);
		m_vecPicShow.push_back(pPicShow);
	}
	m_tabModelPicCtrl.SetCurSel(0);
	if (m_vecPicShow.size())
	{
		m_vecPicShow[0]->ShowWindow(SW_SHOW);
		m_pModelPicShow = m_vecPicShow[0];
	}	

	if (m_tabModelPicCtrl.GetSafeHwnd())
	{
		CRect rtTab;
		m_tabModelPicCtrl.GetClientRect(&rtTab);
		int nTabHead_H = 24;		//tab控件头的高度
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
}

void CMakeModelDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CMakeModelDlg::InitUI()
{
	USES_CONVERSION;
	if (m_pModel)
		m_nModelPicNums = m_pModel->nPicNum;
	
	m_cpListCtrl.SetExtendedStyle(m_cpListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_cpListCtrl.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 36);
	m_cpListCtrl.InsertColumn(1, _T("位置信息"), LVCFMT_CENTER, 120);

	CRect rtTab;
	m_tabModelPicCtrl.GetClientRect(&rtTab);
	for (int i = 0; i < m_nModelPicNums; i++)
	{
		char szTabHeadName[20] = { 0 };
		sprintf_s(szTabHeadName, "第%d页", i + 1);

		m_tabModelPicCtrl.InsertItem(i, A2T(szTabHeadName));

		CPicShow* pPicShow = new CPicShow(this);
		pPicShow->Create(CPicShow::IDD, &m_tabModelPicCtrl);
		pPicShow->ShowWindow(SW_HIDE);
		pPicShow->MoveWindow(&rtTab);
		m_vecPicShow.push_back(pPicShow);
	}
	m_tabModelPicCtrl.SetCurSel(0);
	m_vecPicShow[0]->ShowWindow(SW_SHOW);
	m_pModelPicShow = m_vecPicShow[0];

	m_pRecogInfoDlg = new CRecogInfoDlg;
	m_pRecogInfoDlg->Create(CRecogInfoDlg::IDD, this);
	m_pRecogInfoDlg->ShowWindow(SW_SHOW);	//SW_SHOW

	m_pOmrInfoDlg = new COmrInfoDlg;
	m_pOmrInfoDlg->Create(COmrInfoDlg::IDD, this);
	m_pOmrInfoDlg->ShowWindow(SW_HIDE);	//SW_HIDE

	m_pSNInfoDlg = new CSNInfoSetDlg;
	m_pSNInfoDlg->Create(CSNInfoSetDlg::IDD, this);
	m_pSNInfoDlg->ShowWindow(SW_HIDE);

	CRect rc;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	int sx = rc.Width();
	int sy = rc.Height();
	if (sx > MAX_DLG_WIDTH)
		sx = MAX_DLG_WIDTH;
	if (sy > MAX_DLG_HEIGHT)
		sy = MAX_DLG_HEIGHT;
	MoveWindow(0, 0, sx, sy);
	CenterWindow();

	InitCtrlPosition();
}

void CMakeModelDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nGap = 5;	//控件的间隔
	int nTopGap = 2;	//距离上边边缘的间隔
	int nBottomGap = 2;	//距离下边边缘的间隔
	int nLeftGap = 2;	//距离左边边缘的间隔
	int nRightGap = 2;	//距离右边边缘的间隔
	int nBtnHeigh = 30;
	int	nBtnWidth = 60;
	int nLeftCtrlWidth = 170;	//左边控件栏的宽度
	int nStaticHeight = 20;		//校验点类型Static控件高度
	int nCommoboxHeight = 25;	//校验点combox的高度
	int nCPListCtrlHeight = 280;//校验点list控件的高度
	int nCurrentTop = nTopGap;
	if (GetDlgItem(IDC_STATIC_CPType)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_CPType)->MoveWindow(nLeftGap, nCurrentTop, nLeftCtrlWidth, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (m_comboCheckPointType.GetSafeHwnd())
	{
		m_comboCheckPointType.MoveWindow(nLeftGap, nCurrentTop, nLeftCtrlWidth, nCommoboxHeight);
		nCurrentTop = nCurrentTop + nCommoboxHeight + nGap;
	}
	if (GetDlgItem(IDC_STATIC_List)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_List)->MoveWindow(nLeftGap, nCurrentTop, nLeftCtrlWidth, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (m_cpListCtrl.GetSafeHwnd())
	{
		m_cpListCtrl.MoveWindow(nLeftGap, nCurrentTop, nLeftCtrlWidth, nCPListCtrlHeight);
		nCurrentTop = nCurrentTop + nCPListCtrlHeight + nGap;
	}
	int nGroupHeight = 170;
	int nGroupStaticHeight = 15;	//group文字的高度
	int nTopInGroup = nCurrentTop;

	if (m_pRecogInfoDlg && m_pRecogInfoDlg->GetSafeHwnd())
	{
		m_pRecogInfoDlg->MoveWindow(nLeftGap, nTopInGroup, nLeftCtrlWidth, nGroupHeight);
	}
	if (m_pOmrInfoDlg && m_pOmrInfoDlg->GetSafeHwnd())
	{
		m_pOmrInfoDlg->MoveWindow(nLeftGap, nTopInGroup, nLeftCtrlWidth, nGroupHeight);
	}
	if (m_pSNInfoDlg && m_pSNInfoDlg->GetSafeHwnd())
	{
		m_pSNInfoDlg->MoveWindow(nLeftGap, nTopInGroup, nLeftCtrlWidth, nGroupHeight);
	}

	nCurrentTop = nCurrentTop + nGroupHeight + nGap;
	if (GetDlgItem(IDC_BTN_New)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_New)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
//		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
	if (GetDlgItem(IDC_BTN_ScanModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ScanModel)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
	if (GetDlgItem(IDC_BTN_SelPic)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_SelPic)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
//		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
	if (GetDlgItem(IDC_BTN_RESET)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_RESET)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
	if (GetDlgItem(IDC_BTN_SAVE)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_SAVE)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
//		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
#if 1
	if (GetDlgItem(IDC_BTN_uploadModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_uploadModel)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
#endif
	if (GetDlgItem(IDC_BTN_ExitModelDlg)->GetSafeHwnd())
	{
//		GetDlgItem(IDC_BTN_ExitModelDlg)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		GetDlgItem(IDC_BTN_ExitModelDlg)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
	if (m_tabModelPicCtrl.GetSafeHwnd())
	{
		m_tabModelPicCtrl.MoveWindow(nLeftGap + nLeftCtrlWidth + nGap, nTopGap, cx - nLeftGap - nLeftCtrlWidth - nGap - nRightGap, cy - nTopGap - nBottomGap);

		CRect rtTab;
		m_tabModelPicCtrl.GetClientRect(&rtTab);
		int nTabHead_H = 24;		//tab控件头的高度
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
}

void CMakeModelDlg::InitConf()
{
	if (m_pModel)
	{
		if (m_pModel->nABModel&&m_pModel->nHasHead)
		{
			m_comboCheckPointType.ResetContent();
			m_comboCheckPointType.AddString(_T(""));
//			m_comboCheckPointType.AddString(_T("定点"));
			m_comboCheckPointType.AddString(_T("水平同步头"));
			m_comboCheckPointType.AddString(_T("垂直同步头"));
			m_comboCheckPointType.AddString(_T("卷形校验点"));
			m_comboCheckPointType.AddString(_T("科目校验点"));
			m_comboCheckPointType.AddString(_T("缺考校验点"));
			m_comboCheckPointType.AddString(_T("灰度校验点"));
			m_comboCheckPointType.AddString(_T("空白校验点"));
			m_comboCheckPointType.AddString(_T("考号设置"));
			m_comboCheckPointType.AddString(_T("选择题"));
		}
		else if (m_pModel->nABModel&&!m_pModel->nHasHead)
		{
			m_comboCheckPointType.ResetContent();
			m_comboCheckPointType.AddString(_T(""));
//			m_comboCheckPointType.AddString(_T("定点"));
			m_comboCheckPointType.AddString(_T("卷形校验点"));
			m_comboCheckPointType.AddString(_T("科目校验点"));
			m_comboCheckPointType.AddString(_T("缺考校验点"));
			m_comboCheckPointType.AddString(_T("灰度校验点"));
			m_comboCheckPointType.AddString(_T("空白校验点"));
			m_comboCheckPointType.AddString(_T("考号设置"));
			m_comboCheckPointType.AddString(_T("选择题"));
		}
		else if (!m_pModel->nABModel&&m_pModel->nHasHead)
		{
			m_comboCheckPointType.ResetContent();
			m_comboCheckPointType.AddString(_T(""));
//			m_comboCheckPointType.AddString(_T("定点"));
			m_comboCheckPointType.AddString(_T("水平同步头"));
			m_comboCheckPointType.AddString(_T("垂直同步头"));
			m_comboCheckPointType.AddString(_T("科目校验点"));
			m_comboCheckPointType.AddString(_T("缺考校验点"));
			m_comboCheckPointType.AddString(_T("灰度校验点"));
			m_comboCheckPointType.AddString(_T("空白校验点"));
			m_comboCheckPointType.AddString(_T("考号设置"));
			m_comboCheckPointType.AddString(_T("选择题"));
		}
		else
		{
			m_comboCheckPointType.ResetContent();
			m_comboCheckPointType.AddString(_T(""));
			m_comboCheckPointType.AddString(_T("定点"));
			m_comboCheckPointType.AddString(_T("科目校验点"));
			m_comboCheckPointType.AddString(_T("缺考校验点"));
			m_comboCheckPointType.AddString(_T("灰度校验点"));
//			m_comboCheckPointType.AddString(_T("空白校验点"));
			m_comboCheckPointType.AddString(_T("考号设置"));
			m_comboCheckPointType.AddString(_T("选择题"));
		}
	}
	m_comboCheckPointType.SetCurSel(0);
}

LRESULT CMakeModelDlg::RoiRBtnUp(WPARAM wParam, LPARAM lParam)
{
	cv::Point pt = *(cv::Point*)(wParam);
	if (m_eCurCPType == UNKNOWN)
		return TRUE;

	if (m_eCurCPType == H_HEAD || m_eCurCPType == V_HEAD)
	{
		cv::Rect rtHTracker = cv::Rect(m_pModelPicShow->m_picShow.m_ptHTracker1, m_pModelPicShow->m_picShow.m_ptHTracker2);
		cv::Rect rtVTracker = cv::Rect(m_pModelPicShow->m_picShow.m_ptVTracker1, m_pModelPicShow->m_picShow.m_ptVTracker2);
		if (rtHTracker.contains(pt) || rtVTracker.contains(pt))
		{
			CMenu menu, *pPopup;
			menu.LoadMenu(IDR_MENU_RecogTracker);
			pPopup = menu.GetSubMenu(0);
			CPoint myPoint;
			ClientToScreen(&myPoint);
			GetCursorPos(&myPoint); //鼠标位置  
			return pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
		}
	}
	else if (m_eCurCPType == SN)
	{
		cv::Rect rtSNTracker = cv::Rect(m_pModelPicShow->m_picShow.m_ptSNTracker1, m_pModelPicShow->m_picShow.m_ptSNTracker2);
		if (rtSNTracker.contains(pt))
		{
			CMenu menu, *pPopup;
			menu.LoadMenu(IDR_MENU_RecogSN);
			pPopup = menu.GetSubMenu(0);
			CPoint myPoint;
			ClientToScreen(&myPoint);
			GetCursorPos(&myPoint); //鼠标位置  
			return pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
		}
	}
	else
	{
		if (m_vecTmp.size() > 0)
		{
			CMenu menu, *pPopup;
			menu.LoadMenu(IDR_MENU_AddRecog);
			pPopup = menu.GetSubMenu(0);
			CPoint myPoint;
			ClientToScreen(&myPoint);
			GetCursorPos(&myPoint); //鼠标位置  
			return pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
		}
	}

	int nFind = 0;
	RECTINFO* pRc = NULL;
	nFind = GetRectInfoByPoint(pt, m_eCurCPType, pRc);
	if (nFind < 0 || !pRc)
		return FALSE;

	//下面的这段代码, 不单单适应于ListCtrl  
	CMenu menu, *pPopup;
	menu.LoadMenu(IDR_MENU_RectInPic);
	pPopup = menu.GetSubMenu(0);
	CPoint myPoint;
	ClientToScreen(&myPoint);
	GetCursorPos(&myPoint); //鼠标位置  
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);//GetParent()
	m_ptRBtnUp = pt;
	return TRUE;
}

LRESULT CMakeModelDlg::RoiLBtnUp(WPARAM wParam, LPARAM lParam)
{
	Rect  Rt = *(Rect*)(wParam);
	Mat*  pShowMat = (Mat*)(lParam);

	m_cpListCtrl.SetItemState(m_nCurListCtrlSel, 0, LVIS_DROPHILITED);		// 取消高亮显示
	if (m_eCurCPType == UNKNOWN)
	{
		AfxMessageBox(_T("请先选中校验点类型"));
		return FALSE;
	}

	if (m_eCurCPType != H_HEAD && m_eCurCPType != V_HEAD && m_pModel && m_pModel->nHasHead == 0)
	{
		if (checkOverlap(m_eCurCPType, Rt))
		{
			AfxMessageBox(_T("检测到包含已选区域"));
			return FALSE;
		}
		Recognise(Rt);
		SortRect();
		UpdataCPList();
	}
	else if (m_eCurCPType != H_HEAD && m_eCurCPType != V_HEAD && m_pModel &&  m_pModel->nHasHead != 0)
	{
		if (checkOverlap(m_eCurCPType, Rt))
		{
			AfxMessageBox(_T("检测到包含已选区域"));
			return FALSE;
		}
		RecogByHead(Rt);
	}
	return TRUE;
}

LRESULT CMakeModelDlg::RoiLBtnDown(WPARAM wParam, LPARAM lParam)
{
	cv::Point pt = *(cv::Point*)(wParam);
	if (m_pModel && m_pModel->nHasHead != 0 && m_bShiftKeyDown)		//shift按下
	{
		if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
			return false;
		if (!m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size())
		{
			AfxMessageBox(_T("请先设置水平同步头"));
			return false;
		}
		if (!m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size())
		{
			AfxMessageBox(_T("请先设置垂直同步头"));
			return false;
		}
		int nPosH = -1;
		int nPosV = -1;
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size(); i++)
		{
			RECTINFO& rtPosH = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i];
			if (rtPosH.rt.tl().x <= pt.x && rtPosH.rt.br().x >= pt.x)
			{
				nPosH = i;
				break;
			}
		}
		if (nPosH == -1) return false;

		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size(); i++)
		{
			RECTINFO& rtPosV = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i];
			if (rtPosV.rt.tl().y <= pt.y && rtPosV.rt.br().y >= pt.y)
			{
				nPosV = i;
				break;
			}
		}
		if (nPosV == -1) return false;

		cv::Rect rt;
		rt.x = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nPosH].rt.x;
		rt.y = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[nPosV].rt.y;
		rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nPosH].rt.width;
		rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[nPosV].rt.height;

		if (checkOverlap(m_eCurCPType, rt))
		{
			m_bShiftKeyDown = false;
//			AfxMessageBox(_T("检测到包含已选区域"));
			return FALSE;
		}

		RECTINFO rc;
		rc.rt = rt;
		rc.eCPType = m_eCurCPType;
		rc.nHItem = nPosH;
		rc.nVItem = nPosV;
		if (m_eCurCPType == ABMODEL)
		{
			rc.nThresholdValue = m_nABModelVal;
			rc.fStandardValuePercent = m_fABModelThresholdPercent;

			Rect rtTmp = rc.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecTmp.push_back(rc);
		}
		else if (m_eCurCPType == COURSE)
		{
			rc.nThresholdValue = m_nCourseVal;
			rc.fStandardValuePercent = m_fCourseThresholdPercent;

			Rect rtTmp = rc.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecTmp.push_back(rc);
		}
		else if (m_eCurCPType == QK_CP)
		{
			rc.nThresholdValue = m_nQK_CPVal;
			rc.fStandardValuePercent = m_fQK_CPThresholdPercent;

			Rect rtTmp = rc.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecTmp.push_back(rc);
		}
		else if (m_eCurCPType == GRAY_CP)
		{
			rc.nThresholdValue = m_nGrayVal;
			rc.fStandardValuePercent = m_fGrayThresholdPercent;

			Rect rtTmp = rc.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecTmp.push_back(rc);
		}
		else if (m_eCurCPType == WHITE_CP)
		{
			rc.nThresholdValue = m_nWhiteVal;
			rc.fStandardValuePercent = m_fWhiteThresholdPercent;

			Rect rtTmp = rc.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecTmp.push_back(rc);
		}
		else
		{
			m_bShiftKeyDown = false;
			return FALSE;
		}
//		m_vecTmp.push_back(rc);

		Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
		Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
		
		for (int i = 0; i < m_vecTmp.size(); i++)
		{
			char szAnswerVal[5] = { 0 };
			sprintf_s(szAnswerVal, "%d", i + 1);
			cv::putText(tmp, szAnswerVal, Point(m_vecTmp[i].rt.x + m_vecTmp[i].rt.width / 5, m_vecTmp[i].rt.y + m_vecTmp[i].rt.height * 0.8), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255));
			cv::rectangle(tmp, m_vecTmp[i].rt, CV_RGB(20, 225, 25), 2);
			cv::rectangle(tmp2, m_vecTmp[i].rt, CV_RGB(255, 233, 10), -1);
		}
		cv::addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
		m_pModelPicShow->ShowPic(tmp);
	}
	else
	{
		if (!ShowRectByPoint(pt))
		{
			if (m_vecTmp.size() > 0)
			{
				m_vecTmp.clear();
				ShowRectByCPType(m_eCurCPType);
			}
		}		
	}
	return TRUE;
}

void CMakeModelDlg::OnBnClickedBtnScanmodel()
{
	if (m_strScanSavePath == _T(""))
	{
		LPITEMIDLIST pidRoot = NULL;
		SHGetSpecialFolderLocation(m_hWnd, CSIDL_DRIVES, &pidRoot);
		CString modelPath = g_strCurrentPath + _T("Model\\");
		
		USES_CONVERSION;
		TCHAR Dir[MAX_PATH] = { 0 };
		BROWSEINFO bi;
		ITEMIDLIST *pidl;
		bi.hwndOwner = GetSafeHwnd();
		bi.pidlRoot = pidRoot;				//pidRoot;
		bi.pszDisplayName = Dir;
		bi.lpszTitle = _T("请选择保存路径");
		bi.ulFlags = BIF_RETURNONLYFSDIRS;	//BIF_EDITBOX;	//BIF_RETURNONLYFSDIRS;
		bi.lpfn = NULL;
		bi.lParam = (LPARAM)modelPath.GetBuffer();
		bi.iImage = 0;
		pidl = SHBrowseForFolder(&bi);
		if (pidl == NULL)
			return;
		if (!SHGetPathFromIDList(pidl, Dir))
			return;

		TRACE("%s\n", Dir);


		m_strScanSavePath = Dir;
	}
	// 调用TWAIN 初始化扫描设置
	ReleaseTwain();
	m_bTwainInit = FALSE;
	if (!m_bTwainInit)
	{
		m_bTwainInit = InitTwain(m_hWnd);
		if (!IsValidDriver())
		{
			AfxMessageBox(_T("Unable to load Twain Driver."));
		}
		m_scanSourceArry.RemoveAll();
		ScanSrcInit();
	}

	CScanCtrlDlg dlg(m_scanSourceArry);
	if (dlg.DoModal() != IDOK)
		return;

	GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(FALSE);

	m_Source = m_scanSourceArry.GetAt(dlg.m_nCurrScanSrc);
	int nDuplex = dlg.m_nCurrDuplex;		//单双面,0-单面,1-双面
	int nSize = 1;							//1-A4
	int nPixel = 2;							//0-黑白，1-灰度，2-彩色
	int nResolution = 200;					//dpi: 72, 150, 200, 300

	int nNum = dlg.m_nStudentNum;

	if (nDuplex == 1)
		nNum *= 2;

	if (nNum == 0)
		nNum = TWCPP_ANYCOUNT;
	if (!Acquire(nNum, nDuplex, nSize, nPixel, nResolution))
	{
		TRACE("扫描失败\n");
	}
	GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(TRUE);
}

void CMakeModelDlg::OnBnClickedBtnNew()
{
	USES_CONVERSION;
	CModelInfoDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;

//	m_vecPaperModelInfo.clear();
	std::vector<pPaperModelInfo>::iterator itPaperModelInfo = m_vecPaperModelInfo.begin();
	for (; itPaperModelInfo != m_vecPaperModelInfo.end();)
	{
		pPaperModelInfo pPaperModel = *itPaperModelInfo;
		SAFE_RELEASE(pPaperModel);
		itPaperModelInfo = m_vecPaperModelInfo.erase(itPaperModelInfo);
	}
	m_vecPaperModelInfo.clear();

	GetDlgItem(IDC_BTN_New)->EnableWindow(FALSE);
	m_nModelPicNums = dlg.m_nPaperNum;
	InitTab();
	m_cpListCtrl.DeleteAllItems();

	m_bNewModelFlag = true;
	m_pModel = new MODEL;
	m_pModel->nABModel = dlg.m_bABPaperModel;
	m_pModel->nHasHead = dlg.m_bHasHead;
	InitConf();

	for (int i = 0; i < m_nModelPicNums; i++)
	{
		pPaperModelInfo paperMode = new PaperModelInfo;
		m_vecPaperModelInfo.push_back(paperMode);

		paperMode->strModelPicName = dlg.m_vecPath[i].strName;
		paperMode->strModelPicPath = dlg.m_vecPath[i].strPath;

		
#ifdef PIC_RECTIFY_TEST
		Mat src_img = imread((std::string)(CT2CA)dlg.m_vecPath[i].strPath);	//(std::string)(CT2CA)paperMode->strModelPicPath
		Mat dst;
		Mat rotMat;
		PicRectify(src_img, dst, rotMat);
		Mat matImg;
		if (dst.channels() == 1)
			cvtColor(dst, matImg, CV_GRAY2BGR);
		else
			matImg = dst;
#else
		Mat matImg = imread((std::string)(CT2CA)dlg.m_vecPath[i].strPath);	//(std::string)(CT2CA)paperMode->strModelPicPath
#endif
		paperMode->matSrcImg = matImg;
		paperMode->matDstImg = paperMode->matSrcImg;
		if (i == 0)
			m_pModelPicShow->ShowPic(matImg);
		
		paperMode->nPicW = matImg.cols;
		paperMode->nPicH = matImg.rows;
	}
	SetWindowTextW(_T("*未保存模板*"));
}

void CMakeModelDlg::OnBnClickedBtnSelpic()
{
	if (!m_pModel)
	{
		AfxMessageBox(_T("请先创建模板"));
		return;
	}
	if (!m_bNewModelFlag)
	{
		AfxMessageBox(_T("重新选择图片需要重新设置本页的校验点"));
	}

	CFileDialog dlg(true, _T("*.bmp"), NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		_T("image file(*.bmp;*.png;*.tif;*.tiff;*.jpg)|*.bmp;*.png;*.tif;*.tiff;*.jpg|All Files(*.*)|*.*|"), NULL);
	dlg.m_ofn.lpstrTitle = _T("选择模板图像");
	if (dlg.DoModal() != IDOK)
		return;

	pPaperModelInfo paperMode = NULL;
	paperMode = m_vecPaperModelInfo[m_nCurrTabSel];

	paperMode->strModelPicName = dlg.GetFileName();
	paperMode->strModelPicPath = dlg.GetPathName();

	USES_CONVERSION;
	Mat src_img = imread((std::string)(CT2CA)paperMode->strModelPicPath);	//(std::string)(CT2CA)paperMode->strModelPicPath

	paperMode->matSrcImg = src_img;
	paperMode->matDstImg = paperMode->matSrcImg;
	m_pModelPicShow->ShowPic(src_img);
	
	paperMode->nPicW = src_img.cols;
	paperMode->nPicH = src_img.rows;

	//重新选择图片后，需要重置本页面的所有点信息
	if (m_vecPaperModelInfo.size() <= 0 || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	m_cpListCtrl.DeleteAllItems();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.clear();
	SNLIST::iterator itSn = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
	for (; itSn != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end();)
	{
		pSN_ITEM pSNItem = *itSn;
		itSn = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.erase(itSn);
		SAFE_RELEASE(pSNItem);
	}
	ShowRectByCPType(m_eCurCPType);
}

void CMakeModelDlg::OnBnClickedBtnReset()
{
	if (!m_pModel)
	{
		AfxMessageBox(_T("请先创建模板"));
		return;
	}

	if (m_vecPaperModelInfo.size() <= 0 || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	cv::Mat displayImg = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg;
	m_pModelPicShow->ShowPic(displayImg);

	m_cpListCtrl.DeleteAllItems();
	switch (m_eCurCPType)
	{
	case UNKNOWN:
	case Fix_CP:
		if (m_eCurCPType == Fix_CP || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.clear();
			m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lFix.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size())m_pModel->vecPaperModel[m_nCurrTabSel].lSelFixRoi.clear();
		}
	case H_HEAD:
		if (m_eCurCPType == H_HEAD || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lH_Head.clear();
		}
	case V_HEAD:
		if (m_eCurCPType == V_HEAD || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lV_Head.clear();
		}
	case ABMODEL:
		if (m_eCurCPType == ABMODEL || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lABModel.clear();
		}
	case COURSE:
		if (m_eCurCPType == COURSE || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lCourse.clear();
		}
	case QK_CP:
		if (m_eCurCPType == QK_CP || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lQK_CP.clear();
		}
	case GRAY_CP:
		if (m_eCurCPType == GRAY_CP || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lGray.clear();
		}
	case WHITE_CP:
		if (m_eCurCPType == WHITE_CP || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lWhite.clear();
		}
	case SN:
		if (m_eCurCPType == SN || m_eCurCPType == UNKNOWN)
		{
			SNLIST::iterator itSn = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
			for (; itSn != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end();)
			{
				pSN_ITEM pSNItem = *itSn;
				itSn = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.erase(itSn);
				SAFE_RELEASE(pSNItem);
			}
		}
	case OMR:
		if (m_eCurCPType == OMR || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.clear();
//			if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lOMR2.clear();
		}
	}
}

bool CMakeModelDlg::RecogNewGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc)
{
	cv::cvtColor(matSrcRoi, matSrcRoi, CV_BGR2GRAY);

	return RecogGrayValue(matSrcRoi, rc);
}

inline bool CMakeModelDlg::RecogGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc)
{
	const int channels[1] = { 0 };
	const float* ranges[1];
	const int histSize[1] = { 1 };
	float hranges[2];
	if (rc.eCPType != WHITE_CP)
	{
		hranges[0] = g_nRecogGrayMin;
		hranges[1] = static_cast<float>(rc.nThresholdValue);
		ranges[0] = hranges;
	}
	else
	{
		hranges[0] = static_cast<float>(rc.nThresholdValue);
		hranges[1] = g_nRecogGrayMax_White;	//255			//256时可统计完全空白的点，即RGB值为255的完全空白点;255时只能统计到RGB为254的值，255的值统计不到
		ranges[0] = hranges;
	}
	MatND src_hist;
	cv::calcHist(&matSrcRoi, 1, channels, Mat(), src_hist, 1, histSize, ranges, false);

	rc.fStandardValue = src_hist.at<float>(0);
	return true;
}

inline void CMakeModelDlg::GetThreshold(cv::Mat& matSrc, cv::Mat& matDst)
{
	switch (m_eCurCPType)
	{
	case Fix_CP:
	case H_HEAD:
	case V_HEAD:
	case ABMODEL:
	case COURSE:
	case QK_CP:
		{
#ifdef USES_GETTHRESHOLD_ZTFB	//先计算ROI区域的均值u和标准差p，二值化的阀值取u + 2p，根据正态分布，理论上可以囊括95%以上的范围
			const int channels[1] = { 0 };
			const int histSize[1] = { 150 };
			float hranges[2] = { 0, 150 };
			const float* ranges[1];
			ranges[0] = hranges;
			MatND hist;
			calcHist(&matSrc, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

			int nSum = 0;
			int nDevSum = 0;
			int nCount = 0;
			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nCount += static_cast<int>(binVal);
				nSum += h*binVal;
			}
			float fMean = (float)nSum / nCount;		//均值

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);	//标准差
			int nThreshold = fMean + 2 * fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;

			if (nThreshold > 150) nThreshold = 150;
			threshold(matSrc, matDst, nThreshold, 255, THRESH_BINARY);
#else
			threshold(matSrc, matDst, 60, 255, THRESH_BINARY);
#endif
		}
		break;
	case GRAY_CP: 
	case WHITE_CP:
	case SN:
	case OMR:
	default:
		// 局部自适应阈值的图像二值化
		int blockSize = 25;		//25
		int constValue = 10;
		cv::Mat local;
		cv::adaptiveThreshold(matSrc, matDst, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
		break;
	}
}

bool CMakeModelDlg::Recognise(cv::Rect rtOri)
{
	clock_t start, end;
	start = clock();
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return false;

	Mat imgResult = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtOri); 

	cvtColor(imgResult, imgResult, CV_BGR2GRAY);
	GaussianBlur(imgResult, imgResult, cv::Size(m_nGaussKernel, m_nGaussKernel), 0, 0);
	sharpenImage1(imgResult, imgResult);
#if 1
	GetThreshold(imgResult, imgResult);
#else
	// 局部自适应阈值的图像二值化
	int blockSize = 25;		//25
	int constValue = 10;
	cv::Mat local;
	cv::adaptiveThreshold(imgResult, imgResult, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
#endif
	cv::Canny(imgResult, imgResult, 0, m_nCannyKernel, 5);
	Mat element = getStructuringElement(MORPH_RECT, Size(m_nDelateKernel, m_nDelateKernel));	//Size(6, 6)	普通空白框可识别
	dilate(imgResult, imgResult, element);
#if 1
	IplImage ipl_img(imgResult);

	//the parm. for cvFindContours  
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;

	//提取轮廓  
	cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	bool bResult = false;
	std::vector<Rect>RectCompList;
	for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
	{
		CvRect aRect = cvBoundingRect(contour, 0);
		Rect rm = aRect;

		if (rm.width < 10 || rm.height < 7 || rm.width > 80 || rm.height > 80 || rm.area() < 40 || rm.area() > 6400)
		{//10,7
			TRACE("过滤矩形:(%d,%d,%d,%d), 面积: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
			g_pLogger->information("过滤矩形:(%d,%d,%d,%d), 面积: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
			continue;
		}

		rm.x = rm.x + rtOri.x/* - m_ptFixCP.x*/;
		rm.y = rm.y + rtOri.y/* - m_ptFixCP.y*/;

		RectCompList.push_back(rm);

		RECTINFO rc;
		rc.rt = rm;
		rc.eCPType = m_eCurCPType;
		RECTINFO rcOri;
		rcOri.rt = rtOri;

		if (m_eCurCPType == H_HEAD)
		{
			rc.nThresholdValue = m_nHeadVal;
			rc.fStandardValuePercent = m_fHeadThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.push_back(rc);
		}
		else if (m_eCurCPType == V_HEAD)
		{
			rc.nThresholdValue = m_nHeadVal;
			rc.fStandardValuePercent = m_fHeadThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.push_back(rc);
		}
		else if (m_eCurCPType == ABMODEL)
		{
			rc.nThresholdValue = m_nABModelVal;
			rc.fStandardValuePercent = m_fABModelThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.push_back(rc);
		}
		else if (m_eCurCPType == COURSE)
		{
			rc.nThresholdValue = m_nCourseVal;
			rc.fStandardValuePercent = m_fCourseThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.push_back(rc);
		}
		else if (m_eCurCPType == QK_CP)
		{
			rc.nThresholdValue = m_nQK_CPVal;
			rc.fStandardValuePercent = m_fQK_CPThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.push_back(rc);
		}
		else if (m_eCurCPType == GRAY_CP)
		{
			rc.nThresholdValue = m_nGrayVal;
			rc.fStandardValuePercent = m_fGrayThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.push_back(rc);
		}
		else if (m_eCurCPType == WHITE_CP)		//黑白提卡空白校验点设置不了，以后再解决
		{
			rc.nThresholdValue = m_nWhiteVal;
			rc.fStandardValuePercent = m_fWhiteThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.push_back(rc);
		}
		else if (m_eCurCPType == SN)
		{
			TRACE("SN - rt(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
// 			rc.nThresholdValue = m_nWhiteVal;
// 			rc.fStandardValuePercent = m_fSNThresholdPercent;
// 
// 			Rect rtTmp = rm;
// 			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
// 			RecogGrayValue(matSrcModel, rc);
		}
		else if (m_eCurCPType == OMR)
		{
			TRACE("OMR - rt(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
// 			rc.nThresholdValue = m_nWhiteVal;
// 			rc.fStandardValuePercent = m_fOMRThresholdPercent;
// 
// 			Rect rtTmp = rm;
// 			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
// 			RecogGrayValue(matSrcModel, rc);
		}

		bResult = true;
	}
	if (m_eCurCPType == SN)
	{
		GetSNArry(RectCompList);
	}
	if(m_eCurCPType == OMR)
	{
		GetOmrArry(RectCompList);
	}
#else
	m_vecContours.clear();
	cv::findContours(imgResult.clone(), m_vecContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);	//hsvRe.clone()		//CV_RETR_EXTERNAL	//CV_CHAIN_APPROX_SIMPLE

	bool bResult = false;
	std::vector<Rect>RectCompList;
	for (int i = 0; i < m_vecContours.size(); i++)
	{
		Rect rm = cv::boundingRect(cv::Mat(m_vecContours[i]));

		if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50||rm.area() < 70)
		{
//			TRACE("*****Rect %d x = %d,y = %d, width = %d, high = %d \n", i, rm.x, rm.y, rm.width, rm.height);
			continue;
		}
		
		rm.x = rm.x + rtOri.x/* - m_ptFixCP.x*/;
		rm.y = rm.y + rtOri.y/* - m_ptFixCP.y*/;

		RectCompList.push_back(rm);

		RECTINFO rc;
		rc.rt = rm;
		rc.eCPType = m_eCurCPType;
		RECTINFO rcOri;
		rcOri.rt = rtOri;
		
		if (m_eCurCPType == H_HEAD)
		{
			rc.nThresholdValue = m_nHeadVal;
			rc.fStandardValuePercent = m_fHeadThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.push_back(rc);
		}
		else if (m_eCurCPType == V_HEAD)
		{
			rc.nThresholdValue = m_nHeadVal;
			rc.fStandardValuePercent = m_fHeadThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.push_back(rc);
		}
		else if (m_eCurCPType == ABMODEL)
		{
			rc.nThresholdValue = m_nABModelVal;
			rc.fStandardValuePercent = m_fABModelThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.push_back(rc);
		}
		else if (m_eCurCPType == COURSE)
		{
			rc.nThresholdValue = m_nCourseVal;
			rc.fStandardValuePercent = m_fCourseThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.push_back(rc);
		}
		else if (m_eCurCPType == QK_CP)
		{
			rc.nThresholdValue = m_nQK_CPVal;
			rc.fStandardValuePercent = m_fQK_CPThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.push_back(rc);
		}
		else if (m_eCurCPType == GRAY_CP)
		{
			rc.nThresholdValue = m_nGrayVal;
			rc.fStandardValuePercent = m_fGrayThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.push_back(rc);
		}
		else if (m_eCurCPType == WHITE_CP)		//黑白提卡空白校验点设置不了，以后再解决
		{
			rc.nThresholdValue = m_nWhiteVal;
			rc.fStandardValuePercent = m_fWhiteThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.push_back(rc);
		}
		else if (m_eCurCPType == SN)
		{
			TRACE("SN - rt(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
		}
		else if (m_eCurCPType == OMR)
		{
			TRACE("OMR - rt(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
		}

		bResult = true;
	}
	if (m_eCurCPType == SN)
	{
		GetSNArry(RectCompList);
	}
	if (m_eCurCPType == OMR)
	{
		GetOmrArry(RectCompList);
	}
#endif
	if (m_eCurCPType == Fix_CP)
	{
		if (RectCompList.size() > 0)
		{
			m_ptFixCP = cv::Point(0, 0);
			std::sort(RectCompList.begin(), RectCompList.end(), SortByArea);
			
			RECTINFO rcFixSel;					//把定位点选择的矩形框存入，在扫描匹配的时候根据这个框来识别定点坐标
			rcFixSel.eCPType = m_eCurCPType;
			rcFixSel.rt = rtOri;
			RECTINFO rcFixRt;					//把定位点选择的矩形框存入，在扫描匹配的时候根据这个框来识别定点坐标
			rcFixRt.eCPType = m_eCurCPType;
			rcFixRt.rt = RectCompList[0];

			Rect rtTmp = RectCompList[0];
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			rcFixRt.nThresholdValue = m_nFixVal;
			rcFixRt.fStandardValuePercent = m_fFixThresholdPercent;
			RecogGrayValue(matSrcModel, rcFixRt);

			if (m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size() < 4)
			{
				m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.push_back(rcFixSel);
				m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.push_back(rcFixRt);
			}
			else
			{
				AfxMessageBox(_T("定点最多可设置4个"));
			}			
		}
	}
	end = clock();
//	PaintRecognisedRect();
	if (m_eCurCPType != OMR && m_eCurCPType != SN)
		ShowRectByCPType(m_eCurCPType);

//	end = clock();
	char szLog[200] = { 0 };
	sprintf_s(szLog, "Recognise time: %d, find rect counts: %d.\n", end - start, RectCompList.size());
	g_pLogger->information(szLog);
	TRACE(szLog);
	return bResult;
}

bool CMakeModelDlg::RecogByHead(cv::Rect rtOri)
{
	if (!m_pModel->nHasHead)
		return false;
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return false;
	if (!m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size())
	{
		AfxMessageBox(_T("请先设置水平同步头"));
		return false;
	}
	if (!m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size())
	{
		AfxMessageBox(_T("请先设置垂直同步头"));
		return false;
	}
	int nPosH_B = -1;
	int nPosH_E = -1;
	int nPosV_B = -1;
	int nPosV_E = -1;
	for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size(); i++)
	{
		RECTINFO& rtPosH = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i];
		if ((rtPosH.rt.tl().x >= rtOri.tl().x) && nPosH_B < 0)
		{
			nPosH_B = i;
		}
		if ((rtPosH.rt.br().x == rtOri.br().x) && nPosH_B >= 0)
		{
			nPosH_E = i;
			break;
		}
		if ((rtPosH.rt.br().x > rtOri.br().x) && nPosH_B >= 0)
		{
			nPosH_E = i - 1;
			break;
		}
		if (rtOri.br().x > rtPosH.rt.br().x  && nPosH_B >= 0)
			nPosH_E = i;
	}
	if (nPosH_E - nPosH_B < 0 || (nPosH_E < 0 && nPosH_B < 0))
		return false;

	for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size(); i++)
	{
		RECTINFO& rtPosV = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i];
		if ((rtPosV.rt.tl().y >= rtOri.tl().y) && nPosV_B < 0)
		{
			nPosV_B = i;
		}
		if ((rtPosV.rt.br().y == rtOri.br().y) && nPosV_B >= 0)
		{
			nPosV_E = i;
			break;
		}
		if ((rtPosV.rt.br().y > rtOri.br().y) && nPosV_B >= 0)
		{
			nPosV_E = i - 1;
			break;
		}
		if (rtOri.br().y > rtPosV.rt.br().y && nPosV_B >= 0)
			nPosV_E = i;
	}
	if (nPosV_E - nPosV_B < 0 || (nPosV_B < 0 && nPosV_E < 0))
		return false;

	cv::Rect** arr;
	arr = new cv::Rect*[nPosV_E - nPosV_B + 1];
	for (int i = 0; i < nPosV_E - nPosV_B + 1; i++)
	{
		arr[i] = new cv::Rect[nPosH_E - nPosH_B + 1];
		for (int j = 0; j < nPosH_E - nPosH_B + 1; j++)
		{
			arr[i][j].x = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[j + nPosH_B].rt.tl().x;
			arr[i][j].y = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i + nPosV_B].rt.tl().y;
			arr[i][j].width = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[j + nPosH_B].rt.width;
			arr[i][j].height = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i + nPosV_B].rt.height;
		}
	}

	m_vecTmp.clear();
	for (int i = 0; i < nPosV_E - nPosV_B + 1; i++)
	{
		for (int j = 0; j < nPosH_E - nPosH_B + 1; j++)
		{
			RECTINFO rc;
			rc.rt = arr[i][j];
			rc.eCPType = m_eCurCPType;
			rc.nHItem = j + nPosH_B;
			rc.nVItem = i + nPosV_B;
#if 1
			if (m_eCurCPType == ABMODEL)
			{
				rc.nThresholdValue = m_nABModelVal;
				rc.fStandardValuePercent = m_fABModelThresholdPercent;

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == COURSE)
			{
				rc.nThresholdValue = m_nCourseVal;
				rc.fStandardValuePercent = m_fCourseThresholdPercent;

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == QK_CP)
			{
				rc.nThresholdValue = m_nQK_CPVal;
				rc.fStandardValuePercent = m_fQK_CPThresholdPercent;

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == GRAY_CP)
			{
				rc.nThresholdValue = m_nGrayVal;
				rc.fStandardValuePercent = m_fGrayThresholdPercent;

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == WHITE_CP)
			{
				rc.nThresholdValue = m_nWhiteVal;
				rc.fStandardValuePercent = m_fWhiteThresholdPercent;

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == SN)
			{
				rc.nThresholdValue = m_nSN;
				rc.fStandardValuePercent = m_fSNThresholdPercent;

				switch (m_pSNInfoDlg->m_nCurrentSNVal)
				{
				case 10:
					rc.nTH = j;
					rc.nSnVal = i;
					break;
				case 9:
					rc.nTH = nPosH_E - nPosH_B - j; 
					rc.nSnVal = nPosV_E - nPosV_B - i;
					break;
				case 6:
					rc.nTH = nPosV_E - nPosV_B - i;
					rc.nSnVal = j;
					break;
				case 5:
					rc.nTH = i;
					rc.nSnVal = nPosH_E - nPosH_B - j;
					break;
				}
				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == OMR)
			{
				rc.nThresholdValue = m_nOMR;
				rc.fStandardValuePercent = m_fOMRThresholdPercent;

				switch (m_pOmrInfoDlg->m_nCurrentOmrVal)
				{
				case 42:
					rc.nTH = j;
					rc.nAnswer = i;
					break;
				case 41:
					rc.nTH = nPosH_E - nPosH_B - j;
					rc.nAnswer = i;
					break;
				case 38:
					rc.nTH = j;
					rc.nAnswer = nPosV_E - nPosV_B - i;
					break;
				case 37:
					rc.nTH = nPosH_E - nPosH_B - j;
					rc.nAnswer = nPosV_E - nPosV_B - i;
					break;
				case 26:
					rc.nTH = i;
					rc.nAnswer = j;
					break;
				case 25:
					rc.nTH = nPosV_E - nPosV_B - i;
					rc.nAnswer = j;
					break;
				case 22:
					rc.nTH = i;
					rc.nAnswer = nPosH_E - nPosH_B - j;
					break;
				case 21:
					rc.nTH = nPosV_E - nPosV_B - i;
					rc.nAnswer = nPosH_E - nPosH_B - j;
					break;
				}
				if (m_pOmrInfoDlg->m_bSingle)
					rc.nSingle = 0;
				else
					rc.nSingle = 1;

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
#endif
		}
	}

	ShowTmpRect();
	for (int i = 0; i < nPosV_E - nPosV_B + 1; i++)
	{
		SAFE_RELEASE(arr[i]);
	}
	SAFE_RELEASE(arr);
	

	return true;
}

void CMakeModelDlg::sharpenImage1(const cv::Mat &image, cv::Mat &result)
{
	//创建并初始化滤波模板
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = m_nSharpKernel;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//对图像进行滤波
	cv::filter2D(image, result, image.depth(), kernel);
}

bool CMakeModelDlg::checkValidity()
{
	bool bResult = true;

	USES_CONVERSION;
	if (m_pModel->nHasHead == 0)
	{
		for (int i = 0; i < m_vecPaperModelInfo.size(); i++)
		{
			if (m_vecPaperModelInfo[i]->vecRtFix.size() < 2)
			{
				char szTmp[50] = { 0 };
				sprintf_s(szTmp, "第 %d 页定点设置数量太少，要求至少2个，建议设置4个", i + 1);
				AfxMessageBox(A2T(szTmp));
				bResult = false;
				break;
			}
		}
	}
	else
	{
// 		for (int i = 0; i < m_pModel->vecPaperModel.size(); i++)
// 		{
// 			if (m_vecPaperModelInfo[i]->vecH_Head.size() <= 0)
// 			{
// 				char szTmp[50] = { 0 };
// 				sprintf_s(szTmp, "第 %d 页未设置水平同步头", i + 1);
// 				AfxMessageBox(A2T(szTmp));
// 				bResult = false;
// 				break;
// 			}
// 			if (m_vecPaperModelInfo[i]->vecV_Head.size() <= 0)
// 			{
// 				char szTmp[50] = { 0 };
// 				sprintf_s(szTmp, "第 %d 页未设置垂直同步头", i + 1);
// 				AfxMessageBox(A2T(szTmp));
// 				bResult = false;
// 				break;
// 			}
// 		}
	}

	return bResult;
}

void CMakeModelDlg::OnBnClickedBtnSave()
{
	if (!m_pModel)
	{
		AfxMessageBox(_T("请先创建模板"));
		return;
	}
	
	if (!checkValidity()) return;

	CModelSaveDlg dlg(m_pModel);
	if (dlg.DoModal() != IDOK)
		return;

	m_pModel->nSaveMode = dlg.m_nSaveMode;
	if (dlg.m_nSaveMode == 1)
	{
		m_pModel->strModelName = dlg.m_strModelName;
		m_pModel->strModelDesc = dlg.m_strLocalModelDesc;
	}
	else
	{
		USES_CONVERSION;

		char szModelName[30] = { 0 };
		sprintf_s(szModelName, "%d_%d", dlg.m_nExamID, dlg.m_SubjectID);
		char szModelDesc[300] = { 0 };
		sprintf_s(szModelDesc, "考试名称: %s\r\n科目: %s\r\n年级: %s\r\n考试类型名称: %s", T2A(dlg.m_strExamName), T2A(dlg.m_strSubjectName), T2A(dlg.m_strGradeName), T2A(dlg.m_strExamTypeName));
		m_pModel->nExamID		= dlg.m_nExamID;
		m_pModel->nSubjectID	= dlg.m_SubjectID;
		m_pModel->strModelName	= szModelName;
		m_pModel->strModelDesc	= szModelDesc;
	}

	CString strTitle = _T("");
	strTitle.Format(_T("模板名称: %s"), m_pModel->strModelName);
	SetWindowText(strTitle);

	m_bSavedModelFlag = true;
	
	for (int i = 0; i < m_pModel->vecPaperModel.size(); i++)
	{
		pPAPERMODEL pPaperModel = m_pModel->vecPaperModel[i];
		SAFE_RELEASE(pPaperModel);
	}
	m_pModel->vecPaperModel.clear();

	m_pModel->nPicNum = m_vecPaperModelInfo.size();
	for (int i = 0; i < m_pModel->nPicNum; i++)
	{
		pPAPERMODEL pPaperModel = new PAPERMODEL;
		pPaperModel->strModelPicName = m_vecPaperModelInfo[i]->strModelPicName;

		for (int j = 0; j < m_vecPaperModelInfo[i]->vecHTracker.size(); j++)
			pPaperModel->lSelHTracker.push_back(m_vecPaperModelInfo[i]->vecHTracker[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecVTracker.size(); j++)
			pPaperModel->lSelVTracker.push_back(m_vecPaperModelInfo[i]->vecVTracker[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecRtSel.size(); j++)
			pPaperModel->lSelFixRoi.push_back(m_vecPaperModelInfo[i]->vecRtSel[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecRtFix.size(); j++)
			pPaperModel->lFix.push_back(m_vecPaperModelInfo[i]->vecRtFix[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecOmr2.size(); j++)
			pPaperModel->lOMR2.push_back(m_vecPaperModelInfo[i]->vecOmr2[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecH_Head.size(); j++)
			pPaperModel->lH_Head.push_back(m_vecPaperModelInfo[i]->vecH_Head[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecV_Head.size(); j++)
			pPaperModel->lV_Head.push_back(m_vecPaperModelInfo[i]->vecV_Head[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecABModel.size(); j++)
			pPaperModel->lABModel.push_back(m_vecPaperModelInfo[i]->vecABModel[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecCourse.size(); j++)
			pPaperModel->lCourse.push_back(m_vecPaperModelInfo[i]->vecCourse[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecQK_CP.size(); j++)
			pPaperModel->lQK_CP.push_back(m_vecPaperModelInfo[i]->vecQK_CP[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecGray.size(); j++)
			pPaperModel->lGray.push_back(m_vecPaperModelInfo[i]->vecGray[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecWhite.size(); j++)
			pPaperModel->lWhite.push_back(m_vecPaperModelInfo[i]->vecWhite[j]);
		SNLIST::iterator itSn = m_vecPaperModelInfo[i]->lSN.begin();
		for (; itSn != m_vecPaperModelInfo[i]->lSN.end(); itSn++)
		{
			pSN_ITEM pSnItem = new SN_ITEM;
			pSnItem->nItem = (*itSn)->nItem;
			pSnItem->nRecogVal = (*itSn)->nRecogVal;
			RECTLIST::iterator itRc = (*itSn)->lSN.begin();
			for (; itRc != (*itSn)->lSN.end(); itRc++)
			{
				RECTINFO rc = *itRc;
				pSnItem->lSN.push_back(rc);
			}
// 			pSN_ITEM pSnItem = *itSn;
// 			itSn = m_vecPaperModelInfo[i]->lSN.erase(itSn);
			pPaperModel->lSNInfo.push_back(pSnItem);
		}
		//++ 有同步头的情况下，直接新建模板马上保存，需要设置水平和垂直橡皮筋的长度
		if (m_pModel->nHasHead && m_vecPaperModelInfo[i]->bFirstH && m_pModel->nType == 0)
		{
			m_ptHTracker1 = cv::Point(0, 0);
			m_ptHTracker2 = cv::Point(m_vecPaperModelInfo[i]->matSrcImg.cols, 90);
//			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH = false;

			m_vecPaperModelInfo[i]->rtHTracker.x = m_ptHTracker1.x;
			m_vecPaperModelInfo[i]->rtHTracker.y = m_ptHTracker1.y;
			m_vecPaperModelInfo[i]->rtHTracker.width = m_ptHTracker2.x - m_ptHTracker1.x;
			m_vecPaperModelInfo[i]->rtHTracker.height = m_ptHTracker2.y - m_ptHTracker1.y;
		}
		if (m_pModel->nHasHead && m_vecPaperModelInfo[i]->bFirstV && m_pModel->nType == 0)
		{
			m_ptVTracker1 = cv::Point(m_vecPaperModelInfo[i]->matSrcImg.cols - 90, 0);
			m_ptVTracker2 = cv::Point(m_vecPaperModelInfo[i]->matSrcImg.cols, m_vecPaperModelInfo[i]->matSrcImg.rows);
//			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV = false;

			m_vecPaperModelInfo[i]->rtVTracker.x = m_ptVTracker1.x;
			m_vecPaperModelInfo[i]->rtVTracker.y = m_ptVTracker1.y;
			m_vecPaperModelInfo[i]->rtVTracker.width = m_ptVTracker2.x - m_ptVTracker1.x;
			m_vecPaperModelInfo[i]->rtVTracker.height = m_ptVTracker2.y - m_ptVTracker1.y;
		}
		//--
		pPaperModel->rtHTracker = m_vecPaperModelInfo[i]->rtHTracker;
		pPaperModel->rtVTracker = m_vecPaperModelInfo[i]->rtVTracker;
		pPaperModel->rtSNTracker = m_vecPaperModelInfo[i]->rtSNTracker;

		pPaperModel->nPicW = m_vecPaperModelInfo[i]->nPicW;
		pPaperModel->nPicH = m_vecPaperModelInfo[i]->nPicH;

		m_pModel->vecPaperModel.push_back(pPaperModel);
	}
	USES_CONVERSION;
	CString modelPath = g_strCurrentPath + _T("Model");
	modelPath = modelPath + _T("\\") + m_pModel->strModelName;
	if (SaveModelFile(m_pModel))
	{
		ZipFile(modelPath, modelPath, _T(".mod"));
		AfxMessageBox(_T("保存完成!"));
	}
	else
		AfxMessageBox(_T("保存失败"));
}

bool CMakeModelDlg::SaveModelFile(pMODEL pModel)
{
	USES_CONVERSION;
	CString modelPath = g_strCurrentPath + _T("Model");
	DWORD dwAttr = GetFileAttributesA(T2A(modelPath));
	if (dwAttr == 0xFFFFFFFF)
	{
		CreateDirectoryA(T2A(modelPath), NULL);
	}
	modelPath = modelPath + _T("\\") + pModel->strModelName;
	dwAttr = GetFileAttributesA(T2A(modelPath));
	if (dwAttr == 0xFFFFFFFF)
	{
		dwAttr = CreateDirectoryA(T2A(modelPath), NULL);
	}

	Poco::JSON::Object jsnModel;
	Poco::JSON::Array  jsnPicModel;
	Poco::JSON::Array  jsnPaperModel;				//卷形模板，AB卷用
	for (int i = 0; i < pModel->nPicNum; i++)
	{
		Poco::JSON::Object jsnPaperObj;
		CString strOldPath = m_vecPaperModelInfo[i]->strModelPicPath;
		try
		{
			std::string strUtf8OldPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strOldPath));
			std::string strUtf8ModelPath = CMyCodeConvert::Gb2312ToUtf8(T2A(modelPath));
			
			Poco::File modelPicPath(strUtf8OldPath);	//T2A(strOldPath)
			modelPicPath.copyTo(strUtf8ModelPath);	//T2A(modelPath)
		}
		catch (Poco::Exception &exc)
		{
			std::string strLog;
			strLog.append("file cope error: " + exc.displayText());
			std::string strGBLog = CMyCodeConvert::Utf8ToGb2312(strLog);
			g_pLogger->information(strGBLog);
			TRACE(strGBLog.c_str());
		}

		CString strPicName = pModel->vecPaperModel[i]->strModelPicName;

		Poco::JSON::Array jsnSNArry;
		Poco::JSON::Array jsnSelHTrackerArry;
		Poco::JSON::Array jsnSelVTrackerArry;
		Poco::JSON::Array jsnSelRoiArry;
		Poco::JSON::Array jsnOMRArry;
		Poco::JSON::Array jsnFixCPArry;
		Poco::JSON::Array jsnHHeadArry;
		Poco::JSON::Array jsnVHeadArry;
		Poco::JSON::Array jsnABModelArry;
		Poco::JSON::Array jsnCourseArry;
		Poco::JSON::Array jsnQKArry;
		Poco::JSON::Array jsnGrayCPArry;
		Poco::JSON::Array jsnWhiteCPArry;
		RECTLIST::iterator itFix = pModel->vecPaperModel[i]->lFix.begin();
		for (; itFix != pModel->vecPaperModel[i]->lFix.end(); itFix++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itFix->eCPType);
			jsnObj.set("left", itFix->rt.x);
			jsnObj.set("top", itFix->rt.y);
			jsnObj.set("width", itFix->rt.width);
			jsnObj.set("height", itFix->rt.height);
			jsnObj.set("thresholdValue", itFix->nThresholdValue);
			jsnObj.set("standardValPercent", itFix->fStandardValuePercent);
			jsnObj.set("standardVal", itFix->fStandardValue);
			jsnFixCPArry.add(jsnObj);
		}
		RECTLIST::iterator itHHead = pModel->vecPaperModel[i]->lH_Head.begin();
		for (; itHHead != pModel->vecPaperModel[i]->lH_Head.end(); itHHead++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itHHead->eCPType);
			jsnObj.set("left", itHHead->rt.x);
			jsnObj.set("top", itHHead->rt.y);
			jsnObj.set("width", itHHead->rt.width);
			jsnObj.set("height", itHHead->rt.height);
			jsnObj.set("thresholdValue", itHHead->nThresholdValue);
			jsnObj.set("standardValPercent", itHHead->fStandardValuePercent);
			jsnObj.set("standardVal", itHHead->fStandardValue);
			jsnHHeadArry.add(jsnObj);
		}
		RECTLIST::iterator itVHead = pModel->vecPaperModel[i]->lV_Head.begin();
		for (; itVHead != pModel->vecPaperModel[i]->lV_Head.end(); itVHead++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itVHead->eCPType);
			jsnObj.set("left", itVHead->rt.x);
			jsnObj.set("top", itVHead->rt.y);
			jsnObj.set("width", itVHead->rt.width);
			jsnObj.set("height", itVHead->rt.height);
			jsnObj.set("thresholdValue", itVHead->nThresholdValue);
			jsnObj.set("standardValPercent", itVHead->fStandardValuePercent);
			jsnObj.set("standardVal", itVHead->fStandardValue);
			jsnVHeadArry.add(jsnObj);
		}
		RECTLIST::iterator itABModel = pModel->vecPaperModel[i]->lABModel.begin();
		for (; itABModel != pModel->vecPaperModel[i]->lABModel.end(); itABModel++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itABModel->eCPType);
			jsnObj.set("left", itABModel->rt.x);
			jsnObj.set("top", itABModel->rt.y);
			jsnObj.set("width", itABModel->rt.width);
			jsnObj.set("height", itABModel->rt.height);
			jsnObj.set("hHeadItem", itABModel->nHItem);
			jsnObj.set("vHeadItem", itABModel->nVItem);
			jsnObj.set("thresholdValue", itABModel->nThresholdValue);
			jsnObj.set("standardValPercent", itABModel->fStandardValuePercent);
			jsnObj.set("standardVal", itABModel->fStandardValue);
			jsnABModelArry.add(jsnObj);
		}
		RECTLIST::iterator itCourse = pModel->vecPaperModel[i]->lCourse.begin();
		for (; itCourse != pModel->vecPaperModel[i]->lCourse.end(); itCourse++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itCourse->eCPType);
			jsnObj.set("left", itCourse->rt.x);
			jsnObj.set("top", itCourse->rt.y);
			jsnObj.set("width", itCourse->rt.width);
			jsnObj.set("height", itCourse->rt.height);
			jsnObj.set("hHeadItem", itCourse->nHItem);
			jsnObj.set("vHeadItem", itCourse->nVItem);
			jsnObj.set("thresholdValue", itCourse->nThresholdValue);
			jsnObj.set("standardValPercent", itCourse->fStandardValuePercent);
			jsnObj.set("standardVal", itCourse->fStandardValue);
			jsnCourseArry.add(jsnObj);
		}
		RECTLIST::iterator itQKCP = pModel->vecPaperModel[i]->lQK_CP.begin();
		for (; itQKCP != pModel->vecPaperModel[i]->lQK_CP.end(); itQKCP++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itQKCP->eCPType);
			jsnObj.set("left", itQKCP->rt.x);
			jsnObj.set("top", itQKCP->rt.y);
			jsnObj.set("width", itQKCP->rt.width);
			jsnObj.set("height", itQKCP->rt.height);
			jsnObj.set("hHeadItem", itQKCP->nHItem);
			jsnObj.set("vHeadItem", itQKCP->nVItem);
			jsnObj.set("thresholdValue", itQKCP->nThresholdValue);
			jsnObj.set("standardValPercent", itQKCP->fStandardValuePercent);
			jsnObj.set("standardVal", itQKCP->fStandardValue);
			jsnQKArry.add(jsnObj);
		}
		RECTLIST::iterator itGrayCP = pModel->vecPaperModel[i]->lGray.begin();
		for (; itGrayCP != pModel->vecPaperModel[i]->lGray.end(); itGrayCP++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itGrayCP->eCPType);
			jsnObj.set("left", itGrayCP->rt.x);
			jsnObj.set("top", itGrayCP->rt.y);
			jsnObj.set("width", itGrayCP->rt.width);
			jsnObj.set("height", itGrayCP->rt.height);
			jsnObj.set("hHeadItem", itGrayCP->nHItem);
			jsnObj.set("vHeadItem", itGrayCP->nVItem);
			jsnObj.set("thresholdValue", itGrayCP->nThresholdValue);
			jsnObj.set("standardValPercent", itGrayCP->fStandardValuePercent);
			jsnObj.set("standardVal", itGrayCP->fStandardValue);
			jsnGrayCPArry.add(jsnObj);
		}
		RECTLIST::iterator itWhiteCP = pModel->vecPaperModel[i]->lWhite.begin();
		for (; itWhiteCP != pModel->vecPaperModel[i]->lWhite.end(); itWhiteCP++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itWhiteCP->eCPType);
			jsnObj.set("left", itWhiteCP->rt.x);
			jsnObj.set("top", itWhiteCP->rt.y);
			jsnObj.set("width", itWhiteCP->rt.width);
			jsnObj.set("height", itWhiteCP->rt.height);
			jsnObj.set("hHeadItem", itWhiteCP->nHItem);
			jsnObj.set("vHeadItem", itWhiteCP->nVItem);
			jsnObj.set("thresholdValue", itWhiteCP->nThresholdValue);
			jsnObj.set("standardValPercent", itWhiteCP->fStandardValuePercent);
			jsnObj.set("standardVal", itWhiteCP->fStandardValue);
			jsnWhiteCPArry.add(jsnObj);
		}
		RECTLIST::iterator itSelRoi = pModel->vecPaperModel[i]->lSelFixRoi.begin();
		for (; itSelRoi != pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itSelRoi->eCPType);
			jsnObj.set("left", itSelRoi->rt.x);
			jsnObj.set("top", itSelRoi->rt.y);
			jsnObj.set("width", itSelRoi->rt.width);
			jsnObj.set("height", itSelRoi->rt.height);
			jsnObj.set("thresholdValue", itSelRoi->nThresholdValue);
			jsnObj.set("standardValPercent", itSelRoi->fStandardValuePercent);
//			jsnObj.set("standardVal", itSelRoi->fStandardValue);
			jsnSelRoiArry.add(jsnObj);
		}
		RECTLIST::iterator itSelHTracker = pModel->vecPaperModel[i]->lSelHTracker.begin();
		for (; itSelHTracker != pModel->vecPaperModel[i]->lSelHTracker.end(); itSelHTracker++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itSelHTracker->eCPType);
			jsnObj.set("left", itSelHTracker->rt.x);
			jsnObj.set("top", itSelHTracker->rt.y);
			jsnObj.set("width", itSelHTracker->rt.width);
			jsnObj.set("height", itSelHTracker->rt.height);
			jsnObj.set("thresholdValue", itSelHTracker->nThresholdValue);
			jsnObj.set("standardValPercent", itSelHTracker->fStandardValuePercent);
			//			jsnObj.set("standardVal", itSelHTracker->fStandardValue);
			jsnSelHTrackerArry.add(jsnObj);
		}
		RECTLIST::iterator itSelVTracker = pModel->vecPaperModel[i]->lSelVTracker.begin();
		for (; itSelVTracker != pModel->vecPaperModel[i]->lSelVTracker.end(); itSelVTracker++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itSelVTracker->eCPType);
			jsnObj.set("left", itSelVTracker->rt.x);
			jsnObj.set("top", itSelVTracker->rt.y);
			jsnObj.set("width", itSelVTracker->rt.width);
			jsnObj.set("height", itSelVTracker->rt.height);
			jsnObj.set("thresholdValue", itSelVTracker->nThresholdValue);
			jsnObj.set("standardValPercent", itSelVTracker->fStandardValuePercent);
			//			jsnObj.set("standardVal", itSelVTracker->fStandardValue);
			jsnSelVTrackerArry.add(jsnObj);
		}
		OMRLIST::iterator itOmr = pModel->vecPaperModel[i]->lOMR2.begin();
		for (; itOmr != pModel->vecPaperModel[i]->lOMR2.end(); itOmr++)
		{
			Poco::JSON::Object jsnTHObj;
			Poco::JSON::Array  jsnArry;
			RECTLIST::iterator itOmrSel = itOmr->lSelAnswer.begin();
			for (; itOmrSel != itOmr->lSelAnswer.end(); itOmrSel++)
			{
				Poco::JSON::Object jsnObj;
				jsnObj.set("eType", (int)itOmrSel->eCPType);
				jsnObj.set("nTH", itOmrSel->nTH);
				jsnObj.set("nAnswer", itOmrSel->nAnswer);
				jsnObj.set("nSingle", itOmrSel->nSingle);
				jsnObj.set("nOmrRecogFlag", itOmrSel->nRecogFlag);
				jsnObj.set("left", itOmrSel->rt.x);
				jsnObj.set("top", itOmrSel->rt.y);
				jsnObj.set("width", itOmrSel->rt.width);
				jsnObj.set("height", itOmrSel->rt.height);
				jsnObj.set("hHeadItem", itOmrSel->nHItem);
				jsnObj.set("vHeadItem", itOmrSel->nVItem);
				jsnObj.set("thresholdValue", itOmrSel->nThresholdValue);
				jsnObj.set("standardValPercent", itOmrSel->fStandardValuePercent);
				jsnObj.set("standardVal", itOmrSel->fStandardValue);
				jsnArry.add(jsnObj);
			}
			jsnTHObj.set("nTH", itOmr->nTH);
			jsnTHObj.set("nSingle", itOmr->nSingle);
			jsnTHObj.set("omrlist", jsnArry);
			jsnOMRArry.add(jsnTHObj);
		}
		SNLIST::iterator itSn = pModel->vecPaperModel[i]->lSNInfo.begin();
		for (; itSn != pModel->vecPaperModel[i]->lSNInfo.end(); itSn++)
		{
			Poco::JSON::Object jsnSNObj;
			Poco::JSON::Array  jsnArry;
			RECTLIST::iterator itSnDetail = (*itSn)->lSN.begin();
			for (; itSnDetail != (*itSn)->lSN.end(); itSnDetail++)
			{
				Poco::JSON::Object jsnObj;
				jsnObj.set("eType", (int)itSnDetail->eCPType);
				jsnObj.set("nTH", itSnDetail->nTH);
				jsnObj.set("nSnVal", itSnDetail->nSnVal);
				jsnObj.set("nAnswer", itSnDetail->nAnswer);
				jsnObj.set("nSingle", itSnDetail->nSingle);
				jsnObj.set("nSnRecogFlag", itSnDetail->nRecogFlag);
				jsnObj.set("left", itSnDetail->rt.x);
				jsnObj.set("top", itSnDetail->rt.y);
				jsnObj.set("width", itSnDetail->rt.width);
				jsnObj.set("height", itSnDetail->rt.height);
				jsnObj.set("hHeadItem", itSnDetail->nHItem);
				jsnObj.set("vHeadItem", itSnDetail->nVItem);
				jsnObj.set("thresholdValue", itSnDetail->nThresholdValue);
				jsnObj.set("standardValPercent", itSnDetail->fStandardValuePercent);
				jsnObj.set("standardVal", itSnDetail->fStandardValue);
				jsnArry.add(jsnObj);
			}
			jsnSNObj.set("nItem", (*itSn)->nItem);
			jsnSNObj.set("nRecogVal", (*itSn)->nRecogVal);
			jsnSNObj.set("snList", jsnArry);
			jsnSNArry.add(jsnSNObj);
		}
		jsnPaperObj.set("paperNum", i);
		jsnPaperObj.set("modelPicName", CMyCodeConvert::Gb2312ToUtf8(T2A(strPicName)));		//CMyCodeConvert::Gb2312ToUtf8(T2A(strPicName))
		jsnPaperObj.set("FixCP", jsnFixCPArry);
		jsnPaperObj.set("H_Head", jsnHHeadArry);
		jsnPaperObj.set("V_Head", jsnVHeadArry);
		jsnPaperObj.set("ABModel", jsnABModelArry);
		jsnPaperObj.set("Course", jsnCourseArry);
		jsnPaperObj.set("QKCP", jsnQKArry);
		jsnPaperObj.set("GrayCP", jsnGrayCPArry);
		jsnPaperObj.set("WhiteCP", jsnWhiteCPArry);
		jsnPaperObj.set("selRoiRect", jsnSelRoiArry);
		jsnPaperObj.set("hTrackerRect", jsnSelHTrackerArry);
		jsnPaperObj.set("vTrackerRect", jsnSelVTrackerArry);
		jsnPaperObj.set("selOmrRect", jsnOMRArry);
		jsnPaperObj.set("snList", jsnSNArry);

		jsnPaperObj.set("picW", m_vecPaperModelInfo[i]->nPicW);		//add on 16.8.29
		jsnPaperObj.set("picH", m_vecPaperModelInfo[i]->nPicH);		//add on 16.8.29
		jsnPaperObj.set("rtHTracker.x", m_vecPaperModelInfo[i]->rtHTracker.x);
		jsnPaperObj.set("rtHTracker.y", m_vecPaperModelInfo[i]->rtHTracker.y);
		jsnPaperObj.set("rtHTracker.width", m_vecPaperModelInfo[i]->rtHTracker.width);
		jsnPaperObj.set("rtHTracker.height", m_vecPaperModelInfo[i]->rtHTracker.height);
		jsnPaperObj.set("rtVTracker.x", m_vecPaperModelInfo[i]->rtVTracker.x);
		jsnPaperObj.set("rtVTracker.y", m_vecPaperModelInfo[i]->rtVTracker.y);
		jsnPaperObj.set("rtVTracker.width", m_vecPaperModelInfo[i]->rtVTracker.width);
		jsnPaperObj.set("rtVTracker.height", m_vecPaperModelInfo[i]->rtVTracker.height);
		jsnPaperObj.set("rtSNTracker.x", m_vecPaperModelInfo[i]->rtSNTracker.x);
		jsnPaperObj.set("rtSNTracker.y", m_vecPaperModelInfo[i]->rtSNTracker.y);
		jsnPaperObj.set("rtSNTracker.width", m_vecPaperModelInfo[i]->rtSNTracker.width);
		jsnPaperObj.set("rtSNTracker.height", m_vecPaperModelInfo[i]->rtSNTracker.height);
		
		jsnPicModel.add(jsnPaperObj);
	}
	
	jsnModel.set("modelName", CMyCodeConvert::Gb2312ToUtf8(T2A(pModel->strModelName)));		//CMyCodeConvert::Gb2312ToUtf8(T2A(pModel->strModelName))
	jsnModel.set("modelDesc", CMyCodeConvert::Gb2312ToUtf8(T2A(pModel->strModelDesc)));
	jsnModel.set("modelType", pModel->nType);
	jsnModel.set("modeSaveMode", pModel->nSaveMode);
	jsnModel.set("paperModelCount", pModel->nPicNum);			//此模板有几页试卷(图片)
	jsnModel.set("enableModify", pModel->nEnableModify);		//是否可以修改标识
	jsnModel.set("abPaper", pModel->nABModel);					//是否是AB卷					*************	暂时没加入AB卷的模板	**************
	jsnModel.set("hasHead", pModel->nHasHead);					//是否有同步头
	jsnModel.set("nExamId", pModel->nExamID);
	jsnModel.set("nSubjectId", pModel->nSubjectID);
	jsnModel.set("paperInfo", jsnPicModel);

	std::stringstream jsnString;
	jsnModel.stringify(jsnString, 0);

	std::string strFileData;
#ifdef USES_FILE_ENC
	if(!encString(jsnString.str(), strFileData))
		strFileData = jsnString.str();
#else
	strFileData = jsnString.str();
#endif

	std::string strJsnFile = T2A(modelPath);
	strJsnFile += "\\model.dat";
	ofstream out(strJsnFile);
	if (!out)	return false;
	out << strFileData.c_str();
	out.close();
	
	return true;
}

void CMakeModelDlg::OnBnClickedBtnExitmodeldlg()
{
	if (m_bNewModelFlag && !m_bSavedModelFlag)
		SAFE_RELEASE(m_pModel);
	OnOK();
}

void CMakeModelDlg::ShowRectTracker()
{
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	TRACE("show ShowRectTracker.\n");

	Rect rt;
	if (m_eCurCPType == H_HEAD)
	{
		cv::Point pt1 = m_pModelPicShow->m_picShow.m_ptHTracker1;
		cv::Point pt2 = m_pModelPicShow->m_picShow.m_ptHTracker2;

		TRACE("橡皮筋填充区显示pt1 = (%d, %d), pt2 = (%d, %d)\n", pt1.x, pt1.y, pt2.x, pt2.y);
		rt = cv::Rect(pt1, pt2);
	}
	else if (m_eCurCPType == V_HEAD)
	{
		cv::Point pt1 = m_pModelPicShow->m_picShow.m_ptVTracker1;
		cv::Point pt2 = m_pModelPicShow->m_picShow.m_ptVTracker2;
		rt = cv::Rect(pt1, pt2);
	}
	else if (m_eCurCPType == SN)
	{
		cv::Point pt1 = m_pModelPicShow->m_picShow.m_ptSNTracker1;
		cv::Point pt2 = m_pModelPicShow->m_picShow.m_ptSNTracker2;
		rt = cv::Rect(pt1, pt2);
	}

	Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();

	rectangle(tmp, rt, CV_RGB(255, 20, 50), 2);
	rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);

	cv::addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
	m_pModelPicShow->ShowPic(tmp);
}
bool CMakeModelDlg::ShowRectByPoint(cv::Point pt)
{
	int  nFind = 0;

	m_pCurRectInfo = NULL;
	nFind = GetRectInfoByPoint(pt, m_eCurCPType, m_pCurRectInfo);
	if(nFind < 0)
		return false;

	if (!m_pCurRectInfo)
		return false;

	if (m_pRecogInfoDlg)	m_pRecogInfoDlg->ShowDetailRectInfo(m_pCurRectInfo);

	if (m_pCurRectInfo->eCPType == SN && m_pSNInfoDlg)
		m_pSNInfoDlg->ShowUI(m_pCurRectInfo->nRecogFlag);
	else if (m_pCurRectInfo->eCPType == OMR && m_pOmrInfoDlg)
		m_pOmrInfoDlg->ShowUI(m_pCurRectInfo->nRecogFlag, m_pCurRectInfo->nSingle);
	InitShowSnOmrDlg(m_pCurRectInfo->eCPType);

	Rect rt = m_pCurRectInfo->rt;
	Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();

	cv::rectangle(tmp, rt, CV_RGB(20, 225, 25), 2);
	cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
	CPType eType = m_eCurCPType;
	switch (eType)
	{
	case UNKNOWN:
	case Fix_CP:
		if (eType == Fix_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].eCPType)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].rt;
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case H_HEAD:
		if (eType == H_HEAD || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i].eCPType)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i].rt;
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case V_HEAD:
		if (eType == V_HEAD || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i].eCPType)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i].rt;
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case ABMODEL:
		if (eType == ABMODEL || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i].eCPType)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i].rt;
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case COURSE:
		if (eType == COURSE || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i].eCPType)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i].rt;
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case QK_CP:
		if (eType == QK_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i].eCPType)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i].rt;
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case GRAY_CP:
		if (eType == GRAY_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i].eCPType)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i].rt;
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case WHITE_CP:
		if (eType == WHITE_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i].eCPType)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i].rt;
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case SN:
		if (eType == SN || eType == UNKNOWN)
		{
			SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
			for (int i = 0; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++, i++)
			{
				RECTLIST::iterator itSNRect = (*itSNItem)->lSN.begin();
				for (int j = 0; itSNRect != (*itSNItem)->lSN.end(); itSNRect++, j++)
				{
					if (m_pCurRectInfo != &(*itSNRect))
					{
						RECTINFO rc = *itSNRect;
						cv::Rect rt = rc.rt;
						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d_%d", rc.nTH, rc.nSnVal);
						rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);

						putText(tmp, szAnswerVal, Point(rt.x + rt.width / 10, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
						rectangle(tmp2, rt, CV_RGB(50, 200, 150), -1);
					}
					else
					{
						RECTINFO rc = *itSNRect;
						cv::Rect rt = rc.rt;
						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d_%d", rc.nTH, rc.nSnVal);
						cv::rectangle(tmp, rt, CV_RGB(40, 22, 255), 2);

						putText(tmp, szAnswerVal, Point(rt.x + rt.width / 10, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255));	//CV_FONT_HERSHEY_COMPLEX
						rectangle(tmp2, rt, CV_RGB(50, 55, 255), -1);
					}
				}
			}
		}
	case OMR:
		if (eType == OMR || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); i++)
			{
				RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.begin();
				for (int j = 0; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.end(); itAnswer++, j++)
				{
					if (m_pCurRectInfo != &(*itAnswer))
					{
						RECTINFO rc = *itAnswer;
						cv::Rect rt = rc.rt;
						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer + 65);
						if (rc.nSingle == 0)
						{
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
							rectangle(tmp2, rt, CV_RGB(50, 155, 100), -1);
						}
						else
						{
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));
							rectangle(tmp2, rt, CV_RGB(100, 50, 200), -1);
						}
					}
					else
					{
						RECTINFO rc = *itAnswer;
						cv::Rect rt = rc.rt;
						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer + 65);
						cv::rectangle(tmp, rt, CV_RGB(40, 22, 255), 2);
						if (rc.nSingle == 0)
						{
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0,255));	//CV_FONT_HERSHEY_COMPLEX
							rectangle(tmp2, rt, CV_RGB(50, 55, 255), -1);
						}
						else
						{
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0,255));
							rectangle(tmp2, rt, CV_RGB(100, 250, 150), -1);
						}
					}
				}
			}
		}
	}
	cv::addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
	m_pModelPicShow->ShowPic(tmp);
	return true;
}

void CMakeModelDlg::ShowRectByItem(int nItem)
{
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	bool bFindOmr = false;
	bool bFindSN = false;
	int nSNCount = 0;
	int nOmrCount = 0;
	cv::Rect rt;
#if 1
	int nCount = 0;
	int nCurItem = nItem;
	bool bFind = false;
	if (m_eCurCPType == Fix_CP || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size();
	}
	if (m_eCurCPType == H_HEAD || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size();
	}
	if (m_eCurCPType == V_HEAD || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size();
	}
	if (m_eCurCPType == ABMODEL || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size();
	}
	if (m_eCurCPType == COURSE || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size();
	}
	if (m_eCurCPType == QK_CP || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size();
	}
	if (m_eCurCPType == GRAY_CP || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size();
	}
	if (m_eCurCPType == WHITE_CP || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size();
	}
	if (m_eCurCPType == SN || m_eCurCPType == UNKNOWN)
	{
		if (!bFind)
		{
			SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
			for (int i = 0; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++, i++)
			{
				nSNCount = (*itSNItem)->lSN.size();
				if (nCurItem < nSNCount)
				{
					RECTLIST::iterator itSNRect = (*itSNItem)->lSN.begin();
					for (int j = 0; itSNRect != (*itSNItem)->lSN.end(); itSNRect++, j++)
					{
						if (j == nCurItem)
						{
							RECTINFO rc = *itSNRect;
							rt = rc.rt;
							m_pCurRectInfo = &(*itSNRect);
							bFindSN = true;
							bFind = true;
							break;
						}
					}
					if (bFindSN)
						break;
				}
				else
				{
					nCurItem -= nSNCount;
				}
			}
		}		
	}
	if (m_eCurCPType == OMR || m_eCurCPType == UNKNOWN)
	{
		if (!bFind)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); i++)
			{
				nOmrCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.size();
				if (nCurItem < nOmrCount)
				{
					RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.begin();
					for (int j = 0; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.end(); itAnswer++, j++)
					{
						if (j == nCurItem)
						{
							bFindOmr = true;
							bFind = true;
							rt = itAnswer->rt;
							m_pCurRectInfo = &(*itAnswer);
							break;
						}
					}
					if (bFindOmr)
						break;
				}
				else
				{
					nCurItem -= nOmrCount;
				}
			}
		}		
	}
#else
	switch (m_eCurCPType)
	{
	case Fix_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size() < nItem)
			return;
		rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[nItem].rt;
		m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[nItem];
		break;
	case H_HEAD:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() < nItem)
			return;
		rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nItem].rt;
		m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nItem];
		break;
	case V_HEAD:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size() < nItem)
			return;
		rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[nItem].rt;
		m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[nItem];
		break;
	case ABMODEL:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size() < nItem)
			return;
		rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[nItem].rt;
		m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[nItem];
		break;
	case COURSE:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size() < nItem)
			return;
		rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[nItem].rt;
		m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[nItem];
		break;
	case QK_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size() < nItem)
			return;
		rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[nItem].rt;
		m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[nItem];
		break;
	case GRAY_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size() < nItem)
			return;
		rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[nItem].rt;
		m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[nItem];
		break;
	case WHITE_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size() < nItem)
			return;
		rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[nItem].rt;
		m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[nItem];
		break;
	case SN:
	{
		SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
		for (int i = 0; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++, i++)
		{
			nSNCount = (*itSNItem)->lSN.size();
			if (nItem < nSNCount)
			{
				RECTLIST::iterator itSNRect = (*itSNItem)->lSN.begin();
				for (int j = 0; itSNRect != (*itSNItem)->lSN.end(); itSNRect++, j++)
				{
					if (j == nItem)
					{
						RECTINFO rc = *itSNRect;
						rt = rc.rt;
						m_pCurRectInfo = &(*itSNRect);
						bFindSN = true;
						break;
					}
				}
				if (bFindSN)
					break;
			}
			else
			{
				nItem -= nSNCount;
			}
		}
		break;
	}
	case OMR:
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); i++)
		{
			nOmrCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.size();
			if (nItem < nOmrCount)
			{
				RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.begin();
				for (int j = 0; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.end(); itAnswer++, j++)
				{
					if (j == nItem)
					{
						bFindOmr = true;
						rt = itAnswer->rt;
						m_pCurRectInfo = &(*itAnswer);
						break;
					}
				}
				if (bFindOmr)
					break;
			}
			else
			{
				nItem -= nOmrCount;
			}
		}
		break;
	default: return;
	}
#endif
	if(m_pRecogInfoDlg)	m_pRecogInfoDlg->ShowDetailRectInfo(m_pCurRectInfo);

	Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();

	cv::Point pt = rt.tl();
	pt.x -= 100;
	pt.y -= 100;

	rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
	rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);

	cv::addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
	m_pModelPicShow->ShowPic(tmp, pt);
}

void CMakeModelDlg::ShowTmpRect()
{
	if (!m_pModel || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	cv::Rect rt;
	Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	for (int i = 0; i < m_vecTmp.size(); i++)
	{
		rt = m_vecTmp[i].rt;
		rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
//		rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		if (m_vecTmp[i].eCPType == OMR)
		{
			char szAnswerVal[10] = { 0 };
//			sprintf_s(szAnswerVal, "%c", m_vecTmp[i].nAnswer + 65);
			sprintf_s(szAnswerVal, "%d%c", m_vecTmp[i].nTH, m_vecTmp[i].nAnswer + 65);
			putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp2, rt, CV_RGB(50, 255, 100), -1);
		}
		else if (m_vecTmp[i].eCPType == SN)
		{
			char szAnswerVal[10] = { 0 };
			sprintf_s(szAnswerVal, "%d_%d", m_vecTmp[i].nTH, m_vecTmp[i].nSnVal);
			putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp2, rt, CV_RGB(50, 255, 100), -1);
		}
		else 
			rectangle(tmp2, rt, CV_RGB(150, 100, 255), -1);
		
	}
	cv::addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
	m_pModelPicShow->ShowPic(tmp);
}

void CMakeModelDlg::ShowRectByCPType(CPType eType)
{
	if (!m_pModel || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	cv::Rect rt;
	cv::Point ptFix = cv::Point(0, 0);
	if (m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size())
	{
		cv::Rect rtFix = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[0].rt;
		ptFix = m_ptFixCP;
	}	
	Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	switch (eType)
	{
	case UNKNOWN:
	case Fix_CP:
		if (eType == Fix_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size(); i++)
			{
				rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].rt;
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case H_HEAD:
		if (eType == H_HEAD || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size(); i++)
			{
				rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i].rt;
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			if (eType == H_HEAD && m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() == 0)
			{
				cv::Point pt1 = m_pModelPicShow->m_picShow.m_ptHTracker1;
				cv::Point pt2 = m_pModelPicShow->m_picShow.m_ptHTracker2;

				rt = cv::Rect(pt1, pt2);
				cv::rectangle(tmp, rt, CV_RGB(255, 20, 50), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case V_HEAD:
		if (eType == V_HEAD || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size(); i++)
			{
				rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i].rt;
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			if (eType == V_HEAD && m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size() == 0)
			{
				cv::Point pt1 = m_pModelPicShow->m_picShow.m_ptVTracker1;
				cv::Point pt2 = m_pModelPicShow->m_picShow.m_ptVTracker2;

				rt = cv::Rect(pt1, pt2);
				cv::rectangle(tmp, rt, CV_RGB(255, 20, 50), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case ABMODEL:
		if (eType == ABMODEL || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size(); i++)
			{
				rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i].rt;
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case COURSE:
		if (eType == COURSE || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size(); i++)
			{
				rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i].rt;
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case QK_CP:
		if (eType == QK_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size(); i++)
			{
				rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i].rt;
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case GRAY_CP:
		if (eType == GRAY_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size(); i++)
			{
				rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i].rt;
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case WHITE_CP:
		if (eType == WHITE_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size(); i++)
			{
				rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i].rt;
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case SN:
		if (eType == SN || eType == UNKNOWN)
		{
			SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
			for (; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++)
			{
				RECTLIST::iterator itSNRect = (*itSNItem)->lSN.begin();
				for (; itSNRect != (*itSNItem)->lSN.end(); itSNRect++)
				{
					RECTINFO rc = *itSNRect;
					rt = rc.rt;

					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);

					char szAnswerVal[10] = { 0 };
					sprintf_s(szAnswerVal, "%d_%d", rc.nTH, rc.nSnVal);
					
					cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 10, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
					cv::rectangle(tmp2, rt, CV_RGB(50, 200, 150), -1);
				}
			}
		}
	case OMR:
		if (eType == OMR || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); i++)
			{
				RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.begin();
				for (; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.end(); itAnswer++)
				{
					RECTINFO rc = *itAnswer;
					rt = rc.rt;

					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);

					char szAnswerVal[10] = { 0 };
					sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer + 65);
					if (rc.nSingle == 0)
					{
						cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
						cv::rectangle(tmp2, rt, CV_RGB(50, 255, 100), -1);
					}
					else
					{
						cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 100, 0));
						cv::rectangle(tmp2, rt, CV_RGB(150, 150, 255), -1);
					}
				}
			}
		}
		break;
	default: return;
	}
	cv::addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
	m_pModelPicShow->ShowPic(tmp);
}

CPType CMakeModelDlg::GetComboSelCpType()
{
	CPType eType = UNKNOWN;
	CString strCheckPoint = _T("");
	m_comboCheckPointType.GetLBText(m_ncomboCurrentSel, strCheckPoint);
	if (strCheckPoint == "")
		eType = UNKNOWN;
	else if (strCheckPoint == "定点")
		eType = Fix_CP;
	else if (strCheckPoint == "水平同步头")
		eType = H_HEAD;
	else if (strCheckPoint == "垂直同步头")
		eType = V_HEAD;
	else if (strCheckPoint == "卷形校验点")
		eType = ABMODEL;
	else if (strCheckPoint == "科目校验点")
		eType = COURSE;
	else if (strCheckPoint == "缺考校验点")
		eType = QK_CP;
	else if (strCheckPoint == "灰度校验点")
		eType = GRAY_CP;
	else if (strCheckPoint == "空白校验点")
		eType = WHITE_CP;
	else if (strCheckPoint == "考号设置")
		eType = SN;
	else if (strCheckPoint == "选择题")
		eType = OMR;
	return eType;
}

void CMakeModelDlg::OnCbnSelchangeComboCptype()
{
	if (m_ncomboCurrentSel == m_comboCheckPointType.GetCurSel())
		return;

	m_ncomboCurrentSel	= m_comboCheckPointType.GetCurSel();
	m_eCurCPType		= GetComboSelCpType();
	UpdataCPList();
	ShowRectByCPType(m_eCurCPType);

	switch (m_eCurCPType)
	{
		case SN:
		{
			m_nDelateKernel = 4;
		}
		break;
		default:
			m_nDelateKernel = 6;
			break;
	}

	InitShowSnOmrDlg(m_eCurCPType);
}

void CMakeModelDlg::InitShowSnOmrDlg(CPType eType)
{
	if (eType == OMR)
	{
		m_pOmrInfoDlg->ShowWindow(SW_SHOW);
		m_pRecogInfoDlg->ShowWindow(SW_HIDE);
		m_pSNInfoDlg->ShowWindow(SW_HIDE);
	}
	else if (eType == SN)
	{
		m_pOmrInfoDlg->ShowWindow(SW_HIDE);
		m_pRecogInfoDlg->ShowWindow(SW_HIDE);
		m_pSNInfoDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		m_pOmrInfoDlg->ShowWindow(SW_HIDE);
		m_pRecogInfoDlg->ShowWindow(SW_SHOW);
		m_pSNInfoDlg->ShowWindow(SW_HIDE);
	}
}

void CMakeModelDlg::UpdataCPList()
{
	if (!m_pModel || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	USES_CONVERSION;
	int nCount = 0;
	m_cpListCtrl.DeleteAllItems();
	if (m_eCurCPType == H_HEAD)
	{
		m_pModelPicShow->SetShowTracker(true, false, false);
		if (m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH && m_bNewModelFlag)
		{
			m_ptHTracker1 = cv::Point(0, 0);
			m_ptHTracker2 = cv::Point(m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols, 90);
			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH = false;

			m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.x = m_ptHTracker1.x;
			m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.y = m_ptHTracker1.y;
			m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.width = m_ptHTracker2.x - m_ptHTracker1.x;
			m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.height = m_ptHTracker2.y - m_ptHTracker1.y;
		}
		else
		{
			m_ptHTracker1 = m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.tl();
			m_ptHTracker2 = m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.br();
			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH = false;
		}		
		m_pModelPicShow->m_picShow.setHTrackerPosition(m_ptHTracker1, m_ptHTracker2);
	}
	else if (m_eCurCPType == V_HEAD)
	{
		m_pModelPicShow->SetShowTracker(false, true, false);
		if (m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV && m_bNewModelFlag)
		{
			m_ptVTracker1 = cv::Point(m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols - 90, 0);
			m_ptVTracker2 = cv::Point(m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols, m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows);
			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV = false;

			m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.x = m_ptVTracker1.x;
			m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.y = m_ptVTracker1.y;
			m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.width = m_ptVTracker2.x - m_ptVTracker1.x;
			m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.height = m_ptVTracker2.y - m_ptVTracker1.y;
		}
		else
		{
			m_ptVTracker1 = m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.tl();
			m_ptVTracker2 = m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.br();
			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV = false;
		}		
		m_pModelPicShow->m_picShow.setVTrackerPosition(m_ptVTracker1, m_ptVTracker2);
	}
	else if (m_eCurCPType == SN)
	{
		m_pModelPicShow->SetShowTracker(false, false, true);
// 		if (m_bFistSNTracker)
// 		{
// 			m_ptSNTracker1 = cv::Point(0, 0);
// 			m_ptSNTracker2 = cv::Point(m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols, 90);
// 			m_bFistSNTracker = false;
// 		}
		m_ptSNTracker1 = m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.tl();
		m_ptSNTracker2 = m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.br();
		m_pModelPicShow->m_picShow.setSNTrackerPosition(m_ptSNTracker1, m_ptSNTracker2);
	}
	else
	{
		m_pModelPicShow->SetShowTracker(false, false, false);
	}

	if (m_eCurCPType == Fix_CP || m_eCurCPType == UNKNOWN)			//当当前类型为UNKNOWN时，显示所有的校验点
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size();
	}
	if (m_eCurCPType == H_HEAD || m_eCurCPType == UNKNOWN)			//当当前类型为UNKNOWN时，显示所有的校验点
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size();
	}
	if (m_eCurCPType == V_HEAD || m_eCurCPType == UNKNOWN)
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size();
	}
	if (m_eCurCPType == ABMODEL || m_eCurCPType == UNKNOWN)
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size();
	}
	if (m_eCurCPType == COURSE || m_eCurCPType == UNKNOWN)
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size();
	}
	if (m_eCurCPType == QK_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size();
	}
	if (m_eCurCPType == GRAY_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size();
	}
	if (m_eCurCPType == WHITE_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size();
	}
	if (m_eCurCPType == OMR || m_eCurCPType == UNKNOWN)
	{
		int nOmrCount = 0;
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); i++)
		{
			RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.begin();
			for (int j = nOmrCount; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.end(); itAnswer++, j++)
			{
				RECTINFO rcInfo = *itAnswer;
				char szPosition[50] = { 0 };
				sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
				char szCount[10] = { 0 };
				sprintf_s(szCount, "%d", j + nCount + 1);
				m_cpListCtrl.InsertItem(j + nCount, NULL);
				m_cpListCtrl.SetItemText(j + nCount, 0, (LPCTSTR)A2T(szCount));
				m_cpListCtrl.SetItemText(j + nCount, 1, (LPCTSTR)A2T(szPosition));
			}
			nOmrCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.size();
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += nOmrCount;
	}
	if (m_eCurCPType == SN || m_eCurCPType == UNKNOWN)
	{
		int nSNCount = 0;
		SNLIST::iterator itSN = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
		for (; itSN != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSN++)
		{
			RECTLIST::iterator itSnDetail = (*itSN)->lSN.begin();
			for (int j = nSNCount; itSnDetail != (*itSN)->lSN.end(); itSnDetail++, j++)
			{
				RECTINFO rcInfo = *itSnDetail;
				char szPosition[50] = { 0 };
				sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
				char szCount[10] = { 0 };
				sprintf_s(szCount, "%d", j + nCount + 1);
				m_cpListCtrl.InsertItem(j + nCount, NULL);
				m_cpListCtrl.SetItemText(j + nCount, 0, (LPCTSTR)A2T(szCount));
				m_cpListCtrl.SetItemText(j + nCount, 1, (LPCTSTR)A2T(szPosition));
			}
			nSNCount += (*itSN)->lSN.size();
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += nSNCount;
	}
}

void CMakeModelDlg::OnNMRClickListCheckpoint(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (m_cpListCtrl.GetSelectedCount() <= 0)
		return;

	if (m_eCurCPType == UNKNOWN)
		return;

	m_cpListCtrl.SetItemState(m_nCurListCtrlSel, 0, LVIS_DROPHILITED);		// 取消高亮显示
	m_nCurListCtrlSel = pNMItemActivate->iItem;
	m_cpListCtrl.SetItemState(m_nCurListCtrlSel, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行，失去焦点后也一直显示

	//下面的这段代码, 不单单适应于ListCtrl  
	CMenu menu, *pPopup;
	menu.LoadMenu(IDR_MENU_RectRecognition);
	pPopup = menu.GetSubMenu(0);
	CPoint myPoint;
	ClientToScreen(&myPoint);
	GetCursorPos(&myPoint); //鼠标位置  
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);//GetParent()
}

void CMakeModelDlg::AddRecogSN()
{
	if (m_vecPaperModelInfo.size() <= 0 || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;
	if (m_pModel->nHasHead == 0) RecognizeRectTracker();
	else
	{
		//先清空列表
		SNLIST::iterator itSn = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
		for (; itSn != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end();)
		{
			pSN_ITEM pSNItem = *itSn;
			itSn = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.erase(itSn);
			SAFE_RELEASE(pSNItem);
		}

		cv::Rect rt = cv::Rect(m_ptSNTracker1, m_ptSNTracker2);
		if (rt.x < 0)
			rt.x = 0;
		if (rt.y < 0)
			rt.y = 0;
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows - rt.y;

		RecogByHead(rt);
		for (int i = 0; i < m_vecTmp.size(); i++)
		{
			bool bFind = false;
			SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
			for (; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++)
			{
				if ((*itSNItem)->nItem == m_vecTmp[i].nTH)
				{
					bFind = true;
					(*itSNItem)->lSN.push_back(m_vecTmp[i]);
					break;
				}
			}
			if (!bFind)
			{
				pSN_ITEM pSnItem = new SN_ITEM;
				pSnItem->nItem = m_vecTmp[i].nTH;
				pSnItem->lSN.push_back(m_vecTmp[i]);
				m_vecPaperModelInfo[m_nCurrTabSel]->lSN.push_back(pSnItem);
			}
		}
		UpdataCPList();
	}
}

void CMakeModelDlg::AddRecogRectToList()
{
	if (m_vecPaperModelInfo.size() <= 0 || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	if (m_eCurCPType == OMR)
	{
		CTHSetDlg dlg(m_nStartTH);
		if (dlg.DoModal() != IDOK)
			return;

		m_nStartTH = dlg.m_nStartTH;
	}

	int nAddTH = 0;
	for (int i = 0; i < m_vecTmp.size(); i++)
	{
		if (m_eCurCPType == ABMODEL)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.push_back(m_vecTmp[i]);
		}
		else if (m_eCurCPType == COURSE)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.push_back(m_vecTmp[i]);
		}
		else if (m_eCurCPType == QK_CP)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.push_back(m_vecTmp[i]);
		}
		else if (m_eCurCPType == GRAY_CP)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.push_back(m_vecTmp[i]);
		}
		else if (m_eCurCPType == WHITE_CP)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.push_back(m_vecTmp[i]);
		}
		else if (m_eCurCPType == SN)
		{
#if 0
			bool bFind = false;
			SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
			for (; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++)
			{
				if ((*itSNItem).nItem == m_vecTmp[i].nTH)
				{
					bFind = true;
					pSN_DETAIL pSnDetail = new SN_DETAIL;
					pSnDetail->nVal = m_vecTmp[i].nAnswer;
					pSnDetail->rcSN.rt = m_vecTmp[i].rt;
					itSNItem->lSN.push_back(pSnDetail);
					break;
				}
			}
			if (!bFind)
			{
				SN_ITEM snItem;
				snItem.nItem = m_vecTmp[i].nTH;
				pSN_DETAIL pSnDetail = new SN_DETAIL;
				pSnDetail->nVal = m_vecTmp[i].nAnswer;
				pSnDetail->rcSN.rt = m_vecTmp[i].rt;
				snItem.lSN.push_back(pSnDetail);
				m_vecPaperModelInfo[m_nCurrTabSel]->lSN.push_back(snItem);
			}
#endif
		}
		else if (m_eCurCPType == OMR)
		{
			bool bFind = false;
			for (int j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); j++)
			{
				int nTH = m_vecTmp[i].nTH + m_nStartTH;
				if (m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[j].nTH == nTH)
				{
					bFind = true;
					m_vecTmp[i].nTH += m_nStartTH;	
					m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[j].lSelAnswer.push_back(m_vecTmp[i]);
					break;
				}
			}
			if (!bFind)
			{
				OMR_QUESTION objOmr;
				objOmr.nTH = m_vecTmp[i].nTH + m_nStartTH;
				objOmr.nSingle = m_vecTmp[i].nSingle;
				m_vecTmp[i].nTH += m_nStartTH;
				objOmr.lSelAnswer.push_back(m_vecTmp[i]);
				m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.push_back(objOmr);
				nAddTH++;
			}
		}
	}
	m_nStartTH += nAddTH - 1;

	m_vecTmp.clear();
	UpdataCPList();
	ShowRectByCPType(m_eCurCPType);
}

void CMakeModelDlg::RecognizeRectTracker()
{
	if (m_vecPaperModelInfo.size() <= 0 || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;
	if (m_eCurCPType == H_HEAD)
	{
		m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.clear();
//		if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lH_Head.clear();

		cv::Rect rt = cv::Rect(m_ptHTracker1, m_ptHTracker2);
		if (rt.x < 0)
			rt.x = 0;
		if (rt.y < 0)
			rt.y = 0;
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows - rt.y;
		Recognise(rt);
		
		m_vecPaperModelInfo[m_nCurrTabSel]->vecHTracker.clear();
		RECTINFO rcHTrackerSel;					//水平橡皮筋的区域
		rcHTrackerSel.eCPType = m_eCurCPType;
		rcHTrackerSel.rt = rt;
		m_vecPaperModelInfo[m_nCurrTabSel]->vecHTracker.push_back(rcHTrackerSel);
		m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH = false;
	}
	else if (m_eCurCPType == V_HEAD)
	{
		m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.clear();
//		if (m_pModel && m_pModel->vecPaperModel.size()) m_pModel->vecPaperModel[m_nCurrTabSel].lV_Head.clear();

		cv::Rect rt = cv::Rect(m_ptVTracker1, m_ptVTracker2);
		if (rt.x < 0)
			rt.x = 0;
		if (rt.y < 0)
			rt.y = 0;
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows - rt.y;
		Recognise(rt);

		m_vecPaperModelInfo[m_nCurrTabSel]->vecVTracker.clear();
		RECTINFO rcVTrackerSel;					//垂直橡皮筋的区域
		rcVTrackerSel.eCPType = m_eCurCPType;
		rcVTrackerSel.rt = rt;
		m_vecPaperModelInfo[m_nCurrTabSel]->vecVTracker.push_back(rcVTrackerSel);
		m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV = false;
	}
	else if (m_eCurCPType == SN)
	{
		SNLIST::iterator itSn = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
		for (; itSn != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end();)
		{
			pSN_ITEM pSNItem = *itSn;
			itSn = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.erase(itSn);
			SAFE_RELEASE(pSNItem);
		}

		cv::Rect rt = cv::Rect(m_ptSNTracker1, m_ptSNTracker2);
		if (rt.x < 0)
			rt.x = 0;
		if (rt.y < 0)
			rt.y = 0;
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows - rt.y;
		Recognise(rt);

		for (int i = 0; i < m_vecTmp.size(); i++)
		{
			bool bFind = false;
			SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
			for (; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++)
			{
				if ((*itSNItem)->nItem == m_vecTmp[i].nTH)
				{
					bFind = true;
					(*itSNItem)->lSN.push_back(m_vecTmp[i]);
					break;
				}
			}
			if (!bFind)
			{
				pSN_ITEM pSnItem = new SN_ITEM;
				pSnItem->nItem = m_vecTmp[i].nTH;
				pSnItem->lSN.push_back(m_vecTmp[i]);
				m_vecPaperModelInfo[m_nCurrTabSel]->lSN.push_back(pSnItem);
			}
		}
	}
	
	SortRect();
	UpdataCPList();
}

void CMakeModelDlg::DeleteRectInfoOnList()
{
	int nItem = m_nCurListCtrlSel;
	if (m_eCurCPType == OMR)
	{
		int nOmrCount = 0;
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); i++)
		{
			nOmrCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.size();
			if (nItem < nOmrCount)
			{
				nItem = i;	//获取当前选择的所属题号
				break;
			}
			else
			{
				nItem -= nOmrCount;
			}
		}
	}
	if (DeleteRectInfo(m_eCurCPType, nItem))
	{
		UpdataCPList();
		ShowRectByCPType(m_eCurCPType);
	}	
}
void CMakeModelDlg::DelRectInfoOnPic()
{
	RECTINFO* pRc = NULL;
	int nFind = 0;
	nFind = GetRectInfoByPoint(m_ptRBtnUp, m_eCurCPType, pRc);
	if (nFind < 0)
		return;
	
	if (pRc && pRc->eCPType != UNKNOWN)
	{
		if (DeleteRectInfo(pRc->eCPType, nFind))
		{
			UpdataCPList();
			ShowRectByCPType(m_eCurCPType);
		}
	}	
}
BOOL CMakeModelDlg::DeleteRectInfo(CPType eType, int nItem)
{
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return FALSE;

	std::vector<RECTINFO>::iterator it;
	switch (eType)
	{
	case Fix_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.erase(it);

		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.erase(it);
		break;
	case H_HEAD:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.erase(it);
		break;
	case V_HEAD:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.erase(it);
		break;
	case ABMODEL:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.erase(it);
		break;
	case COURSE:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.erase(it);
		break;
	case QK_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.erase(it);
		break;
	case GRAY_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.erase(it);
		break;
	case WHITE_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.erase(it);
		break;
	case OMR:
		//******************	单独处理，OMR的删除，需要将整题的选项都删除	***********************************
		{
			if (m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size() < 0)
				return FALSE;
			std::vector<OMR_QUESTION>::iterator itOmr;
			itOmr = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.begin() + nItem;
			if (itOmr != m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.end())
				m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.erase(itOmr);
			break;
		}
	default: return FALSE;
	}
	return TRUE;
}

BOOL CMakeModelDlg::PreTranslateMessage(MSG* pMsg)
{
	ProcessMessage(*pMsg);
	return CDialog::PreTranslateMessage(pMsg);
}
void CMakeModelDlg::OnLvnKeydownListCheckpoint(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	*pResult = 0;
	if (pLVKeyDow->wVKey == VK_UP)
	{
		m_cpListCtrl.SetItemState(m_nCurListCtrlSel, 0, LVIS_DROPHILITED);		// 取消高亮显示

		m_nCurListCtrlSel--;
		if (m_nCurListCtrlSel <= 0)
			m_nCurListCtrlSel = 0;
		ShowRectByItem(m_nCurListCtrlSel);

		m_cpListCtrl.SetItemState(m_nCurListCtrlSel, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行，失去焦点后也一直显示
	}
	else if (pLVKeyDow->wVKey == VK_DOWN)
	{
		m_cpListCtrl.SetItemState(m_nCurListCtrlSel, 0, LVIS_DROPHILITED);		// 取消高亮显示

		m_nCurListCtrlSel++;
		if (m_nCurListCtrlSel >= m_cpListCtrl.GetItemCount() - 1)
			m_nCurListCtrlSel = m_cpListCtrl.GetItemCount() - 1;
		ShowRectByItem(m_nCurListCtrlSel);

		m_cpListCtrl.SetItemState(m_nCurListCtrlSel, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行，失去焦点后也一直显示
	}
}

void CMakeModelDlg::OnNMDblclkListCheckpoint(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	m_cpListCtrl.SetItemState(m_nCurListCtrlSel, 0, LVIS_DROPHILITED);		// 取消高亮显示

	m_nCurListCtrlSel = pNMItemActivate->iItem;
	ShowRectByItem(m_nCurListCtrlSel);

	//++ test	高亮显示一行，失去焦点后也一直显示
//	m_cpListCtrl.SetItemState(m_nCurListCtrlSel, LVIS_SELECTED, LVIS_SELECTED);
	m_cpListCtrl.SetItemState(m_nCurListCtrlSel, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行，失去焦点后也一直显示
	//--
}

void CMakeModelDlg::SortRect()
{
	if (!m_pModel || m_vecPaperModelInfo.size() == 0)
		return;

	switch (m_eCurCPType)
	{
	case H_HEAD:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.end(), SortByPositionX);
		break;
	case V_HEAD:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.end(), SortByPositionY);
		break;
	case ABMODEL:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.end(), SortByPositionX);
		break;
	case COURSE:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.end(), SortByPositionX);
		break;
	case QK_CP:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.end(), SortByPositionX);
		break;
	case GRAY_CP:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.end(), SortByPositionX);
		break;
	case WHITE_CP:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.end(), SortByPositionX);
		break;
	default: return;
	}
}

inline int CMakeModelDlg::GetRectInfoByPoint(cv::Point pt, CPType eType, RECTINFO*& pRc)
{
	int  nFind = -1;
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return nFind;
	
	switch (eType)
	{
		case UNKNOWN:
		case Fix_CP:
			if (eType == Fix_CP || eType == UNKNOWN)
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size(); i++)
				{
					if (m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].rt.contains(pt))
					{
						nFind = i;
						pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i];
						break;
					}
				}
			}
		case H_HEAD:
			if (eType == H_HEAD || eType == UNKNOWN)
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size(); i++)
				{
					if (m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i].rt.contains(pt))
					{
						nFind = i;
						pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i];
						break;
					}
				}
			}
		case V_HEAD:
			if (eType == V_HEAD || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size(); i++)
					{
						if (m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i].rt.contains(pt))
						{
							nFind = i;
							pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i];
							break;
						}
					}
				}
			}
		case ABMODEL:
			if (eType == ABMODEL || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size(); i++)
					{
						if (m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i].rt.contains(pt))
						{
							nFind = i;
							pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i];
							break;
						}
					}
				}
			}
		case COURSE:
			if (eType == COURSE || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size(); i++)
					{
						if (m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i].rt.contains(pt))
						{
							nFind = i;
							pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i];
							break;
						}
					}
				}
			}
		case QK_CP:
			if (eType == QK_CP || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size(); i++)
					{
						if (m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i].rt.contains(pt))
						{
							nFind = i;
							pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i];
							break;
						}
					}
				}
			}
		case GRAY_CP:
			if (eType == GRAY_CP || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size(); i++)
					{
						if (m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i].rt.contains(pt))
						{
							nFind = i;
							pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i];
							break;
						}
					}
				}
			}
		case WHITE_CP:
			if (eType == WHITE_CP || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size(); i++)
					{
						if (m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i].rt.contains(pt))
						{
							nFind = i;
							pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i];
							break;
						}
					}
				}
			}
		case SN:
			if (eType == SN || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
					for (int i = 0; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++, i++)
					{
						RECTLIST::iterator itSNRect = (*itSNItem)->lSN.begin();
						for (int j = 0; itSNRect != (*itSNItem)->lSN.end(); itSNRect++, j++)
						{
							if (itSNRect->rt.contains(pt))
							{
								nFind = i;
								pRc = &(*itSNRect);
								break;
							}
						}
						if (nFind >= 0)	break;
					}
				}
			}
	case OMR:
		if (eType == OMR || eType == UNKNOWN)
		{
			if (nFind < 0)
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); i++)
				{
					RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.begin();
					for (int j = 0; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.end(); itAnswer++, j++)
					{
						if (itAnswer->rt.contains(pt))
						{
							nFind = i;
							pRc = &(*itAnswer);
							break;
						}
					}
					if (nFind >= 0) break;
				}
			}
		}
	}

	return nFind;
}

bool CMakeModelDlg::PicRotate()
{
	if (m_vecPaperModelInfo.size() < m_nCurrTabSel)
		return false;

	if (m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() <= 1)
		return false;

	int nMaxItem = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() - 1;
	cv::Point pt1, pt2;
	pt1.x = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[0].rt.x + m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[0].rt.width / 2 + 0.5;
	pt1.y = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[0].rt.y + m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[0].rt.height / 2 + 0.5;

	pt2.x = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nMaxItem].rt.x + m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nMaxItem].rt.width / 2 + 0.5;
	pt2.y = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nMaxItem].rt.y + m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nMaxItem].rt.height / 2 + 0.5;

	float k = (float)(pt2.y - pt1.y) / (pt2.x - pt1.x);
	float fAngle = atan(k) / CV_PI * 180;
	TRACE("pt1(%d,%d), pt2(%d,%d), 斜率: %f, 旋转角度: %f\n", pt1.x, pt1.y, pt2.x, pt2.y, k, fAngle);

	Point2f center(pt2.x, pt2.y);
	Mat affine_matrix = getRotationMatrix2D(center, fAngle, 1.0);

	Mat inputImg = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg;
	Mat tmpImg;
	warpAffine(inputImg, tmpImg, affine_matrix, inputImg.size());
	namedWindow("warpAffine", 0);
	imshow("warpAffine", tmpImg);
	m_pModelPicShow->ShowPic(tmpImg);

	return true;
}

LRESULT CMakeModelDlg::HTrackerChange(WPARAM wParam, LPARAM lParam)
{
	m_ptHTracker1 = m_pModelPicShow->m_picShow.m_ptHTracker1;
	m_ptHTracker2 = m_pModelPicShow->m_picShow.m_ptHTracker2;
	ShowRectTracker();

	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return true;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.x = m_ptHTracker1.x;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.y = m_ptHTracker1.y;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.width = m_ptHTracker2.x - m_ptHTracker1.x;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtHTracker.height = m_ptHTracker2.y - m_ptHTracker1.y;
	m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH = false;
	return true;
}

LRESULT CMakeModelDlg::VTrackerChange(WPARAM wParam, LPARAM lParam)
{
	m_ptVTracker1 = m_pModelPicShow->m_picShow.m_ptVTracker1;
	m_ptVTracker2 = m_pModelPicShow->m_picShow.m_ptVTracker2;
	ShowRectTracker();

	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return true;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.x = m_ptVTracker1.x;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.y = m_ptVTracker1.y;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.width = m_ptVTracker2.x - m_ptVTracker1.x;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtVTracker.height = m_ptVTracker2.y - m_ptVTracker1.y;
	m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV = false;
	return true;
}

LRESULT CMakeModelDlg::SNTrackerChange(WPARAM wParam, LPARAM lParam)
{
	m_ptSNTracker1 = m_pModelPicShow->m_picShow.m_ptSNTracker1;
	m_ptSNTracker2 = m_pModelPicShow->m_picShow.m_ptSNTracker2;
	ShowRectTracker();

	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return true;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.x = m_ptSNTracker1.x;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.y = m_ptSNTracker1.y;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.width = m_ptSNTracker2.x - m_ptSNTracker1.x;
	m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.height = m_ptSNTracker2.y - m_ptSNTracker1.y;
	return true;
}

void CMakeModelDlg::GetSNArry(std::vector<cv::Rect>& rcList)
{
	if (rcList.size() <= 0)
		return;
	int nMaxRow = 1;
	int nMaxCols = 1;

	m_vecTmp.clear();
	std::vector<Rect> rcList_X = rcList;
	std::vector<Rect> rcList_XY = rcList;
	std::sort(rcList_X.begin(), rcList_X.end(), SortByPositionX2);


	int nW = rcList_X[0].width;				//矩形框平均宽度
	int nH = rcList_X[0].height;			//矩形框平均高度
	int nWInterval = 0;						//矩形间的X轴平均间隔
	int nHInterval = 0;						//矩形间的Y轴平均间隔

	int nX = rcList_X[0].width * 0.2 + 0.5;		//判断属于同一列的X轴偏差
	int nY = rcList_X[0].height * 0.3 + 0.5;	//判断属于同一行的Y轴偏差


	std::sort(rcList_XY.begin(), rcList_XY.end(), SortByPositionXYInterval);

	for (int i = 1; i < rcList_XY.size(); i++)
	{
		int nTmp = rcList_XY[i].y - rcList_XY[i - 1].y;
		if (abs(rcList_XY[i].y - rcList_XY[i - 1].y) > nY)
		{
			nMaxRow++;
			nHInterval += abs(rcList_XY[i].y - rcList_XY[i - 1].y - rcList_XY[i - 1].height);
		}

		nW += rcList_XY[i].width;
		nH += rcList_XY[i].height;
	}
	for (int i = 1; i < rcList_X.size(); i++)
	{
		int nTmp = rcList_X[i].x - rcList_X[i - 1].x;
		if (abs(rcList_X[i].x - rcList_X[i - 1].x) > nX)
		{
			nMaxCols++;
			nWInterval += abs(rcList_X[i].x - rcList_X[i - 1].x - rcList_X[i - 1].width);
		}
	}

	nW = nW / rcList_XY.size() + 0.5;
	nH = nH / rcList_XY.size() + 0.5;
	if (nMaxCols > 1)
		nWInterval = nWInterval / (nMaxCols - 1) + 0.5;
	if (nMaxRow > 1)
		nHInterval = nHInterval / (nMaxRow - 1) + 0.5;

	TRACE("检测到框选了%d * %d的矩形区\n", nMaxRow, nMaxCols);

	for (int i = 0; i < rcList_XY.size(); i++)
	{
		TRACE("omr2 rt%d: (%d,%d,%d,%d)\n", i + 1, rcList_XY[i].x, rcList_XY[i].y, rcList_XY[i].width, rcList_XY[i].height);
	}

	int x = 0, y = 0;	//x-列，y-行
	for (int i = 0; i < rcList_XY.size(); i++)
	{
#if 1
		int dx, dy;
		if(i == 0)
			dy = 0;
		else
		{
			dy = rcList_XY[i].y - rcList_XY[i - 1].y;

			if (dy > 6)
			{
				y++;
				x = 0;
			}
			else
				x++;
		}
#else
		int x = (float)(rcList_XY[i].x - rcList_XY[0].x) / (nW + nWInterval) + 0.5;	//列
		int y = (float)(rcList_XY[i].y - rcList_XY[0].y) / (nH + nHInterval) + 0.5;	//行
#endif
		TRACE("第几行几列: %d行%d列, 差值: x-%d, y-%d, (nW + nWInterval) = %d, (nH + nHInterval) = %d\n", y, x, rcList_XY[i].x - rcList_XY[0].x, rcList_XY[i].y - rcList_XY[0].y, nW + nWInterval, nH + nHInterval);

		RECTINFO rc;
		rc.rt = rcList_XY[i];
		rc.eCPType = m_eCurCPType;
		rc.nThresholdValue = m_nSN;
		rc.fStandardValuePercent = m_fSNThresholdPercent;
		rc.nRecogFlag = m_pSNInfoDlg->m_nCurrentSNVal;

		switch (m_pSNInfoDlg->m_nCurrentSNVal)
		{
		case 10:	//1010
			rc.nTH = x;
			rc.nSnVal = y;
			break;
		case 9:		//1001
			rc.nTH = nMaxCols - x - 1;
			rc.nSnVal = nMaxRow - y - 1;
			break;
		case 6:		//0110
			rc.nTH = nMaxRow - y - 1;
			rc.nSnVal = x;
			break;
		case 5:		//0101
			rc.nTH = y;
			rc.nSnVal = nMaxCols - x - 1;
			break;
		}
		
		Rect rtTmp = rcList[i];
		Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
		RecogGrayValue(matSrcModel, rc);

		m_vecTmp.push_back(rc);
	}

	ShowTmpRect();
}

void CMakeModelDlg::GetOmrArry(std::vector<cv::Rect>& rcList)
{
	if (rcList.size() <= 0)
		return;
	int nMaxRow		= 1;
	int nMaxCols	= 1;

	m_vecTmp.clear();
	std::vector<Rect> rcList_X = rcList;
	std::vector<Rect> rcList_XY = rcList;
	std::sort(rcList_X.begin(), rcList_X.end(), SortByPositionX2);
//	std::sort(rcList.begin(), rcList.end(), SortByPositionY2);

// 	for (int i = 0; i < rcList.size(); i++)
// 	{
// 		TRACE("omr1 rt%d: (%d,%d,%d,%d)\n", i + 1, rcList[i].x, rcList[i].y, rcList[i].width, rcList[i].height);
// 	}
// 	TRACE("-----------------\n");
// 	for (int i = 0; i < rcList_X.size(); i++)
// 	{
// 		TRACE("omr2 rt%d: (%d,%d,%d,%d)\n", i + 1, rcList_X[i].x, rcList_X[i].y, rcList_X[i].width, rcList_X[i].height);
// 	}

	int nW = rcList_X[0].width;				//矩形框平均宽度
	int nH = rcList_X[0].height;			//矩形框平均高度
	int nWInterval = 0;						//矩形间的X轴平均间隔
	int nHInterval = 0;						//矩形间的Y轴平均间隔

	int nX = rcList_X[0].width * 0.2 + 0.5;		//判断属于同一列的X轴偏差
	int nY = rcList_X[0].height * 0.3 + 0.5;	//判断属于同一行的Y轴偏差



//	TRACE("-----------------\n");
	std::sort(rcList_XY.begin(), rcList_XY.end(), SortByPositionXYInterval);
// 	for (int i = 0; i < rcList_XY.size(); i++)
// 	{
// 		TRACE("omr2 rt%d: (%d,%d,%d,%d)\n", i + 1, rcList_XY[i].x, rcList_XY[i].y, rcList_XY[i].width, rcList_XY[i].height);
// 	}

	for (int i = 1; i < rcList_XY.size(); i++)
	{
		int nTmp = rcList_XY[i].y - rcList_XY[i - 1].y;
		if (abs(rcList_XY[i].y - rcList_XY[i - 1].y) > nY)
		{
			nMaxRow++;
			nHInterval += abs(rcList_XY[i].y - rcList_XY[i - 1].y - rcList_XY[i - 1].height);
		}

		nW += rcList_XY[i].width;
		nH += rcList_XY[i].height;
	}
	for (int i = 1; i < rcList_X.size(); i++)
	{
		int nTmp = rcList_X[i].x - rcList_X[i - 1].x;
		if (abs(rcList_X[i].x - rcList_X[i - 1].x) > nX)
		{
			nMaxCols++;
			nWInterval += abs(rcList_X[i].x - rcList_X[i - 1].x - rcList_X[i - 1].width);
		}
	}

	nW = nW / rcList_XY.size() + 0.5;
	nH = nH / rcList_XY.size() + 0.5;
	if (nMaxCols > 1)
		nWInterval = nWInterval / (nMaxCols - 1) + 0.5;
	if (nMaxRow > 1)
		nHInterval = nHInterval / (nMaxRow - 1) + 0.5;

	TRACE("检测到框选了%d * %d的矩形区\n", nMaxRow, nMaxCols);

	int x = 0, y = 0;	//x-列，y-行
	for (int i = 0; i < rcList_XY.size(); i++)
	{
#if 1
		int dx, dy;
		if (i == 0)
			dy = 0;
		else
		{
			dy = rcList_XY[i].y - rcList_XY[i - 1].y;

			if (dy > 6)
			{
				y++;
				x = 0;
			}
			else
				x++;
		}
#else
		int x = (float)(rcList_XY[i].x - rcList_XY[0].x) / (nW + nWInterval) + 0.5;	//列
		int y = (float)(rcList_XY[i].y - rcList_XY[0].y) / (nH + nHInterval) + 0.5;	//行
#endif

		TRACE("第几行几列: %d行%d列, 差值: x-%d, y-%d\n", x, y, rcList_XY[i].x - rcList_XY[0].x, rcList_XY[i].y - rcList_XY[0].y);

		RECTINFO rc;
		rc.rt = rcList_XY[i];
		rc.eCPType = m_eCurCPType;
		rc.nThresholdValue = m_nOMR;
		rc.fStandardValuePercent = m_fOMRThresholdPercent;
		rc.nRecogFlag = m_pOmrInfoDlg->m_nCurrentOmrVal;

		switch (m_pOmrInfoDlg->m_nCurrentOmrVal)
		{
		case 42:	//101010
			rc.nTH = x;
			rc.nAnswer = y;
			break;
		case 41:	//101001
			rc.nTH = nMaxCols - x - 1;		//nMaxRow - x - 1;
			rc.nAnswer = y;
			break;
		case 38:	//100110
			rc.nTH = x;
			rc.nAnswer = nMaxRow - y - 1;	//nMaxCols - y - 1
			break;
		case 37:	//100101
			rc.nTH = nMaxCols - x - 1;		//nMaxRow - x - 1
			rc.nAnswer = nMaxRow - y - 1;	//nMaxCols - y - 1
			break;
		case 26:
			rc.nTH = y;
			rc.nAnswer = x;
			break;
		case 25:
			rc.nTH = nMaxCols - y - 1;
			rc.nAnswer = x;
			break;
		case 22:
			rc.nTH = y;
			rc.nAnswer = nMaxCols - x - 1;		//nMaxRow - x - 1
			break;
		case 21:
			rc.nTH = nMaxRow - y - 1;			//nMaxCols - y - 1
			rc.nAnswer = nMaxCols - x - 1;		//nMaxRow - x - 1
			break;
		}
		if (m_pOmrInfoDlg->m_bSingle)
			rc.nSingle = 0;
		else
			rc.nSingle = 1;
		Rect rtTmp = rcList[i];
		Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
		RecogGrayValue(matSrcModel, rc);

		m_vecTmp.push_back(rc);
	}

	ShowTmpRect();
}

void CMakeModelDlg::setUploadModelInfo(CString& strName, CString& strModelPath, int nExamId, int nSubjectId)
{
	USES_CONVERSION;
	std::string strPath = T2A(strModelPath);
	std::string strMd5;

	strMd5 = calcFileMd5(strPath);

	CString strUser = _T("");
	CString strEzs = _T("");
#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();

	strEzs = pDlg->m_strEzs;
	strUser = pDlg->m_strUserName;
#else
	CScanToolDlg* pDlg = (CScanToolDlg*)AfxGetMainWnd();	//GetParent();
	strEzs = pDlg->m_strEzs;
	strUser = pDlg->m_strUserName;
#endif
	

	ST_MODELINFO stModelInfo;
	ZeroMemory(&stModelInfo, sizeof(ST_MODELINFO));
	stModelInfo.nExamID = nExamId;
	stModelInfo.nSubjectID = nSubjectId;
	sprintf_s(stModelInfo.szUserNo, "%s", T2A(strUser));
	sprintf_s(stModelInfo.szModelName, "%s.mod", T2A(strName));
	sprintf_s(stModelInfo.szEzs, "%s", T2A(strEzs));
	strncpy(stModelInfo.szMD5, strMd5.c_str(), strMd5.length());

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_SETMODELINFO;
	pTcpTask->nPkgLen = sizeof(ST_MODELINFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stModelInfo, sizeof(ST_MODELINFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
}


void CMakeModelDlg::OnBnClickedBtnuploadmodel()
{
	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("All Files (*.*)|*.*;)||"),
					NULL);
	if (dlg.DoModal() != IDOK)
		return;

	USES_CONVERSION;
	std::string strJsnModel = T2A(dlg.GetPathName());

	std::string strJsnData;
	std::ifstream in(strJsnModel);
	if (!in)
		return;

	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);					//不过滤空格
		strJsnData.append(strJsnLine);
	}
	in.close();
		
	m_pModel = LoadMakePaperData(strJsnData);
	if (!m_pModel)	return;

	//++ 将PDF转JPG
	int nPos1 = strJsnModel.rfind('\\');
	int nPos2 = strJsnModel.rfind('.');
	std::string strBaseName = strJsnModel.substr(nPos1 + 1, nPos2 - nPos1 - 1);
	std::string strPdfPath = T2A(dlg.GetFolderPath());
	strPdfPath.append("\\" + strBaseName + ".pdf");

	bool bResult = Pdf2Jpg(strPdfPath, T2A(m_pModel->strModelName));
	if (!bResult)
	{
		AfxMessageBox(_T("pdf转jpg失败"));
		return;
	}
	//--

	InitModelRecog(m_pModel);

	m_nModelPicNums = m_pModel->nPicNum;
	InitTab();
	InitConf();
	m_vecPaperModelInfo.clear();
	for (int i = 0; i < m_pModel->nPicNum; i++)
	{
		CString strPicPath = g_strCurrentPath + _T("Model\\") + m_pModel->strModelName + _T("\\") + m_pModel->vecPaperModel[i]->strModelPicName;

		pPaperModelInfo pPaperModel = new PaperModelInfo;
		m_vecPaperModelInfo.push_back(pPaperModel);
		pPaperModel->nPaper = i;
		pPaperModel->strModelPicPath = strPicPath;
		pPaperModel->strModelPicName = m_pModel->vecPaperModel[i]->strModelPicName;

		pPaperModel->matSrcImg = imread((std::string)(CT2CA)strPicPath);
		pPaperModel->matDstImg = pPaperModel->matSrcImg;

		Mat src_img;
		src_img = m_vecPaperModelInfo[i]->matDstImg;
		m_vecPicShow[i]->ShowPic(src_img);
		
		pPaperModel->nPicW = src_img.cols;
		pPaperModel->nPicH = src_img.rows;

		pPaperModel->rtHTracker = m_pModel->vecPaperModel[i]->rtHTracker;
		pPaperModel->rtVTracker = m_pModel->vecPaperModel[i]->rtVTracker;
		pPaperModel->rtSNTracker = m_pModel->vecPaperModel[i]->rtSNTracker;

		RECTLIST::iterator itSelHTracker = m_pModel->vecPaperModel[i]->lSelHTracker.begin();
		for (; itSelHTracker != m_pModel->vecPaperModel[i]->lSelHTracker.end(); itSelHTracker++)
		{
			pPaperModel->vecHTracker.push_back(*itSelHTracker);
		}
		RECTLIST::iterator itSelVTracker = m_pModel->vecPaperModel[i]->lSelVTracker.begin();
		for (; itSelVTracker != m_pModel->vecPaperModel[i]->lSelVTracker.end(); itSelVTracker++)
		{
			pPaperModel->vecVTracker.push_back(*itSelVTracker);
		}
		RECTLIST::iterator itSelRoi = m_pModel->vecPaperModel[i]->lSelFixRoi.begin();
		for (; itSelRoi != m_pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++)
		{
			pPaperModel->vecRtSel.push_back(*itSelRoi);
		}
		OMRLIST::iterator itOmr2 = m_pModel->vecPaperModel[i]->lOMR2.begin();
		for (; itOmr2 != m_pModel->vecPaperModel[i]->lOMR2.end(); itOmr2++)
		{
			pPaperModel->vecOmr2.push_back(*itOmr2);
		}
		RECTLIST::iterator itFix = m_pModel->vecPaperModel[i]->lFix.begin();
		for (; itFix != m_pModel->vecPaperModel[i]->lFix.end(); itFix++)
		{
			pPaperModel->vecRtFix.push_back(*itFix);
		}
		RECTLIST::iterator itHHead = m_pModel->vecPaperModel[i]->lH_Head.begin();
		for (; itHHead != m_pModel->vecPaperModel[i]->lH_Head.end(); itHHead++)
		{
			pPaperModel->vecH_Head.push_back(*itHHead);
		}
		RECTLIST::iterator itVHead = m_pModel->vecPaperModel[i]->lV_Head.begin();
		for (; itVHead != m_pModel->vecPaperModel[i]->lV_Head.end(); itVHead++)
		{
			pPaperModel->vecV_Head.push_back(*itVHead);
		}
		RECTLIST::iterator itABModel = m_pModel->vecPaperModel[i]->lABModel.begin();
		for (; itABModel != m_pModel->vecPaperModel[i]->lABModel.end(); itABModel++)
		{
			pPaperModel->vecABModel.push_back(*itABModel);
		}
		RECTLIST::iterator itCourse = m_pModel->vecPaperModel[i]->lCourse.begin();
		for (; itCourse != m_pModel->vecPaperModel[i]->lCourse.end(); itCourse++)
		{
			pPaperModel->vecCourse.push_back(*itCourse);
		}
		RECTLIST::iterator itQK = m_pModel->vecPaperModel[i]->lQK_CP.begin();
		for (; itQK != m_pModel->vecPaperModel[i]->lQK_CP.end(); itQK++)
		{
			pPaperModel->vecQK_CP.push_back(*itQK);
		}
		RECTLIST::iterator itGray = m_pModel->vecPaperModel[i]->lGray.begin();
		for (; itGray != m_pModel->vecPaperModel[i]->lGray.end(); itGray++)
		{
			pPaperModel->vecGray.push_back(*itGray);
		}
		RECTLIST::iterator itWhite = m_pModel->vecPaperModel[i]->lWhite.begin();
		for (; itWhite != m_pModel->vecPaperModel[i]->lWhite.end(); itWhite++)
		{
			pPaperModel->vecWhite.push_back(*itWhite);
		}
		SNLIST::iterator itSn = m_pModel->vecPaperModel[i]->lSNInfo.begin();
		for (; itSn != m_pModel->vecPaperModel[i]->lSNInfo.end(); itSn++)
		{
			pSN_ITEM pSnItem = new SN_ITEM;
			pSnItem->nItem = (*itSn)->nItem;
			pSnItem->nRecogVal = (*itSn)->nRecogVal;
			RECTLIST::iterator itRc = (*itSn)->lSN.begin();
			for (; itRc != (*itSn)->lSN.end(); itRc++)
			{
				RECTINFO rc = *itRc;
				pSnItem->lSN.push_back(rc);
			}
			pPaperModel->lSN.push_back(pSnItem);
		}

		ShowRectByCPType(m_eCurCPType);
		UpdataCPList();
	}
}

void CMakeModelDlg::CopyImage(HANDLE hBitmap, TW_IMAGEINFO& info)
{
	SetImage(hBitmap, info.BitsPerPixel);
}

void CMakeModelDlg::SetImage(HANDLE hBitmap, int bits)
{
	CDIB dib;
	dib.CreateFromHandle(hBitmap, bits);

	BITMAPFILEHEADER bFile;
	::ZeroMemory(&bFile, sizeof(bFile));
	memcpy((void *)&bFile.bfType, "BM", 2);
	bFile.bfSize = dib.GetDIBSize() + sizeof(bFile);
	bFile.bfOffBits = sizeof(BITMAPINFOHEADER) + dib.GetPaletteSize()*sizeof(RGBQUAD) + sizeof(BITMAPFILEHEADER);
	unsigned char *pBits = (unsigned char *)malloc(bFile.bfSize);
	memcpy(pBits, &bFile, sizeof(BITMAPFILEHEADER));
	memcpy(pBits + sizeof(BITMAPFILEHEADER), dib.m_pVoid, dib.GetDIBSize());

	BYTE *p = pBits;
	BITMAPFILEHEADER fheader;
	memcpy(&fheader, p, sizeof(BITMAPFILEHEADER));
	BITMAPINFOHEADER bmphdr;
	p += sizeof(BITMAPFILEHEADER);
	memcpy(&bmphdr, p, sizeof(BITMAPINFOHEADER));
	int w = bmphdr.biWidth;
	int h = bmphdr.biHeight;
	p = pBits + fheader.bfOffBits;

	int nChannel = (bmphdr.biBitCount == 1) ? 1 : bmphdr.biBitCount / 8;
	int depth = (bmphdr.biBitCount == 1) ? IPL_DEPTH_1U : IPL_DEPTH_8U;
	IplImage *pIpl2 = cvCreateImage(cvSize(w, h), depth, nChannel);

	int height;
	bool isLowerLeft = bmphdr.biHeight > 0;
	height = (bmphdr.biHeight > 0) ? bmphdr.biHeight : -bmphdr.biHeight;
	CopyData(pIpl2->imageData, (char*)p, bmphdr.biSizeImage, isLowerLeft, height);
	free(pBits);
	pBits = NULL;

	// 	IplImage* pIpl = DIB2IplImage(dib);
	// 	cv::Mat matTest = cv::cvarrToMat(pIpl);
	USES_CONVERSION;

	
	static int nModelScan = 1;
	char szPicPath[MAX_PATH] = { 0 };
	sprintf_s(szPicPath, "%s\\model%d.jpg", T2A(m_strScanSavePath), nModelScan);
	nModelScan++;


	cv::Mat matTest2 = cv::cvarrToMat(pIpl2);
//	cv::Mat matTest3 = matTest2.clone();


	std::string strPicName = szPicPath;
	imwrite(strPicName, matTest2);

	std::string strLog = "Get model pic: " + strPicName;
	g_pLogger->information(strLog);

	cvReleaseImage(&pIpl2);
}

void CMakeModelDlg::ScanDone(int nStatus)
{
	TRACE("扫描完成\n");
	AfxMessageBox(_T("扫描完成"));
	CString strSelect = _T("/root,");
	strSelect.Append(m_strScanSavePath);
	ShellExecute(NULL, _T("open"), _T("explorer.exe"), strSelect, NULL, SW_SHOWNORMAL);
}

BOOL CMakeModelDlg::ScanSrcInit()
{
	USES_CONVERSION;
	if (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &m_Source))
	{
		TW_IDENTITY temp_Source = m_Source;
		m_scanSourceArry.Add(temp_Source);
		while (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &m_Source))
		{
			TW_IDENTITY temp_Source = m_Source;
			m_scanSourceArry.Add(temp_Source);
		}
		m_bSourceSelected = TRUE;
	}
	else
	{
		m_bSourceSelected = FALSE;
	}
	return m_bSourceSelected;
}

void CMakeModelDlg::OnDestroy()
{
	__super::OnDestroy();
	
	ReleaseTwain();

	SAFE_RELEASE(m_pRecogInfoDlg);
	SAFE_RELEASE(m_pOmrInfoDlg);
	SAFE_RELEASE(m_pSNInfoDlg);

	if (m_bNewModelFlag && !m_bSavedModelFlag && m_pModel != NULL)
		SAFE_RELEASE(m_pModel);

	std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
	for (; itPic != m_vecPicShow.end();)
	{
		CPicShow* pModelPicShow = *itPic;
		if (pModelPicShow)
		{
			delete pModelPicShow;
			pModelPicShow = NULL;
		}
		itPic = m_vecPicShow.erase(itPic);
	}

	std::vector<pPaperModelInfo>::iterator itPaperModelInfo = m_vecPaperModelInfo.begin();
	for (; itPaperModelInfo != m_vecPaperModelInfo.end();)
	{
		pPaperModelInfo pPaperModel = *itPaperModelInfo;
		if (pPaperModel)
		{
			delete pPaperModel;
			pPaperModel = NULL;
		}
		itPaperModelInfo = m_vecPaperModelInfo.erase(itPaperModelInfo);
	}
}

LRESULT CMakeModelDlg::ShiftKeyDown(WPARAM wParam, LPARAM lParam)
{
	m_bShiftKeyDown = true;
	return TRUE;
}

LRESULT CMakeModelDlg::ShiftKeyUp(WPARAM wParam, LPARAM lParam)
{
	m_bShiftKeyDown = false;
	return TRUE;
}


void CMakeModelDlg::OnNMHoverListCheckpoint(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;		//**********	这里如果不响应，同时返回结果值不为1的话，	****************
						//**********	就会产生产生TRACK SELECT，也就是鼠标悬停	****************
						//**********	一段时间后，所在行自动被选中
}

inline bool CMakeModelDlg::checkOverlap(CPType eType, cv::Rect rtSrc)
{
	bool bResult = false;
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return bResult;

	switch (eType)
	{
		case Fix_CP:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].rt;
					if (rt.contains(rtSrc.tl()) || rt.contains(rtSrc.br()) || rtSrc.contains(rt.tl()) || rtSrc.contains(rt.br()))
					{
						bResult = true;
						break;
					}
					if (rt.tl().x >= rtSrc.tl().x && rt.br().x <= rtSrc.br().x && rt.tl().y <= rtSrc.tl().y && rt.br().y >= rtSrc.br().y)
					{
						bResult = true;
						break;
					}
					if (rtSrc.tl().x >= rt.tl().x && rtSrc.br().x <= rt.br().x && rtSrc.tl().y <= rt.tl().y && rtSrc.br().y >= rt.br().y)
					{
						bResult = true;
						break;
					}
				}
			}
			break;
		case H_HEAD:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i].rt;
					if (rt.contains(rtSrc.tl()) || rt.contains(rtSrc.br()) || rtSrc.contains(rt.tl()) || rtSrc.contains(rt.br()))
					{
						bResult = true;
						break;
					}
					if (rt.tl().x >= rtSrc.tl().x && rt.br().x <= rtSrc.br().x && rt.tl().y <= rtSrc.tl().y && rt.br().y >= rtSrc.br().y)
					{
						bResult = true;
						break;
					}
					if (rtSrc.tl().x >= rt.tl().x && rtSrc.br().x <= rt.br().x && rtSrc.tl().y <= rt.tl().y && rtSrc.br().y >= rt.br().y)
					{
						bResult = true;
						break;
					}
				}
			}
			break;
		case V_HEAD:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i].rt;
					if (rt.contains(rtSrc.tl()) || rt.contains(rtSrc.br()) || rtSrc.contains(rt.tl()) || rtSrc.contains(rt.br()))
					{
						bResult = true;
						break;
					}
					if (rt.tl().x >= rtSrc.tl().x && rt.br().x <= rtSrc.br().x && rt.tl().y <= rtSrc.tl().y && rt.br().y >= rtSrc.br().y)
					{
						bResult = true;
						break;
					}
					if (rtSrc.tl().x >= rt.tl().x && rtSrc.br().x <= rt.br().x && rtSrc.tl().y <= rt.tl().y && rtSrc.br().y >= rt.br().y)
					{
						bResult = true;
						break;
					}
				}
			}
			break;
		case ABMODEL:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i].rt;
					if (rt.contains(rtSrc.tl()) || rt.contains(rtSrc.br()) || rtSrc.contains(rt.tl()) || rtSrc.contains(rt.br()))
					{
						bResult = true;
						break;
					}
					if (rt.tl().x >= rtSrc.tl().x && rt.br().x <= rtSrc.br().x && rt.tl().y <= rtSrc.tl().y && rt.br().y >= rtSrc.br().y)
					{
						bResult = true;
						break;
					}
					if (rtSrc.tl().x >= rt.tl().x && rtSrc.br().x <= rt.br().x && rtSrc.tl().y <= rt.tl().y && rtSrc.br().y >= rt.br().y)
					{
						bResult = true;
						break;
					}
				}
			}
			break;
		case COURSE:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i].rt;
					if (rt.contains(rtSrc.tl()) || rt.contains(rtSrc.br()) || rtSrc.contains(rt.tl()) || rtSrc.contains(rt.br()))
					{
						bResult = true;
						break;
					}
					if (rt.tl().x >= rtSrc.tl().x && rt.br().x <= rtSrc.br().x && rt.tl().y <= rtSrc.tl().y && rt.br().y >= rtSrc.br().y)
					{
						bResult = true;
						break;
					}
					if (rtSrc.tl().x >= rt.tl().x && rtSrc.br().x <= rt.br().x && rtSrc.tl().y <= rt.tl().y && rtSrc.br().y >= rt.br().y)
					{
						bResult = true;
						break;
					}
				}
			}
			break;
		case QK_CP:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i].rt;
					if (rt.contains(rtSrc.tl()) || rt.contains(rtSrc.br()) || rtSrc.contains(rt.tl()) || rtSrc.contains(rt.br()))
					{
						bResult = true;
						break;
					}
					if (rt.tl().x >= rtSrc.tl().x && rt.br().x <= rtSrc.br().x && rt.tl().y <= rtSrc.tl().y && rt.br().y >= rtSrc.br().y)
					{
						bResult = true;
						break;
					}
					if (rtSrc.tl().x >= rt.tl().x && rtSrc.br().x <= rt.br().x && rtSrc.tl().y <= rt.tl().y && rtSrc.br().y >= rt.br().y)
					{
						bResult = true;
						break;
					}
				}
			}
			break;
		case GRAY_CP:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i].rt;
					if (rt.contains(rtSrc.tl()) || rt.contains(rtSrc.br()) || rtSrc.contains(rt.tl()) || rtSrc.contains(rt.br()))
					{
						bResult = true;
						break;
					}
					if (rt.tl().x >= rtSrc.tl().x && rt.br().x <= rtSrc.br().x && rt.tl().y <= rtSrc.tl().y && rt.br().y >= rtSrc.br().y)
					{
						bResult = true;
						break;
					}
					if (rtSrc.tl().x >= rt.tl().x && rtSrc.br().x <= rt.br().x && rtSrc.tl().y <= rt.tl().y && rtSrc.br().y >= rt.br().y)
					{
						bResult = true;
						break;
					}
				}
			}
			break;
		case WHITE_CP:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i].rt;
					if (rt.contains(rtSrc.tl()) || rt.contains(rtSrc.br()) || rtSrc.contains(rt.tl()) || rtSrc.contains(rt.br()))
					{
						bResult = true;
						break;
					}
					if (rt.tl().x >= rtSrc.tl().x && rt.br().x <= rtSrc.br().x && rt.tl().y <= rtSrc.tl().y && rt.br().y >= rtSrc.br().y)
					{
						bResult = true;
						break;
					}
					if (rtSrc.tl().x >= rt.tl().x && rtSrc.br().x <= rt.br().x && rtSrc.tl().y <= rt.tl().y && rtSrc.br().y >= rt.br().y)
					{
						bResult = true;
						break;
					}
				}
			}
			break;
		case SN:
			{
				//不进行区域重叠检测
			}
			break;
		case OMR:
			{
				//不进行区域重叠检测
			}
			break;
		default:
			break;
	}
	return bResult;
}

void CMakeModelDlg::InitParam()
{
	std::string strLog;
	std::string strFile = g_strCurrentPath + "param.dat";
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strFile);
	try
	{
		Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));

		m_nGaussKernel = pConf->getInt("MakeModel_Recog.gauseKernel", 5);
		m_nSharpKernel = pConf->getInt("MakeModel_Recog.sharpKernel", 5);
		m_nCannyKernel = pConf->getInt("MakeModel_Recog.cannyKernel", 90);
		m_nDelateKernel = pConf->getInt("MakeModel_Recog.delateKernel", 6);
		m_nErodeKernel = pConf->getInt("MakeModel_Recog.eRodeKernel", 2);

		m_nWhiteVal = pConf->getInt("MakeModel_Threshold.white", 225);
		m_nHeadVal	= pConf->getInt("MakeModel_Threshold.head", 136);
		m_nABModelVal = pConf->getInt("MakeModel_Threshold.abModel", 150);
		m_nCourseVal = pConf->getInt("MakeModel_Threshold.course", 150);
		m_nQK_CPVal = pConf->getInt("MakeModel_Threshold.qk", 150);
		m_nGrayVal	= pConf->getInt("MakeModel_Threshold.gray", 150);
		m_nFixVal	= pConf->getInt("MakeModel_Threshold.fix", 150);
		m_nOMR		= pConf->getInt("MakeModel_Threshold.omr", 230);
		m_nSN		= pConf->getInt("MakeModel_Threshold.sn", 200);

		m_fHeadThresholdPercent		= pConf->getDouble("MakeModel_RecogPercent.head", 0.75);
		m_fABModelThresholdPercent	= pConf->getDouble("MakeModel_RecogPercent.abModel", 0.75);
		m_fCourseThresholdPercent	= pConf->getDouble("MakeModel_RecogPercent.course", 0.75);
		m_fQK_CPThresholdPercent	= pConf->getDouble("MakeModel_RecogPercent.qk", 0.75);
		m_fFixThresholdPercent		= pConf->getDouble("MakeModel_RecogPercent.fix", 0.8);
		m_fGrayThresholdPercent		= pConf->getDouble("MakeModel_RecogPercent.gray", 0.75);
		m_fWhiteThresholdPercent	= pConf->getDouble("MakeModel_RecogPercent.white", 0.75);
		m_fOMRThresholdPercent		= pConf->getDouble("MakeModel_RecogPercent.omr", 1.5);
		m_fSNThresholdPercent		= pConf->getDouble("MakeModel_RecogPercent.sn", 1.5);
		strLog = "读取参数完成";
	}
	catch (Poco::Exception& exc)
	{
		strLog = "读取参数失败，使用默认参数 " + CMyCodeConvert::Utf8ToGb2312(exc.displayText());
		m_nGaussKernel = 5;
		m_nSharpKernel = 5;
		m_nCannyKernel = 90;
		m_nDelateKernel = 6;
		m_nErodeKernel = 2;

		m_nWhiteVal = 225;
		m_nHeadVal	= 136;
		m_nABModelVal = 150;
		m_nCourseVal = 150;
		m_nQK_CPVal = 150;
		m_nGrayVal	= 150;
		m_nFixVal	= 150;
		m_nOMR		= 230;
		m_nSN		= 200;

		m_fHeadThresholdPercent		= 0.75;
		m_fABModelThresholdPercent	= 0.75;
		m_fCourseThresholdPercent	= 0.75;
		m_fQK_CPThresholdPercent	= 0.75;
		m_fFixThresholdPercent		= 0.8;
		m_fGrayThresholdPercent		= 0.75;
		m_fWhiteThresholdPercent	= 0.75;
		m_fOMRThresholdPercent		= 1.5;
		m_fSNThresholdPercent		= 1.5;
	}
	g_pLogger->information(strLog);
}

// MakeModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanTool3Dlg.h"
//#include "GuideDlg.h"
#include "MakeModelDlg.h"
#include "afxdialogex.h"
#include "ModelSaveDlg.h"
#include <afxinet.h>
#include "ModelInfoDlg.h"
#include "THSetDlg.h"
//#include "ScanCtrlDlg.h"
#include "Net_Cmd_Protocol.h"
//#include "./pdf2jpg/MuPDFConvert.h"
#include "AdanceSetMgrDlg.h"
#include "ScanModelPaperDlg.h"
#include "ExamInfoDlg.h"
#include "NewMessageBox.h"
#include "RecogCharacterDlg.h"
using namespace std;
using namespace cv;
// CMakeModelDlg 对话框

IMPLEMENT_DYNAMIC(CMakeModelDlg, CDialog)

CMakeModelDlg::CMakeModelDlg(pMODEL pModel /*= NULL*/, CWnd* pParent /*=NULL*/)
	: CDialog(CMakeModelDlg::IDD, pParent)
	, m_pModelPicShow(NULL), m_nGaussKernel(5), m_nSharpKernel(5), m_nThresholdKernel(150), m_nCannyKernel(90), m_nDilateKernel(6), m_nErodeKernel(2), m_nDilateKernel_DefCommon(6), m_nDilateKernel_DefSn(6)
	, m_pModel(pModel), m_bNewModelFlag(false), m_nModelPicNums(1), m_nCurrTabSel(0), m_bSavedModelFlag(false), m_ncomboCurrentSel(0), m_eCurCPType(UNKNOWN)
	, m_nCurListCtrlSel(0), m_nStartTH(0)
	, m_nWhiteVal(225), m_nHeadVal(150), m_nPaginationVal(150), m_nABModelVal(150), m_nCourseVal(150), m_nQK_CPVal(150), m_nWJ_CPVal(150), m_nGrayVal(150), m_nFixVal(150), m_nOMR(230), m_nSN(200), m_nCharacterThreshold(150), m_nThreshold_DefSn(150), m_nThreshold_DefOmr(150), m_nCharacterConfidence(60)
	, m_fHeadThresholdPercent(0.75), m_fPaginationThresholdPercent(0.75), m_fABModelThresholdPercent(0.75), m_fCourseThresholdPercent(0.75), m_fQK_CPThresholdPercent_Fix(1.5), m_fWJ_CPThresholdPercent_Fix(1.5), m_fQK_CPThresholdPercent_Head(1.2), m_fWJ_CPThresholdPercent_Head(1.2), m_fFixThresholdPercent(0.80)
	, m_fGrayThresholdPercent(0.75), m_fWhiteThresholdPercent(0.75), m_fOMRThresholdPercent_Fix(1.5), m_fSNThresholdPercent_Fix(1.5), m_fOMRThresholdPercent_Head(1.2), m_fSNThresholdPercent_Head(1.2)
	, m_pCurRectInfo(NULL)
	, m_bFistHTracker(true), m_bFistVTracker(true), m_bFistSNTracker(true)
	, m_pRecogInfoDlg(NULL), m_pOmrInfoDlg(NULL), m_pSNInfoDlg(NULL), m_pElectOmrDlg(NULL)
	, m_bShiftKeyDown(false)/*, m_pScanThread(NULL)*/
	, _pTWAINApp(NULL)
#ifdef USE_TESSERACT
	, m_pTess(NULL)
#endif
{
	InitParam();
}

CMakeModelDlg::~CMakeModelDlg()
{
	SAFE_RELEASE(m_pRecogInfoDlg);
	SAFE_RELEASE(m_pOmrInfoDlg);
	SAFE_RELEASE(m_pSNInfoDlg);
	SAFE_RELEASE(m_pElectOmrDlg);

// 	if (m_bNewModelFlag && !m_bSavedModelFlag && m_pModel != NULL)
// 		SAFE_RELEASE(m_pModel);

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
	ON_BN_CLICKED(IDC_BTN_MAKEMODEL_SAVE, &CMakeModelDlg::OnBnClickedBtnSave)
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
	ON_COMMAND(ID_LeftRotate, &CMakeModelDlg::LeftRotate)
	ON_COMMAND(ID_RightRotate, &CMakeModelDlg::RightRotate)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CheckPoint, &CMakeModelDlg::OnNMDblclkListCheckpoint)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_CheckPoint, &CMakeModelDlg::OnLvnKeydownListCheckpoint)
//	ON_BN_CLICKED(IDC_BTN_SaveRecogInfo, &CMakeModelDlg::OnBnClickedBtnSaverecoginfo)
	ON_MESSAGE(WM_CV_HTrackerChange, &CMakeModelDlg::HTrackerChange)
	ON_MESSAGE(WM_CV_VTrackerChange, &CMakeModelDlg::VTrackerChange)
	ON_MESSAGE(WM_CV_SNTrackerChange, &CMakeModelDlg::SNTrackerChange)
//	ON_BN_CLICKED(IDC_BTN_uploadModel, &CMakeModelDlg::OnBnClickedBtnuploadmodel)
	ON_BN_CLICKED(IDC_BTN_ScanModel, &CMakeModelDlg::OnBnClickedBtnScanmodel)
	ON_MESSAGE(WM_CV_ShiftDown, &CMakeModelDlg::ShiftKeyDown)
	ON_MESSAGE(WM_CV_ShiftUp, &CMakeModelDlg::ShiftKeyUp)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_HOVER, IDC_LIST_CheckPoint, &CMakeModelDlg::OnNMHoverListCheckpoint)
	ON_BN_CLICKED(IDC_BTN_AdvancedSetting, &CMakeModelDlg::OnBnClickedBtnAdvancedsetting)
	ON_MESSAGE(MSG_SCAN_DONE, &CMakeModelDlg::ScanDone)
	ON_MESSAGE(MSG_SCAN_ERR, &CMakeModelDlg::ScanErr)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CMakeModelDlg 消息处理程序
BOOL CMakeModelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
#ifdef USE_TESSERACT
	m_pTess = new tesseract::TessBaseAPI();
	m_pTess->Init(NULL, "chi_sim", tesseract::OEM_DEFAULT);
#endif

	USES_CONVERSION;
	InitUI();
	InitConf();
	InitScanner();
	m_scanThread.CreateThread();
	if (m_pModel)
	{
		m_vecPaperModelInfo.clear();

		for (int i = 0; i < m_pModel->nPicNum; i++)
		{
			CString strPicPath = g_strCurrentPath + _T("Model\\") + A2T(m_pModel->strModelName.c_str()) + _T("\\") + A2T(m_pModel->vecPaperModel[i]->strModelPicName.c_str());

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
			m_vecPicShow[i]->ShowPic(src_img, cv::Point(0,0), 1.0);

			pPaperModel->nPicW = src_img.cols;
			pPaperModel->nPicH = src_img.rows;
			
			pPaperModel->bFirstH = false;
			pPaperModel->bFirstV = false;
			pPaperModel->rtHTracker = m_pModel->vecPaperModel[i]->rtHTracker;
			pPaperModel->rtVTracker = m_pModel->vecPaperModel[i]->rtVTracker;
//			pPaperModel->rtSNTracker = m_pModel->vecPaperModel[i]->rtSNTracker;
			pPaperModel->rcSNTracker = m_pModel->vecPaperModel[i]->rcSNTracker;

			CHARACTER_ANCHOR_AREA_LIST::iterator itRecogCharRt = m_pModel->vecPaperModel[i]->lCharacterAnchorArea.begin();
			for (; itRecogCharRt != m_pModel->vecPaperModel[i]->lCharacterAnchorArea.end(); itRecogCharRt++)
			{
				pST_CHARACTER_ANCHOR_AREA pStCharacterAnchorArea = new ST_CHARACTER_ANCHOR_AREA();
				pStCharacterAnchorArea->nIndex = (*itRecogCharRt)->nIndex;
				pStCharacterAnchorArea->nCannyKernel = (*itRecogCharRt)->nCannyKernel;
				pStCharacterAnchorArea->nDilateKernel = (*itRecogCharRt)->nDilateKernel;
				pStCharacterAnchorArea->nGaussKernel = (*itRecogCharRt)->nGaussKernel;
				pStCharacterAnchorArea->nSharpKernel = (*itRecogCharRt)->nSharpKernel;
				pStCharacterAnchorArea->nThresholdValue = (*itRecogCharRt)->nThresholdValue;
				pStCharacterAnchorArea->nCharacterConfidence = (*itRecogCharRt)->nCharacterConfidence;
				pStCharacterAnchorArea->rt = (*itRecogCharRt)->rt;
				for (auto itCharPoint : (*itRecogCharRt)->vecCharacterRt)
				{
					pST_CHARACTER_ANCHOR_POINT pStCharAnchorPoint = new ST_CHARACTER_ANCHOR_POINT();
					pStCharAnchorPoint->nIndex = itCharPoint->nIndex;
					pStCharAnchorPoint->fConfidence = itCharPoint->fConfidence;
					pStCharAnchorPoint->rc = itCharPoint->rc;
					pStCharAnchorPoint->strVal = itCharPoint->strVal;
					pStCharacterAnchorArea->vecCharacterRt.push_back(pStCharAnchorPoint);
				}
				pPaperModel->vecCharacterLocation.push_back(pStCharacterAnchorArea);

//				pPaperModel->vecCharacterLocation.push_back(*itRecogCharRt);
			}
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
			RECTLIST::iterator itPage = m_pModel->vecPaperModel[i]->lPagination.begin();
			for (; itPage != m_pModel->vecPaperModel[i]->lPagination.end(); itPage++)
			{
				pPaperModel->vecPagination.push_back(*itPage);
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
			RECTLIST::iterator itWJ = m_pModel->vecPaperModel[i]->lWJ_CP.begin();
			for (; itWJ != m_pModel->vecPaperModel[i]->lWJ_CP.end(); itWJ++)
			{
				pPaperModel->vecQK_CP.push_back(*itWJ);
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
			OMRLIST::iterator itOmr2 = m_pModel->vecPaperModel[i]->lOMR2.begin();
			for (; itOmr2 != m_pModel->vecPaperModel[i]->lOMR2.end(); itOmr2++)
			{
				pPaperModel->vecOmr2.push_back(*itOmr2);
			}
			ELECTOMR_LIST::iterator itElectOmr = m_pModel->vecPaperModel[i]->lElectOmr.begin();
			for (; itElectOmr != m_pModel->vecPaperModel[i]->lElectOmr.end(); itElectOmr++)
			{
				pPaperModel->vecElectOmr.push_back(*itElectOmr);
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
		strTitle.Format(_T("模板名称: %s"), A2T(m_pModel->strModelName.c_str()));
		SetWindowText(strTitle);

		m_pSNInfoDlg->InitType(m_pModel->nZkzhType);
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

	//删除表头科目部分，重新插入科目
	while (m_cpListCtrl.DeleteColumn(0));
	m_cpListCtrl.DeleteAllItems();
	m_cpListCtrl.SetExtendedStyle(m_cpListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_cpListCtrl.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 36);
	m_cpListCtrl.InsertColumn(1, _T("位置信息"), LVCFMT_CENTER, 120);

	//释放图像显示控件
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

	CRect rtTab;
	m_tabModelPicCtrl.DeleteAllItems();
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

	if (m_pRecogInfoDlg)
	{
		m_pRecogInfoDlg->DestroyWindow();
		SAFE_RELEASE(m_pRecogInfoDlg);
	}
	m_pRecogInfoDlg = new CRecogInfoDlg;
	m_pRecogInfoDlg->Create(CRecogInfoDlg::IDD, this);
	m_pRecogInfoDlg->ShowWindow(SW_SHOW);	//SW_SHOW

	if (m_pOmrInfoDlg)
	{
		m_pOmrInfoDlg->DestroyWindow();
		SAFE_RELEASE(m_pOmrInfoDlg);
	}
	m_pOmrInfoDlg = new COmrInfoDlg;
	m_pOmrInfoDlg->Create(COmrInfoDlg::IDD, this);
	m_pOmrInfoDlg->ShowWindow(SW_HIDE);	//SW_HIDE

	if (m_pSNInfoDlg)
	{
		m_pSNInfoDlg->DestroyWindow();
		SAFE_RELEASE(m_pSNInfoDlg);
	}
	m_pSNInfoDlg = new CSNInfoSetDlg;
	m_pSNInfoDlg->Create(CSNInfoSetDlg::IDD, this);
	m_pSNInfoDlg->ShowWindow(SW_HIDE);

	if (m_pElectOmrDlg)
	{
		m_pElectOmrDlg->DestroyWindow();
		SAFE_RELEASE(m_pElectOmrDlg);
	}
	m_pElectOmrDlg = new ElectOmrDlg;
	m_pElectOmrDlg->Create(ElectOmrDlg::IDD, this);
	m_pElectOmrDlg->ShowWindow(SW_HIDE);

#if 0
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
#endif
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
	if (m_pElectOmrDlg && m_pElectOmrDlg->GetSafeHwnd())
	{
		m_pElectOmrDlg->MoveWindow(nLeftGap, nTopInGroup, nLeftCtrlWidth, nGroupHeight);
	}
#if 1
	nCurrentTop = nCurrentTop + nGroupHeight + nGap;

	int nBtnW = (nLeftCtrlWidth - nGap) / 2;
	if (GetDlgItem(IDC_BTN_AdvancedSetting)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_AdvancedSetting)->MoveWindow(nLeftGap, nCurrentTop, nLeftCtrlWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
	if (GetDlgItem(IDC_BTN_SelPic)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_SelPic)->MoveWindow(nLeftGap, nCurrentTop, nBtnW, nBtnHeigh);
	}
	if (GetDlgItem(IDC_BTN_RESET)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_RESET)->MoveWindow(nLeftGap + nBtnW + nGap, nCurrentTop, nBtnW, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}

	if (GetDlgItem(IDC_BTN_New)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_New)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
	}
	if (GetDlgItem(IDC_BTN_ScanModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ScanModel)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
// 	if (GetDlgItem(IDC_BTN_SelPic)->GetSafeHwnd())
// 	{
// 		GetDlgItem(IDC_BTN_SelPic)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
// 		//		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
// 	}
// 	if (GetDlgItem(IDC_BTN_RESET)->GetSafeHwnd())
// 	{
// 		GetDlgItem(IDC_BTN_RESET)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
// 		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
// 	}
	if (GetDlgItem(IDC_BTN_MAKEMODEL_SAVE)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_MAKEMODEL_SAVE)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		//		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
// 	if (GetDlgItem(IDC_BTN_AdvancedSetting)->GetSafeHwnd())
// 	{
// 		GetDlgItem(IDC_BTN_AdvancedSetting)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
// 		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
// 	}
	if (GetDlgItem(IDC_BTN_ExitModelDlg)->GetSafeHwnd())
	{
		//		GetDlgItem(IDC_BTN_ExitModelDlg)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		GetDlgItem(IDC_BTN_ExitModelDlg)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
#else
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
	if (GetDlgItem(IDC_BTN_MAKEMODEL_SAVE)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_MAKEMODEL_SAVE)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
//		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
#if 1
	if (GetDlgItem(IDC_BTN_AdvancedSetting)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_AdvancedSetting)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
#endif
	if (GetDlgItem(IDC_BTN_ExitModelDlg)->GetSafeHwnd())
	{
//		GetDlgItem(IDC_BTN_ExitModelDlg)->MoveWindow(nLeftGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		GetDlgItem(IDC_BTN_ExitModelDlg)->MoveWindow(nLeftGap + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
#endif

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
			m_comboCheckPointType.AddString(_T("违纪校验点"));
			m_comboCheckPointType.AddString(_T("灰度校验点"));
			m_comboCheckPointType.AddString(_T("空白校验点"));
			m_comboCheckPointType.AddString(_T("考号设置"));
			m_comboCheckPointType.AddString(_T("选择/判断题"));
			m_comboCheckPointType.AddString(_T("选做题"));
		}
		else if (m_pModel->nABModel&&!m_pModel->nHasHead)
		{
			m_comboCheckPointType.ResetContent();
			m_comboCheckPointType.AddString(_T(""));
//			m_comboCheckPointType.AddString(_T("定点"));
			m_comboCheckPointType.AddString(_T("卷形校验点"));
			m_comboCheckPointType.AddString(_T("科目校验点"));
			m_comboCheckPointType.AddString(_T("缺考校验点"));
			m_comboCheckPointType.AddString(_T("违纪校验点"));
			m_comboCheckPointType.AddString(_T("灰度校验点"));
			m_comboCheckPointType.AddString(_T("空白校验点"));
			m_comboCheckPointType.AddString(_T("考号设置"));
			m_comboCheckPointType.AddString(_T("选择/判断题"));
			m_comboCheckPointType.AddString(_T("选做题"));
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
			m_comboCheckPointType.AddString(_T("违纪校验点"));
			m_comboCheckPointType.AddString(_T("灰度校验点"));
			m_comboCheckPointType.AddString(_T("空白校验点"));
			m_comboCheckPointType.AddString(_T("考号设置"));
			m_comboCheckPointType.AddString(_T("选择/判断题"));
			m_comboCheckPointType.AddString(_T("选做题"));
		}
		else
		{
			if (m_pModel->nUseWordAnchorPoint)
			{
				m_comboCheckPointType.ResetContent();
				m_comboCheckPointType.AddString(_T(""));
				m_comboCheckPointType.AddString(_T("文字定位区"));
				if (m_pModel->nUsePagination)
					m_comboCheckPointType.AddString(_T("页码"));
				m_comboCheckPointType.AddString(_T("科目校验点"));
				m_comboCheckPointType.AddString(_T("缺考校验点"));
				m_comboCheckPointType.AddString(_T("违纪校验点"));
				m_comboCheckPointType.AddString(_T("灰度校验点"));
				//m_comboCheckPointType.AddString(_T("空白校验点"));
				m_comboCheckPointType.AddString(_T("考号设置"));
				m_comboCheckPointType.AddString(_T("选择/判断题"));
				m_comboCheckPointType.AddString(_T("选做题"));
			}
			else
			{
				m_comboCheckPointType.ResetContent();
				m_comboCheckPointType.AddString(_T(""));
				m_comboCheckPointType.AddString(_T("定点"));
				if (m_pModel->nUsePagination)
					m_comboCheckPointType.AddString(_T("页码"));
				m_comboCheckPointType.AddString(_T("科目校验点"));
				m_comboCheckPointType.AddString(_T("缺考校验点"));
				m_comboCheckPointType.AddString(_T("违纪校验点"));
				m_comboCheckPointType.AddString(_T("灰度校验点"));
				//m_comboCheckPointType.AddString(_T("空白校验点"));
				m_comboCheckPointType.AddString(_T("考号设置"));
				m_comboCheckPointType.AddString(_T("选择/判断题"));
				m_comboCheckPointType.AddString(_T("选做题"));
			#ifdef TEST_GRAY_WHITE
				m_comboCheckPointType.AddString(_T("空白校验点"));	//空白校验区
			#endif
			}
		}
	}
	m_comboCheckPointType.SetCurSel(0);
	m_eCurCPType = UNKNOWN;
}

LRESULT CMakeModelDlg::RoiRBtnUp(WPARAM wParam, LPARAM lParam)
{
	cv::Point pt = *(cv::Point*)(wParam);
	if (m_eCurCPType == UNKNOWN)
	{
	#ifdef TEST_MODEL_ROTATION
		CMenu menu, *pPopup;
		menu.LoadMenu(IDR_MENU_Rotation);
		pPopup = menu.GetSubMenu(0);
		CPoint myPoint;
		ClientToScreen(&myPoint);
		GetCursorPos(&myPoint); //鼠标位置  
		pPopup->RemoveMenu(ID_PicsExchange, MF_BYCOMMAND);		//删除不需要的选项
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
	#endif
		return TRUE;
	}

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

#if 0
			menu.LoadMenu(IDR_MENU_RecogSN);
#else
			if (m_pSNInfoDlg->m_nZkzhType == 2)		//条码时，只添加区域，不识别
				menu.LoadMenu(IDR_MENU_AddRecog);
			else
				menu.LoadMenu(IDR_MENU_RecogSN);
#endif

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
	nFind = GetRectInfoByPoint(GetSrcSavePoint(pt), m_eCurCPType, pRc);
	if (nFind < 0 || !pRc)
	{
	#ifdef TEST_MODEL_ROTATION
		CMenu menu, *pPopup;
		menu.LoadMenu(IDR_MENU_Rotation);
		pPopup = menu.GetSubMenu(0);
		CPoint myPoint;
		ClientToScreen(&myPoint);
		GetCursorPos(&myPoint); //鼠标位置  
		pPopup->RemoveMenu(ID_PicsExchange, MF_BYCOMMAND);		//删除不需要的选项
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
	#endif
		return TRUE;
	}

	//下面的这段代码, 不单单适应于ListCtrl  
	CMenu menu, *pPopup;
	menu.LoadMenu(IDR_MENU_RectInPic);
	pPopup = menu.GetSubMenu(0);
	CPoint myPoint;
	ClientToScreen(&myPoint);
	GetCursorPos(&myPoint); //鼠标位置  
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);//GetParent()
	m_ptRBtnUp = GetSrcSavePoint(pt);
	return TRUE;
}

LRESULT CMakeModelDlg::RoiLBtnUp(WPARAM wParam, LPARAM lParam)
{
	Rect  Rt = *(Rect*)(wParam);
	Mat*  pShowMat = (Mat*)(lParam);

	m_cpListCtrl.SetItemState(m_nCurListCtrlSel, 0, LVIS_DROPHILITED);		// 取消高亮显示
	if (m_eCurCPType == UNKNOWN)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "请先选中校验点类型");
		dlg.DoModal();
		return FALSE;
	}

	if (m_eCurCPType != H_HEAD && m_eCurCPType != V_HEAD && m_pModel && m_pModel->nHasHead == 0)
	{
		Rt = GetSrcSaveRect(Rt);
	#ifdef USE_TESSERACT
		if (m_eCurCPType == CHARACTER_AREA)
		{
			if (checkOverlap(m_eCurCPType, Rt))
			{
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, "检测到包含已选区域");
				dlg.DoModal();
				return FALSE;
			}
			RecogCharacterArea(Rt);
			SortRect();
			UpdataCPList();
		}
		else if (m_eCurCPType == WHITE_CP)	//WHITE_GRAY_AREA
		{
			if (checkOverlap(m_eCurCPType, Rt))
			{
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, "检测到包含已选区域");
				dlg.DoModal();
				return FALSE;
			}
			RecogWhiteAreaGray(Rt);
		}
		else
		{
			if (checkOverlap(m_eCurCPType, Rt))
			{
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, "检测到包含已选区域");
				dlg.DoModal();
				return FALSE;
			}
			Recognise(Rt);
			SortRect();
			UpdataCPList();
		}
	#else
		if (checkOverlap(m_eCurCPType, Rt))
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "检测到包含已选区域");
			dlg.DoModal();
			return FALSE;
		}
		Recognise(Rt);
		SortRect();
		UpdataCPList();
	#endif
	}
	else if (m_eCurCPType != H_HEAD && m_eCurCPType != V_HEAD && m_pModel &&  m_pModel->nHasHead != 0)
	{
		Rt = GetSrcSaveRect(Rt);
		if (checkOverlap(m_eCurCPType, Rt))
		{
//			AfxMessageBox(_T("检测到包含已选区域"));
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "检测到包含已选区域");
			dlg.DoModal();
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
//			AfxMessageBox(_T("请先设置水平同步头"));
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "请先设置水平同步头");
			dlg.DoModal();
			return false;
		}
		if (!m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size())
		{
//			AfxMessageBox(_T("请先设置垂直同步头"));
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "请先设置垂直同步头");
			dlg.DoModal();
			return false;
		}

		pt = GetSrcSavePoint(pt);
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
		if (m_eCurCPType == PAGINATION)
		{
			rc.nThresholdValue = m_nPaginationVal;
			rc.fStandardValuePercent = m_fPaginationThresholdPercent;

			Rect rtTmp = rc.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecTmp.push_back(rc);
		}
		else if (m_eCurCPType == ABMODEL)
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
			rc.fStandardValuePercent = m_fQK_CPThresholdPercent_Head;

			Rect rtTmp = rc.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecTmp.push_back(rc);
		}
		else if (m_eCurCPType == WJ_CP)
		{
			rc.nThresholdValue = m_nWJ_CPVal;
			rc.fStandardValuePercent = m_fWJ_CPThresholdPercent_Head;

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
	CScanModelPaperDlg dlg(this);
	dlg.SetScanSrc(m_vecScanSrc);
	dlg.DoModal();
	m_strScanSavePath = dlg.m_strSavePath;

	GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(FALSE);

#if 0
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

#ifdef TEST_SCAN_THREAD
//	m_scanThread.setScanPath(m_strScanSavePath);
	m_scanThread.PostThreadMessage(MSG_START_SCAN, NULL, NULL);

// 	if(m_pScanThread)
// 	{
// 		m_pScanThread->
// 	}
	
#else
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

	CScanCtrlDlg dlg(m_scanSourceArry, true, false);
	if (dlg.DoModal() != IDOK)
		return;

	GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(FALSE);

	m_Source = m_scanSourceArry.GetAt(dlg.m_nCurrScanSrc);
	int nDuplex = dlg.m_nCurrDuplex;		//单双面,0-单面,1-双面

	bool bShowScanSrcUI = g_bShowScanSrcUI;
	
	int nSize = TWSS_NONE;							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3	TWSS_NONE-定制
	int nPixel = 2;							//0-黑白，1-灰度，2-彩色
	int nResolution = 200;					//dpi: 72, 150, 200, 300

	if (dlg.m_nCurrPaperSize == 0)
	{
		nSize = TWSS_A4LETTER;
//		nResolution = 200;
	}
	else if (dlg.m_nCurrPaperSize == 1)
	{
		nSize = TWSS_A3;
//		nResolution = 150;
	}
	else
	{
		nSize = TWSS_NONE;
//		nResolution = 150;
	}

	int nNum = dlg.m_nStudentNum;

	if (nDuplex == 1)
		nNum *= 2;

	if (nNum == 0)
		nNum = TWCPP_ANYCOUNT;

	if (dlg.m_bAdvancedSetting)
		bShowScanSrcUI = true;

	if (!Acquire(nNum, nDuplex, nSize, nPixel, nResolution, bShowScanSrcUI))
	{
		TRACE("扫描失败\n");
	}
	GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(TRUE);
#endif
#endif
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

	if (_pCurrExam_)
		m_pModel->nExamID = _pCurrExam_->nExamID;
	if (_pCurrSub_)
		m_pModel->nSubjectID = _pCurrSub_->nSubjID;

	InitConf();
	
	for (int i = 0; i < m_nModelPicNums; i++)
	{
		pPaperModelInfo paperMode = new PaperModelInfo;
		m_vecPaperModelInfo.push_back(paperMode);

		paperMode->strModelPicName = T2A(dlg.m_vecPath[i].strName);
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

		//试卷纸张类型判断
		int nL, nW;
		if (matImg.cols > matImg.rows)
		{
			nL = matImg.cols;
			nW = matImg.rows;
		}
		else
		{
			nL = matImg.rows;
			nW = matImg.cols;
		}
		if (i == 0)
		{
			if (nW > 1350 && nW < 1800)
			{
				if (nL > 2000 && nL < 2500)
				{
					m_pModel->nScanDpi = 200;
					m_pModel->nScanSize = 1;	//A4
				}
			}
			else if (nW > 2000 && nW < 2500)
			{
				if (nL > 2850 && nL < 3450)
				{
					m_pModel->nScanDpi = 200;
					m_pModel->nScanSize = 2;	//A3
				}
			}
// 			else if (nW > 1450 && nW < 1900)
// 			{
// 				if (nL > 2150 && nL < 2600)
// 				{
// 					m_pModel->nScanDpi = 150;
// 					m_pModel->nScanSize = 2;	//A3
// 				}
// 			}
		}
	}
	SetWindowTextW(_T("*未保存模板*"));
}

void CMakeModelDlg::OnBnClickedBtnSelpic()
{
	if (!m_pModel)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "请先创建模板");
		dlg.DoModal();
		return;
	}
	if (!m_bNewModelFlag)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "重新选择图片需要重新设置本页的校验点");
		dlg.DoModal();
	}

	CFileDialog dlg(true, _T("*.bmp"), NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		_T("image file(*.bmp;*.png;*.tif;*.tiff;*.jpg)|*.bmp;*.png;*.tif;*.tiff;*.jpg|All Files(*.*)|*.*|"), NULL);
	dlg.m_ofn.lpstrTitle = _T("选择模板图像");
	if (dlg.DoModal() != IDOK)
		return;

	USES_CONVERSION;
	pPaperModelInfo paperMode = NULL;
	paperMode = m_vecPaperModelInfo[m_nCurrTabSel];

	paperMode->strModelPicName = T2A(dlg.GetFileName());
	paperMode->strModelPicPath = dlg.GetPathName();

	Mat src_img = imread((std::string)(CT2CA)paperMode->strModelPicPath);	//(std::string)(CT2CA)paperMode->strModelPicPath

	paperMode->matSrcImg = src_img;
	paperMode->matDstImg = paperMode->matSrcImg;
	m_pModelPicShow->ShowPic(src_img);
	
	paperMode->nPicW = src_img.cols;
	paperMode->nPicH = src_img.rows;

	//重新选择图片后，需要重置本页面的所有点信息
	if (m_vecPaperModelInfo.size() <= 0 || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	//旋转方向重置
	m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes = 0;
	m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 1;

	m_cpListCtrl.DeleteAllItems();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.clear();
	m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.clear();
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
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "请先创建模板");
		dlg.DoModal();
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
		}
	case H_HEAD:
		if (m_eCurCPType == H_HEAD || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.clear();
		}
	case V_HEAD:
		if (m_eCurCPType == V_HEAD || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.clear();
		}
	case ABMODEL:
		if (m_eCurCPType == ABMODEL || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.clear();
		}
	case COURSE:
		if (m_eCurCPType == COURSE || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.clear();
		}
	case QK_CP:
		if (m_eCurCPType == QK_CP || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.clear();
		}
	case WJ_CP:
		if (m_eCurCPType == WJ_CP || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.clear();
		}
	case GRAY_CP:
		if (m_eCurCPType == GRAY_CP || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.clear();
		}
	case WHITE_CP:
		if (m_eCurCPType == WHITE_CP || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.clear();
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
		}
	case ELECT_OMR:
		if (m_eCurCPType == ELECT_OMR || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.clear();
			if (m_pElectOmrDlg)
				m_pElectOmrDlg->ReleaseData();
		}
	case CHARACTER_AREA:
		if (m_eCurCPType == CHARACTER_AREA || m_eCurCPType == UNKNOWN)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.clear();
		}
	}
}

pTW_IDENTITY CMakeModelDlg::GetScanSrc(int nIndex)
{
	return _pTWAINApp->getDataSource(nIndex);
}

void CMakeModelDlg::InitScanner()
{
	if (_pTWAINApp)
	{
		_pTWAINApp->exit();
		SAFE_RELEASE(_pTWAINApp);
	}

	_pTWAINApp = new TwainApp(m_hWnd);

	TW_IDENTITY *pAppID = _pTWAINApp->getAppIdentity();

	pAppID->Version.MajorNum = 2;
	pAppID->Version.MinorNum = 1;
	pAppID->Version.Language = TWLG_ENGLISH_CANADIAN;
	pAppID->Version.Country = TWCY_CANADA;
	SSTRCPY(pAppID->Version.Info, sizeof(pAppID->Version.Info), "2.1.1");
	pAppID->ProtocolMajor = TWON_PROTOCOLMAJOR;
	pAppID->ProtocolMinor = TWON_PROTOCOLMINOR;
	pAppID->SupportedGroups = DF_APP2 | DG_IMAGE | DG_CONTROL;
	SSTRCPY(pAppID->Manufacturer, sizeof(pAppID->Manufacturer), "TWAIN Working Group");
	SSTRCPY(pAppID->ProductFamily, sizeof(pAppID->ProductFamily), "Sample");
	SSTRCPY(pAppID->ProductName, sizeof(pAppID->ProductName), "MFC Supported Caps");

	_pTWAINApp->connectDSM();
	if (_pTWAINApp->m_DSMState >= 3)
	{
		pTW_IDENTITY pID = NULL;
		int   i = 0;
		int   index = 0;
		int   nDefault = -1;

		// Emply the list the refill
		m_vecScanSrc.clear();

		if (NULL != (pID = _pTWAINApp->getDefaultDataSource())) // Get Default
		{
			nDefault = pID->Id;
		}
		USES_CONVERSION;
		while (NULL != (pID = _pTWAINApp->getDataSource((TW_INT16)i)))
		{
			m_vecScanSrc.push_back(A2T(pID->ProductName));
			if (LB_ERR == index)
			{
				break;
			}

			i++;
		}
		_pTWAINApp->disconnectDSM();
	}
}

LRESULT CMakeModelDlg::ScanDone(WPARAM wParam, LPARAM lParam)
{
	pST_SCAN_RESULT pResult = (pST_SCAN_RESULT)wParam;
	if (pResult)
	{
		TRACE("扫描完成消息。%s\n", pResult->strResult.c_str());
		g_pLogger->information(pResult->strResult);

		if (pResult->bScanOK)	//扫描完成
		{
			GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(TRUE);
			AfxMessageBox(_T("扫描完成"));
			CString strSelect = _T("/root,");
			strSelect.Append(m_strScanSavePath);
			ShellExecute(NULL, _T("open"), _T("explorer.exe"), strSelect, NULL, SW_SHOWNORMAL);
		}

		delete pResult;
		pResult = NULL;
	}
	return 1;
}

LRESULT CMakeModelDlg::ScanErr(WPARAM wParam, LPARAM lParam)
{
	pST_SCAN_RESULT pResult = (pST_SCAN_RESULT)wParam;
	if (pResult)
	{
		TRACE("扫描错误。%s\n", pResult->strResult.c_str());
		GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(TRUE);
		delete pResult;
		pResult = NULL;
	}
	return 1;
}
bool CMakeModelDlg::RecogNewGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc)
{
//	cv::cvtColor(matSrcRoi, matSrcRoi, CV_BGR2GRAY);

	return RecogGrayValue(matSrcRoi, rc);
}

inline bool CMakeModelDlg::RecogGrayValue(cv::Mat& matSrcRoi, RECTINFO& rc)
{
	cv::cvtColor(matSrcRoi, matSrcRoi, CV_BGR2GRAY);
	cv::GaussianBlur(matSrcRoi, matSrcRoi, cv::Size(m_nGaussKernel, m_nGaussKernel), 0, 0);
	sharpenImage1(matSrcRoi, matSrcRoi);
	
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
//	cv::calcHist(&matSrcRoi, 1, channels, Mat(), src_hist, 1, histSize, ranges, true, false);

	rc.fStandardValue = src_hist.at<float>(0);
	rc.fStandardArea = rc.rt.area();
	rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea;

#if 1
	MatND mean;
	MatND stddev;
	meanStdDev(matSrcRoi, mean, stddev);

	IplImage *src;
	src = &IplImage(mean);
	rc.fStandardMeanGray = cvGetReal2D(src, 0, 0);

	IplImage *src2;
	src2 = &IplImage(stddev);
	rc.fStandardStddev = cvGetReal2D(src2, 0, 0);
#else
	MatND src_hist2;
	const int histSize2[1] = { 256 };	//rc.nThresholdValue - g_nRecogGrayMin
	const float* ranges2[1];
	float hranges2[2];
	hranges2[0] = 0;
	hranges2[1] = 255;
	ranges2[0] = hranges2;
	cv::calcHist(&matSrcRoi, 1, channels, Mat(), src_hist2, 1, histSize2, ranges2, true, false);
	int nCount = 0;
	int nArea = 0;
	for (int i = 0; i < 256; i++)
	{
//		TRACE("i = %d, val = %f\n", i, src_hist2.at<float>(i));
		nArea += src_hist2.at<float>(i);
		nCount += i * src_hist2.at<float>(i);
	}
	rc.fStandardMeanGray = (float)nCount / nArea;
//	rc.fStandardMeanGray = nCount / rc.fStandardArea;
#endif

#ifdef Test_Data
//	GetStandardValue(rc);
#endif
	return true;
}

void CMakeModelDlg::GetStandardValue(RECTINFO& rc)
{
	if (rc.eCPType == SN)
	{
		switch (rc.nSnVal)
		{
			case 0:	rc.fStandardValue = 295; rc.fStandardArea = 756; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 1:	rc.fStandardValue = 260; rc.fStandardArea = 798; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 2:	rc.fStandardValue = 320; rc.fStandardArea = 756; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 3:	rc.fStandardValue = 285; rc.fStandardArea = 756; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 4:	rc.fStandardValue = 285; rc.fStandardArea = 798; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 5:	rc.fStandardValue = 295; rc.fStandardArea = 320; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 6:	rc.fStandardValue = 315; rc.fStandardArea = 756; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 7:	rc.fStandardValue = 262; rc.fStandardArea = 798; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 8:	rc.fStandardValue = 340; rc.fStandardArea = 756; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
			case 9:	rc.fStandardValue = 290; rc.fStandardArea = 756; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;
		}
	}
	else if (rc.eCPType == OMR)
	{
		switch (rc.nAnswer)
		{
			case 0:	rc.fStandardValue = 282.980011; rc.fStandardArea = 1036.040039; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;	//A
			case 1:	rc.fStandardValue = 301.286652; rc.fStandardArea = 1032.920044; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;	//B
			case 2:	rc.fStandardValue = 280.253326; rc.fStandardArea = 1033.773315; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;	//C
			case 3:	rc.fStandardValue = 293.640015; rc.fStandardArea = 1036.533325; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;	//D
			case 4:	rc.fStandardValue = 277.673340; rc.fStandardArea = 1034.079956; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;	//E
			case 5:	rc.fStandardValue = 268.899994; rc.fStandardArea = 1030.706665; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;	//F
			case 6:	rc.fStandardValue = 291.893341; rc.fStandardArea = 1035.920044; rc.fStandardDensity = rc.fStandardValue / rc.fStandardArea; break;	//G
		}
	}
}

int CMakeModelDlg::GetRectsInArea(cv::Mat& matSrc, RECTINFO rc, int nMinW, int nMaxW, int nMinH, int nMaxH, int nFindContoursModel /*= CV_RETR_EXTERNAL*/)
{
	cv::Mat imgResult = matSrc;

	cv::cvtColor(imgResult, imgResult, CV_BGR2GRAY);
	cv::GaussianBlur(imgResult, imgResult, cv::Size(rc.nGaussKernel, rc.nGaussKernel), 0, 0);
	sharpenImage1(imgResult, imgResult);

	cv::threshold(imgResult, imgResult, rc.nThresholdValue, 255, cv::THRESH_OTSU | cv::THRESH_BINARY);

	cv::Canny(imgResult, imgResult, 0, rc.nCannyKernel, 5);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(rc.nDilateKernel, rc.nDilateKernel));	//Size(6, 6)	普通空白框可识别
	cv::dilate(imgResult, imgResult, element);

	IplImage ipl_img(imgResult);

	//the parm. for cvFindContours  
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;

	//提取轮廓  
	cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), nFindContoursModel, CV_CHAIN_APPROX_SIMPLE);

	cv::Rect rtMax;		//记录最大矩形，识别同步头时用来排除非同步头框
	bool bResult = false;
	std::vector<cv::Rect>RectCompList;

	for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
	{
		CvRect aRect = cvBoundingRect(contour, 0);
		cv::Rect rm = aRect;

		if (rm.width < nMinW || rm.height < nMinH || rm.width > nMaxW || rm.height > nMaxH)
		{
			//TRACE("过滤矩形:(%d,%d,%d,%d), 面积: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
			continue;
		}

		RectCompList.push_back(rm);
	}
	cvReleaseMemStorage(&storage);

	return RectCompList.size();
}

inline void CMakeModelDlg::GetThreshold(cv::Mat& matSrc, cv::Mat& matDst)
{
	int nRealThreshold = -1;
	switch (m_eCurCPType)
	{
	case Fix_CP: nRealThreshold = m_nFixVal; break;
	case H_HEAD: nRealThreshold = m_nHeadVal; break;
	case V_HEAD: nRealThreshold = m_nHeadVal; break;
	case PAGINATION: nRealThreshold = m_nPaginationVal; break;
	case ABMODEL: nRealThreshold = m_nABModelVal; break;
	case COURSE: nRealThreshold = m_nCourseVal; break;
	case QK_CP: nRealThreshold = m_nQK_CPVal; break;
	case WJ_CP: nRealThreshold = m_nWJ_CPVal; break;
	case CHARACTER_AREA:
		{
//			nRealThreshold = m_nCharacterThreshold; break;
			double dThread = threshold(matSrc, matDst, m_nCharacterThreshold, 255, THRESH_OTSU | THRESH_BINARY);
			TRACE("文字识别大津法处理阀值：%f\n", dThread);
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

	if (nRealThreshold > 0)
	{
	#ifdef USES_GETTHRESHOLD_ZTFB	//先计算ROI区域的均值u和标准差p，二值化的阀值取u + 2p，根据正态分布，理论上可以囊括95%以上的范围
		const int channels[1] = { 0 };
		const int histSize[1] = { nRealThreshold };
		float hranges[2] = { 0, nRealThreshold };
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
		int nThreshold = nRealThreshold;
		if (nCount > 0)
		{
			float fMean = (float)nSum / nCount;		//均值

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);	//标准差
			nThreshold = fMean + 2 * fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;
		}
		else
			nThreshold = nRealThreshold;
		if (nThreshold > nRealThreshold) nThreshold = nRealThreshold;
		threshold(matSrc, matDst, nThreshold, 255, THRESH_BINARY);
	#else
		threshold(matSrc, matDst, 60, 255, THRESH_BINARY);
	#endif
	}
}

bool CMakeModelDlg::Recognise(cv::Rect rtOri)
{
	clock_t start, end;
	start = clock();
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return false;
	
	//假坐标
	cv::Rect rtShowRect = GetShowFakePosRect(rtOri);
	Mat imgResult = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtShowRect);

	cvtColor(imgResult, imgResult, CV_BGR2GRAY);
	GaussianBlur(imgResult, imgResult, cv::Size(m_nGaussKernel, m_nGaussKernel), 0, 0);
	sharpenImage1(imgResult, imgResult);

	GetThreshold(imgResult, imgResult);

	cv::Canny(imgResult, imgResult, 0, m_nCannyKernel, 5);
	Mat element = getStructuringElement(MORPH_RECT, Size(m_nDilateKernel, m_nDilateKernel));	//Size(6, 6)	普通空白框可识别
	dilate(imgResult, imgResult, element);

	IplImage ipl_img(imgResult);

	//the parm. for cvFindContours  
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;

	//提取轮廓  
	cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	Rect rtMax;		//记录最大矩形，识别同步头时用来排除非同步头框
	bool bResult = false;
	std::vector<Rect>RectCompList;	//假坐标
	for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++)
	{
		CvRect aRect = cvBoundingRect(contour, 0);
		//假坐标
		Rect rm = aRect;

		if (rm.width < 10 || rm.height < 7 || rm.width > 90 || rm.height > 90 || rm.area() < 40 || rm.area() > 8100)
		{//10,7
			TRACE("过滤矩形:(%d,%d,%d,%d), 面积: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
			g_pLogger->information("过滤矩形:(%d,%d,%d,%d), 面积: %d", rm.x, rm.y, rm.width, rm.height, rm.area());
			continue;
		}

		rm.x = rm.x + rtShowRect.x;
		rm.y = rm.y + rtShowRect.y;

		RectCompList.push_back(rm);

		RECTINFO rc;
		rc.rt = GetSrcSaveRect(rm);
		rc.eCPType = m_eCurCPType;
		rc.nGaussKernel = m_nGaussKernel;
		rc.nSharpKernel = m_nSharpKernel;
		rc.nCannyKernel = m_nCannyKernel;
		rc.nDilateKernel = m_nDilateKernel;
		RECTINFO rcOri;
		rcOri.rt = rtOri;
		rcOri.nGaussKernel = m_nGaussKernel;
		rcOri.nSharpKernel = m_nSharpKernel;
		rcOri.nCannyKernel = m_nCannyKernel;
		rcOri.nDilateKernel = m_nDilateKernel;

		if (m_eCurCPType == H_HEAD)
		{
			rc.nThresholdValue = m_nHeadVal;
			rc.fStandardValuePercent = m_fHeadThresholdPercent;
			
			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			if (rm.area() > rtMax.area() && rc.fStandardDensity > 0.4)
				rtMax = GetSrcSaveRect(rm);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.push_back(rc);
		}
		else if (m_eCurCPType == V_HEAD)
		{
			rc.nThresholdValue = m_nHeadVal;
			rc.fStandardValuePercent = m_fHeadThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			if (rm.area() > rtMax.area() && rc.fStandardDensity > 0.4)
				rtMax = GetSrcSaveRect(rm);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.push_back(rc);
		}
		else if (m_eCurCPType == PAGINATION)
		{
			rc.nThresholdValue = m_nPaginationVal;
			rc.fStandardValuePercent = m_fPaginationThresholdPercent;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.push_back(rc);
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
			rc.fStandardValuePercent = m_fQK_CPThresholdPercent_Fix;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.push_back(rc);
		}
		else if (m_eCurCPType == WJ_CP)
		{
			rc.nThresholdValue = m_nWJ_CPVal;
			rc.fStandardValuePercent = m_fWJ_CPThresholdPercent_Fix;

			Rect rtTmp = rm;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rc);

			m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.push_back(rc);
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
		else if (m_eCurCPType == ELECT_OMR)
		{
			//选做题信息处理
		}

		bResult = true;
	}
	cvReleaseMemStorage(&storage);

	//二次过滤同步头
#if 1
	if (m_eCurCPType == H_HEAD)
	{
		int nDeviation = 10;
		std::vector<RECTINFO>::iterator itHead = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.begin();
		for (; itHead != m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.end();)
		{
			if ((float)itHead->rt.area() / rtMax.area() < 0.8 && ((itHead->rt.y < rtMax.y - nDeviation) || (itHead->rt.y > rtMax.y + rtMax.height + nDeviation)))
			{
				TRACE("1-当前矩形(%d,%d,%d,%d),面积%d,最大矩形(%d,%d,%d,%d),面积%d\n", itHead->rt.x, itHead->rt.y, itHead->rt.width, itHead->rt.height, itHead->rt.area(), rtMax.x, rtMax.y, rtMax.width, rtMax.height, rtMax.area());
				itHead = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.erase(itHead);
			}
			else if ((float)itHead->rt.area() / rtMax.area() < 0.1 || (itHead->rt.width > rtMax.width && itHead->rt.height < rtMax.height * 0.3) || (itHead->rt.height > rtMax.height && itHead->rt.width < rtMax.width * 0.3))
			{
				TRACE("2-当前矩形(%d,%d,%d,%d),面积%d,最大矩形(%d,%d,%d,%d),面积%d\n", itHead->rt.x, itHead->rt.y, itHead->rt.width, itHead->rt.height, itHead->rt.area(), rtMax.x, rtMax.y, rtMax.width, rtMax.height, rtMax.area());
				itHead = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.erase(itHead);
			}
			else if (itHead->fStandardDensity < 0.3)
			{
				TRACE("3-当前矩形(%d,%d,%d,%d),面积%d, 密度%.3f,最大矩形(%d,%d,%d,%d),面积%d\n", itHead->rt.x, itHead->rt.y, itHead->rt.width, itHead->rt.height, itHead->rt.area(), itHead->fStandardDensity, rtMax.x, rtMax.y, rtMax.width, rtMax.height, rtMax.area());
				itHead = m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.erase(itHead);
			}
			else
				itHead++;
		}
	}
	if (m_eCurCPType == V_HEAD)
	{
		int nDeviation = 10;
		std::vector<RECTINFO>::iterator itHead = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.begin();
		for (; itHead != m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.end();)
		{
			if ((float)itHead->rt.area() / rtMax.area() < 0.8 && ((itHead->rt.x < rtMax.x - nDeviation) || (itHead->rt.x > rtMax.x + rtMax.width + nDeviation)))
			{
				TRACE("1-当前矩形(%d,%d,%d,%d),面积%d,最大矩形(%d,%d,%d,%d),面积%d\n", itHead->rt.x, itHead->rt.y, itHead->rt.width, itHead->rt.height, itHead->rt.area(), rtMax.x, rtMax.y, rtMax.width, rtMax.height, rtMax.area());
				itHead = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.erase(itHead);
			}
			else if ((float)itHead->rt.area() / rtMax.area() < 0.1 || (itHead->rt.width > rtMax.width && itHead->rt.height < rtMax.height * 0.3) || (itHead->rt.height > rtMax.height && itHead->rt.width < rtMax.width * 0.3))
			{
				TRACE("2-当前矩形(%d,%d,%d,%d),面积%d,最大矩形(%d,%d,%d,%d),面积%d\n", itHead->rt.x, itHead->rt.y, itHead->rt.width, itHead->rt.height, itHead->rt.area(), rtMax.x, rtMax.y, rtMax.width, rtMax.height, rtMax.area());
				itHead = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.erase(itHead);
			}
			else if (itHead->fStandardDensity < 0.3)
			{
				TRACE("3-当前矩形(%d,%d,%d,%d),面积%d, 密度%.3f,最大矩形(%d,%d,%d,%d),面积%d\n", itHead->rt.x, itHead->rt.y, itHead->rt.width, itHead->rt.height, itHead->rt.area(), itHead->fStandardDensity, rtMax.x, rtMax.y, rtMax.width, rtMax.height, rtMax.area());
				itHead = m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.erase(itHead);
			}
			else
				itHead++;
		}
	}
#endif

	if (m_eCurCPType == SN)
	{
		GetSNArry(RectCompList);
	}
	if(m_eCurCPType == OMR)
	{
		GetOmrArry(RectCompList);
	}
	if(m_eCurCPType == ELECT_OMR)
	{
		GetElectOmrInfo(RectCompList);
	}

	if (m_eCurCPType == Fix_CP)
	{
		if (RectCompList.size() > 0)
		{
			std::sort(RectCompList.begin(), RectCompList.end(), SortByArea);
			
			RECTINFO rcFixSel;					//把定位点选择的矩形框存入，在扫描匹配的时候根据这个框来识别定点坐标
			rcFixSel.eCPType = m_eCurCPType;

			rcFixSel.nGaussKernel = m_nGaussKernel;
			rcFixSel.nSharpKernel = m_nSharpKernel;
			rcFixSel.nCannyKernel = m_nCannyKernel;
			rcFixSel.nDilateKernel = m_nDilateKernel;

			rcFixSel.rt = rtOri;
			RECTINFO rcFixRt;					//把定位点选择的矩形框存入，在扫描匹配的时候根据这个框来识别定点坐标
			rcFixRt.eCPType = m_eCurCPType;

			rcFixRt.nGaussKernel = m_nGaussKernel;
			rcFixRt.nSharpKernel = m_nSharpKernel;
			rcFixRt.nCannyKernel = m_nCannyKernel;
			rcFixRt.nDilateKernel = m_nDilateKernel;

			bool bFind = false;
			for (int i = 0; i < RectCompList.size(); i++)
			{
				rcFixRt.rt = GetSrcSaveRect(RectCompList[i]);

				Rect rtTmp = RectCompList[i];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				rcFixRt.nThresholdValue = m_nFixVal;
				rcFixRt.fStandardValuePercent = m_fFixThresholdPercent;
				RecogGrayValue(matSrcModel, rcFixRt);
				if (rcFixRt.fStandardDensity > 0.3)
				{
					bFind = true;
					break;
				}
			}
			if (!bFind)
				return bResult;

			if (m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size() < 4)
			{
				m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel.push_back(rcFixSel);
				m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.push_back(rcFixRt);
			}
			else
			{
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, "定点最多可设置4个");
				dlg.DoModal();
			}			
		}
	}
	end = clock();
	//	PaintRecognisedRect();
	TRACE("****** 7\n");
	if (m_eCurCPType != OMR && m_eCurCPType != SN)
		ShowRectByCPType(m_eCurCPType);

//	end = clock();
	char szLog[200] = { 0 };
	sprintf_s(szLog, "Recognise time: %d, find rect counts: %d.\n", end - start, RectCompList.size());
	g_pLogger->information(szLog);
	TRACE(szLog);
	return bResult;
}

bool CMakeModelDlg::RecogCharacterArea(cv::Rect rtOri)
{
	bool bResult = true;
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return false;

	//假坐标
	cv::Rect rtShowRect = GetShowFakePosRect(rtOri);
	Mat imgResult = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtShowRect);
	cvtColor(imgResult, imgResult, CV_BGR2GRAY);
	GaussianBlur(imgResult, imgResult, cv::Size(m_nGaussKernel, m_nGaussKernel), 0, 0);
	sharpenImage1(imgResult, imgResult);

	GetThreshold(imgResult, imgResult);

// 	Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
// 	erode(imgResult, imgResult, element);

#ifdef USE_TESSERACT
	#if 0
	//旋转水平，防歪斜

	//计算水平和垂直投影
	if (rtOri.width >= rtOri.height)	//水平的
	{
		//先水平投影，
	}
	else
	{
	}
	#endif
	//第一次识别结果
	m_pTess->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);

	m_pTess->SetVariable("tessedit_char_whitelist", "");
	m_pTess->SetImage((uchar*)imgResult.data, imgResult.cols, imgResult.rows, 1, imgResult.cols);
	char* out = m_pTess->GetUTF8Text();
	std::string strRecogVal1 = CMyCodeConvert::Utf8ToGb2312(out);
	if (strRecogVal1.empty())
		return false;
	USES_CONVERSION;
	CRecogCharacterDlg dlg(A2T(strRecogVal1.c_str()));
	if (dlg.DoModal() != IDOK)
		return false;

	clock_t start, end;
	start = clock();
//	m_pTess->SetImage((uchar*)imgResult.data, imgResult.cols, imgResult.rows, 1, imgResult.cols);
	 
	std::string strWhiteList = T2A(dlg.m_strVal);
	m_pTess->SetVariable("tessedit_char_whitelist", CMyCodeConvert::Gb2312ToUtf8(strWhiteList).c_str());

	m_pTess->Recognize(0);
	end = clock();
	TRACE("识别文字时间: %d\n", end - start);

	Mat imgSrc	= m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	std::string strOut;
	tesseract::ResultIterator* ri = m_pTess->GetIterator();
	tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;	//RIL_WORD
	if (ri != 0)
	{
		int nIndex = 1;

		pST_CHARACTER_ANCHOR_AREA pstRecogCharacterRt = new ST_CHARACTER_ANCHOR_AREA();
		int j = 0;
		for (j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); j++)
			if (m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[j]->nIndex > j + 1)
				break;
		pstRecogCharacterRt->nIndex = j + 1;
		pstRecogCharacterRt->nThresholdValue = m_nCharacterThreshold;
		pstRecogCharacterRt->nGaussKernel = m_nGaussKernel;
		pstRecogCharacterRt->nSharpKernel = m_nSharpKernel;
		pstRecogCharacterRt->nCannyKernel = m_nCannyKernel;
		pstRecogCharacterRt->nDilateKernel = m_nDilateKernel;
		pstRecogCharacterRt->nCharacterConfidence = m_nCharacterConfidence;
		pstRecogCharacterRt->rt = rtOri;

		//重复字临时登记列表，后面删除所有重复的字
		std::vector<std::string> vecRepeatWord;
		do
		{
			const char* word = ri->GetUTF8Text(level);
			float conf = ri->Confidence(level);
			if (word && strcmp(word, " ") != 0 && conf >= pstRecogCharacterRt->nCharacterConfidence)
			{
				int x1, y1, x2, y2;
				ri->BoundingBox(level, &x1, &y1, &x2, &y2);
				Point start, end;
				start.x = rtShowRect.x + x1;
				start.y = rtShowRect.y + y1;
				end.x = rtShowRect.x + x2;
				end.y = rtShowRect.y + y2;
				Rect rtSrc(start, end);		//假坐标

				pST_CHARACTER_ANCHOR_POINT pstCharRt = new ST_CHARACTER_ANCHOR_POINT();
				pstCharRt->nIndex = nIndex;
				pstCharRt->fConfidence = conf;
				pstCharRt->rc.eCPType = CHARACTER_AREA;
				pstCharRt->rc.rt = GetSrcSaveRect(rtSrc);
				pstCharRt->rc.nTH = pstRecogCharacterRt->nIndex;	//记录下当前文字属于第几个大文字识别区
				pstCharRt->rc.nAnswer = nIndex;						//记录下当前文字属于当前文字识别区中的第几个识别的文字

				pstCharRt->rc.nThresholdValue = m_nCharacterThreshold;
				pstCharRt->rc.nGaussKernel = m_nGaussKernel;
				pstCharRt->rc.nSharpKernel = m_nSharpKernel;
				pstCharRt->rc.nCannyKernel = m_nCannyKernel;
				pstCharRt->rc.nDilateKernel = m_nDilateKernel;
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtSrc);
				RecogGrayValue(matSrcModel, pstCharRt->rc);

				pstCharRt->strVal = CMyCodeConvert::Utf8ToGb2312(word);

				//**********	需要删除所有重复的字，保证识别的文字没有重复字
				for (auto item : pstRecogCharacterRt->vecCharacterRt)
					if (item->strVal == pstCharRt->strVal)
					{
						vecRepeatWord.push_back(item->strVal);
						break;
					}

				pstRecogCharacterRt->vecCharacterRt.push_back(pstCharRt);
				nIndex++;
			}
		} while (ri->Next(level));

		if (pstRecogCharacterRt->vecCharacterRt.size() > 0)
		{
			Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();

			//需要删除所有重复的字，保证识别的文字没有重复字
			for (int i = 0; i < vecRepeatWord.size(); i++)
			{
				std::vector<pST_CHARACTER_ANCHOR_POINT>::iterator itCharAnchorPoint = pstRecogCharacterRt->vecCharacterRt.begin();
				for (; itCharAnchorPoint != pstRecogCharacterRt->vecCharacterRt.end(); )
					if ((*itCharAnchorPoint)->strVal == vecRepeatWord[i])
						itCharAnchorPoint = pstRecogCharacterRt->vecCharacterRt.erase(itCharAnchorPoint);
					else
						itCharAnchorPoint++;
			}
			cv::Rect rtTmp = GetShowFakePosRect(pstRecogCharacterRt->rt);
			cv::rectangle(imgSrc, rtTmp, CV_RGB(181, 115, 173), 2);
			cv::rectangle(tmp, rtTmp, CV_RGB(170, 215, 111), -1);
			for (int i = 0; i < pstRecogCharacterRt->vecCharacterRt.size(); i++)
			{
				cv::Rect rt = GetShowFakePosRect(pstRecogCharacterRt->vecCharacterRt[i]->rc.rt);
				cv::rectangle(imgSrc, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp, rt, CV_RGB(168, 86, 157), -1);
			}
			m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.push_back(pstRecogCharacterRt);
			std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.end(), SortByCharAnchorArea);

			cv::addWeighted(imgSrc, 0.5, tmp, 0.5, 0, imgSrc);

			//计算此文字识别区内的矩形数量，在进行图像方向判断时有用
			cv::Mat matCharactArea = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtShowRect);
			RECTINFO rcModelCharactArea = pstRecogCharacterRt->vecCharacterRt[0]->rc;
			rcModelCharactArea.rt = GetShowFakePosRect(pstRecogCharacterRt->rt);
			cv::Rect rtTmp2 = GetShowFakePosRect(pstRecogCharacterRt->vecCharacterRt[0]->rc.rt); //假坐标
			int nMinW = rtTmp2.width * 0.7;
			int nMaxW = rtTmp2.width * 1.3;
			int nMinH = rtTmp2.height * 0.7;
			int nMaxH = rtTmp2.height * 1.3;
			pstRecogCharacterRt->nRects = GetRectsInArea(matCharactArea, rcModelCharactArea, nMinW, nMaxW, nMinH, nMaxH);
		}
		else
			SAFE_RELEASE(pstRecogCharacterRt);
	}
	m_pModelPicShow->ShowPic(imgSrc);
#endif
	return bResult;
}

bool CMakeModelDlg::RecogWhiteAreaGray(cv::Rect rtOri)
{
	//假坐标
	cv::Rect rtShowRect = GetShowFakePosRect(rtOri);

	RECTINFO rc;
	rc.rt = rtOri;
	rc.eCPType = m_eCurCPType;
	rc.nGaussKernel = m_nGaussKernel;
	rc.nSharpKernel = m_nSharpKernel;
	rc.nCannyKernel = m_nCannyKernel;
	rc.nDilateKernel = m_nDilateKernel;

	rc.nThresholdValue = 220;

	Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtShowRect);
	RecogGrayValue(matSrcModel, rc);

	m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.push_back(rc);
	return true;
}

void CMakeModelDlg::horizontalProjectionMat(cv::Mat srcImg, VEC_PROJECT& vecResult)
{
	Mat binImg;
	blur(srcImg, binImg, Size(3, 3));
	threshold(binImg, binImg, 0, 255, CV_THRESH_OTSU);
	int perPixelValue = 0;//每个像素的值  
	int width = srcImg.cols;
	int height = srcImg.rows;
	int* projectValArry = new int[height];//创建一个储存每行白色像素个数的数组  
	memset(projectValArry, 0, height * 4);//初始化数组  
	for (int col = 0; col < height; col++)//遍历每个像素点  
	{
		for (int row = 0; row < width; row++)
		{
			perPixelValue = binImg.at<uchar>(col, row);
			if (perPixelValue == 0)//如果是白底黑字  
			{
				projectValArry[col]++;
			}
		}
	}

	//定义一个全255矩阵，全白
// 	Mat horizontalProjectionMat(height, width, CV_8UC1, Scalar(255));//创建画布  
// 	for (int i = 0; i < height; i++)//水平直方图  
// 	{
// 		for (int j = 0; j < projectValArry[i]; j++)
// 		{
// 			perPixelValue = 0;
// 			horizontalProjectionMat.at<uchar>(i, width - 1 - j) = perPixelValue;//设置直方图为黑色  
// 		}
// 	}

	int startIndex = 0;//记录进入字符区的索引  
	int endIndex = 0;//记录进入空白区域的索引  
	bool inBlock = false;//是否遍历到了字符区内  
	int nArea = 0;		//字符区域的面积，即黑点的数量
	for (int i = 0; i < srcImg.rows; i++)
	{
		if (!inBlock && projectValArry[i] != 0)//进入字符区  
		{
			inBlock = true;
			startIndex = i;
			nArea = 0;
		}
		else if (inBlock && projectValArry[i] == 0)//进入空白区  
		{
			endIndex = i;
			inBlock = false;
//			Mat roiImg = srcImg(Range(startIndex, endIndex + 1), Range(0, srcImg.cols));//从原图中截取有图像的区域  
			ST_PROJECTION stProjection;
			stProjection.nStartIndex = startIndex;
			stProjection.nEndIndex = endIndex;
			stProjection.nArea = nArea;
//			stProjection.matProjection = roiImg;
			vecResult.push_back(stProjection);
		}
		else if (inBlock && projectValArry[i] > 0)
		{
			nArea += projectValArry[i];
		}
	}
	if (inBlock)
	{
		endIndex = srcImg.rows;
		inBlock = false;
//		Mat roiImg = srcImg(Range(startIndex, endIndex), Range(0, srcImg.cols));//从原图中截取有图像的区域  
		ST_PROJECTION stProjection;
		stProjection.nStartIndex = startIndex;
		stProjection.nEndIndex = endIndex;
		stProjection.nArea = nArea;
//		stProjection.matProjection = roiImg;
		vecResult.push_back(stProjection);
	}
	delete[] projectValArry;
}

void CMakeModelDlg::verticalProjectionMat(cv::Mat srcImg, VEC_PROJECT& vecResult)
{
	cv::Mat binImg;
	cv::blur(srcImg, binImg, Size(3, 3));
	threshold(binImg, binImg, 0, 255, CV_THRESH_OTSU);
	int perPixelValue;//每个像素的值  
	int width = srcImg.cols;
	int height = srcImg.rows;
	int* projectValArry = new int[width];//创建用于储存每列白色像素个数的数组  
	memset(projectValArry, 0, width * 4);//初始化数组  
	for (int col = 0; col < width; col++)
	{
		for (int row = 0; row < height; row++)
		{
			perPixelValue = binImg.at<uchar>(row, col);
			if (perPixelValue == 0)//如果是白底黑字  
			{
				projectValArry[col]++;
			}
		}
	}
	
	//定义一个全255矩阵，全白
// 	cv::Mat verticalProjectionMat(height, width, CV_8UC1, cv::Scalar(255));//垂直投影的画布
// 	for (int i = 0; i < width; i++)//垂直投影直方图  
// 	{
// 		for (int j = 0; j < projectValArry[i]; j++)
// 		{
// 			perPixelValue = 0;  //直方图设置为黑色    
// 			verticalProjectionMat.at<uchar>(height - 1 - j, i) = perPixelValue;
// 		}
// 	}

	int startIndex = 0;//记录进入字符区的索引  
	int endIndex = 0;//记录进入空白区域的索引  
	bool inBlock = false;//是否遍历到了字符区内  
	int nArea = 0;		//字符区域的面积，即黑点的数量
	for (int i = 0; i < srcImg.cols; i++)//cols=width  
	{
		if (!inBlock && projectValArry[i] != 0)//进入字符区  
		{
			inBlock = true;
			startIndex = i;
			nArea = 0;
		}
		else if (projectValArry[i] == 0 && inBlock)//进入空白区  
		{
			endIndex = i;
			inBlock = false;
			cv::Mat roiImg = srcImg(cv::Range(0, srcImg.rows), cv::Range(startIndex, endIndex + 1));
			ST_PROJECTION stProjection;
			stProjection.nStartIndex = startIndex;
			stProjection.nEndIndex	= endIndex;
			stProjection.nArea		= nArea;
//			stProjection.matProjection = roiImg;
			vecResult.push_back(stProjection);
		}
		else if (inBlock && projectValArry[i] > 0)
		{
			nArea += projectValArry[i];
		}
	}

	if (inBlock)
	{
		endIndex = srcImg.cols;
		inBlock = false;
		cv::Mat roiImg = srcImg(cv::Range(0, srcImg.rows), cv::Range(startIndex, endIndex));
		ST_PROJECTION stProjection;
		stProjection.nStartIndex = startIndex;
		stProjection.nEndIndex = endIndex;
		stProjection.nArea = nArea;
//		stProjection.matProjection = roiImg;
		vecResult.push_back(stProjection);
	}
	delete[] projectValArry;
}

bool CMakeModelDlg::RecogByHead(cv::Rect rtOri)
{
	if (!m_pModel->nHasHead)
		return false;
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return false;
	if (!m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size())
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "请先设置水平同步头");
		dlg.DoModal();
		return false;
	}
	if (!m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size())
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "请先设置垂直同步头");
		dlg.DoModal();
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

	if (m_eCurCPType == ELECT_OMR)
	{
		if (nPosV_E - nPosV_B > 0 && nPosH_E - nPosH_B > 0)
		{
//			AfxMessageBox(_T("选择区域不合法，请重新选择！"));
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "选择区域不合法，请重新选择！");
			dlg.DoModal();
			return false;
		}
		if (!m_pElectOmrDlg->m_pCurrentGroup)
		{
//			AfxMessageBox(_T("当前选做题信息不存在，请新建!"));
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "当前选做题信息不存在，请新建!");
			dlg.DoModal();
			return false;
		}
		if (!m_pElectOmrDlg->checkValid())
		{
//			AfxMessageBox(_T("当前选做题信息未保存，请先保存此题选做题信息!"));
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "当前选做题信息未保存，请先保存!");
			dlg.DoModal();
			return false;
		}
		if (m_pElectOmrDlg->m_pCurrentGroup->nAllCount < (nPosV_E - nPosV_B + 1)*(nPosH_E - nPosH_B + 1))
		{
//			AfxMessageBox(_T("识别出的选项数超出范围!"));
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "识别出的选项数超出范围");
			dlg.DoModal();
			return false;
		}
	}

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

			rc.nGaussKernel = m_nGaussKernel;
			rc.nSharpKernel = m_nSharpKernel;
			rc.nCannyKernel = m_nCannyKernel;
			rc.nDilateKernel = m_nDilateKernel;
#if 1
			if (m_eCurCPType == PAGINATION)
			{
				rc.nThresholdValue = m_nPaginationVal;
				rc.fStandardValuePercent = m_fPaginationThresholdPercent;

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == ABMODEL)
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
				rc.fStandardValuePercent = m_fQK_CPThresholdPercent_Head;

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == WJ_CP)
			{
				rc.nThresholdValue = m_nWJ_CPVal;
				rc.fStandardValuePercent = m_fWJ_CPThresholdPercent_Head;

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
				rc.fStandardValuePercent = m_fSNThresholdPercent_Head;

				switch (GetSnSavePosFlag(m_pSNInfoDlg->m_nCurrentSNVal))
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
				rc.nRecogFlag = GetSnSavePosFlag(m_pSNInfoDlg->m_nCurrentSNVal);

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == OMR)
			{
				rc.nThresholdValue = m_nOMR;
				rc.fStandardValuePercent = m_fOMRThresholdPercent_Head;

				switch (GetOmrSavePosFlag(m_pOmrInfoDlg->m_nCurrentOmrVal))
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
				if (m_pOmrInfoDlg->m_nSingle == 0)
					rc.nSingle = 0;
				else if (m_pOmrInfoDlg->m_nSingle == 1)
					rc.nSingle = 1;
				else
					rc.nSingle = 2;
				rc.nRecogFlag = GetOmrSavePosFlag(m_pOmrInfoDlg->m_nCurrentOmrVal);

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
			else if (m_eCurCPType == ELECT_OMR)
			{
				rc.nThresholdValue = m_nOMR;
				rc.fStandardValuePercent = m_fOMRThresholdPercent_Head;

				if (nPosV_E - nPosV_B == 0)		//垂直同步头一样，说明是水平排列的
				{
					rc.nTH = m_pElectOmrDlg->m_pCurrentGroup->nGroupID;
					rc.nAnswer = j;
				}
				else         //水平同步头一样，说明是垂直排列的
				{
					rc.nTH = m_pElectOmrDlg->m_pCurrentGroup->nGroupID;
					rc.nAnswer = i;
				}

				Rect rtTmp = arr[i][j];
				Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
				RecogGrayValue(matSrcModel, rc);

				m_vecTmp.push_back(rc);
			}
#endif
		}
	}
	if (m_eCurCPType == SN || m_eCurCPType == OMR || m_eCurCPType == ELECT_OMR)
		std::sort(m_vecTmp.begin(), m_vecTmp.end(), SortByTH);


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
			int nCount = 0;
			if (m_pModel->nUsePagination)
			{
				nCount = m_vecPaperModelInfo[i]->vecPagination.size() + m_vecPaperModelInfo[i]->vecABModel.size() + m_vecPaperModelInfo[i]->vecCourse.size() + m_vecPaperModelInfo[i]->vecElectOmr.size() + m_vecPaperModelInfo[i]->vecGray.size() \
					+ m_vecPaperModelInfo[i]->vecOmr2.size() + m_vecPaperModelInfo[i]->vecQK_CP.size() + m_vecPaperModelInfo[i]->vecWhite.size() + m_vecPaperModelInfo[i]->lSN.size();
			}
			else
			{
				nCount = m_vecPaperModelInfo[i]->vecABModel.size() + m_vecPaperModelInfo[i]->vecCourse.size() + m_vecPaperModelInfo[i]->vecElectOmr.size() + m_vecPaperModelInfo[i]->vecGray.size() \
					+ m_vecPaperModelInfo[i]->vecOmr2.size() + m_vecPaperModelInfo[i]->vecQK_CP.size() + m_vecPaperModelInfo[i]->vecWhite.size() + m_vecPaperModelInfo[i]->lSN.size();
			}
			if (m_pModel->nUseWordAnchorPoint == 0)
			{
				if (nCount > 0 && m_vecPaperModelInfo[i]->vecRtFix.size() < 3)
				{
					char szTmp[50] = { 0 };
					sprintf_s(szTmp, "第 %d 页定点设置数量太少，建议设置4个", i + 1);
					CNewMessageBox dlg;
					dlg.setShowInfo(2, 1, szTmp);
					dlg.DoModal();
					bResult = false;
					break;
				}
			}
			else
			{
				int nWordCount = 0;
				for (auto item : m_vecPaperModelInfo[i]->vecCharacterLocation)
					nWordCount += item->vecCharacterRt.size();

				if (nCount > 0 && (m_vecPaperModelInfo[i]->vecCharacterLocation.size() <= 0 || nWordCount < m_pModel->nCharacterAnchorPoint))
				{
					char szTmp[100] = { 0 };
					sprintf_s(szTmp, "第 %d 页文字识别点数量太少，至少需要识别到%d个字", i + 1, m_pModel->nCharacterAnchorPoint);
					CNewMessageBox dlg;
					dlg.setShowInfo(2, 1, szTmp);
					dlg.DoModal();
					bResult = false;
					break;
				}
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

	if (m_pModel->nUsePagination && bResult)
	{
		//页码检查
		for (int i = 0; i < m_vecPaperModelInfo.size(); i++)
		{
			if (m_vecPaperModelInfo[i]->vecPagination.size() <= 0)
			{
				char szTmp[50] = { 0 };
				sprintf_s(szTmp, "第 %d 页未设置\"页码标识\"", i + 1);
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, szTmp);
				dlg.DoModal();
				bResult = false;
			}
			//准考证号检查
			if (i % 2 == 0 && m_vecPaperModelInfo[i]->lSN.size() == 0)	//奇数页检查准考证号是否设置，默认是双面扫描，背面不需要检查准考证号
			{
				char szTmp[50] = { 0 };
				sprintf_s(szTmp, "第 %d 页准考证号未设置", i + 1);
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, szTmp);
				dlg.DoModal();
				bResult = false;
			}
		}
	}
	else if (m_pModel->nUsePagination == 0)
	{
		//准考证号检查
		if (m_vecPaperModelInfo[0]->lSN.size() == 0)
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "第1页准考证号未设置");
			dlg.DoModal();
			bResult = false;
		}
	}

	//omr漏点检查
	for (int i = 0; i < m_vecPaperModelInfo.size(); i++)
	{
		int nOmrQuestions = m_vecPaperModelInfo[i]->vecOmr2.size();
		for (int j = 0; j < nOmrQuestions; j++)
		{
			int nOmrItems = m_vecPaperModelInfo[i]->vecOmr2[j].lSelAnswer.size();
			RECTLIST::iterator itOmr = m_vecPaperModelInfo[i]->vecOmr2[j].lSelAnswer.begin();
			for (; itOmr != m_vecPaperModelInfo[i]->vecOmr2[j].lSelAnswer.end(); itOmr++)
			{
				if (itOmr->nAnswer > nOmrItems - 1)
				{
					char szTmp[100] = { 0 };
					sprintf_s(szTmp, "第 %d 页第 %d 题选项数为%d个,实际选项顺序超出，请检查!", i + 1, m_vecPaperModelInfo[i]->vecOmr2[j].nTH, nOmrItems);
					CNewMessageBox dlg;
					dlg.setShowInfo(2, 1, szTmp);
					dlg.DoModal();
					bResult = false;
					break;
				}
			}
			if(!bResult)
				break;
		}
		if (!bResult)
			break;
	}

	return bResult;
}

void CMakeModelDlg::OnBnClickedBtnSave()
{
	if (!m_pModel)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "请先创建模板");
		dlg.DoModal();
		return;
	}
	
	if (!checkValidity()) return;

	CModelSaveDlg dlg(m_pModel);
	if (dlg.DoModal() != IDOK)
		return;

	USES_CONVERSION;
	m_pModel->nSaveMode = dlg.m_nSaveMode;
	if (dlg.m_nSaveMode == 1)
	{
		m_pModel->strModelName = T2A(dlg.m_strModelName);
		m_pModel->strModelDesc = T2A(dlg.m_strLocalModelDesc);
	}
	else
	{
		char szModelName[150] = { 0 };
		sprintf_s(szModelName, "%s_%s_N_%d_%d", T2A(dlg.m_strExamName), T2A(dlg.m_strSubjectName), dlg.m_nExamID, dlg.m_SubjectID);
		char szModelDesc[300] = { 0 };
		sprintf_s(szModelDesc, "考试名称: %s\r\n科目: %s\r\n年级: %s\r\n考试类型名称: %s", T2A(dlg.m_strExamName), T2A(dlg.m_strSubjectName), T2A(dlg.m_strGradeName), T2A(dlg.m_strExamTypeName));
		m_pModel->nExamID		= dlg.m_nExamID;
		m_pModel->nSubjectID	= dlg.m_SubjectID;
		m_pModel->strModelName	= szModelName;
		m_pModel->strModelDesc	= szModelDesc;
	}

	CString strTitle = _T("");
	strTitle.Format(_T("模板名称: %s"), A2T(m_pModel->strModelName.c_str()));
	SetWindowText(strTitle);

	m_bSavedModelFlag = true;
	
	for (int i = 0; i < m_pModel->vecPaperModel.size(); i++)
	{
		pPAPERMODEL pPaperModel = m_pModel->vecPaperModel[i];
		SAFE_RELEASE(pPaperModel);
	}
	m_pModel->vecPaperModel.clear();

	m_pModel->nZkzhType = m_pSNInfoDlg->m_nZkzhType;
	bool bHasElectOmr = false;

	m_pModel->nPicNum = m_vecPaperModelInfo.size();
	for (int i = 0; i < m_pModel->nPicNum; i++)
	{
		pPAPERMODEL pPaperModel = new PAPERMODEL;
		pPaperModel->strModelPicName = m_vecPaperModelInfo[i]->strModelPicName;
		
		//++同步头模式时，添加4个定点
		RecogFixWithHead(i);
		//--

		for (int j = 0; j < m_vecPaperModelInfo[i]->vecCharacterLocation.size(); j++)
			pPaperModel->lCharacterAnchorArea.push_back(m_vecPaperModelInfo[i]->vecCharacterLocation[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecHTracker.size(); j++)
			pPaperModel->lSelHTracker.push_back(m_vecPaperModelInfo[i]->vecHTracker[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecVTracker.size(); j++)
			pPaperModel->lSelVTracker.push_back(m_vecPaperModelInfo[i]->vecVTracker[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecRtSel.size(); j++)
			pPaperModel->lSelFixRoi.push_back(m_vecPaperModelInfo[i]->vecRtSel[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecRtFix.size(); j++)
			pPaperModel->lFix.push_back(m_vecPaperModelInfo[i]->vecRtFix[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecH_Head.size(); j++)
			pPaperModel->lH_Head.push_back(m_vecPaperModelInfo[i]->vecH_Head[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecV_Head.size(); j++)
			pPaperModel->lV_Head.push_back(m_vecPaperModelInfo[i]->vecV_Head[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecPagination.size(); j++)
			pPaperModel->lPagination.push_back(m_vecPaperModelInfo[i]->vecPagination[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecABModel.size(); j++)
			pPaperModel->lABModel.push_back(m_vecPaperModelInfo[i]->vecABModel[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecCourse.size(); j++)
			pPaperModel->lCourse.push_back(m_vecPaperModelInfo[i]->vecCourse[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecQK_CP.size(); j++)
			pPaperModel->lQK_CP.push_back(m_vecPaperModelInfo[i]->vecQK_CP[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecWJ_CP.size(); j++)
			pPaperModel->lWJ_CP.push_back(m_vecPaperModelInfo[i]->vecWJ_CP[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecGray.size(); j++)
			pPaperModel->lGray.push_back(m_vecPaperModelInfo[i]->vecGray[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecWhite.size(); j++)
			pPaperModel->lWhite.push_back(m_vecPaperModelInfo[i]->vecWhite[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecOmr2.size(); j++)
			pPaperModel->lOMR2.push_back(m_vecPaperModelInfo[i]->vecOmr2[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecElectOmr.size(); j++)
		{
			pPaperModel->lElectOmr.push_back(m_vecPaperModelInfo[i]->vecElectOmr[j]);
			bHasElectOmr = true;
		}
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
			m_ptHTracker2 = cv::Point(m_vecPaperModelInfo[i]->matDstImg.cols, 90);	//m_vecPaperModelInfo[i]->matSrcImg.cols
//			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH = false;

			m_vecPaperModelInfo[i]->rtHTracker.x = m_ptHTracker1.x;
			m_vecPaperModelInfo[i]->rtHTracker.y = m_ptHTracker1.y;
			m_vecPaperModelInfo[i]->rtHTracker.width = m_ptHTracker2.x - m_ptHTracker1.x;
			m_vecPaperModelInfo[i]->rtHTracker.height = m_ptHTracker2.y - m_ptHTracker1.y;
		}
		if (m_pModel->nHasHead && m_vecPaperModelInfo[i]->bFirstV && m_pModel->nType == 0)
		{
			m_ptVTracker1 = cv::Point(m_vecPaperModelInfo[i]->matDstImg.cols - 90, 0);	//m_vecPaperModelInfo[i]->matSrcImg.cols
			m_ptVTracker2 = cv::Point(m_vecPaperModelInfo[i]->matDstImg.cols, m_vecPaperModelInfo[i]->matDstImg.rows);	//m_vecPaperModelInfo[i]->matSrcImg.cols  m_vecPaperModelInfo[i]->matSrcImg.rows
//			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV = false;

			m_vecPaperModelInfo[i]->rtVTracker.x = m_ptVTracker1.x;
			m_vecPaperModelInfo[i]->rtVTracker.y = m_ptVTracker1.y;
			m_vecPaperModelInfo[i]->rtVTracker.width = m_ptVTracker2.x - m_ptVTracker1.x;
			m_vecPaperModelInfo[i]->rtVTracker.height = m_ptVTracker2.y - m_ptVTracker1.y;
		}
		//--
		pPaperModel->rtHTracker = m_vecPaperModelInfo[i]->rtHTracker;
		pPaperModel->rtVTracker = m_vecPaperModelInfo[i]->rtVTracker;
//		pPaperModel->rtSNTracker = m_vecPaperModelInfo[i]->rtSNTracker;
		pPaperModel->rcSNTracker = m_vecPaperModelInfo[i]->rcSNTracker;

		pPaperModel->nPicW = m_vecPaperModelInfo[i]->nPicW;
		pPaperModel->nPicH = m_vecPaperModelInfo[i]->nPicH;

		m_pModel->vecPaperModel.push_back(pPaperModel);
	}

	if (!bHasElectOmr)
		m_pModel->nHasElectOmr = 0;

	SetCursor(LoadCursor(NULL, IDC_WAIT));
	
	CString modelPath = g_strCurrentPath + _T("Model");
	modelPath = modelPath + _T("\\") + A2T(m_pModel->strModelName.c_str());
	if (SaveModelFile(m_pModel))
	{
//		ZipFile(modelPath, modelPath, _T(".mod"));
		CZipObj zipObj;
		zipObj.setLogger(g_pLogger);
		zipObj.ZipFile(modelPath, modelPath, _T(".mod"));

		//直接上传模板
		CString strModelFullPath = modelPath + _T(".mod");
		UploadModel(strModelFullPath, m_pModel);
//		AfxMessageBox(_T("保存完成!"));
		CNewMessageBox dlg;
		dlg.setShowInfo(3, 1, "保存完成!");
		dlg.DoModal();
	}
	else
	{
//		AfxMessageBox(_T("保存失败"));
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "保存失败");
		dlg.DoModal();
	}
	
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}

bool CMakeModelDlg::SaveModelFile(pMODEL pModel)
{
	USES_CONVERSION;
	CString modelPath = g_strCurrentPath + _T("Model");
	DWORD dwAttr = GetFileAttributesA(T2A(modelPath));
	if (dwAttr == 0xFFFFFFFF)
	{
		CreateDirectoryA(T2A(modelPath), NULL);
		{
			std::string strLog = "创建文件夹失败: " + std::string(T2A(modelPath));
			g_pLogger->information(strLog);
			return false;
		}
	}
	modelPath = modelPath + _T("\\") + A2T(pModel->strModelName.c_str());
	dwAttr = GetFileAttributesA(T2A(modelPath));
	if (dwAttr == 0xFFFFFFFF)
	{
		dwAttr = CreateDirectoryA(T2A(modelPath), NULL);
		if (!dwAttr)
		{
			std::string strLog = "创建文件夹失败: " + std::string(T2A(modelPath));
			g_pLogger->information(strLog);
			return false;
		}
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

		CString strPicName = A2T(pModel->vecPaperModel[i]->strModelPicName.c_str());

		Poco::JSON::Array jsnSNArry;
		Poco::JSON::Array jsnSelHTrackerArry;
		Poco::JSON::Array jsnSelVTrackerArry;
		Poco::JSON::Array jsnSelRoiArry;
		Poco::JSON::Array jsnFixCPArry;
		Poco::JSON::Array jsnHHeadArry;
		Poco::JSON::Array jsnVHeadArry;
		Poco::JSON::Array jsnPaginationArry;
		Poco::JSON::Array jsnABModelArry;
		Poco::JSON::Array jsnCourseArry;
		Poco::JSON::Array jsnQKArry;
		Poco::JSON::Array jsnWJArry;
		Poco::JSON::Array jsnGrayCPArry;
		Poco::JSON::Array jsnWhiteCPArry;
		Poco::JSON::Array jsnOMRArry;
		Poco::JSON::Array jsnElectOmrArry;
		Poco::JSON::Array jsnCharacterAnchorAreaArry;
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
			jsnObj.set("standardArea", itFix->fStandardArea);
			jsnObj.set("standardDensity", itFix->fStandardDensity);
			jsnObj.set("standardMeanGray", itFix->fStandardMeanGray);
			jsnObj.set("standardStddev", itFix->fStandardStddev);

			jsnObj.set("gaussKernel", itFix->nGaussKernel);
			jsnObj.set("sharpKernel", itFix->nSharpKernel);
			jsnObj.set("cannyKernel", itFix->nCannyKernel);
			jsnObj.set("dilateKernel", itFix->nDilateKernel);
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
			jsnObj.set("standardArea", itHHead->fStandardArea);
			jsnObj.set("standardDensity", itHHead->fStandardDensity);
			jsnObj.set("standardMeanGray", itHHead->fStandardMeanGray);
			jsnObj.set("standardStddev", itHHead->fStandardStddev);

			jsnObj.set("gaussKernel", itHHead->nGaussKernel);
			jsnObj.set("sharpKernel", itHHead->nSharpKernel);
			jsnObj.set("cannyKernel", itHHead->nCannyKernel);
			jsnObj.set("dilateKernel", itHHead->nDilateKernel);
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
			jsnObj.set("standardArea", itVHead->fStandardArea);
			jsnObj.set("standardDensity", itVHead->fStandardDensity);
			jsnObj.set("standardMeanGray", itVHead->fStandardMeanGray);
			jsnObj.set("standardStddev", itVHead->fStandardStddev);

			jsnObj.set("gaussKernel", itVHead->nGaussKernel);
			jsnObj.set("sharpKernel", itVHead->nSharpKernel);
			jsnObj.set("cannyKernel", itVHead->nCannyKernel);
			jsnObj.set("dilateKernel", itVHead->nDilateKernel);
			jsnVHeadArry.add(jsnObj);
		}
		RECTLIST::iterator itPage = pModel->vecPaperModel[i]->lPagination.begin();
		for (; itPage != pModel->vecPaperModel[i]->lPagination.end(); itPage++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itPage->eCPType);
			jsnObj.set("left", itPage->rt.x);
			jsnObj.set("top", itPage->rt.y);
			jsnObj.set("width", itPage->rt.width);
			jsnObj.set("height", itPage->rt.height);
			jsnObj.set("hHeadItem", itPage->nHItem);
			jsnObj.set("vHeadItem", itPage->nVItem);
			jsnObj.set("thresholdValue", itPage->nThresholdValue);
			jsnObj.set("standardValPercent", itPage->fStandardValuePercent);
			jsnObj.set("standardVal", itPage->fStandardValue);
			jsnObj.set("standardArea", itPage->fStandardArea);
			jsnObj.set("standardDensity", itPage->fStandardDensity);
			jsnObj.set("standardMeanGray", itPage->fStandardMeanGray);
			jsnObj.set("standardStddev", itPage->fStandardStddev);

			jsnObj.set("gaussKernel", itPage->nGaussKernel);
			jsnObj.set("sharpKernel", itPage->nSharpKernel);
			jsnObj.set("cannyKernel", itPage->nCannyKernel);
			jsnObj.set("dilateKernel", itPage->nDilateKernel);
			jsnPaginationArry.add(jsnObj);
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
			jsnObj.set("standardArea", itABModel->fStandardArea);
			jsnObj.set("standardDensity", itABModel->fStandardDensity);
			jsnObj.set("standardMeanGray", itABModel->fStandardMeanGray);
			jsnObj.set("standardStddev", itABModel->fStandardStddev);

			jsnObj.set("gaussKernel", itABModel->nGaussKernel);
			jsnObj.set("sharpKernel", itABModel->nSharpKernel);
			jsnObj.set("cannyKernel", itABModel->nCannyKernel);
			jsnObj.set("dilateKernel", itABModel->nDilateKernel);
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
			jsnObj.set("standardArea", itCourse->fStandardArea);
			jsnObj.set("standardDensity", itCourse->fStandardDensity);
			jsnObj.set("standardMeanGray", itCourse->fStandardMeanGray);
			jsnObj.set("standardStddev", itCourse->fStandardStddev);

			jsnObj.set("gaussKernel", itCourse->nGaussKernel);
			jsnObj.set("sharpKernel", itCourse->nSharpKernel);
			jsnObj.set("cannyKernel", itCourse->nCannyKernel);
			jsnObj.set("dilateKernel", itCourse->nDilateKernel);
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
			jsnObj.set("standardArea", itQKCP->fStandardArea);
			jsnObj.set("standardDensity", itQKCP->fStandardDensity);
			jsnObj.set("standardMeanGray", itQKCP->fStandardMeanGray);
			jsnObj.set("standardStddev", itQKCP->fStandardStddev);

			jsnObj.set("gaussKernel", itQKCP->nGaussKernel);
			jsnObj.set("sharpKernel", itQKCP->nSharpKernel);
			jsnObj.set("cannyKernel", itQKCP->nCannyKernel);
			jsnObj.set("dilateKernel", itQKCP->nDilateKernel);
			jsnQKArry.add(jsnObj);
		}
		RECTLIST::iterator itWJCP = pModel->vecPaperModel[i]->lWJ_CP.begin();
		for (; itWJCP != pModel->vecPaperModel[i]->lWJ_CP.end(); itWJCP++)
		{
			Poco::JSON::Object jsnObj;
			jsnObj.set("eType", (int)itWJCP->eCPType);
			jsnObj.set("left", itWJCP->rt.x);
			jsnObj.set("top", itWJCP->rt.y);
			jsnObj.set("width", itWJCP->rt.width);
			jsnObj.set("height", itWJCP->rt.height);
			jsnObj.set("hHeadItem", itWJCP->nHItem);
			jsnObj.set("vHeadItem", itWJCP->nVItem);
			jsnObj.set("thresholdValue", itWJCP->nThresholdValue);
			jsnObj.set("standardValPercent", itWJCP->fStandardValuePercent);
			jsnObj.set("standardVal", itWJCP->fStandardValue);
			jsnObj.set("standardArea", itWJCP->fStandardArea);
			jsnObj.set("standardDensity", itWJCP->fStandardDensity);
			jsnObj.set("standardMeanGray", itWJCP->fStandardMeanGray);
			jsnObj.set("standardStddev", itWJCP->fStandardStddev);

			jsnObj.set("gaussKernel", itWJCP->nGaussKernel);
			jsnObj.set("sharpKernel", itWJCP->nSharpKernel);
			jsnObj.set("cannyKernel", itWJCP->nCannyKernel);
			jsnObj.set("dilateKernel", itWJCP->nDilateKernel);
			jsnWJArry.add(jsnObj);
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
			jsnObj.set("standardArea", itGrayCP->fStandardArea);
			jsnObj.set("standardDensity", itGrayCP->fStandardDensity);
			jsnObj.set("standardMeanGray", itGrayCP->fStandardMeanGray);
			jsnObj.set("standardStddev", itGrayCP->fStandardStddev);

			jsnObj.set("gaussKernel", itGrayCP->nGaussKernel);
			jsnObj.set("sharpKernel", itGrayCP->nSharpKernel);
			jsnObj.set("cannyKernel", itGrayCP->nCannyKernel);
			jsnObj.set("dilateKernel", itGrayCP->nDilateKernel);
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
			jsnObj.set("standardArea", itWhiteCP->fStandardArea);
			jsnObj.set("standardDensity", itWhiteCP->fStandardDensity);
			jsnObj.set("standardMeanGray", itWhiteCP->fStandardMeanGray);
			jsnObj.set("standardStddev", itWhiteCP->fStandardStddev);

			jsnObj.set("gaussKernel", itWhiteCP->nGaussKernel);
			jsnObj.set("sharpKernel", itWhiteCP->nSharpKernel);
			jsnObj.set("cannyKernel", itWhiteCP->nCannyKernel);
			jsnObj.set("dilateKernel", itWhiteCP->nDilateKernel);
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

			jsnObj.set("gaussKernel", itSelRoi->nGaussKernel);
			jsnObj.set("sharpKernel", itSelRoi->nSharpKernel);
			jsnObj.set("cannyKernel", itSelRoi->nCannyKernel);
			jsnObj.set("dilateKernel", itSelRoi->nDilateKernel);
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

			jsnObj.set("gaussKernel", itSelHTracker->nGaussKernel);
			jsnObj.set("sharpKernel", itSelHTracker->nSharpKernel);
			jsnObj.set("cannyKernel", itSelHTracker->nCannyKernel);
			jsnObj.set("dilateKernel", itSelHTracker->nDilateKernel);
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

			jsnObj.set("gaussKernel", itSelVTracker->nGaussKernel);
			jsnObj.set("sharpKernel", itSelVTracker->nSharpKernel);
			jsnObj.set("cannyKernel", itSelVTracker->nCannyKernel);
			jsnObj.set("dilateKernel", itSelVTracker->nDilateKernel);
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
				jsnObj.set("standardArea", itOmrSel->fStandardArea);
				jsnObj.set("standardDensity", itOmrSel->fStandardDensity);
				jsnObj.set("standardMeanGray", itOmrSel->fStandardMeanGray);
				jsnObj.set("standardStddev", itOmrSel->fStandardStddev);

				jsnObj.set("gaussKernel", itOmrSel->nGaussKernel);
				jsnObj.set("sharpKernel", itOmrSel->nSharpKernel);
				jsnObj.set("cannyKernel", itOmrSel->nCannyKernel);
				jsnObj.set("dilateKernel", itOmrSel->nDilateKernel);
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
				jsnObj.set("standardArea", itSnDetail->fStandardArea);
				jsnObj.set("standardDensity", itSnDetail->fStandardDensity);
				jsnObj.set("standardMeanGray", itSnDetail->fStandardMeanGray);
				jsnObj.set("standardStddev", itSnDetail->fStandardStddev);

				jsnObj.set("gaussKernel", itSnDetail->nGaussKernel);
				jsnObj.set("sharpKernel", itSnDetail->nSharpKernel);
				jsnObj.set("cannyKernel", itSnDetail->nCannyKernel);
				jsnObj.set("dilateKernel", itSnDetail->nDilateKernel);
				jsnArry.add(jsnObj);
			}
			jsnSNObj.set("nItem", (*itSn)->nItem);
			jsnSNObj.set("nRecogVal", (*itSn)->nRecogVal);
			jsnSNObj.set("snList", jsnArry);
			jsnSNArry.add(jsnSNObj);
		}
		ELECTOMR_LIST::iterator itElectOmr = pModel->vecPaperModel[i]->lElectOmr.begin();
		for (; itElectOmr != pModel->vecPaperModel[i]->lElectOmr.end(); itElectOmr++)
		{
			pModel->nHasElectOmr = 1;		//设置标识

			Poco::JSON::Object jsnTHObj;
			Poco::JSON::Array  jsnArry;
			RECTLIST::iterator itOmrSel = itElectOmr->lItemInfo.begin();
			for (; itOmrSel != itElectOmr->lItemInfo.end(); itOmrSel++)
			{
				Poco::JSON::Object jsnObj;
				jsnObj.set("eType", (int)itOmrSel->eCPType);
				jsnObj.set("nTH", itOmrSel->nTH);
				jsnObj.set("nAnswer", itOmrSel->nAnswer);
				jsnObj.set("left", itOmrSel->rt.x);
				jsnObj.set("top", itOmrSel->rt.y);
				jsnObj.set("width", itOmrSel->rt.width);
				jsnObj.set("height", itOmrSel->rt.height);
				jsnObj.set("hHeadItem", itOmrSel->nHItem);
				jsnObj.set("vHeadItem", itOmrSel->nVItem);
				jsnObj.set("thresholdValue", itOmrSel->nThresholdValue);
				jsnObj.set("standardValPercent", itOmrSel->fStandardValuePercent);
				jsnObj.set("standardVal", itOmrSel->fStandardValue);
				jsnObj.set("standardArea", itOmrSel->fStandardArea);
				jsnObj.set("standardDensity", itOmrSel->fStandardDensity);
				jsnObj.set("standardMeanGray", itOmrSel->fStandardMeanGray);
				jsnObj.set("standardStddev", itOmrSel->fStandardStddev);

				jsnObj.set("gaussKernel", itOmrSel->nGaussKernel);
				jsnObj.set("sharpKernel", itOmrSel->nSharpKernel);
				jsnObj.set("cannyKernel", itOmrSel->nCannyKernel);
				jsnObj.set("dilateKernel", itOmrSel->nDilateKernel);
				jsnArry.add(jsnObj);
			}
			jsnTHObj.set("nGroupID", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnTHObj.set("nAllCount", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnTHObj.set("nRealCount", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnTHObj.set("omrlist", jsnArry);
			jsnElectOmrArry.add(jsnTHObj);
		}
		CHARACTER_ANCHOR_AREA_LIST::iterator itRecogCharInfo = pModel->vecPaperModel[i]->lCharacterAnchorArea.begin();
		for (; itRecogCharInfo != pModel->vecPaperModel[i]->lCharacterAnchorArea.end(); itRecogCharInfo++)
		{
			Poco::JSON::Object jsnCharacterAnchorAreaObj;
			Poco::JSON::Array  jsnArry;
			std::vector<pST_CHARACTER_ANCHOR_POINT>::iterator itCharRt = (*itRecogCharInfo)->vecCharacterRt.begin();
			for (; itCharRt != (*itRecogCharInfo)->vecCharacterRt.end(); itCharRt++)
			{
				RECTINFO rcTmp = (*itCharRt)->rc;
				Poco::JSON::Object jsnObj;
				jsnObj.set("eType", (int)rcTmp.eCPType);
				jsnObj.set("nTH", rcTmp.nTH);
				jsnObj.set("nAnswer", rcTmp.nAnswer);
				jsnObj.set("left", rcTmp.rt.x);
				jsnObj.set("top", rcTmp.rt.y);
				jsnObj.set("width", rcTmp.rt.width);
				jsnObj.set("height", rcTmp.rt.height);
				jsnObj.set("hHeadItem", rcTmp.nHItem);
				jsnObj.set("vHeadItem", rcTmp.nVItem);
				jsnObj.set("thresholdValue", rcTmp.nThresholdValue);
				jsnObj.set("standardValPercent", rcTmp.fStandardValuePercent);
				jsnObj.set("standardVal", rcTmp.fStandardValue);
				jsnObj.set("standardArea", rcTmp.fStandardArea);
				jsnObj.set("standardDensity", rcTmp.fStandardDensity);
				jsnObj.set("standardMeanGray", rcTmp.fStandardMeanGray);
				jsnObj.set("standardStddev", rcTmp.fStandardStddev);

				jsnObj.set("gaussKernel", rcTmp.nGaussKernel);
				jsnObj.set("sharpKernel", rcTmp.nSharpKernel);
				jsnObj.set("cannyKernel", rcTmp.nCannyKernel);
				jsnObj.set("dilateKernel", rcTmp.nDilateKernel);

				//------------------
				jsnObj.set("nIndex", (*itCharRt)->nIndex);
				jsnObj.set("fConfidence", (*itCharRt)->fConfidence);
				jsnObj.set("strRecogChar", CMyCodeConvert::Gb2312ToUtf8((*itCharRt)->strVal));
				jsnArry.add(jsnObj);
			}
			jsnCharacterAnchorAreaObj.set("nIndex", (*itRecogCharInfo)->nIndex);
			jsnCharacterAnchorAreaObj.set("nThreshold", (*itRecogCharInfo)->nThresholdValue);
			jsnCharacterAnchorAreaObj.set("nConfidence", (*itRecogCharInfo)->nCharacterConfidence);
			jsnCharacterAnchorAreaObj.set("nRectsInArea", (*itRecogCharInfo)->nRects);

			jsnCharacterAnchorAreaObj.set("gaussKernel", (*itRecogCharInfo)->nGaussKernel);
			jsnCharacterAnchorAreaObj.set("sharpKernel", (*itRecogCharInfo)->nSharpKernel);
			jsnCharacterAnchorAreaObj.set("cannyKernel", (*itRecogCharInfo)->nCannyKernel);
			jsnCharacterAnchorAreaObj.set("dilateKernel", (*itRecogCharInfo)->nDilateKernel);

			jsnCharacterAnchorAreaObj.set("left", (*itRecogCharInfo)->rt.x);
			jsnCharacterAnchorAreaObj.set("top", (*itRecogCharInfo)->rt.y);
			jsnCharacterAnchorAreaObj.set("width", (*itRecogCharInfo)->rt.width);
			jsnCharacterAnchorAreaObj.set("height", (*itRecogCharInfo)->rt.height);

			jsnCharacterAnchorAreaObj.set("characterAnchorPointList", jsnArry);
			jsnCharacterAnchorAreaArry.add(jsnCharacterAnchorAreaObj);
		}
		jsnPaperObj.set("paperNum", i);
		jsnPaperObj.set("modelPicName", CMyCodeConvert::Gb2312ToUtf8(T2A(strPicName)));		//CMyCodeConvert::Gb2312ToUtf8(T2A(strPicName))
		jsnPaperObj.set("FixCP", jsnFixCPArry);
		jsnPaperObj.set("H_Head", jsnHHeadArry);
		jsnPaperObj.set("V_Head", jsnVHeadArry);
		jsnPaperObj.set("Pagination", jsnPaginationArry);
		jsnPaperObj.set("ABModel", jsnABModelArry);
		jsnPaperObj.set("Course", jsnCourseArry);
		jsnPaperObj.set("QKCP", jsnQKArry);
		jsnPaperObj.set("WJCP", jsnWJArry);
		jsnPaperObj.set("GrayCP", jsnGrayCPArry);
		jsnPaperObj.set("WhiteCP", jsnWhiteCPArry);
		jsnPaperObj.set("selRoiRect", jsnSelRoiArry);
		jsnPaperObj.set("hTrackerRect", jsnSelHTrackerArry);
		jsnPaperObj.set("vTrackerRect", jsnSelVTrackerArry);
		jsnPaperObj.set("selOmrRect", jsnOMRArry);
		jsnPaperObj.set("snList", jsnSNArry);
		jsnPaperObj.set("electOmrList", jsnElectOmrArry);
		jsnPaperObj.set("characterAnchorArea", jsnCharacterAnchorAreaArry);

		jsnPaperObj.set("picW", pModel->vecPaperModel[i]->nPicW);		//add on 16.8.29
		jsnPaperObj.set("picH", pModel->vecPaperModel[i]->nPicH);		//add on 16.8.29
		jsnPaperObj.set("rtHTracker.x", pModel->vecPaperModel[i]->rtHTracker.x);
		jsnPaperObj.set("rtHTracker.y", pModel->vecPaperModel[i]->rtHTracker.y);
		jsnPaperObj.set("rtHTracker.width", pModel->vecPaperModel[i]->rtHTracker.width);
		jsnPaperObj.set("rtHTracker.height", pModel->vecPaperModel[i]->rtHTracker.height);
		jsnPaperObj.set("rtVTracker.x", pModel->vecPaperModel[i]->rtVTracker.x);
		jsnPaperObj.set("rtVTracker.y", pModel->vecPaperModel[i]->rtVTracker.y);
		jsnPaperObj.set("rtVTracker.width", pModel->vecPaperModel[i]->rtVTracker.width);
		jsnPaperObj.set("rtVTracker.height", pModel->vecPaperModel[i]->rtVTracker.height);
		jsnPaperObj.set("rtSNTracker.x", pModel->vecPaperModel[i]->rcSNTracker.rt.x);
		jsnPaperObj.set("rtSNTracker.y", pModel->vecPaperModel[i]->rcSNTracker.rt.y);
		jsnPaperObj.set("rtSNTracker.width", pModel->vecPaperModel[i]->rcSNTracker.rt.width);
		jsnPaperObj.set("rtSNTracker.height", pModel->vecPaperModel[i]->rcSNTracker.rt.height);
		jsnPaperObj.set("rtSNTracker.nRecogFlag", pModel->vecPaperModel[i]->rcSNTracker.nRecogFlag);
		
		jsnPicModel.add(jsnPaperObj);
	}
	
	jsnModel.set("modelName", CMyCodeConvert::Gb2312ToUtf8(pModel->strModelName));		//CMyCodeConvert::Gb2312ToUtf8(T2A(pModel->strModelName))
	jsnModel.set("modelDesc", CMyCodeConvert::Gb2312ToUtf8(pModel->strModelDesc));
	jsnModel.set("modelType", pModel->nType);
	jsnModel.set("modeSaveMode", pModel->nSaveMode);
	jsnModel.set("paperModelCount", pModel->nPicNum);			//此模板有几页试卷(图片)
	jsnModel.set("enableModify", pModel->nEnableModify);		//是否可以修改标识
	jsnModel.set("abPaper", pModel->nABModel);					//是否是AB卷					*************	暂时没加入AB卷的模板	**************
	jsnModel.set("hasHead", pModel->nHasHead);					//是否有同步头
	jsnModel.set("hasElectOmr", pModel->nHasElectOmr);			//是否有选做题
	jsnModel.set("nZkzhType", pModel->nZkzhType);				//准考证号识别类型
	jsnModel.set("nScanDpi", pModel->nScanDpi);					//扫描的dpi设置
	jsnModel.set("nScanSize", pModel->nScanSize);				//扫描用的纸张类型，1-a4, 2-a3, 3-定制
	jsnModel.set("nScanType", pModel->nScanType);				//扫描模式：1-灰度扫描，2-彩色扫描
	jsnModel.set("nScanAutoCut", pModel->nAutoCut);				//扫描仪是否自动裁剪，超长卡不能裁剪
	jsnModel.set("nUseWordAnchorPoint", pModel->nUseWordAnchorPoint);		//是否使用文字定点来定位识别
	jsnModel.set("nCharacterAnchorPoint", pModel->nCharacterAnchorPoint);	//用来计算矩形位置的文字定点个数
	jsnModel.set("nUsePagination", pModel->nUsePagination);					//是否使用页码标识
	
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
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return false;

	int  nFind = 0;

	m_pCurRectInfo = NULL;
	nFind = GetRectInfoByPoint(GetSrcSavePoint(pt), m_eCurCPType, m_pCurRectInfo);
	if(nFind < 0)
		return false;

	if (!m_pCurRectInfo)
		return false;

	if (m_pRecogInfoDlg)	m_pRecogInfoDlg->ShowDetailRectInfo(m_pCurRectInfo);

	InitShowSnOmrDlg(m_pCurRectInfo->eCPType);
	if (m_pCurRectInfo->eCPType == SN && m_pSNInfoDlg)
		m_pSNInfoDlg->ShowUI(GetSnShowFakePosFlag(m_pCurRectInfo->nRecogFlag));
	else if (m_pCurRectInfo->eCPType == OMR && m_pOmrInfoDlg)
		m_pOmrInfoDlg->ShowUI(GetOmrShowFakePosFlag(m_pCurRectInfo->nRecogFlag), m_pCurRectInfo->nSingle);
	else if (m_pCurRectInfo->eCPType == ELECT_OMR && m_pElectOmrDlg)
		m_pElectOmrDlg->showUI(m_pCurRectInfo->nTH);

	Rect rt = GetShowFakePosRect(m_pCurRectInfo->rt);
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
				cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel[i].rt);
				cv::rectangle(tmp, rtTmp, CV_RGB(181, 115, 173), 2);

				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].eCPType)
				{
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].rt);
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
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i].rt);
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
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i].rt);
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case PAGINATION:
		if (eType == PAGINATION || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[i].eCPType)
				{
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[i].rt);
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
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i].rt);
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
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i].rt);
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
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i].rt);
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				}
			}
		}
	case WJ_CP:
		if (eType == WJ_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size(); i++)
			{
				if (i != nFind || m_pCurRectInfo->eCPType != m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[i].eCPType)
				{
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[i].rt);
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
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i].rt);
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
					cv::Rect rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i].rt);
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
						cv::Rect rt = GetShowFakePosRect(rc.rt);
						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d_%d", rc.nTH, rc.nSnVal);
						rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);

						putText(tmp, szAnswerVal, Point(rt.x + rt.width / 10, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
						rectangle(tmp2, rt, CV_RGB(50, 200, 150), -1);
					}
					else
					{
						RECTINFO rc = *itSNRect;
						cv::Rect rt = GetShowFakePosRect(rc.rt);
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
						cv::Rect rt = GetShowFakePosRect(rc.rt);
						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer + 65);
						if (rc.nSingle == 0)
						{
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
							rectangle(tmp2, rt, CV_RGB(50, 155, 100), -1);
						}
						else if(rc.nSingle == 1)
						{
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));
							rectangle(tmp2, rt, CV_RGB(100, 50, 200), -1);
						}
						else
						{
							sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer % 2 ? 'F' : 'T');
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));
							rectangle(tmp2, rt, CV_RGB(208, 81, 206), -1);
						}
					}
					else
					{
						RECTINFO rc = *itAnswer;
						cv::Rect rt = GetShowFakePosRect(rc.rt);
						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer + 65);
						cv::rectangle(tmp, rt, CV_RGB(40, 22, 255), 2);
						if (rc.nSingle == 0)
						{
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0,255));	//CV_FONT_HERSHEY_COMPLEX
							rectangle(tmp2, rt, CV_RGB(50, 55, 255), -1);
						}
						else if(rc.nSingle == 1)
						{
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0,255));
							rectangle(tmp2, rt, CV_RGB(100, 250, 150), -1);
						}
						else
						{
							sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer % 2 ? 'F' : 'T');
							putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));
							rectangle(tmp2, rt, CV_RGB(208, 81, 206), -1);
						}
					}
				}
			}
		}
	case ELECT_OMR:
		if (eType == ELECT_OMR || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); i++)
			{
				RECTLIST::iterator itElectOmr = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.begin();
				for (; itElectOmr != m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.end(); itElectOmr++)
				{
					if (m_pCurRectInfo != &(*itElectOmr))
					{
						RECTINFO rc = *itElectOmr;
						rt = GetShowFakePosRect(rc.rt);

						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d_%c", rc.nTH, rc.nAnswer + 65);

						cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 10, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
						cv::rectangle(tmp2, rt, CV_RGB(40, 190, 135), -1);
					}
					else
					{
						RECTINFO rc = *itElectOmr;
						rt = GetShowFakePosRect(rc.rt);

						char szAnswerVal[10] = { 0 };
						sprintf_s(szAnswerVal, "%d_%c", rc.nTH, rc.nAnswer + 65);

						cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 10, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
						cv::rectangle(tmp2, rt, CV_RGB(40, 205, 150), -1);
					}
				}
			}
		}
	case CHARACTER_AREA:
		if (eType == CHARACTER_AREA || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
			{
				cv::Rect rtTmp = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->rt;
				cv::rectangle(tmp, rtTmp, CV_RGB(181, 115, 173), 2);
				cv::rectangle(tmp2, rtTmp, CV_RGB(170, 215, 111), -1);
				for(int j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size(); j++)
				{
					if (m_pCurRectInfo != &m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc)
					{
						RECTINFO rc = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc;
						rt = GetShowFakePosRect(rc.rt);

						cv::rectangle(tmp2, rt, CV_RGB(168, 86, 157), -1);
					}
					else
					{
						RECTINFO rc = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc;
						rt = GetShowFakePosRect(rc.rt);

						cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 3);
						cv::rectangle(tmp2, rt, CV_RGB(40, 205, 150), -1);
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
	bool bFindElectOmr = false;
	bool bFindCharacter = false;
	int nSNCount = 0;
	int nOmrCount = 0;
	int nElectOmrCount = 0;
	int nCharacterCount = 0;
	cv::Rect rt;
#if 1
	int nCount = 0;
	int nCurItem = nItem;
	bool bFind = false;

	Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();

	if (m_eCurCPType == Fix_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size(); i++)
		{
			cv::Rect rtTmp1 = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel[i].rt);
			cv::rectangle(tmp, rtTmp1, CV_RGB(181, 115, 173), 2);

			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size();
	}
	if (m_eCurCPType == H_HEAD || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size();
	}
	if (m_eCurCPType == V_HEAD || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size();
	}
	if (m_eCurCPType == PAGINATION || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size();
	}
	if (m_eCurCPType == ABMODEL || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size();
	}
	if (m_eCurCPType == COURSE || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size();
	}
	if (m_eCurCPType == QK_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size();
	}
	if (m_eCurCPType == WJ_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size();
	}
	if (m_eCurCPType == GRAY_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size();
	}
	if (m_eCurCPType == WHITE_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size(); i++)
		{
			cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i].rt);
			cv::rectangle(tmp2, rtTmp, CV_RGB(255, 233, 10), -1);
		}
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size() > nCurItem)
		{
			bFind = true;
			rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[nCurItem].rt);
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size();
	}
	if (m_eCurCPType == SN || m_eCurCPType == UNKNOWN)
	{
		SNLIST::iterator itSNItem = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
		for (; itSNItem != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end(); itSNItem++)
		{
			RECTLIST::iterator itSNRect = (*itSNItem)->lSN.begin();
			for (; itSNRect != (*itSNItem)->lSN.end(); itSNRect++)
			{
				RECTINFO rc = *itSNRect;
				cv::Rect rtTmp = GetShowFakePosRect(rc.rt);

				char szAnswerVal[10] = { 0 };
				sprintf_s(szAnswerVal, "%d_%d", rc.nTH, rc.nSnVal);

				cv::putText(tmp, szAnswerVal, Point(rtTmp.x + rtTmp.width / 10, rtTmp.y + rtTmp.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255));	//CV_FONT_HERSHEY_COMPLEX	Scalar(255, 0, 0) Point(rt.x + rt.width / 10, rt.y + rt.height / 2)
				cv::rectangle(tmp2, rtTmp, CV_RGB(50, 200, 150), -1);	//CV_RGB(50, 200, 150)
			}
		}
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
							rt = GetShowFakePosRect(rc.rt);
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
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size(); i++)
		{
			RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.begin();
			for (; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2[i].lSelAnswer.end(); itAnswer++)
			{
				RECTINFO rc = *itAnswer;
				cv::Rect rtTmp = GetShowFakePosRect(rc.rt);
				
				char szAnswerVal[10] = { 0 };
				sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer + 65);
				if (rc.nSingle == 0)
				{
					cv::putText(tmp, szAnswerVal, Point(rtTmp.x + rtTmp.width / 5, rtTmp.y + rtTmp.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX	Scalar(255, 0, 0)
					cv::rectangle(tmp2, rtTmp, CV_RGB(50, 255, 100), -1);
				}
				else if(rc.nSingle == 1)
				{
					cv::putText(tmp, szAnswerVal, Point(rtTmp.x + rtTmp.width / 5, rtTmp.y + rtTmp.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 100, 0));
					cv::rectangle(tmp2, rtTmp, CV_RGB(150, 150, 255), -1);
				}
				else
				{
					sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer % 2 ? 'F' : 'T');
					putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));
					rectangle(tmp2, rt, CV_RGB(208, 81, 206), -1);
				}
			}
		}
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
							rt = GetShowFakePosRect(itAnswer->rt);
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
	if (m_eCurCPType == ELECT_OMR || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); i++)
		{
			RECTLIST::iterator itElectOmr = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.begin();
			for (; itElectOmr != m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.end(); itElectOmr++)
			{
				RECTINFO rc = *itElectOmr;
				cv::Rect rtTmp = GetShowFakePosRect(rc.rt);

				char szAnswerVal[10] = { 0 };
				sprintf_s(szAnswerVal, "%d_%c", rc.nTH, rc.nAnswer + 65);

				cv::putText(tmp, szAnswerVal, Point(rtTmp.x + rtTmp.width / 10, rtTmp.y + rtTmp.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				cv::rectangle(tmp2, rtTmp, CV_RGB(40, 205, 150), -1);
			}
		}
		if (!bFind)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); i++)
			{
				nElectOmrCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.size();
				if (nCurItem < nElectOmrCount)
				{
					RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.begin();
					for (int j = 0; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.end(); itAnswer++, j++)
					{
						if (j == nCurItem)
						{
							bFindElectOmr = true;
							bFind = true;
							rt = GetShowFakePosRect(itAnswer->rt);
							m_pCurRectInfo = &(*itAnswer);
							break;
						}
					}
					if (bFindElectOmr)
						break;
				}
				else
				{
					nCurItem -= nElectOmrCount;
				}
			}
		}
	}
	if (m_eCurCPType == CHARACTER_AREA || m_eCurCPType == UNKNOWN)
	{
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
		{
			for (int j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size(); j++)
			{
				RECTINFO rc = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc;
				cv::Rect rtTmp = GetShowFakePosRect(rc.rt);

				cv::rectangle(tmp2, rtTmp, CV_RGB(168, 86, 157), -1);
			}
		}
		if (!bFind)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
			{
				nCharacterCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size();
				if (nCurItem < nCharacterCount)
				{
					for (int j = 0; j < nCharacterCount; j++)
					{
						if (j == nCurItem)
						{
							bFindCharacter = true;
							bFind = true;
							rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc.rt);
							m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc;
							break;
						}
					}
					if (bFindCharacter)
						break;
				}
				else
				{
					nCurItem -= nCharacterCount;
				}
			}
		}
	}

	if (m_pRecogInfoDlg)	m_pRecogInfoDlg->ShowDetailRectInfo(m_pCurRectInfo);

	InitShowSnOmrDlg(m_pCurRectInfo->eCPType);
	if (m_pCurRectInfo->eCPType == SN && m_pSNInfoDlg)
	{
		m_pSNInfoDlg->InitType(m_pCurRectInfo->nZkzhType);
		m_pSNInfoDlg->ShowUI(GetSnShowFakePosFlag(m_pCurRectInfo->nRecogFlag));
	}
	else if (m_pCurRectInfo->eCPType == OMR && m_pOmrInfoDlg)
		m_pOmrInfoDlg->ShowUI(GetOmrShowFakePosFlag(m_pCurRectInfo->nRecogFlag), m_pCurRectInfo->nSingle);
	else if (m_pCurRectInfo->eCPType == ELECT_OMR && m_pElectOmrDlg)
		m_pElectOmrDlg->showUI(m_pCurRectInfo->nTH);

	cv::Point pt = rt.tl();
	if (pt.x < m_pModelPicShow->m_iX)
		pt.x -= 100;
	else if (pt.x > m_pModelPicShow->m_iX + m_pModelPicShow->m_client.Width())
		pt.x -= 100;
	else
		pt.x = m_pModelPicShow->m_iX;
	if (pt.y < m_pModelPicShow->m_iY)
		pt.y -= 100;
	else if (pt.y > m_pModelPicShow->m_iY + m_pModelPicShow->m_client.Height())
		pt.y -= 100;
	else
		pt.y = m_pModelPicShow->m_iY;

// 	pt.x -= 100;
// 	pt.y -= 100;

	rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
	rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
#else
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
	if (m_eCurCPType == PAGINATION || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size();
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
	if (m_eCurCPType == WJ_CP || m_eCurCPType == UNKNOWN)
	{
		if (!bFind && m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size() > nCurItem)
		{
			bFind = true;
			rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[nCurItem].rt;
			m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[nCurItem];
		}
		if (m_eCurCPType == UNKNOWN)
			nCurItem -= m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size();
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
	if(m_eCurCPType == ELECT_OMR || UNKNOWN)
	{
		if(!bFind)
		{
			for(int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); i++)
			{
				nElectOmrCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.size();
				if(nCurItem < nElectOmrCount)
				{
					RECTLIST::iterator itAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.begin();
					for (int j = 0; itAnswer != m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.end(); itAnswer++, j++)
					{
						if (j == nCurItem)
						{
							bFindElectOmr = true;
							bFind = true;
							rt = itAnswer->rt;
							m_pCurRectInfo = &(*itAnswer);
							break;
						}
					}
					if (bFindElectOmr)
						break;
				}
				else
				{
					nCurItem -= nElectOmrCount;
				}
			}
		}
	}
	if (m_eCurCPType == CHARACTER_AREA || UNKNOWN)
	{
		if (!bFind)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
			{
				nCharacterCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i].vecCharacterRt.size();
				if (nCurItem < nCharacterCount)
				{
					for(int j = 0; j < nCharacterCount; j++)
					{
						if (j == nCurItem)
						{
							bFindCharacter = true;
							bFind = true;
							rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i].vecCharacterRt[j].rc.rt;
							m_pCurRectInfo = &m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i].vecCharacterRt[j].rc;
							break;
						}
					}
					if (bFindCharacter)
						break;
				}
				else
				{
					nCurItem -= nCharacterCount;
				}
			}
		}
	}

	if(m_pRecogInfoDlg)	m_pRecogInfoDlg->ShowDetailRectInfo(m_pCurRectInfo);

	InitShowSnOmrDlg(m_pCurRectInfo->eCPType);
	if (m_pCurRectInfo->eCPType == SN && m_pSNInfoDlg)
	{
		m_pSNInfoDlg->InitType(m_pCurRectInfo->nZkzhType);
		m_pSNInfoDlg->ShowUI(GetSnShowFakePosFlag(m_pCurRectInfo->nRecogFlag));
	}
	else if (m_pCurRectInfo->eCPType == OMR && m_pOmrInfoDlg)
		m_pOmrInfoDlg->ShowUI(GetOmrShowFakePosFlag(m_pCurRectInfo->nRecogFlag), m_pCurRectInfo->nSingle);
	else if (m_pCurRectInfo->eCPType == ELECT_OMR && m_pElectOmrDlg)
		m_pElectOmrDlg->showUI(m_pCurRectInfo->nTH);

	Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();

	cv::Point pt = rt.tl();
	pt.x -= 100;
	pt.y -= 100;

	rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
	rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
#endif
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
		rt = GetShowFakePosRect(m_vecTmp[i].rt);
		rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
//		rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		if (m_vecTmp[i].eCPType == OMR)
		{
			char szAnswerVal[10] = { 0 };
			if (m_vecTmp[i].nSingle == 2)
				sprintf_s(szAnswerVal, "%d%c", m_vecTmp[i].nTH, m_vecTmp[i].nAnswer % 2 ? 'F' : 'T');
			else
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
		else if (m_vecTmp[i].eCPType == ELECT_OMR)
		{
			char szAnswerVal[10] = { 0 };
			sprintf_s(szAnswerVal, "%d_%c", m_vecTmp[i].nTH, m_vecTmp[i].nAnswer + 65);
			putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp2, rt, CV_RGB(40, 255, 110), -1);
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
	Mat tmp = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	Mat tmp2 = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.clone();
	switch (eType)
	{
	case UNKNOWN:
	case Fix_CP:
		if (eType == Fix_CP || eType == UNKNOWN)
		{
			if (!m_pModel->nUseWordAnchorPoint)	//只有在不使用文字定位点的时候显示定点
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix.size(); i++)
				{
					cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecRtSel[i].rt);
					cv::rectangle(tmp, rtTmp, CV_RGB(181, 115, 173), 2);
					cv::rectangle(tmp2, rtTmp, CV_RGB(170, 215, 111), -1);

					rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecRtFix[i].rt);
					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
					cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
				}
			}
		}
	case H_HEAD:
		if (eType == H_HEAD || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size(); i++)
			{
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head[i].rt);
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			if (eType == H_HEAD && m_vecPaperModelInfo[m_nCurrTabSel]->vecH_Head.size() == 0)
			{
				cv::Point pt1 = GetShowFakePosPoint(m_pModelPicShow->m_picShow.m_ptHTracker1);
				cv::Point pt2 = GetShowFakePosPoint(m_pModelPicShow->m_picShow.m_ptHTracker2);

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
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head[i].rt);
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			if (eType == V_HEAD && m_vecPaperModelInfo[m_nCurrTabSel]->vecV_Head.size() == 0)
			{
				cv::Point pt1 = GetShowFakePosPoint(m_pModelPicShow->m_picShow.m_ptVTracker1);
				cv::Point pt2 = GetShowFakePosPoint(m_pModelPicShow->m_picShow.m_ptVTracker2);

				rt = cv::Rect(pt1, pt2);
				cv::rectangle(tmp, rt, CV_RGB(255, 20, 50), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case PAGINATION:
		if (eType == PAGINATION || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size(); i++)
			{
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[i].rt);
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case ABMODEL:
		if (eType == ABMODEL || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel.size(); i++)
			{
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecABModel[i].rt);
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case COURSE:
		if (eType == COURSE || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse.size(); i++)
			{
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecCourse[i].rt);
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case QK_CP:
		if (eType == QK_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP.size(); i++)
			{
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecQK_CP[i].rt);
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case WJ_CP:
		if (eType == WJ_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size(); i++)
			{
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[i].rt);
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case GRAY_CP:
		if (eType == GRAY_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecGray.size(); i++)
			{
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecGray[i].rt);
				cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				cv::rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
	case WHITE_CP:
		if (eType == WHITE_CP || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite.size(); i++)
			{
				rt = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecWhite[i].rt);
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
					rt = GetShowFakePosRect(rc.rt);

					cv::rectangle(tmp, rt, CV_RGB(255, 255, 0), 2);	//CV_RGB(255, 0, 0)

					char szAnswerVal[10] = { 0 };
					sprintf_s(szAnswerVal, "%d_%d", rc.nTH, rc.nSnVal);
					
					cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 10, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255));	//CV_FONT_HERSHEY_COMPLEX	Scalar(255, 0, 0) Point(rt.x + rt.width / 10, rt.y + rt.height / 2)
					cv::rectangle(tmp2, rt, CV_RGB(50, 200, 150), -1);	//CV_RGB(50, 200, 150)
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
					rt = GetShowFakePosRect(rc.rt);

					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);

					char szAnswerVal[10] = { 0 };
					sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer + 65);
					if (rc.nSingle == 0)
					{
						cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX	Scalar(255, 0, 0)
						cv::rectangle(tmp2, rt, CV_RGB(50, 255, 100), -1);
					}
					else if(rc.nSingle == 1)
					{
						cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 100, 0));
						cv::rectangle(tmp2, rt, CV_RGB(150, 150, 255), -1);
					}
					else
					{
						sprintf_s(szAnswerVal, "%d%c", rc.nTH, rc.nAnswer % 2 ? 'F': 'T');
						putText(tmp, szAnswerVal, Point(rt.x + rt.width / 5, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));
						rectangle(tmp2, rt, CV_RGB(208, 81, 206), -1);
					}
				}
			}
		}
	case ELECT_OMR:
		if (eType == ELECT_OMR || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); i++)
			{
				RECTLIST::iterator itElectOmr = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.begin();
				for (; itElectOmr != m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.end(); itElectOmr++)
				{
					RECTINFO rc = *itElectOmr;
					rt = GetShowFakePosRect(rc.rt);

					cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);

					char szAnswerVal[10] = { 0 };
					sprintf_s(szAnswerVal, "%d_%c", rc.nTH, rc.nAnswer + 65);

					cv::putText(tmp, szAnswerVal, Point(rt.x + rt.width / 10, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
					cv::rectangle(tmp2, rt, CV_RGB(40, 205, 150), -1);
				}
			}
		}
	case CHARACTER_AREA:
		if (eType == CHARACTER_AREA || eType == UNKNOWN)
		{
			if (m_pModel->nUseWordAnchorPoint)
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
				{
					cv::Rect rtTmp = GetShowFakePosRect(m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->rt);
					cv::rectangle(tmp, rtTmp, CV_RGB(181, 115, 173), 2);
					cv::rectangle(tmp2, rtTmp, CV_RGB(170, 215, 111), -1);
					for (int j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size(); j++)
					{
						RECTINFO rc = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc;
						rt = GetShowFakePosRect(rc.rt);

						cv::rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
						cv::rectangle(tmp2, rt, CV_RGB(168, 86, 157), -1);
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
	else if (strCheckPoint == "违纪校验点")
		eType = WJ_CP;
	else if (strCheckPoint == "灰度校验点")
		eType = GRAY_CP;
	else if (strCheckPoint == "空白校验点")
		eType = WHITE_CP;
	else if (strCheckPoint == "考号设置")
		eType = SN;
	else if (strCheckPoint == "选择/判断题")
		eType = OMR;
	else if (strCheckPoint == "选做题")
		eType = ELECT_OMR;
	else if (strCheckPoint == "文字定位区")
		eType = CHARACTER_AREA;
	else if (strCheckPoint == "空白校验区")
		eType = WHITE_GRAY_AREA;
	else if (strCheckPoint == "页码")
		eType = PAGINATION;
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
			m_nDilateKernel = m_nDilateKernel_Sn;
		}
		break;
		default:
			m_nDilateKernel = m_nDilateKernel_Common;
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
		m_pElectOmrDlg->ShowWindow(SW_HIDE);
	}
	else if (eType == SN)
	{
		m_pOmrInfoDlg->ShowWindow(SW_HIDE);
		m_pRecogInfoDlg->ShowWindow(SW_HIDE);
		m_pSNInfoDlg->ShowWindow(SW_SHOW);
		m_pElectOmrDlg->ShowWindow(SW_HIDE);
	}
	else if (eType == ELECT_OMR)
	{
		m_pOmrInfoDlg->ShowWindow(SW_HIDE);
		m_pRecogInfoDlg->ShowWindow(SW_HIDE);
		m_pSNInfoDlg->ShowWindow(SW_HIDE);
		m_pElectOmrDlg->ShowWindow(SW_SHOW);

		if (!m_pModel || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
			return;
		m_pElectOmrDlg->InitGroupInfo(m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr, m_nCurrTabSel);
	}
	else
	{
		m_pOmrInfoDlg->ShowWindow(SW_HIDE);
		m_pRecogInfoDlg->ShowWindow(SW_SHOW);
		m_pSNInfoDlg->ShowWindow(SW_HIDE);
		m_pElectOmrDlg->ShowWindow(SW_HIDE);
	}
}

void CMakeModelDlg::UpdataCPList()
{
	if (!m_pModel || m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return;

	USES_CONVERSION;
	int nCount = 0;
	m_cpListCtrl.DeleteAllItems();
	UpdateCPListByType();
	if (m_eCurCPType == H_HEAD)
	{
		m_pModelPicShow->SetShowTracker(true, false, false);
		if (m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH && m_bNewModelFlag)
		{
			m_ptHTracker1 = cv::Point(0, 0);
			m_ptHTracker2 = cv::Point(m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols, 90);	//m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols
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
		m_pModelPicShow->m_picShow.setHTrackerPosition(GetShowFakePosPoint(m_ptHTracker1), GetShowFakePosPoint(m_ptHTracker2));
	}
	else if (m_eCurCPType == V_HEAD)
	{
		m_pModelPicShow->SetShowTracker(false, true, false);
		if (m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV && m_bNewModelFlag)
		{
			m_ptVTracker1 = cv::Point(m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols - 90, 0);	//m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols
			m_ptVTracker2 = cv::Point(m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols, m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows);	//m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols  m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows
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
		m_pModelPicShow->m_picShow.setVTrackerPosition(GetShowFakePosPoint(m_ptVTracker1), GetShowFakePosPoint(m_ptVTracker2));
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
// 		m_ptSNTracker1 = m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.tl();
// 		m_ptSNTracker2 = m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.br();
		m_ptSNTracker1 = m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.rt.tl();
		m_ptSNTracker2 = m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.rt.br();
		SNLIST::iterator itSN = m_vecPaperModelInfo[m_nCurrTabSel]->lSN.begin();
		if (itSN != m_vecPaperModelInfo[m_nCurrTabSel]->lSN.end())
		{
			RECTLIST::iterator itSnDetail = (*itSN)->lSN.begin();
			if (itSnDetail != (*itSN)->lSN.end())
			{
				m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.nRecogFlag = itSnDetail->nRecogFlag;
			}
		}
		m_pModelPicShow->m_picShow.setSNTrackerPosition(GetShowFakePosPoint(m_ptSNTracker1), GetShowFakePosPoint(m_ptSNTracker2));
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
	if (m_eCurCPType == PAGINATION || m_eCurCPType == UNKNOWN)
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size();
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
	if (m_eCurCPType == WJ_CP || m_eCurCPType == UNKNOWN)
	{
		for (int i = nCount; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size() + nCount; i++)
		{
			RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[i - nCount];
			char szPosition[50] = { 0 };
			sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", i + 1);
			m_cpListCtrl.InsertItem(i, NULL);
			m_cpListCtrl.SetItemText(i, 0, (LPCTSTR)A2T(szCount));
			m_cpListCtrl.SetItemText(i, 1, (LPCTSTR)A2T(szPosition));
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size();
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
	if (m_eCurCPType == ELECT_OMR || m_eCurCPType == UNKNOWN)
	{
		int nElectOmrCount = 0;
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); i++)
		{
			RECTLIST::iterator itElectOmr = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.begin();
			for (int j = nElectOmrCount; itElectOmr != m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.end(); itElectOmr++, j++)
			{
				RECTINFO rcInfo = *itElectOmr;
				char szPosition[50] = { 0 };
				sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
				char szCount[10] = { 0 };
				sprintf_s(szCount, "%d", j + nCount + 1);
				m_cpListCtrl.InsertItem(j + nCount, NULL);
				m_cpListCtrl.SetItemText(j + nCount, 0, (LPCTSTR)A2T(szCount));
				m_cpListCtrl.SetItemText(j + nCount, 1, (LPCTSTR)A2T(szPosition));
			}
			nElectOmrCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.size();
		}
		if (m_eCurCPType == UNKNOWN)
			nCount += nElectOmrCount;
	}
	if (m_eCurCPType == CHARACTER_AREA || m_eCurCPType == UNKNOWN)
	{
		int nCharacterRtCount = 0;
		if (m_eCurCPType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
			{
				for (int j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size(); j++)
				{
					RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc;
					char szPosition[50] = { 0 };
					sprintf_s(szPosition, "(%d,%d,%d,%d)", rcInfo.rt.x, rcInfo.rt.y, rcInfo.rt.width, rcInfo.rt.height);
					char szCount[10] = { 0 };
					sprintf_s(szCount, "%d", j + nCount + nCharacterRtCount + 1);
					m_cpListCtrl.InsertItem(j + nCount + nCharacterRtCount, NULL);
					m_cpListCtrl.SetItemText(j + nCount + nCharacterRtCount, 0, (LPCTSTR)A2T(szCount));
					m_cpListCtrl.SetItemText(j + nCount + nCharacterRtCount, 1, (LPCTSTR)A2T(szPosition));
				}
				nCharacterRtCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size();
			}
			nCount += nCharacterRtCount;
		}
		else
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
			{
				for (int j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size(); j++)
				{
					RECTINFO rcInfo = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc;
					char szConfidence[20] = { 0 };
					sprintf_s(szConfidence, "%.2f", m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->fConfidence);
					char szSize[30] = { 0 };
					sprintf_s(szSize, "(%d,%d)", rcInfo.rt.width, rcInfo.rt.height);
					char szVal[10] = { 0 };
					sprintf_s(szVal, "%s", m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->strVal.c_str());
					char szCount[10] = { 0 };
					sprintf_s(szCount, "%d", j + nCount + nCharacterRtCount + 1);
					m_cpListCtrl.InsertItem(j + nCount + nCharacterRtCount, NULL);
					m_cpListCtrl.SetItemText(j + nCount + nCharacterRtCount, 0, (LPCTSTR)A2T(szCount));
					m_cpListCtrl.SetItemText(j + nCount + nCharacterRtCount, 1, (LPCTSTR)A2T(szVal));
					m_cpListCtrl.SetItemText(j + nCount + nCharacterRtCount, 2, (LPCTSTR)A2T(szConfidence));
					m_cpListCtrl.SetItemText(j + nCount + nCharacterRtCount, 3, (LPCTSTR)A2T(szSize));
				}
				nCharacterRtCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size();
			}
		}
	}
}

void CMakeModelDlg::UpdateCPListByType()
{
	m_cpListCtrl.DeleteAllItems();
	//删除表头，工具重新插入属性信息
	while (m_cpListCtrl.DeleteColumn(1));
	switch(m_eCurCPType)
	{
		case CHARACTER_AREA:
			m_cpListCtrl.InsertColumn(1, _T("值"), LVCFMT_CENTER, 30);
			m_cpListCtrl.InsertColumn(2, _T("概率"), LVCFMT_CENTER, 45);
			m_cpListCtrl.InsertColumn(3, _T("大小"), LVCFMT_CENTER, 55);
			break;
		case Fix_CP:
		case H_HEAD:
		case V_HEAD:
		case ABMODEL:
		case COURSE:
		case GRAY_CP:
		case QK_CP:
		case WJ_CP:
		case SN:
		case OMR:
		default:
			m_cpListCtrl.InsertColumn(1, _T("位置信息"), LVCFMT_CENTER, 120);
			break;
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
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows - rt.y;

		RecogByHead(rt);
		m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.nDilateKernel = m_nDilateKernel;
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

	if (m_eCurCPType == SN && m_pSNInfoDlg->m_nZkzhType == 2)
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
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows - rt.y;

		RECTINFO rc;
		rc.eCPType = m_eCurCPType;
		rc.nZkzhType = 2;
		rc.rt = rt;

		pSN_ITEM pSnItem = new SN_ITEM;
		pSnItem->nItem = 0;
		pSnItem->lSN.push_back(rc);
		m_vecPaperModelInfo[m_nCurrTabSel]->lSN.push_back(pSnItem);

		UpdataCPList(); 
		ShowRectByCPType(m_eCurCPType);
		return;
	}

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
		if (m_eCurCPType == PAGINATION)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.push_back(m_vecTmp[i]);
		}
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
		else if (m_eCurCPType == WJ_CP)
		{
			m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.push_back(m_vecTmp[i]);
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
		else if (m_eCurCPType == ELECT_OMR)
		{
			bool bError = false;
			bool bFind = false;
			for (int k = 0; k < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); k++)
			{
				if (m_vecTmp[i].nTH == m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].sElectOmrGroupInfo.nGroupID)
				{

					if (m_vecTmp.size() - i + m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].lItemInfo.size() > m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].sElectOmrGroupInfo.nAllCount)
					{
						bError = true;
						break;
					}

					bFind = true;
					m_vecTmp[i].nAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].lItemInfo.size();		//修改内部题号，根据当前列表中已经存在的个数来算
					m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].lItemInfo.push_back(m_vecTmp[i]);
					break;
				}
			}
			if (bError)
			{
//				AfxMessageBox(_T("此组选做题实际选项总数超出范围"));
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, "此组选做题实际选项总数超出范围");
				dlg.DoModal();
				break;
			}
			if (!bFind)
			{
				if (!m_pElectOmrDlg->m_pCurrentGroup)
				{
//					AfxMessageBox(_T("添加失败，当前选做题信息为空"));
					CNewMessageBox dlg;
					dlg.setShowInfo(2, 1, "添加失败，当前选做题信息为空");
					dlg.DoModal();
					break;
				}
				ELECTOMR_QUESTION sElectOmr;
				sElectOmr.sElectOmrGroupInfo.nGroupID = m_pElectOmrDlg->m_pCurrentGroup->nGroupID;
				sElectOmr.sElectOmrGroupInfo.nAllCount = m_pElectOmrDlg->m_pCurrentGroup->nAllCount;
				sElectOmr.sElectOmrGroupInfo.nRealCount = m_pElectOmrDlg->m_pCurrentGroup->nRealCount;
				sElectOmr.lItemInfo.push_back(m_vecTmp[i]);
				m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.push_back(sElectOmr);
			}
		}
	}
	m_nStartTH += nAddTH - 1;

	if (m_eCurCPType == OMR)
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.end(), SortByOmrTH);

	m_vecTmp.clear();
	UpdataCPList();
	ShowRectByCPType(m_eCurCPType);

#ifdef Test_TraceLog
	float fMeanArea[7] = { 0 };
	float fMeanDensity[7] = { 0 };
	float fMeanVal[7] = { 0 };
	for (auto pstItem : m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2)
	{
		std::string strRectVal;
		for (auto rc : pstItem.lSelAnswer)
		{
			fMeanArea[rc.nAnswer] += rc.fStandardArea;
			fMeanVal[rc.nAnswer] += rc.fStandardValue;
			fMeanDensity[rc.nAnswer] += rc.fStandardDensity;
			strRectVal.append(Poco::format("项(%c) 面积: %f, 密度: %f, val: %f, valPer: %f; ", (char)(rc.nAnswer + 65), (double)rc.fStandardArea, (double)rc.fStandardDensity, (double)rc.fStandardValue, (double)rc.fStandardValuePercent));
		}
		std::string strRectLog = Poco::format("矩形题号: %d, %s", pstItem.nTH, strRectVal);
		TRACE("%s\n", strRectLog.c_str());
	}
	int nOmrCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecOmr2.size();
	for (int i = 0; i < 7; i++)
		TRACE("OMR平均值信息：%c--平均面积= %f, 平均值= %f, 平均密度= %f\n", i + 65, fMeanArea[i] / nOmrCount, fMeanVal[i] / nOmrCount, fMeanDensity[i] / nOmrCount);
#endif
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
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows - rt.y;
		Recognise(rt);
		
		m_vecPaperModelInfo[m_nCurrTabSel]->vecHTracker.clear();
		RECTINFO rcHTrackerSel;					//水平橡皮筋的区域
		rcHTrackerSel.eCPType = m_eCurCPType;
		
		rcHTrackerSel.nGaussKernel = m_nGaussKernel;
		rcHTrackerSel.nSharpKernel = m_nSharpKernel;
		rcHTrackerSel.nCannyKernel = m_nCannyKernel;
		rcHTrackerSel.nDilateKernel = m_nDilateKernel;

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
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows - rt.y;
		Recognise(rt);

		m_vecPaperModelInfo[m_nCurrTabSel]->vecVTracker.clear();
		RECTINFO rcVTrackerSel;					//垂直橡皮筋的区域
		rcVTrackerSel.eCPType = m_eCurCPType;

		rcVTrackerSel.nGaussKernel = m_nGaussKernel;
		rcVTrackerSel.nSharpKernel = m_nSharpKernel;
		rcVTrackerSel.nCannyKernel = m_nCannyKernel;
		rcVTrackerSel.nDilateKernel = m_nDilateKernel;

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
		if (rt.br().x > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols)
			rt.width = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.cols - rt.x;
		if (rt.br().y > m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows)
			rt.height = m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg.rows - rt.y;
		Recognise(rt);
		
		ShowTmpRect();
		m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.nDilateKernel = m_nDilateKernel;
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
#ifdef Test_TraceLog
		for (auto pstItem : m_vecPaperModelInfo[m_nCurrTabSel]->lSN)
		{
			std::string strRectVal;
			for (auto rc : pstItem->lSN)
				strRectVal.append(Poco::format("项(%d) 面积: %f, 密度: %f, val: %f, valPer: %f; ", rc.nSnVal, (double)rc.fStandardArea, (double)rc.fStandardDensity, (double)rc.fStandardValue, (double)rc.fStandardValuePercent));
			std::string strRectLog = Poco::format("矩形题号: %d, %s", pstItem->nItem, strRectVal);
			TRACE("%s\n", strRectLog.c_str());
		}
#endif
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
	else if (m_eCurCPType == ELECT_OMR)
	{
		int nElectOmr = 0;
		for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); i++)
		{
			nElectOmr = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.size();
			if (nItem < nElectOmr)
			{
				nItem = i;
				break;
			}
			else
				nItem -= nElectOmr;
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
	case PAGINATION:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.erase(it);
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
	case WJ_CP:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size() < 0)
			return FALSE;
		it = m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.begin() + nItem;
		if (it != m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.end())
			m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.erase(it);
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
	case SN:
		if (m_vecPaperModelInfo[m_nCurrTabSel]->lSN.size() < 0)
			return FALSE;

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
	case ELECT_OMR:
		{
			if (m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size() < 0)
				return FALSE;
			std::vector<ELECTOMR_QUESTION>::iterator itElectOmr = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.begin() + nItem;
			if (itElectOmr != m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.end())
				m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.erase(itElectOmr);
		}
		break;
	case CHARACTER_AREA:
		{
			if (m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size() < 0)
				return FALSE;
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
			{
				int nCharacterCount = 0;
				nCharacterCount = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size();
				if (nItem < nCharacterCount)
				{
					std::vector<pST_CHARACTER_ANCHOR_POINT>::iterator itCharacter = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.begin() + nItem;
					if (itCharacter != m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.end())
						m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.erase(itCharacter);

					if (m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size() == 0)
					{
						std::vector<pST_CHARACTER_ANCHOR_AREA>::iterator itCharacterArea = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.begin() + i;
						if (itCharacterArea != m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.end())
							m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.erase(itCharacterArea);
					}
					break;
				}
				else
					nItem -= nCharacterCount;
			}
		}
		break;
	default: return FALSE;
	}
	return TRUE;
}

BOOL CMakeModelDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
// 	if (ProcessMessage(*pMsg))
// 		return TRUE;
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
	if (pNMItemActivate->iItem < 0)
		return;

	if (m_nCurListCtrlSel < m_cpListCtrl.GetItemCount())
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
	case PAGINATION:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.end(), SortByPositionX);
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
	case WJ_CP:
		std::sort(m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.begin(), m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.end(), SortByPositionX);
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
		case PAGINATION:
			if (eType == PAGINATION || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size(); i++)
					{
						if (m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[i].rt.contains(pt))
						{
							nFind = i;
							pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[i];
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
		case WJ_CP:
			if (eType == WJ_CP || eType == UNKNOWN)
			{
				if (nFind < 0)
				{
					for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size(); i++)
					{
						if (m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[i].rt.contains(pt))
						{
							nFind = i;
							pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[i];
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
	case ELECT_OMR:
		if (eType == ELECT_OMR || eType == UNKNOWN)
		{
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); i++)
			{
				RECTLIST::iterator itElectOmr = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.begin();
				for (; itElectOmr != m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[i].lItemInfo.end(); itElectOmr++)
				{
					if (itElectOmr->rt.contains(pt))
					{
						nFind = i;
						pRc = &(*itElectOmr);
						break;
					}					
				}
				if (nFind > 0) break;
			}
		}
	case CHARACTER_AREA:
		if (eType == CHARACTER_AREA || eType == UNKNOWN)
		{
			int nCount = 0;
			for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
			{
				for (int j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size(); j++)
				{
					if (m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc.rt.contains(pt))
					{
						nFind = nCount + j;
						pRc = &m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc;
						break;
					}
				}
				if(nFind > 0) break;
				nCount += m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size();
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

	Mat inputImg = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg;	//m_vecPaperModelInfo[m_nCurrTabSel]->matSrcImg
	Mat tmpImg;
	warpAffine(inputImg, tmpImg, affine_matrix, inputImg.size());
	namedWindow("warpAffine", 0);
	imshow("warpAffine", tmpImg);
	m_pModelPicShow->ShowPic(tmpImg);

	return true;
}

LRESULT CMakeModelDlg::HTrackerChange(WPARAM wParam, LPARAM lParam)
{
	m_ptHTracker1 = GetSrcSavePoint(m_pModelPicShow->m_picShow.m_ptHTracker1);
	m_ptHTracker2 = GetSrcSavePoint(m_pModelPicShow->m_picShow.m_ptHTracker2);
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
	m_ptVTracker1 = GetSrcSavePoint(m_pModelPicShow->m_picShow.m_ptVTracker1);
	m_ptVTracker2 = GetSrcSavePoint(m_pModelPicShow->m_picShow.m_ptVTracker2);
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
	m_ptSNTracker1 = GetSrcSavePoint(m_pModelPicShow->m_picShow.m_ptSNTracker1);
	m_ptSNTracker2 = GetSrcSavePoint(m_pModelPicShow->m_picShow.m_ptSNTracker2);
	ShowRectTracker();

	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return true;
// 	m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.x = m_ptSNTracker1.x;
// 	m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.y = m_ptSNTracker1.y;
// 	m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.width = m_ptSNTracker2.x - m_ptSNTracker1.x;
// 	m_vecPaperModelInfo[m_nCurrTabSel]->rtSNTracker.height = m_ptSNTracker2.y - m_ptSNTracker1.y;
	m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.rt.x = m_ptSNTracker1.x;
	m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.rt.y = m_ptSNTracker1.y;
	m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.rt.width = m_ptSNTracker2.x - m_ptSNTracker1.x;
	m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.rt.height = m_ptSNTracker2.y - m_ptSNTracker1.y;
	
	m_pSNInfoDlg->ShowUI(GetSnShowFakePosFlag(m_vecPaperModelInfo[m_nCurrTabSel]->rcSNTracker.nRecogFlag));
	return true;
}

void CMakeModelDlg::GetSNArry(std::vector<cv::Rect>& rcList)
{
	m_vecTmp.clear();
	if (rcList.size() <= 0)
		return;
	int nMaxRow = 1;
	int nMaxCols = 1;

	//++计算平均的矩形大小，过滤污染点
	int nMeanW = 0;
	int nMeanH = 0;
	for (int i = 0; i < rcList.size(); i++)
	{
		nMeanW += rcList[i].width;
		nMeanH += rcList[i].height;
	}
	nMeanW = (float)nMeanW / rcList.size() + 0.5;
	nMeanH = (float)nMeanH / rcList.size() + 0.5;
	std::vector<cv::Rect>::iterator itTmp = rcList.begin();
	for (; itTmp != rcList.end();)
	{
		if (itTmp->width < nMeanW * 0.6 && itTmp->height < nMeanH * 0.6)
		{
			TRACE("erase rect: (%d, %d, %d, %d)\n", itTmp->x, itTmp->y, itTmp->width, itTmp->height);
			itTmp = rcList.erase(itTmp);
		}
		else
			itTmp++;
	}
	//--

	std::vector<Rect> rcList_X = rcList;
	std::vector<Rect> rcList_XY = rcList;
	std::sort(rcList_X.begin(), rcList_X.end(), SortByPositionX2);


	int nW = rcList_X[0].width;				//矩形框平均宽度
	int nH = rcList_X[0].height;			//矩形框平均高度
	int nWInterval = 0;						//矩形间的X轴平均间隔
	int nHInterval = 0;						//矩形间的Y轴平均间隔

#if 1
	int nX, nY;
	switch (m_pSNInfoDlg->m_nCurrentSNVal)
	{
		case 10:	//1010
			nX = rcList_X[0].width * 0.2 + 0.5;		//判断属于同一列的X轴偏差
			nY = rcList_X[0].height * 0.3 + 0.5;	//判断属于同一行的Y轴偏差
			break;
		case 9:		//1001
			nX = rcList_X[0].width * 0.2 + 0.5;		//判断属于同一列的X轴偏差
			nY = rcList_X[0].height * 0.3 + 0.5;	//判断属于同一行的Y轴偏差
			break;
		case 6:		//0110
			nY = rcList_X[0].width * 0.2 + 0.5;		//判断属于同一列的X轴偏差
			nX = rcList_X[0].height * 0.3 + 0.5;	//判断属于同一行的Y轴偏差
			break;
		case 5:		//0101
			nY = rcList_X[0].width * 0.2 + 0.5;		//判断属于同一列的X轴偏差
			nX = rcList_X[0].height * 0.3 + 0.5;	//判断属于同一行的Y轴偏差
			break;
	}
#else
	int nX = rcList_X[0].width * 0.2 + 0.5;		//判断属于同一列的X轴偏差
	int nY = rcList_X[0].height * 0.3 + 0.5;	//判断属于同一行的Y轴偏差
#endif

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

			if (dy > 9)
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
		rc.rt = GetSrcSaveRect(rcList_XY[i]);
		rc.eCPType = m_eCurCPType;

		rc.nGaussKernel = m_nGaussKernel;
		rc.nSharpKernel = m_nSharpKernel;
		rc.nCannyKernel = m_nCannyKernel;
		rc.nDilateKernel = m_nDilateKernel;

		rc.nThresholdValue = m_nSN;
		rc.fStandardValuePercent = m_fSNThresholdPercent_Fix;
		rc.nRecogFlag = GetSnSavePosFlag(m_pSNInfoDlg->m_nCurrentSNVal);

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
		
		Rect rtTmp = rcList_XY[i];
		Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
		RecogGrayValue(matSrcModel, rc);
		if (rc.fStandardDensity < 0.1)
		{
			CString strInfo = _T("");
			strInfo.Format(_T("第%d个考号的选项%d密度值太低，可能阀值设置太低"), rc.nTH, rc.nSnVal);
			USES_CONVERSION;
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, T2A(strInfo));
			dlg.DoModal();
		}

		m_vecTmp.push_back(rc);
	}

	//检查矩形重叠，并进行提醒
	if (m_vecTmp.size() > 2)
	{
		bool bOverlap = false;
		std::vector<RECTINFO>::iterator itTmpRC1 = m_vecTmp.begin();
		for (; itTmpRC1 != m_vecTmp.end();)
		{
			std::vector<RECTINFO>::iterator itTmpRC2 = itTmpRC1 + 1;
			for (; itTmpRC2 != m_vecTmp.end(); itTmpRC2++)
			{
				cv::Point pt1 = itTmpRC2->rt.tl();
				cv::Point pt2 = itTmpRC2->rt.br();
				cv::Point pt3 = cv::Point(itTmpRC2->rt.x + itTmpRC2->rt.width, itTmpRC2->rt.y);
				cv::Point pt4 = cv::Point(itTmpRC2->rt.x, itTmpRC2->rt.y + itTmpRC2->rt.height);

				cv::Point pt5 = itTmpRC1->rt.tl();
				cv::Point pt6 = itTmpRC1->rt.br();
				cv::Point pt7 = cv::Point(itTmpRC1->rt.x + itTmpRC1->rt.width, itTmpRC1->rt.y);
				cv::Point pt8 = cv::Point(itTmpRC1->rt.x, itTmpRC1->rt.y + itTmpRC1->rt.height);
				//if (itTmpRC1->rt.contains(itTmpRC2->rt.tl()) || itTmpRC1->rt.contains(itTmpRC2->rt.br()) || itTmpRC2->rt.contains(itTmpRC1->rt.tl()) || itTmpRC2->rt.contains(itTmpRC1->rt.br()))
				if (itTmpRC1->rt.contains(pt1) || itTmpRC1->rt.contains(pt2) || itTmpRC1->rt.contains(pt3) || itTmpRC1->rt.contains(pt4) \
					|| itTmpRC2->rt.contains(pt5) || itTmpRC2->rt.contains(pt6) || itTmpRC2->rt.contains(pt7) || itTmpRC2->rt.contains(pt8))
				{
					bOverlap = true;
					break;
				}
			}
			if (bOverlap)
				break;
			else
				itTmpRC1++;
		}
		if (bOverlap)
		{
			std::string strOverlapTip = "检测到识别的矩形中有重叠的矩形，请仔细查看";
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, strOverlapTip);
			dlg.DoModal();
		}
	}

	std::sort(m_vecTmp.begin(), m_vecTmp.end(), SortByTH);

	//SN漏点检查
	if (m_pModel->nZkzhType == 1)
	{
		std::map<int, int> mapTH;
		for (int i = 0; i < m_vecTmp.size(); i++)
		{
			bool bFind = false;
			std::map<int, int>::iterator itMap = mapTH.begin();
			for (; itMap != mapTH.end(); itMap++)
			{
				if (m_vecTmp[i].nTH == (int)itMap->first)
				{
					bFind = true;
					itMap->second++;
					break;
				}
			}
			if (!bFind)
				mapTH.insert(std::pair<int, int>(m_vecTmp[i].nTH, 1));
		}
		std::map<int, int>::iterator itMap = mapTH.begin();
		for (; itMap != mapTH.end(); itMap++)
		{
			if (itMap->second != 10)
			{
				char szTmp[100] = { 0 };
				sprintf_s(szTmp, "第 %d 列考号数为%d个，正常应该有10个，请检查!", (int)itMap->first, itMap->second);
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, szTmp);
				dlg.DoModal();
			}
		}
	}

	ShowTmpRect();
}

void CMakeModelDlg::GetOmrArry(std::vector<cv::Rect>& rcList)
{
	m_vecTmp.clear();
	if (rcList.size() <= 0)
		return;
	int nMaxRow		= 1;
	int nMaxCols	= 1;

	//++计算平均的矩形大小，过滤污染点
	int nMeanW = 0;
	int nMeanH = 0;
	for (int i = 0; i < rcList.size(); i++)
	{
		nMeanW += rcList[i].width;
		nMeanH += rcList[i].height;
	}
	nMeanW = (float)nMeanW / rcList.size() + 0.5;
	nMeanH = (float)nMeanH / rcList.size() + 0.5;
	std::vector<cv::Rect>::iterator itTmp = rcList.begin();
	for (; itTmp != rcList.end();)
	{
		if (itTmp->width < nMeanW * 0.7 && itTmp->height < nMeanH * 0.7)
		{
			TRACE("erase rect: (%d, %d, %d, %d)\n", itTmp->x, itTmp->y, itTmp->width, itTmp->height);
			itTmp = rcList.erase(itTmp);
		}
		else
			itTmp++;
	}
	//--

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
	if (nX < 9) nX = 9;
	if (nY < 9) nY = 9;


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

	std::vector<int> vecLowThresholds;	//检查计算的密度阀值太低(接近0)的题号，后面删除此题号矩形
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

			if (dy > 9)
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

		TRACE("第几行几列: %d行%d列, 差值: x-%d, y-%d\n", y, x, rcList_XY[i].x - rcList_XY[0].x, rcList_XY[i].y - rcList_XY[0].y);

		RECTINFO rc;
		rc.rt = GetSrcSaveRect(rcList_XY[i]);
		rc.eCPType = m_eCurCPType;

		rc.nGaussKernel = m_nGaussKernel;
		rc.nSharpKernel = m_nSharpKernel;
		rc.nCannyKernel = m_nCannyKernel;
		rc.nDilateKernel = m_nDilateKernel;

		rc.nThresholdValue = m_nOMR;
		rc.fStandardValuePercent = m_fOMRThresholdPercent_Fix;
		rc.nRecogFlag = GetOmrSavePosFlag(m_pOmrInfoDlg->m_nCurrentOmrVal);

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
			rc.nTH = nMaxRow - y - 1;	//nMaxCols - y - 1;
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
		rc.nSingle = m_pOmrInfoDlg->m_nSingle;
		Rect rtTmp = rcList_XY[i];
		Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
		Mat matTest = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg;
		RecogGrayValue(matSrcModel, rc);
		if (rc.fStandardDensity < 0.1)
		{
			bool bFind = false;
			for (auto nItem : vecLowThresholds)
			{
				if (rc.nTH == nItem)
				{
					bFind = true;
					break;
				}
			}
			if (!bFind)
				vecLowThresholds.push_back(rc.nTH);

			CString strInfo = _T("");
			strInfo.Format(_T("第%d题的选项%c密度值太低，可能阀值设置太低"), rc.nTH, rc.nAnswer + 65);

			USES_CONVERSION;
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, T2A(strInfo));
			dlg.DoModal();
		}
		m_vecTmp.push_back(rc);
	}

	//将已经提示密度值太低的题号矩形删除
	for (auto nItem : vecLowThresholds)
	{
		std::vector<RECTINFO>::iterator itTmpRC = m_vecTmp.begin();
		for (; itTmpRC != m_vecTmp.end();)
		{
			if (itTmpRC->nTH == nItem)
			{
				TRACE("erase 密度太低矩形: th = %d, (%d, %d, %d, %d)\n", itTmpRC->nTH, itTmpRC->rt.x, itTmpRC->rt.y, itTmpRC->rt.width, itTmpRC->rt.height);
				itTmpRC = m_vecTmp.erase(itTmpRC);
			}
			else
				itTmpRC++;
		}
	}

	//检查矩形重叠，并进行提醒
	if (m_vecTmp.size() > 2)
	{
		bool bOverlap = false;
		std::vector<RECTINFO>::iterator itTmpRC1 = m_vecTmp.begin();
		for (; itTmpRC1 != m_vecTmp.end();)
		{
			std::vector<RECTINFO>::iterator itTmpRC2 = itTmpRC1 + 1;
			for (; itTmpRC2 != m_vecTmp.end(); itTmpRC2++)
			{
				cv::Point pt1 = itTmpRC2->rt.tl();
				cv::Point pt2 = itTmpRC2->rt.br();
				cv::Point pt3 = cv::Point(itTmpRC2->rt.x + itTmpRC2->rt.width, itTmpRC2->rt.y);
				cv::Point pt4 = cv::Point(itTmpRC2->rt.x, itTmpRC2->rt.y + itTmpRC2->rt.height);

				cv::Point pt5 = itTmpRC1->rt.tl();
				cv::Point pt6 = itTmpRC1->rt.br();
				cv::Point pt7 = cv::Point(itTmpRC1->rt.x + itTmpRC1->rt.width, itTmpRC1->rt.y);
				cv::Point pt8 = cv::Point(itTmpRC1->rt.x, itTmpRC1->rt.y + itTmpRC1->rt.height);
				//if (itTmpRC1->rt.contains(itTmpRC2->rt.tl()) || itTmpRC1->rt.contains(itTmpRC2->rt.br()) || itTmpRC2->rt.contains(itTmpRC1->rt.tl()) || itTmpRC2->rt.contains(itTmpRC1->rt.br()))
				if (itTmpRC1->rt.contains(pt1) || itTmpRC1->rt.contains(pt2) || itTmpRC1->rt.contains(pt3) || itTmpRC1->rt.contains(pt4) \
					|| itTmpRC2->rt.contains(pt5) || itTmpRC2->rt.contains(pt6) || itTmpRC2->rt.contains(pt7) || itTmpRC2->rt.contains(pt8))				
				{
					bOverlap = true;
					break;
				}
			}
			if (bOverlap)
				break;
			else
				itTmpRC1++;
		}
		if (bOverlap)
		{
			std::string strOverlapTip = "检测到识别的矩形中有重叠的矩形，请仔细查看";
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, strOverlapTip);
			dlg.DoModal();
		}
	}

	std::sort(m_vecTmp.begin(), m_vecTmp.end(), SortByTH);
	if (m_pOmrInfoDlg->m_nSingle == 2)
	{
		//判断题检查，选项数必须为2
		int nOptions = 0;
		for (int i = 0; i < m_vecTmp.size(); i++)
		{
			if (i == 0)
			{
				nOptions++;
				continue;
			}
			if (m_vecTmp[i].nTH == m_vecTmp[i - 1].nTH)
				nOptions++;
			else
			{
				if (nOptions == 2)	//上一题的选项数为2，当前题的选项数置1
					nOptions = 1;
				else
					break;	//上一题的选项数!=2
			}
		}
		if (nOptions != 2)	//最后一题的选项判断也在这里处理
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "判断题的选项数不符合条件");
			dlg.DoModal();

			m_vecTmp.clear();
			return;
		}
	}

	//omr漏点检查
	std::map<int, int> mapTH;
	for (int i = 0; i < m_vecTmp.size(); i++)
	{
		bool bFind = false;
		std::map<int, int>::iterator itMap = mapTH.begin();
		for(; itMap != mapTH.end(); itMap++)
		{
			if (m_vecTmp[i].nTH == (int)itMap->first)
			{
				bFind = true;
				itMap->second++;
				break;
			}
		}
		if (!bFind)
			mapTH.insert(std::pair<int, int>(m_vecTmp[i].nTH, 1));
	}
	std::map<int, int>::iterator itMap = mapTH.begin();
	for (; itMap != mapTH.end(); itMap++)
	{
		for (int i = 0; i < m_vecTmp.size(); i++)
		{
			if ((int)itMap->first == m_vecTmp[i].nTH)
			{
				if (m_vecTmp[i].nAnswer > itMap->second - 1)
				{
					char szTmp[100] = { 0 };
					sprintf_s(szTmp, "第 %d 题选项数为%d个,实际选项顺序超出，请检查!", m_vecTmp[i].nTH, itMap->second);
					CNewMessageBox dlg;
					dlg.setShowInfo(2, 1, szTmp);
					dlg.DoModal();
					break;
				}
			}
		}
	}

	ShowTmpRect();
}

void CMakeModelDlg::GetElectOmrInfo(std::vector<cv::Rect>& rcList)
{
	m_vecTmp.clear();
	if (rcList.size() <= 0)
		return;
	int nMaxRow = 1;
	int nMaxCols = 1;

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
	if (nMaxCols > 1 && nMaxRow > 1)
	{
//		AfxMessageBox(_T("选择区域不合法，请重新选择！"));
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "选择区域不合法，请重新选择！");
		dlg.DoModal();
		return;
	}

	//如果选择的区域中矩形个数超过当前组的总选项数，报错

	//m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr
	if (!m_pElectOmrDlg->m_pCurrentGroup)
	{
//		AfxMessageBox(_T("当前选做题信息不存在，请新建!"));
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "当前选做题信息不存在，请新建！");
		dlg.DoModal();
		return;
	}
	if (!m_pElectOmrDlg->checkValid())
	{
//		AfxMessageBox(_T("当前选做题信息未保存，请先保存此题选做题信息!"));
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "当前选做题信息未保存，请先保存！");
		dlg.DoModal();
		return;
	}
	if (m_pElectOmrDlg->m_pCurrentGroup->nAllCount < rcList_XY.size())
	{
//		AfxMessageBox(_T("识别出的选项数超出范围!"));
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "识别出的选项数超出范围！");
		dlg.DoModal();
		return;
	}
	
	//按顺序设置题号和位置信息
	for (int i = 0; i < rcList_XY.size(); i++)
	{
		RECTINFO rc;
		rc.rt = GetSrcSaveRect(rcList_XY[i]);
		rc.eCPType = m_eCurCPType;

		rc.nGaussKernel = m_nGaussKernel;
		rc.nSharpKernel = m_nSharpKernel;
		rc.nCannyKernel = m_nCannyKernel;
		rc.nDilateKernel = m_nDilateKernel;

		rc.nThresholdValue = m_nOMR;
		rc.fStandardValuePercent = m_fOMRThresholdPercent_Fix;
		rc.nTH = m_pElectOmrDlg->m_pCurrentGroup->nGroupID;
		rc.nAnswer = i;

		Rect rtTmp = rcList_XY[i];
		Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
		RecogGrayValue(matSrcModel, rc);

		m_vecTmp.push_back(rc);
	}
	std::sort(m_vecTmp.begin(), m_vecTmp.end(), SortByTH);

//	ShowTmpRect();

	for (int i = 0; i < m_vecTmp.size(); i++)
	{
		bool bError = false;
		bool bFind = false;
		for (int k = 0; k < m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.size(); k++)
		{
			if (m_vecTmp[i].nTH == m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].sElectOmrGroupInfo.nGroupID)
			{
				if (m_vecTmp.size() - i + m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].lItemInfo.size() > m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].sElectOmrGroupInfo.nAllCount)
				{
					bError = true;
					break;
				}

				bFind = true;
				m_vecTmp[i].nAnswer = m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].lItemInfo.size();		//修改内部题号，根据当前列表中已经存在的个数来算
				m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr[k].lItemInfo.push_back(m_vecTmp[i]);
				break;
			}
		}
		if (bError)
		{
//			AfxMessageBox(_T("此组选做题实际选项总数超出范围"));
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "此组选做题实际选项总数超出范围");
			dlg.DoModal();
			break;
		}
		if (!bFind)
		{
			ELECTOMR_QUESTION sElectOmr;
			sElectOmr.sElectOmrGroupInfo.nGroupID = m_pElectOmrDlg->m_pCurrentGroup->nGroupID;
			sElectOmr.sElectOmrGroupInfo.nAllCount = m_pElectOmrDlg->m_pCurrentGroup->nAllCount;
			sElectOmr.sElectOmrGroupInfo.nRealCount = m_pElectOmrDlg->m_pCurrentGroup->nRealCount;
			sElectOmr.lItemInfo.push_back(m_vecTmp[i]);
			m_vecPaperModelInfo[m_nCurrTabSel]->vecElectOmr.push_back(sElectOmr);
		}
	}
	m_vecTmp.clear();
}

void CMakeModelDlg::setUploadModelInfo(CString& strName, CString& strModelPath, int nExamId, int nSubjectId)
{
	USES_CONVERSION;
	std::string strPath = T2A(strModelPath);
	std::string strMd5;

	strMd5 = calcFileMd5(strPath);

	CString strUser = _T("");
	CString strEzs = _T("");

	strUser = A2T(_strUserName_.c_str());
	strEzs = A2T(_strEzs_.c_str());
	
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


// void CMakeModelDlg::OnBnClickedBtnuploadmodel()
// {
// 	CFileDialog dlg(TRUE,
// 					NULL,
// 					NULL,
// 					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
// 					_T("All Files (*.*)|*.*;)||"),
// 					NULL);
// 	if (dlg.DoModal() != IDOK)
// 		return;
// 
// 	USES_CONVERSION;
// 	std::string strJsnModel = T2A(dlg.GetPathName());
// 
// 	std::string strJsnData;
// 	std::ifstream in(strJsnModel);
// 	if (!in)
// 		return;
// 
// 	std::string strJsnLine;
// 	while (!in.eof())
// 	{
// 		getline(in, strJsnLine);					//不过滤空格
// 		strJsnData.append(strJsnLine);
// 	}
// 	in.close();
// 		
// 	m_pModel = LoadMakePaperData(strJsnData);
// 	if (!m_pModel)	return;
// 
// 	//++ 将PDF转JPG
// 	int nPos1 = strJsnModel.rfind('\\');
// 	int nPos2 = strJsnModel.rfind('.');
// 	std::string strBaseName = strJsnModel.substr(nPos1 + 1, nPos2 - nPos1 - 1);
// 	std::string strPdfPath = T2A(dlg.GetFolderPath());
// 	strPdfPath.append("\\" + strBaseName + ".pdf");
// 
// 	bool bResult = Pdf2Jpg(strPdfPath, m_pModel->strModelName);
// 	if (!bResult)
// 	{
// 		AfxMessageBox(_T("pdf转jpg失败"));
// 		return;
// 	}
// 	//--
// 
// 	InitModelRecog(m_pModel);
// 
// 	m_nModelPicNums = m_pModel->nPicNum;
// 	InitTab();
// 	InitConf();
// 	m_vecPaperModelInfo.clear();
// 	for (int i = 0; i < m_pModel->nPicNum; i++)
// 	{
// 		CString strPicPath = g_strCurrentPath + _T("Model\\") + A2T(m_pModel->strModelName.c_str()) + _T("\\") + A2T(m_pModel->vecPaperModel[i]->strModelPicName.c_str());
// 
// 		pPaperModelInfo pPaperModel = new PaperModelInfo;
// 		m_vecPaperModelInfo.push_back(pPaperModel);
// 		pPaperModel->nPaper = i;
// 		pPaperModel->strModelPicPath = strPicPath;
// 		pPaperModel->strModelPicName = m_pModel->vecPaperModel[i]->strModelPicName;
// 
// 		pPaperModel->matSrcImg = imread((std::string)(CT2CA)strPicPath);
// 		pPaperModel->matDstImg = pPaperModel->matSrcImg;
// 
// 		Mat src_img;
// 		src_img = m_vecPaperModelInfo[i]->matDstImg;
// 		m_vecPicShow[i]->ShowPic(src_img);
// 		
// 		pPaperModel->nPicW = src_img.cols;
// 		pPaperModel->nPicH = src_img.rows;
// 
// 		pPaperModel->rtHTracker = m_pModel->vecPaperModel[i]->rtHTracker;
// 		pPaperModel->rtVTracker = m_pModel->vecPaperModel[i]->rtVTracker;
// //		pPaperModel->rtSNTracker = m_pModel->vecPaperModel[i]->rtSNTracker;
// 		pPaperModel->rcSNTracker = m_pModel->vecPaperModel[i]->rcSNTracker;
// 
// 		RECTLIST::iterator itSelHTracker = m_pModel->vecPaperModel[i]->lSelHTracker.begin();
// 		for (; itSelHTracker != m_pModel->vecPaperModel[i]->lSelHTracker.end(); itSelHTracker++)
// 		{
// 			pPaperModel->vecHTracker.push_back(*itSelHTracker);
// 		}
// 		RECTLIST::iterator itSelVTracker = m_pModel->vecPaperModel[i]->lSelVTracker.begin();
// 		for (; itSelVTracker != m_pModel->vecPaperModel[i]->lSelVTracker.end(); itSelVTracker++)
// 		{
// 			pPaperModel->vecVTracker.push_back(*itSelVTracker);
// 		}
// 		RECTLIST::iterator itSelRoi = m_pModel->vecPaperModel[i]->lSelFixRoi.begin();
// 		for (; itSelRoi != m_pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++)
// 		{
// 			pPaperModel->vecRtSel.push_back(*itSelRoi);
// 		}
// 		OMRLIST::iterator itOmr2 = m_pModel->vecPaperModel[i]->lOMR2.begin();
// 		for (; itOmr2 != m_pModel->vecPaperModel[i]->lOMR2.end(); itOmr2++)
// 		{
// 			pPaperModel->vecOmr2.push_back(*itOmr2);
// 		}
// 		RECTLIST::iterator itFix = m_pModel->vecPaperModel[i]->lFix.begin();
// 		for (; itFix != m_pModel->vecPaperModel[i]->lFix.end(); itFix++)
// 		{
// 			pPaperModel->vecRtFix.push_back(*itFix);
// 		}
// 		RECTLIST::iterator itHHead = m_pModel->vecPaperModel[i]->lH_Head.begin();
// 		for (; itHHead != m_pModel->vecPaperModel[i]->lH_Head.end(); itHHead++)
// 		{
// 			pPaperModel->vecH_Head.push_back(*itHHead);
// 		}
// 		RECTLIST::iterator itVHead = m_pModel->vecPaperModel[i]->lV_Head.begin();
// 		for (; itVHead != m_pModel->vecPaperModel[i]->lV_Head.end(); itVHead++)
// 		{
// 			pPaperModel->vecV_Head.push_back(*itVHead);
// 		}
// 		RECTLIST::iterator itABModel = m_pModel->vecPaperModel[i]->lABModel.begin();
// 		for (; itABModel != m_pModel->vecPaperModel[i]->lABModel.end(); itABModel++)
// 		{
// 			pPaperModel->vecABModel.push_back(*itABModel);
// 		}
// 		RECTLIST::iterator itCourse = m_pModel->vecPaperModel[i]->lCourse.begin();
// 		for (; itCourse != m_pModel->vecPaperModel[i]->lCourse.end(); itCourse++)
// 		{
// 			pPaperModel->vecCourse.push_back(*itCourse);
// 		}
// 		RECTLIST::iterator itQK = m_pModel->vecPaperModel[i]->lQK_CP.begin();
// 		for (; itQK != m_pModel->vecPaperModel[i]->lQK_CP.end(); itQK++)
// 		{
// 			pPaperModel->vecQK_CP.push_back(*itQK);
// 		}
// 		RECTLIST::iterator itGray = m_pModel->vecPaperModel[i]->lGray.begin();
// 		for (; itGray != m_pModel->vecPaperModel[i]->lGray.end(); itGray++)
// 		{
// 			pPaperModel->vecGray.push_back(*itGray);
// 		}
// 		RECTLIST::iterator itWhite = m_pModel->vecPaperModel[i]->lWhite.begin();
// 		for (; itWhite != m_pModel->vecPaperModel[i]->lWhite.end(); itWhite++)
// 		{
// 			pPaperModel->vecWhite.push_back(*itWhite);
// 		}
// 		SNLIST::iterator itSn = m_pModel->vecPaperModel[i]->lSNInfo.begin();
// 		for (; itSn != m_pModel->vecPaperModel[i]->lSNInfo.end(); itSn++)
// 		{
// 			pSN_ITEM pSnItem = new SN_ITEM;
// 			pSnItem->nItem = (*itSn)->nItem;
// 			pSnItem->nRecogVal = (*itSn)->nRecogVal;
// 			RECTLIST::iterator itRc = (*itSn)->lSN.begin();
// 			for (; itRc != (*itSn)->lSN.end(); itRc++)
// 			{
// 				RECTINFO rc = *itRc;
// 				pSnItem->lSN.push_back(rc);
// 			}
// 			pPaperModel->lSN.push_back(pSnItem);
// 		}
// 
// 		ShowRectByCPType(m_eCurCPType);
// 		UpdataCPList();
// 	}
// }

// void CMakeModelDlg::CopyImage(HANDLE hBitmap, TW_IMAGEINFO& info)
// {
// 	SetImage(hBitmap, info.BitsPerPixel);
// }
// 
// void CMakeModelDlg::SetImage(HANDLE hBitmap, int bits)
// {
// 	CDIB dib;
// 	dib.CreateFromHandle(hBitmap, bits);
// 
// 	BITMAPFILEHEADER bFile;
// 	::ZeroMemory(&bFile, sizeof(bFile));
// 	memcpy((void *)&bFile.bfType, "BM", 2);
// 	bFile.bfSize = dib.GetDIBSize() + sizeof(bFile);
// 	bFile.bfOffBits = sizeof(BITMAPINFOHEADER) + dib.GetPaletteSize()*sizeof(RGBQUAD) + sizeof(BITMAPFILEHEADER);
// 	unsigned char *pBits = NULL;
// 	try
// 	{
// 		pBits = (unsigned char *)malloc(bFile.bfSize);
// 	}
// 	catch (...)
// 	{
// 		char szLog[100] = { 0 };
// 		sprintf_s(szLog, "获取内存失败。");
// 		g_pLogger->information(szLog);
// 		return;
// 	}
// 
// 	memcpy(pBits, &bFile, sizeof(BITMAPFILEHEADER));
// 	memcpy(pBits + sizeof(BITMAPFILEHEADER), dib.m_pVoid, dib.GetDIBSize());
// 
// 	BYTE *p = pBits;
// 	BITMAPFILEHEADER fheader;
// 	memcpy(&fheader, p, sizeof(BITMAPFILEHEADER));
// 	BITMAPINFOHEADER bmphdr;
// 	p += sizeof(BITMAPFILEHEADER);
// 	memcpy(&bmphdr, p, sizeof(BITMAPINFOHEADER));
// 	int w = bmphdr.biWidth;
// 	int h = bmphdr.biHeight;
// 	p = pBits + fheader.bfOffBits;
// 
// 	int nChannel = (bmphdr.biBitCount == 1) ? 1 : bmphdr.biBitCount / 8;
// 	int depth = (bmphdr.biBitCount == 1) ? IPL_DEPTH_1U : IPL_DEPTH_8U;
// 	IplImage *pIpl2 = cvCreateImage(cvSize(w, h), depth, nChannel);
// 
// 	int height;
// 	bool isLowerLeft = bmphdr.biHeight > 0;
// 	height = (bmphdr.biHeight > 0) ? bmphdr.biHeight : -bmphdr.biHeight;
// 	CopyData(pIpl2->imageData, (char*)p, bmphdr.biSizeImage, isLowerLeft, height);
// 	free(pBits);
// 	pBits = NULL;
// 
// 	// 	IplImage* pIpl = DIB2IplImage(dib);
// 	// 	cv::Mat matTest = cv::cvarrToMat(pIpl);
// 	USES_CONVERSION;
// 
// 	
// 	static int nModelScan = 1;
// 	char szPicPath[MAX_PATH] = { 0 };
// 	sprintf_s(szPicPath, "%s\\model%d.jpg", T2A(m_strScanSavePath), nModelScan);
// 	nModelScan++;
// 
// 
// 	cv::Mat matTest2 = cv::cvarrToMat(pIpl2);
// //	cv::Mat matTest3 = matTest2.clone();
// 
// 	std::string strLog;
// 	std::string strPicName = szPicPath;
// 	try
// 	{
// 		imwrite(strPicName, matTest2);
// 		strLog = "Get model pic: " + strPicName;
// 	}
// 	catch (...)
// 	{
// 		AfxMessageBox(_T("写文件失败"));
// 		strLog = "Get model pic: " + strPicName + " failed.";
// 	}
// 
// 	g_pLogger->information(strLog);
// 
// 	cvReleaseImage(&pIpl2);
// }
// 
// void CMakeModelDlg::ScanDone(int nStatus)
// {
// 	if (nStatus < 0)
// 		return;
// 
// 	TRACE("扫描完成\n");
// 	AfxMessageBox(_T("扫描完成"));
// 	CString strSelect = _T("/root,");
// 	strSelect.Append(m_strScanSavePath);
// 	ShellExecute(NULL, _T("open"), _T("explorer.exe"), strSelect, NULL, SW_SHOWNORMAL);
// }

// BOOL CMakeModelDlg::ScanSrcInit()
// {
// 	USES_CONVERSION;
// 	if (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &m_Source))
// 	{
// 		TW_IDENTITY temp_Source = m_Source;
// 		m_scanSourceArry.Add(temp_Source);
// 		while (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &m_Source))
// 		{
// 			TW_IDENTITY temp_Source = m_Source;
// 			m_scanSourceArry.Add(temp_Source);
// 		}
// 		m_bSourceSelected = TRUE;
// 	}
// 	else
// 	{
// 		m_bSourceSelected = FALSE;
// 	}
// 	return m_bSourceSelected;
// }

void CMakeModelDlg::OnDestroy()
{
	__super::OnDestroy();
	
//	ReleaseTwain();
	if (_pTWAINApp)
	{
		_pTWAINApp->exit();
		SAFE_RELEASE(_pTWAINApp);
	}
#ifdef USE_TESSERACT
	SAFE_RELEASE(m_pTess);
#endif
	SAFE_RELEASE(m_pRecogInfoDlg);
	SAFE_RELEASE(m_pOmrInfoDlg);
	SAFE_RELEASE(m_pSNInfoDlg);
	SAFE_RELEASE(m_pElectOmrDlg);

// 	if (m_bNewModelFlag && !m_bSavedModelFlag && m_pModel != NULL)
// 		SAFE_RELEASE(m_pModel);

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
		case PAGINATION:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecPagination[i].rt;
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
		case WJ_CP:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP.size(); i++)
				{
					cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecWJ_CP[i].rt;
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
		case ELECT_OMR:
			{
				//不进行区域重叠检测
			}
			break;
		case CHARACTER_AREA:
			{
				for (int i = 0; i < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation.size(); i++)
				{
					for (int j = 0; j < m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt.size(); j++)
					{
						cv::Rect rt = m_vecPaperModelInfo[m_nCurrTabSel]->vecCharacterLocation[i]->vecCharacterRt[j]->rc.rt;
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
		m_nDilateKernel = pConf->getInt("MakeModel_Recog.delateKernel", 6);
		m_nErodeKernel = pConf->getInt("MakeModel_Recog.eRodeKernel", 2);

		m_nDilateKernel_DefSn = pConf->getInt("MakeModel_Recog.delateKernel_sn", 6);
		m_nDilateKernel_DefCommon = pConf->getInt("MakeModel_Recog.delateKernel", 6);
		m_nDilateKernel_Sn = m_nDilateKernel_DefSn;
		m_nDilateKernel_Common = m_nDilateKernel_DefCommon;

		m_nWhiteVal = pConf->getInt("MakeModel_Threshold.white", 225);
		m_nHeadVal	= pConf->getInt("MakeModel_Threshold.head", 136);
		m_nPaginationVal = pConf->getInt("MakeModel_Threshold.pagination", 150);
		m_nABModelVal = pConf->getInt("MakeModel_Threshold.abModel", 150);
		m_nCourseVal = pConf->getInt("MakeModel_Threshold.course", 150);
		m_nQK_CPVal = pConf->getInt("MakeModel_Threshold.qk", 150);
		m_nWJ_CPVal = pConf->getInt("MakeModel_Threshold.wj", 150);
		m_nGrayVal	= pConf->getInt("MakeModel_Threshold.gray", 150);
		m_nFixVal	= pConf->getInt("MakeModel_Threshold.fix", 150);
		m_nOMR		= pConf->getInt("MakeModel_Threshold.omr", 230);
		m_nSN		= pConf->getInt("MakeModel_Threshold.sn", 200);
		m_nCharacterThreshold	= pConf->getInt("MakeModel_Threshold.title", 150);

		m_nThreshold_DefFix = m_nFixVal;
		m_nThreshold_DefGray = m_nGrayVal;
		m_nThreshold_DefSn = m_nSN;
		m_nThreshold_DefOmr = m_nOMR;

		m_fHeadThresholdPercent		= pConf->getDouble("MakeModel_RecogPercent_Common.head", 0.75);
		m_fPaginationThresholdPercent = pConf->getDouble("MakeModel_RecogPercent_Common.pagination", 0.75);
		m_fABModelThresholdPercent	= pConf->getDouble("MakeModel_RecogPercent_Common.abModel", 0.75);
		m_fCourseThresholdPercent	= pConf->getDouble("MakeModel_RecogPercent_Common.course", 0.75);		
		m_fFixThresholdPercent		= pConf->getDouble("MakeModel_RecogPercent_Common.fix", 0.8);
		m_fGrayThresholdPercent		= pConf->getDouble("MakeModel_RecogPercent_Common.gray", 0.75);
		m_fWhiteThresholdPercent	= pConf->getDouble("MakeModel_RecogPercent_Common.white", 0.75); 
		
		m_fQK_CPThresholdPercent_Fix	= pConf->getDouble("MakeModel_RecogPercent_Fix.qk", 1.5);
		m_fWJ_CPThresholdPercent_Fix	= pConf->getDouble("MakeModel_RecogPercent_Fix.wj", 1.5);
		m_fOMRThresholdPercent_Fix		= pConf->getDouble("MakeModel_RecogPercent_Fix.omr", 1.5);
		m_fSNThresholdPercent_Fix		= pConf->getDouble("MakeModel_RecogPercent_Fix.sn", 1.5);

		m_fQK_CPThresholdPercent_Head	= pConf->getDouble("MakeModel_RecogPercent_Head.qk", 1.2);
		m_fWJ_CPThresholdPercent_Head = pConf->getDouble("MakeModel_RecogPercent_Head.wj", 1.2);
		m_fOMRThresholdPercent_Head		= pConf->getDouble("MakeModel_RecogPercent_Head.omr", 1.2);
		m_fSNThresholdPercent_Head		= pConf->getDouble("MakeModel_RecogPercent_Head.sn", 1.2);

		m_fDefPersentFix = m_fFixThresholdPercent;
		m_fDefPersentGray = m_fGrayThresholdPercent;
		m_fDefPersentQkWj = m_fQK_CPThresholdPercent_Fix;
		m_fDefPersentSN = m_fSNThresholdPercent_Fix;
		m_fDefPersentOmr = m_fOMRThresholdPercent_Fix;

		strLog = "读取参数完成";
	}
	catch (Poco::Exception& exc)
	{
		strLog = "读取参数失败，使用默认参数 " + CMyCodeConvert::Utf8ToGb2312(exc.displayText());
		m_nGaussKernel = 5;
		m_nSharpKernel = 5;
		m_nCannyKernel = 90;
		m_nDilateKernel = 6;
		m_nErodeKernel = 2;

		m_nDilateKernel_DefSn = 6;
		m_nDilateKernel_DefCommon = 6;
		m_nDilateKernel_Sn = 6;
		m_nDilateKernel_Common = 6;

		m_nWhiteVal = 225;
		m_nHeadVal	= 136;
		m_nPaginationVal = 150;
		m_nABModelVal = 150;
		m_nCourseVal = 150;
		m_nQK_CPVal = 150;
		m_nWJ_CPVal = 150;
		m_nGrayVal	= 150;
		m_nFixVal	= 150;
		m_nOMR		= 230;
		m_nSN		= 200;
		m_nCharacterThreshold	= 150;

		m_fHeadThresholdPercent		= 0.75;
		m_fPaginationThresholdPercent = 0.75;
		m_fABModelThresholdPercent	= 0.75;
		m_fCourseThresholdPercent	= 0.75;
		m_fQK_CPThresholdPercent_Head	= 1.2;
		m_fWJ_CPThresholdPercent_Head = 1.2;
		m_fQK_CPThresholdPercent_Fix = 1.5;
		m_fWJ_CPThresholdPercent_Fix = 1.5;
		m_fFixThresholdPercent		= 0.8;
		m_fGrayThresholdPercent		= 0.75;
		m_fWhiteThresholdPercent	= 0.75;
		m_fOMRThresholdPercent_Fix	= 1.5;
		m_fSNThresholdPercent_Fix	= 1.5;
		m_fOMRThresholdPercent_Head = 1.2;
		m_fSNThresholdPercent_Head	= 1.2;
	}
	g_pLogger->information(strLog);
}

void CMakeModelDlg::RecogFixWithHead(int i)
{
	if (m_pModel->nHasHead)
	{
		int nHCounts = m_vecPaperModelInfo[i]->vecH_Head.size();
		int nVCounts = m_vecPaperModelInfo[i]->vecV_Head.size();

		if (nHCounts < 2 || nVCounts < 2)
			return;

		m_vecPaperModelInfo[i]->vecRtFix.clear();
		m_vecPaperModelInfo[i]->vecRtSel.clear();

		RECTINFO rcFix_H1, rcFix_H2, rcFix_V1, rcFix_V2;
		rcFix_H1.eCPType = rcFix_H2.eCPType = rcFix_V1.eCPType = rcFix_V2.eCPType = Fix_CP;

		rcFix_H1 = m_vecPaperModelInfo[i]->vecH_Head[0];
		rcFix_H2 = m_vecPaperModelInfo[i]->vecH_Head[nHCounts - 1];
		rcFix_V1 = m_vecPaperModelInfo[i]->vecV_Head[0];
		rcFix_V2 = m_vecPaperModelInfo[i]->vecV_Head[nVCounts - 1];

		RECTINFO rcFixSel_1, rcFixSel_2, rcFixSel_3, rcFixSel_4;
		rcFixSel_1.eCPType = rcFixSel_2.eCPType = rcFixSel_3.eCPType = rcFixSel_4.eCPType = Fix_CP;

		RECTINFO rcFix_HV;
		rcFix_HV.eCPType = Fix_CP;
		rcFix_HV.nThresholdValue = m_nFixVal;
		rcFix_HV.fStandardValuePercent = m_fFixThresholdPercent;
		rcFix_H1.nThresholdValue = m_nFixVal;
		rcFix_H1.fStandardValuePercent = m_fFixThresholdPercent;
		rcFix_H2.nThresholdValue = m_nFixVal;
		rcFix_H2.fStandardValuePercent = m_fFixThresholdPercent;
		rcFix_V1.nThresholdValue = m_nFixVal;
		rcFix_V1.fStandardValuePercent = m_fFixThresholdPercent;
		rcFix_V2.nThresholdValue = m_nFixVal;
		rcFix_V2.fStandardValuePercent = m_fFixThresholdPercent;
		//判断交叉定位点
		if (abs(rcFix_H1.rt.x - rcFix_V1.rt.x) < 5 && abs(rcFix_H1.rt.y - rcFix_V1.rt.y) < 5)
		{
			rcFix_HV.rt.x = rcFix_H2.rt.x;
			rcFix_HV.rt.y = rcFix_V2.rt.y;
			rcFix_HV.rt.width = rcFix_H2.rt.width;
			rcFix_HV.rt.height = rcFix_V2.rt.height;

			Rect rtTmp = rcFix_HV.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rcFix_HV);

			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_H1);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_H2);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_V2);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_HV);

			cv::Point pt1, pt2;
			pt1 = rcFix_H1.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_H1.rt.br() + cv::Point(50, 50);
			rcFixSel_1.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_H2.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_H2.rt.br() + cv::Point(50, 50);
			rcFixSel_2.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_V2.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_V2.rt.br() + cv::Point(50, 50);
			rcFixSel_3.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_HV.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_HV.rt.br() + cv::Point(50, 50);
			rcFixSel_4.rt = cv::Rect(pt1, pt2);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_1);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_2);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_3);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_4);
		}
		if (abs(rcFix_H1.rt.x - rcFix_V2.rt.x) < 5 && abs(rcFix_H1.rt.y - rcFix_V2.rt.y) < 5)
		{
			rcFix_HV.rt.x = rcFix_H2.rt.x;
			rcFix_HV.rt.y = rcFix_V1.rt.y;
			rcFix_HV.rt.width = rcFix_H2.rt.width;
			rcFix_HV.rt.height = rcFix_V1.rt.height;

			Rect rtTmp = rcFix_HV.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rcFix_HV);

			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_V1);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_HV);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_H1);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_H2);

			cv::Point pt1, pt2;
			pt1 = rcFix_H1.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_H1.rt.br() + cv::Point(50, 50);
			rcFixSel_1.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_H2.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_H2.rt.br() + cv::Point(50, 50);
			rcFixSel_2.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_V1.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_V1.rt.br() + cv::Point(50, 50);
			rcFixSel_3.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_HV.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_HV.rt.br() + cv::Point(50, 50);
			rcFixSel_4.rt = cv::Rect(pt1, pt2);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_3);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_4);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_1);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_2);
		}
		if (abs(rcFix_H2.rt.x - rcFix_V1.rt.x) < 5 && abs(rcFix_H2.rt.y - rcFix_V1.rt.y) < 5)
		{
			rcFix_HV.rt.x = rcFix_H1.rt.x;
			rcFix_HV.rt.y = rcFix_V2.rt.y;
			rcFix_HV.rt.width = rcFix_H1.rt.width;
			rcFix_HV.rt.height = rcFix_V2.rt.height;

			Rect rtTmp = rcFix_HV.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rcFix_HV);

			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_H1);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_H2);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_V2);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_HV);

			cv::Point pt1, pt2;
			pt1 = rcFix_H1.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_H1.rt.br() + cv::Point(50, 50);
			rcFixSel_1.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_H2.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_H2.rt.br() + cv::Point(50, 50);
			rcFixSel_2.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_V2.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_V2.rt.br() + cv::Point(50, 50);
			rcFixSel_3.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_HV.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_HV.rt.br() + cv::Point(50, 50);
			rcFixSel_4.rt = cv::Rect(pt1, pt2);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_1);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_2);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_3);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_4);
		}
		if (abs(rcFix_H2.rt.x - rcFix_V2.rt.x) < 5 && abs(rcFix_H2.rt.y - rcFix_V2.rt.y) < 5)
		{
			rcFix_HV.rt.x = rcFix_H1.rt.x;
			rcFix_HV.rt.y = rcFix_V1.rt.y;
			rcFix_HV.rt.width = rcFix_H1.rt.width;
			rcFix_HV.rt.height = rcFix_V1.rt.height;

			Rect rtTmp = rcFix_HV.rt;
			Mat matSrcModel = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg(rtTmp);
			RecogGrayValue(matSrcModel, rcFix_HV);

			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_HV);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_V1);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_H2);
			m_vecPaperModelInfo[i]->vecRtFix.push_back(rcFix_H1);

			cv::Point pt1, pt2;
			pt1 = rcFix_H1.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_H1.rt.br() + cv::Point(50, 50);
			rcFixSel_1.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_H2.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_H2.rt.br() + cv::Point(50, 50);
			rcFixSel_2.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_V1.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_V1.rt.br() + cv::Point(50, 50);
			rcFixSel_3.rt = cv::Rect(pt1, pt2);
			pt1 = rcFix_HV.rt.tl() - cv::Point(50, 50);
			pt2 = rcFix_HV.rt.br() + cv::Point(50, 50);
			rcFixSel_4.rt = cv::Rect(pt1, pt2);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_4);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_3);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_2);
			m_vecPaperModelInfo[i]->vecRtSel.push_back(rcFixSel_1);
		}
	}
}

bool CMakeModelDlg::UploadModel(CString strModelPath, pMODEL pModel)
{
	if (pModel->nSaveMode == 1)
		return true;

	//++选做题的模板信息随模板信息上传
	std::string strElectOmrInfo;
	if (pModel->nHasElectOmr)
	{
		Poco::JSON::Object jsnDataObj;
		Poco::JSON::Array jsnElectOmrArry;
		for (int i = 0; i < pModel->vecPaperModel.size(); i++)
		{
			Poco::JSON::Object jsnPaperElectOmrObj;
			Poco::JSON::Array jsnPaperElectOmrArry;		//单页试卷上的选做题信息
			ELECTOMR_LIST::iterator itElectOmr = pModel->vecPaperModel[i]->lElectOmr.begin();
			for (; itElectOmr != pModel->vecPaperModel[i]->lElectOmr.end(); itElectOmr++)
			{
				Poco::JSON::Object jsnElectOmr;
				jsnElectOmr.set("paperId", i + 1);
				jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
				jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
				jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
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
		}

		std::stringstream jsnOmrString;
		jsnElectOmrArry.stringify(jsnOmrString, 0);
		strElectOmrInfo = jsnOmrString.str();
		TRACE("%s\n", strElectOmrInfo.c_str());
	}

	USES_CONVERSION;
	std::string strPath = T2A(strModelPath);
	std::string strMd5;

	strMd5 = calcFileMd5(strPath);

	ST_MODELINFO stModelInfo;
	ZeroMemory(&stModelInfo, sizeof(ST_MODELINFO));
	stModelInfo.nExamID = pModel->nExamID;
	stModelInfo.nSubjectID = pModel->nSubjectID;

	sprintf_s(stModelInfo.szModelName, "%s.mod", pModel->strModelName.c_str());
	strncpy(stModelInfo.szMD5, strMd5.c_str(), strMd5.length());

	if (strElectOmrInfo.length() > sizeof(stModelInfo.szElectOmr))
	{
		char szLog[200] = { 0 };
		CString strErr;
		sprintf_s(szLog, "选做题信息长度(%d)超过发送结构体的存储空间(%d),无法进行后续提交操作", strElectOmrInfo.length(), sizeof(stModelInfo.szElectOmr));
		g_pLogger->information(szLog);
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "选做题信息太多，无法操作");
		dlg.DoModal();
		return false;
	}

	strncpy(stModelInfo.szElectOmr, strElectOmrInfo.c_str(), strElectOmrInfo.length());


	sprintf_s(stModelInfo.szUserNo, "%s", _strUserName_.c_str());
	sprintf_s(stModelInfo.szEzs, "%s", _strEzs_.c_str());

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_SETMODELINFO;
	pTcpTask->nPkgLen = sizeof(ST_MODELINFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stModelInfo, sizeof(ST_MODELINFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
	return true;
}


void CMakeModelDlg::OnBnClickedBtnAdvancedsetting()
{
	AdvanceParam stAdvanceParam;
	if (m_pModel)
	{
		stAdvanceParam.nScanDpi		= m_pModel->nScanDpi;
		stAdvanceParam.nScanPaperSize = m_pModel->nScanSize;
		stAdvanceParam.nScanType	= m_pModel->nScanType;
		stAdvanceParam.nAutoCut		= m_pModel->nAutoCut;
		
		stAdvanceParam.nUseWordAnchorPoint	= m_pModel->nUseWordAnchorPoint;
		stAdvanceParam.nUsePagination		= m_pModel->nUsePagination;
	}
	stAdvanceParam.nCurrentZkzhSensitivity = m_nDilateKernel_Sn;
	stAdvanceParam.nCurrentOmrSensitivity = m_nDilateKernel_Common;
	stAdvanceParam.nDefZkzhSensitivity	= m_nDilateKernel_DefSn;
	stAdvanceParam.nDefOmrSensitivity	= m_nDilateKernel_DefCommon;
	
	stAdvanceParam.nCurrentFixThreshold = m_nFixVal;
	stAdvanceParam.nCurrentGrayThreshold = m_nGrayVal;
	stAdvanceParam.nCurrentQkWjThreshold = m_nQK_CPVal;
	stAdvanceParam.nCurrentZkzhThreshold = m_nSN;
	stAdvanceParam.nCurrentOmrThreshold = m_nOMR;
	stAdvanceParam.nDefFixThreshold		= m_nThreshold_DefFix;
	stAdvanceParam.nDefGrayThreshold	= m_nThreshold_DefGray;
	stAdvanceParam.nDefZkzhThreshold	= m_nThreshold_DefSn;
	stAdvanceParam.nDefOmrThreshold		= m_nThreshold_DefOmr;

	stAdvanceParam.nPersentFix	= m_fFixThresholdPercent * 100;
	stAdvanceParam.nPersentGray = m_fGrayThresholdPercent * 100;
	stAdvanceParam.nPersentQkWj = m_fQK_CPThresholdPercent_Fix * 100;
	stAdvanceParam.nPersentZkzh = m_fSNThresholdPercent_Fix * 100;
	stAdvanceParam.nPersentOmr	= m_fOMRThresholdPercent_Fix * 100;
	stAdvanceParam.nDefPersentFix	= m_fDefPersentFix * 100;
	stAdvanceParam.nDefPersentGray	= m_fDefPersentGray * 100;
	stAdvanceParam.nDefPersentQkWj	= m_fDefPersentQkWj * 100;
	stAdvanceParam.nDefPersentZkzh	= m_fDefPersentSN * 100;
	stAdvanceParam.nDefPersentOmr	= m_fDefPersentOmr * 100;

	if (m_pModel)
		stAdvanceParam.nCharacterAnchorPoint	= m_pModel->nCharacterAnchorPoint;
	stAdvanceParam.nDefCharacterAnchorPoint = 4;
	stAdvanceParam.nCharacterConfidence		= m_nCharacterConfidence;
	stAdvanceParam.nDefCharacterConfidence	= 60;

	CAdanceSetMgrDlg dlg(m_pModel, stAdvanceParam);
	if (dlg.DoModal() != IDOK)
		return;
	if (!m_pModel)
	{
		return;
	}
	m_pModel->nScanDpi	= dlg._stSensitiveParam.nScanDpi;
	m_pModel->nAutoCut	= dlg._stSensitiveParam.nAutoCut;
	m_pModel->nScanSize = dlg._stSensitiveParam.nScanPaperSize;
	m_pModel->nScanType = dlg._stSensitiveParam.nScanType;
	m_nDilateKernel_Sn	= dlg._stSensitiveParam.nCurrentZkzhSensitivity;
	m_nDilateKernel_Common = dlg._stSensitiveParam.nCurrentOmrSensitivity;
	
	m_nFixVal	= dlg._stSensitiveParam.nCurrentFixThreshold;
	m_nGrayVal	= dlg._stSensitiveParam.nCurrentGrayThreshold;
	m_nQK_CPVal = dlg._stSensitiveParam.nCurrentQkWjThreshold;
	m_nWJ_CPVal = dlg._stSensitiveParam.nCurrentQkWjThreshold;
	m_nSN	= dlg._stSensitiveParam.nCurrentZkzhThreshold;
	m_nOMR	= dlg._stSensitiveParam.nCurrentOmrThreshold;
	m_fFixThresholdPercent		= dlg._stSensitiveParam.nPersentFix / 100.0;
	m_fGrayThresholdPercent		= dlg._stSensitiveParam.nPersentGray / 100.0;
	m_fQK_CPThresholdPercent_Fix = dlg._stSensitiveParam.nPersentQkWj / 100.0;
	m_fWJ_CPThresholdPercent_Fix = dlg._stSensitiveParam.nPersentQkWj / 100.0;
	m_fSNThresholdPercent_Fix	= dlg._stSensitiveParam.nPersentZkzh / 100.0;
	m_fOMRThresholdPercent_Fix	= dlg._stSensitiveParam.nPersentOmr / 100.0;

	if (m_pModel->nUseWordAnchorPoint != dlg._stSensitiveParam.nUseWordAnchorPoint || \
		m_pModel->nUsePagination != dlg._stSensitiveParam.nUsePagination)
	{
		m_pModel->nUseWordAnchorPoint	= dlg._stSensitiveParam.nUseWordAnchorPoint;
		m_pModel->nUsePagination		= dlg._stSensitiveParam.nUsePagination;
		if (m_pModel->vecPaperModel.size() <= 2)	//如果模板页数不超过2张，则不设置多页模式，多页模式必须3张以上，且扫描用双面扫描
			m_pModel->nUsePagination = 0;
		InitConf();
		
		m_ncomboCurrentSel = m_comboCheckPointType.GetCurSel();
		m_eCurCPType = GetComboSelCpType();
		UpdataCPList();
		ShowRectByCPType(m_eCurCPType);

		switch (m_eCurCPType)
		{
			case SN:
				{
					m_nDilateKernel = m_nDilateKernel_Sn;
				}
				break;
			default:
				m_nDilateKernel = m_nDilateKernel_Common;
				break;
		}

		InitShowSnOmrDlg(m_eCurCPType);
	}
	m_pModel->nCharacterAnchorPoint = dlg._stSensitiveParam.nCharacterAnchorPoint;
	m_nCharacterConfidence			= dlg._stSensitiveParam.nCharacterConfidence;

	switch (m_eCurCPType)
	{
		case SN:
		{
			m_nDilateKernel = m_nDilateKernel_Sn;
		}
			break;
		default:
			m_nDilateKernel = m_nDilateKernel_Common;
			break;
	}
}


BOOL CMakeModelDlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 242, 250

	return TRUE;
}


HBRUSH CMakeModelDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_CPType || CurID == IDC_STATIC_List)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

void CMakeModelDlg::ReInitModel(pMODEL pModel)
{
//	SAFE_RELEASE(m_pModel);

	m_pModel = pModel;
	InitUI();
	InitConf();

	USES_CONVERSION;
	if (m_pModel)
	{
		std::vector<pPaperModelInfo>::iterator itPaperModelInfo = m_vecPaperModelInfo.begin();
		for (; itPaperModelInfo != m_vecPaperModelInfo.end();)
		{
			pPaperModelInfo pPaperModel = *itPaperModelInfo;
			SAFE_RELEASE(pPaperModel);
			itPaperModelInfo = m_vecPaperModelInfo.erase(itPaperModelInfo);
		}
		m_vecPaperModelInfo.clear();

		for (int i = 0; i < m_pModel->nPicNum; i++)
		{
			CString strPicPath = g_strCurrentPath + _T("Model\\") + A2T(m_pModel->strModelName.c_str()) + _T("\\") + A2T(m_pModel->vecPaperModel[i]->strModelPicName.c_str());

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
			m_vecPicShow[i]->ShowPic(src_img, cv::Point(0, 0), 1.0);

			pPaperModel->nPicW = src_img.cols;
			pPaperModel->nPicH = src_img.rows;

			pPaperModel->bFirstH = false;
			pPaperModel->bFirstV = false;
			pPaperModel->rtHTracker = m_pModel->vecPaperModel[i]->rtHTracker;
			pPaperModel->rtVTracker = m_pModel->vecPaperModel[i]->rtVTracker;
			//			pPaperModel->rtSNTracker = m_pModel->vecPaperModel[i]->rtSNTracker;
			pPaperModel->rcSNTracker = m_pModel->vecPaperModel[i]->rcSNTracker;

			CHARACTER_ANCHOR_AREA_LIST::iterator itRecogCharRt = m_pModel->vecPaperModel[i]->lCharacterAnchorArea.begin();
			for (; itRecogCharRt != m_pModel->vecPaperModel[i]->lCharacterAnchorArea.end(); itRecogCharRt++)
			{
				pST_CHARACTER_ANCHOR_AREA pStCharacterAnchorArea = new ST_CHARACTER_ANCHOR_AREA();
				pStCharacterAnchorArea->nIndex = (*itRecogCharRt)->nIndex;
				pStCharacterAnchorArea->nCannyKernel = (*itRecogCharRt)->nCannyKernel;
				pStCharacterAnchorArea->nDilateKernel = (*itRecogCharRt)->nDilateKernel;
				pStCharacterAnchorArea->nGaussKernel = (*itRecogCharRt)->nGaussKernel;
				pStCharacterAnchorArea->nSharpKernel = (*itRecogCharRt)->nSharpKernel;
				pStCharacterAnchorArea->nThresholdValue = (*itRecogCharRt)->nThresholdValue;
				pStCharacterAnchorArea->nCharacterConfidence = (*itRecogCharRt)->nCharacterConfidence;
				pStCharacterAnchorArea->rt = (*itRecogCharRt)->rt;
				for (auto itCharPoint : (*itRecogCharRt)->vecCharacterRt)
				{
					pST_CHARACTER_ANCHOR_POINT pStCharAnchorPoint = new ST_CHARACTER_ANCHOR_POINT();
					pStCharAnchorPoint->nIndex = itCharPoint->nIndex;
					pStCharAnchorPoint->fConfidence = itCharPoint->fConfidence;
					pStCharAnchorPoint->rc = itCharPoint->rc;
					pStCharAnchorPoint->strVal = itCharPoint->strVal;
					pStCharacterAnchorArea->vecCharacterRt.push_back(pStCharAnchorPoint);
				}
				pPaperModel->vecCharacterLocation.push_back(pStCharacterAnchorArea);

//				pPaperModel->vecCharacterLocation.push_back(*itRecogCharRt);
			}
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
			RECTLIST::iterator itPage = m_pModel->vecPaperModel[i]->lPagination.begin();
			for (; itPage != m_pModel->vecPaperModel[i]->lPagination.end(); itPage++)
			{
				pPaperModel->vecPagination.push_back(*itPage);
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
			RECTLIST::iterator itWJ = m_pModel->vecPaperModel[i]->lWJ_CP.begin();
			for (; itWJ != m_pModel->vecPaperModel[i]->lWJ_CP.end(); itWJ++)
			{
				pPaperModel->vecWJ_CP.push_back(*itWJ);
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
			OMRLIST::iterator itOmr2 = m_pModel->vecPaperModel[i]->lOMR2.begin();
			for (; itOmr2 != m_pModel->vecPaperModel[i]->lOMR2.end(); itOmr2++)
			{
				pPaperModel->vecOmr2.push_back(*itOmr2);
			}
			ELECTOMR_LIST::iterator itElectOmr = m_pModel->vecPaperModel[i]->lElectOmr.begin();
			for (; itElectOmr != m_pModel->vecPaperModel[i]->lElectOmr.end(); itElectOmr++)
			{
				pPaperModel->vecElectOmr.push_back(*itElectOmr);
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
		strTitle.Format(_T("模板名称: %s"), A2T(m_pModel->strModelName.c_str()));
		SetWindowText(strTitle);

		m_pSNInfoDlg->InitType(m_pModel->nZkzhType);
	}
	else
	{
		CString strTitle = _T("未保存模板");
		SetWindowText(strTitle);
	}
}

void CMakeModelDlg::CreateNewModel(std::vector<MODELPICPATH>& vecPath)
{
	USES_CONVERSION;
	std::vector<pPaperModelInfo>::iterator itPaperModelInfo = m_vecPaperModelInfo.begin();
	for (; itPaperModelInfo != m_vecPaperModelInfo.end();)
	{
		pPaperModelInfo pPaperModel = *itPaperModelInfo;
		SAFE_RELEASE(pPaperModel);
		itPaperModelInfo = m_vecPaperModelInfo.erase(itPaperModelInfo);
	}
	m_vecPaperModelInfo.clear();

	m_nStartTH = 0;

//	GetDlgItem(IDC_BTN_New)->EnableWindow(FALSE);
	m_nModelPicNums = vecPath.size();
	InitTab();
	m_cpListCtrl.DeleteAllItems();

	m_bNewModelFlag = true;
	m_pModel = new MODEL;
	m_pModel->nABModel = 0;
	m_pModel->nHasHead = 0;

	if (_pCurrExam_)
		m_pModel->nExamID = _pCurrExam_->nExamID;
	if (_pCurrSub_)
		m_pModel->nSubjectID = _pCurrSub_->nSubjID;

	InitConf();

	for (int i = 0; i < m_nModelPicNums; i++)
	{
		pPaperModelInfo paperMode = new PaperModelInfo;
		m_vecPaperModelInfo.push_back(paperMode);

		paperMode->strModelPicName = T2A(vecPath[i].strName);
		paperMode->strModelPicPath = vecPath[i].strPath;
		
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
		Mat matImg = imread((std::string)(CT2CA)vecPath[i].strPath);	//(std::string)(CT2CA)paperMode->strModelPicPath
#endif
		paperMode->matSrcImg = matImg;
		paperMode->matDstImg = paperMode->matSrcImg;
		if (i == 0)
			m_pModelPicShow->ShowPic(matImg);

		paperMode->nPicW = matImg.cols;
		paperMode->nPicH = matImg.rows;

		//试卷纸张类型判断
		int nL, nW;
		if (matImg.cols > matImg.rows)
		{
			nL = matImg.cols;
			nW = matImg.rows;
		}
		else
		{
			nL = matImg.rows;
			nW = matImg.cols;
		}
		if (i == 0)
		{
			if (nW > 1350 && nW < 1800)
			{
				if (nL > 2000 && nL < 2500)
				{
					m_pModel->nScanDpi = 200;
					m_pModel->nScanSize = 1;	//A4
				}
			}
			else if (nW > 2000 && nW < 2500)
			{
				if (nL > 2850 && nL < 3450)
				{
					m_pModel->nScanDpi = 200;
					m_pModel->nScanSize = 2;	//A3
				}
			}
			// 			else if (nW > 1450 && nW < 1900)
			// 			{
			// 				if (nL > 2150 && nL < 2600)
			// 				{
			// 					m_pModel->nScanDpi = 150;
			// 					m_pModel->nScanSize = 2;	//A3
			// 				}
			// 			}
		}
	}
	SetWindowTextW(_T("*未保存模板*"));
}

void CMakeModelDlg::SaveNewModel()
{
	if (!m_pModel)
	{
//		AfxMessageBox(_T("请先创建模板"));
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "请先创建模板");
		dlg.DoModal();
		return;
	}

	if (!checkValidity()) return;

	CExamInfoDlg dlg(m_pModel);
	if (dlg.DoModal() != IDOK)
		return;

	USES_CONVERSION;
	m_pModel->nSaveMode = 2;
	char szModelName[150] = { 0 };
	sprintf_s(szModelName, "%s_%s_N_%d_%d", T2A(dlg.m_strExamName), T2A(dlg.m_strSubjectName), dlg.m_nExamID, dlg.m_SubjectID);

	char szModelDesc[300] = { 0 };
	sprintf_s(szModelDesc, "考试名称: %s\r\n科目: %s\r\n年级: %s\r\n考试类型名称: %s", T2A(dlg.m_strExamName), T2A(dlg.m_strSubjectName), T2A(dlg.m_strGradeName), T2A(dlg.m_strExamTypeName));
	m_pModel->nExamID = dlg.m_nExamID;
	m_pModel->nSubjectID = dlg.m_SubjectID;
	m_pModel->strModelName = szModelName;
	m_pModel->strModelDesc = szModelDesc;

	CString strTitle = _T("");
	strTitle.Format(_T("模板名称: %s"), A2T(m_pModel->strModelName.c_str()));
	SetWindowText(strTitle);

	m_bSavedModelFlag = true;

	for (int i = 0; i < m_pModel->vecPaperModel.size(); i++)
	{
		pPAPERMODEL pPaperModel = m_pModel->vecPaperModel[i];
		SAFE_RELEASE(pPaperModel);
	}
	m_pModel->vecPaperModel.clear();

	m_pModel->nZkzhType = m_pSNInfoDlg->m_nZkzhType;
	bool bHasElectOmr = false;

	m_pModel->nPicNum = m_vecPaperModelInfo.size();
	for (int i = 0; i < m_pModel->nPicNum; i++)
	{
		pPAPERMODEL pPaperModel = new PAPERMODEL;
		pPaperModel->strModelPicName = m_vecPaperModelInfo[i]->strModelPicName;

		//++同步头模式时，添加4个定点
		RecogFixWithHead(i);
		//--

		for (int j = 0; j < m_vecPaperModelInfo[i]->vecCharacterLocation.size(); j++)
		{
			pST_CHARACTER_ANCHOR_AREA pStCharacterAnchorArea = new ST_CHARACTER_ANCHOR_AREA();
			pStCharacterAnchorArea->nIndex = m_vecPaperModelInfo[i]->vecCharacterLocation[j]->nIndex;
			pStCharacterAnchorArea->nCannyKernel = m_vecPaperModelInfo[i]->vecCharacterLocation[j]->nCannyKernel;
			pStCharacterAnchorArea->nDilateKernel = m_vecPaperModelInfo[i]->vecCharacterLocation[j]->nDilateKernel;
			pStCharacterAnchorArea->nGaussKernel = m_vecPaperModelInfo[i]->vecCharacterLocation[j]->nGaussKernel;
			pStCharacterAnchorArea->nSharpKernel = m_vecPaperModelInfo[i]->vecCharacterLocation[j]->nSharpKernel;
			pStCharacterAnchorArea->nThresholdValue = m_vecPaperModelInfo[i]->vecCharacterLocation[j]->nThresholdValue;
			pStCharacterAnchorArea->rt = m_vecPaperModelInfo[i]->vecCharacterLocation[j]->rt;
			for (auto itCharPoint : m_vecPaperModelInfo[i]->vecCharacterLocation[j]->vecCharacterRt)
			{
				pST_CHARACTER_ANCHOR_POINT pStCharAnchorPoint = new ST_CHARACTER_ANCHOR_POINT();
				pStCharAnchorPoint->nIndex = itCharPoint->nIndex;
				pStCharAnchorPoint->fConfidence = itCharPoint->fConfidence;
				pStCharAnchorPoint->rc = itCharPoint->rc;
				pStCharAnchorPoint->strVal = itCharPoint->strVal;
				pStCharacterAnchorArea->vecCharacterRt.push_back(pStCharAnchorPoint);
			}
			pPaperModel->lCharacterAnchorArea.push_back(pStCharacterAnchorArea);
		}
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecHTracker.size(); j++)
			pPaperModel->lSelHTracker.push_back(m_vecPaperModelInfo[i]->vecHTracker[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecVTracker.size(); j++)
			pPaperModel->lSelVTracker.push_back(m_vecPaperModelInfo[i]->vecVTracker[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecRtSel.size(); j++)
			pPaperModel->lSelFixRoi.push_back(m_vecPaperModelInfo[i]->vecRtSel[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecRtFix.size(); j++)
			pPaperModel->lFix.push_back(m_vecPaperModelInfo[i]->vecRtFix[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecH_Head.size(); j++)
			pPaperModel->lH_Head.push_back(m_vecPaperModelInfo[i]->vecH_Head[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecV_Head.size(); j++)
			pPaperModel->lV_Head.push_back(m_vecPaperModelInfo[i]->vecV_Head[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecPagination.size(); j++)
			pPaperModel->lPagination.push_back(m_vecPaperModelInfo[i]->vecPagination[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecABModel.size(); j++)
			pPaperModel->lABModel.push_back(m_vecPaperModelInfo[i]->vecABModel[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecCourse.size(); j++)
			pPaperModel->lCourse.push_back(m_vecPaperModelInfo[i]->vecCourse[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecQK_CP.size(); j++)
			pPaperModel->lQK_CP.push_back(m_vecPaperModelInfo[i]->vecQK_CP[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecWJ_CP.size(); j++)
			pPaperModel->lWJ_CP.push_back(m_vecPaperModelInfo[i]->vecWJ_CP[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecGray.size(); j++)
			pPaperModel->lGray.push_back(m_vecPaperModelInfo[i]->vecGray[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecWhite.size(); j++)
			pPaperModel->lWhite.push_back(m_vecPaperModelInfo[i]->vecWhite[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecOmr2.size(); j++)
			pPaperModel->lOMR2.push_back(m_vecPaperModelInfo[i]->vecOmr2[j]);
		for (int j = 0; j < m_vecPaperModelInfo[i]->vecElectOmr.size(); j++)
		{
			pPaperModel->lElectOmr.push_back(m_vecPaperModelInfo[i]->vecElectOmr[j]);
			bHasElectOmr = true;
		}
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
			m_ptHTracker2 = cv::Point(m_vecPaperModelInfo[i]->matDstImg.cols, 90);	//m_vecPaperModelInfo[i]->matSrcImg.cols
			//			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstH = false;

			m_vecPaperModelInfo[i]->rtHTracker.x = m_ptHTracker1.x;
			m_vecPaperModelInfo[i]->rtHTracker.y = m_ptHTracker1.y;
			m_vecPaperModelInfo[i]->rtHTracker.width = m_ptHTracker2.x - m_ptHTracker1.x;
			m_vecPaperModelInfo[i]->rtHTracker.height = m_ptHTracker2.y - m_ptHTracker1.y;
		}
		if (m_pModel->nHasHead && m_vecPaperModelInfo[i]->bFirstV && m_pModel->nType == 0)
		{
			m_ptVTracker1 = cv::Point(m_vecPaperModelInfo[i]->matDstImg.cols - 90, 0);		//m_vecPaperModelInfo[i]->matSrcImg.cols
			m_ptVTracker2 = cv::Point(m_vecPaperModelInfo[i]->matDstImg.cols, m_vecPaperModelInfo[i]->matDstImg.rows);	//m_vecPaperModelInfo[i]->matSrcImg.cols  m_vecPaperModelInfo[i]->matSrcImg.rows
			//			m_vecPaperModelInfo[m_nCurrTabSel]->bFirstV = false;

			m_vecPaperModelInfo[i]->rtVTracker.x = m_ptVTracker1.x;
			m_vecPaperModelInfo[i]->rtVTracker.y = m_ptVTracker1.y;
			m_vecPaperModelInfo[i]->rtVTracker.width = m_ptVTracker2.x - m_ptVTracker1.x;
			m_vecPaperModelInfo[i]->rtVTracker.height = m_ptVTracker2.y - m_ptVTracker1.y;
		}
		//--
		pPaperModel->rtHTracker = m_vecPaperModelInfo[i]->rtHTracker;
		pPaperModel->rtVTracker = m_vecPaperModelInfo[i]->rtVTracker;
		//		pPaperModel->rtSNTracker = m_vecPaperModelInfo[i]->rtSNTracker;
		pPaperModel->rcSNTracker = m_vecPaperModelInfo[i]->rcSNTracker;

		pPaperModel->nPicW = m_vecPaperModelInfo[i]->nPicW;
		pPaperModel->nPicH = m_vecPaperModelInfo[i]->nPicH;

		m_pModel->vecPaperModel.push_back(pPaperModel);
	}

	if (!bHasElectOmr)
		m_pModel->nHasElectOmr = 0;

	SetCursor(LoadCursor(NULL, IDC_WAIT));

	CString modelPath = g_strCurrentPath + _T("Model");
	modelPath = modelPath + _T("\\") + A2T(m_pModel->strModelName.c_str());
	if (SaveModelFile(m_pModel))
	{
		//		ZipFile(modelPath, modelPath, _T(".mod"));
		CZipObj zipObj;
		zipObj.setLogger(g_pLogger);
		zipObj.ZipFile(modelPath, modelPath, _T(".mod"));

		//直接上传模板
		CString strModelFullPath = modelPath + _T(".mod");
		UploadModel(strModelFullPath, m_pModel);

		CNewMessageBox dlg;
		dlg.setShowInfo(3, 1, "保存完成");
		dlg.DoModal();
//		AfxMessageBox(_T("保存完成!"));
	}
	else
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "保存失败");
		dlg.DoModal();
//		AfxMessageBox(_T("保存失败"));
	}

	SetCursor(LoadCursor(NULL, IDC_ARROW));
}


void CMakeModelDlg::LeftRotate()
{
#if 1
	++m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes;
	m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes = m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes < 0 ? m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes + 4 : m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes;
	switch (m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes % 4)
	{
		case 0:
			m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 1;
			break;
		case 1:
			m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 3;
			break;
		case 2:
			m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 4;
			break;
		case 3:
			m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 2;
			break;
	}
 	RotateImg(m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg, 3);	//原图进行旋转

	//------------------------------------
	m_ncomboCurrentSel = m_comboCheckPointType.GetCurSel();
	m_eCurCPType = GetComboSelCpType();
	UpdataCPList();
	ShowRectByCPType(m_eCurCPType);

	switch (m_eCurCPType)
	{
		case SN:
			{
				m_nDilateKernel = m_nDilateKernel_Sn;
			}
			break;
		default:
			m_nDilateKernel = m_nDilateKernel_Common;
			break;
	}

	InitShowSnOmrDlg(m_eCurCPType);
#else
	m_pModelPicShow->SetRotateDir(3);	//设置旋转方向，1:针对原始图像需要进行的旋转，正向，不需要旋转，2：右转90, 3：左转90, 4：右转180

//	ShowRectByCPType(m_eCurCPType);
#endif
}

void CMakeModelDlg::RightRotate()
{
#if 1
	--m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes;
	m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes = m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes < 0 ? m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes + 4 : m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes;
	switch (m_vecPaperModelInfo[m_nCurrTabSel]->nRotateTimes % 4)
	{
		case 0:
			m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 1;
			break;
		case 1:
			m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 3;
			break;
		case 2:
			m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 4;
			break;
		case 3:
			m_vecPaperModelInfo[m_nCurrTabSel]->nDirection = 2;
			break;
	}
	RotateImg(m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg, 2);	//原图进行旋转

	//------------------------------------
	m_ncomboCurrentSel = m_comboCheckPointType.GetCurSel();
	m_eCurCPType = GetComboSelCpType();
	UpdataCPList();
	ShowRectByCPType(m_eCurCPType);

	switch (m_eCurCPType)
	{
		case SN:
			{
				m_nDilateKernel = m_nDilateKernel_Sn;
			}
			break;
		default:
			m_nDilateKernel = m_nDilateKernel_Common;
			break;
	}

	InitShowSnOmrDlg(m_eCurCPType);
#else
	m_pModelPicShow->SetRotateDir(2);	//设置旋转方向，1:针对原始图像需要进行的旋转，正向，不需要旋转，2：右转90, 3：左转90, 4：右转180
#endif
}

cv::Rect CMakeModelDlg::GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Rect rtResult;
	if (nOrientation == 1)	//matSrc正向	模板图像的旋转方向
	{
		rtResult = rt;
	}
	else if (nOrientation == 2)	//matSrc右转90度
	{
		cv::Point pt1, pt2;
		pt1.x = nH - rt.tl().y;
		pt1.y = rt.tl().x;
		pt2.x = nH - rt.br().y;
		pt2.y = rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 3)	//matSrc左转90度
	{
		cv::Point pt1, pt2;
		pt1.x = rt.tl().y;
		pt1.y = nW - rt.tl().x;
		pt2.x = rt.br().y;
		pt2.y = nW - rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 4)	//matSrc右转180度
	{
		cv::Point pt1, pt2;
		pt1.x = nW - rt.tl().x;
		pt1.y = nH - rt.tl().y;
		pt2.x = nW - rt.br().x;
		pt2.y = nH - rt.br().y;
		rtResult = cv::Rect(pt1, pt2);
	}
	return rtResult;
}

cv::Point CMakeModelDlg::GetPointByOrientation(cv::Rect& rtPic, cv::Point pt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Point ptResult;
	if (nOrientation == 1)	//matSrc正向	模板图像的旋转方向
	{
		ptResult = pt;
	}
	else if (nOrientation == 2)	//matSrc右转90度
	{
		cv::Point pt1, pt2;
		pt1.x = nH - pt.y;
		pt1.y = pt.x;
		ptResult = pt1;
	}
	else if (nOrientation == 3)	//matSrc左转90度
	{
		cv::Point pt1, pt2;
		pt1.x = pt.y;
		pt1.y = nW - pt.x;
		ptResult = pt1;
	}
	else if (nOrientation == 4)	//matSrc右转180度
	{
		cv::Point pt1, pt2;
		pt1.x = nW - pt.x;
		pt1.y = nH - pt.y;
		ptResult = pt1;
	}
	return ptResult;
}

cv::Rect CMakeModelDlg::GetRectToSave(cv::Rect& rtPic, cv::Rect rt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Rect rtResult;
	if (nOrientation == 1)	//matSrc正向	模板图像的旋转方向
	{
		rtResult = rt;
	}
	else if (nOrientation == 2)	//matSrc右转90度，实际保存坐标进行左旋90
	{
		cv::Point pt1, pt2;
		pt1.x = rt.tl().y;
		pt1.y = nW - rt.tl().x;
		pt2.x = rt.br().y;
		pt2.y = nW - rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 3)	//matSrc左转90度，实际保存坐标进行右旋90
	{
		cv::Point pt1, pt2;
		pt1.x = nH - rt.tl().y;
		pt1.y = rt.tl().x;
		pt2.x = nH - rt.br().y;
		pt2.y = rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 4)	//matSrc右转180度
	{
		cv::Point pt1, pt2;
		pt1.x = nW - rt.tl().x;
		pt1.y = nH - rt.tl().y;
		pt2.x = nW - rt.br().x;
		pt2.y = nH - rt.br().y;
		rtResult = cv::Rect(pt1, pt2);
	}
	return rtResult;
}

cv::Point CMakeModelDlg::GetPointToSave(cv::Rect& rtPic, cv::Point pt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Point ptResult;
	if (nOrientation == 1)	//matSrc正向	模板图像的旋转方向
	{
		ptResult = pt;
	}
	else if (nOrientation == 2)	//matSrc右转90度，实际保存坐标进行左旋90
	{
		cv::Point pt1, pt2;
		pt1.x = pt.y;
		pt1.y = nW - pt.x;
		ptResult = pt1;
	}
	else if (nOrientation == 3)	//matSrc左转90度，实际保存坐标进行右旋90
	{
		cv::Point pt1, pt2;
		pt1.x = nH - pt.y;
		pt1.y = pt.x;
		ptResult = pt1;		
	}
	else if (nOrientation == 4)	//matSrc右转180度
	{
		cv::Point pt1, pt2;
		pt1.x = nW - pt.x;
		pt1.y = nH - pt.y;
		ptResult = pt1;
	}
	return ptResult;
}

void CMakeModelDlg::RotateImg(cv::Mat& imgMat, int nDirection /*= 1*/)
{
	switch (nDirection)
	{
		case 1:	break;
		case 2:
			{
				Mat dst;
				transpose(imgMat, dst);	//左旋90，镜像 
				flip(dst, imgMat, 1);		//右旋90，模板图像需要左旋90，原图即需要右旋90
			}
			break;
		case 3:
			{
				Mat dst;
				transpose(imgMat, dst);	//左旋90，镜像 
				flip(dst, imgMat, 0);		//左旋90，模板图像需要右旋90，原图即需要左旋90				 
			}
			break;
		case 4:
			{
				Mat dst;
				transpose(imgMat, dst);	//左旋90，镜像 
				Mat dst2;
				flip(dst, dst2, 1);
				Mat dst5;
				transpose(dst2, dst5);
				flip(dst5, imgMat, 1);	//右旋180
			}
			break;
		default: break;
	}
}

cv::Rect CMakeModelDlg::GetShowFakePosRect(cv::Rect rt)
{
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return rt;

	cv::Rect rtModelPic;
	rtModelPic.width = m_vecPaperModelInfo[m_nCurrTabSel]->nPicW;
	rtModelPic.height = m_vecPaperModelInfo[m_nCurrTabSel]->nPicH;
	return GetRectByOrientation(rtModelPic, rt, m_vecPaperModelInfo[m_nCurrTabSel]->nDirection);
}

cv::Point CMakeModelDlg::GetShowFakePosPoint(cv::Point pt)
{
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return pt;

	cv::Rect rtModelPic;
	rtModelPic.width = m_vecPaperModelInfo[m_nCurrTabSel]->nPicW;
	rtModelPic.height = m_vecPaperModelInfo[m_nCurrTabSel]->nPicH;
	return GetPointByOrientation(rtModelPic, pt, m_vecPaperModelInfo[m_nCurrTabSel]->nDirection);
}

cv::Rect CMakeModelDlg::GetSrcSaveRect(cv::Rect rt)
{
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return rt;

	cv::Rect rtModelPic;
	rtModelPic.width = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols;
	rtModelPic.height = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows;
	return GetRectToSave(rtModelPic, rt, m_vecPaperModelInfo[m_nCurrTabSel]->nDirection);
}

cv::Point CMakeModelDlg::GetSrcSavePoint(cv::Point pt)
{
	if (m_vecPaperModelInfo.size() <= m_nCurrTabSel)
		return pt;

	cv::Rect rtModelPic;
	rtModelPic.width = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.cols;
	rtModelPic.height = m_vecPaperModelInfo[m_nCurrTabSel]->matDstImg.rows;
	return GetPointToSave(rtModelPic, pt, m_vecPaperModelInfo[m_nCurrTabSel]->nDirection);
}

int CMakeModelDlg::GetSnShowFakePosFlag(int nRecogFlag)
{
	int nResult = nRecogFlag;
	switch (nRecogFlag)
	{
		case 10:	//1010
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 10;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 5;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 6;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 9;
			break;
		case 9:	//1001
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 9;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 6;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 5;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 10;
			break;
		case 6:	//0110
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 6;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 10;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 9;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 5;
			break;
		case 5:	//0101
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 5;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 9;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 10;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 6;
			break;
	}
	return nResult;
}

int CMakeModelDlg::GetSnSavePosFlag(int nRecogFlag)
{
	int nResult = nRecogFlag;
	switch (nRecogFlag)
	{
		case 10:	//1010
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 10;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 6;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 5;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 9;
			break;
		case 9:	//1001
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 9;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 5;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 6;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 10;
			break;
		case 6:	//0110
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 6;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 9;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 10;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 5;
			break;
		case 5:	//0101
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 5;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 10;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 9;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 6;
			break;
	}
	return nResult;
}

int CMakeModelDlg::GetOmrShowFakePosFlag(int nRecogFlag)
{
	int nResult = nRecogFlag;
	switch (nRecogFlag)
	{
		case 42: //101010
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 42;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 22;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 25;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 37;
			break;
		case 41:	//101001
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 41;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 21;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 26;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 38;
			break;
		case 38:	//100110
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 38;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 22;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 26;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 41;
			break;
		case 37:	//100101
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 37;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 25;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 22;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 42;
			break;
		case 26:	//011010
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 26;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 41;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 38;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 21;
			break;
		case 25:	//011001
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 25;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 42;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 37;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 22;
			break;
		case 22:	//010110
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 22;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 37;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 42;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 25;
			break;
		case 21:	//010101
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 21;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 38;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 41;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 26;
			break;
	}
	return nResult;
}

int CMakeModelDlg::GetOmrSavePosFlag(int nRecogFlag)
{
	int nResult = nRecogFlag;
	switch (nRecogFlag)
	{
		case 42: //101010
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 42;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 25;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 22;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 37;
			break;
		case 41:	//101001
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 41;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 26;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 21;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 38;
			break;
		case 38:	//100110
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 38;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 26;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 22;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 41;
			break;
		case 37:	//100101
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 37;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 22;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 25;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 42;
			break;
		case 26:	//011010
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 26;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 38;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 41;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 21;
			break;
		case 25:	//011001
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 25;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 37;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 42;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 22;
			break;
		case 22:	//010110
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 22;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 42;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 37;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 25;
			break;
		case 21:	//010101
			if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 1)
				nResult = 21;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 2)	//src已经右转90了
				nResult = 41;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 3)	//src左转90了
				nResult = 38;
			else if (m_vecPaperModelInfo[m_nCurrTabSel]->nDirection == 4)
				nResult = 26;
			break;
	}
	return nResult;
}


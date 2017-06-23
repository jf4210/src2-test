// MakeModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "NewMakeModelDlg.h"
#include "afxdialogex.h"
#include "ScanModelPaperDlg.h"
#include "NewMessageBox.h"
#include "CreateModelDlg.h"
// CMakeModelDlg 对话框

IMPLEMENT_DYNAMIC(CNewMakeModelDlg, CDialog)

CNewMakeModelDlg::CNewMakeModelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMakeModelDlg::IDD, pParent)
	, m_nStatusSize(30), _pTWAINApp(NULL), m_pNewModelDlg(NULL), m_pMakeModelDlg(NULL), m_pModel(NULL)
{

}

CNewMakeModelDlg::~CNewMakeModelDlg()
{
}

void CNewMakeModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_ScanPaper, m_bmpBtnScan);
	DDX_Control(pDX, IDC_BTN_SaveModel, m_bmpBtnSave);
	DDX_Control(pDX, IDC_BTN_UploadPic, m_bmpBtnUpload);
	DDX_Control(pDX, IDC_BTN_InputPic, m_bmpBtnDown);
	DDX_Control(pDX, IDC_BTN_NewMakeModel, m_bmpBtnNew);
	DDX_Control(pDX, IDC_COMBO_MakeModel_Subject, m_comboSubject);
}


BEGIN_MESSAGE_MAP(CNewMakeModelDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_ScanPaper, &CNewMakeModelDlg::OnBnClickedBtnScanpaper)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_MESSAGE(MSG_SCAN_DONE, &CNewMakeModelDlg::ScanDone)
	ON_MESSAGE(MSG_SCAN_ERR, &CNewMakeModelDlg::ScanErr)
	ON_BN_CLICKED(IDC_BTN_NewMakeModel, &CNewMakeModelDlg::OnBnClickedBtnNewmakemodel)
	ON_CBN_SELCHANGE(IDC_COMBO_MakeModel_Subject, &CNewMakeModelDlg::OnCbnSelchangeComboMakemodelSubject)
	ON_BN_CLICKED(IDC_BTN_SaveModel, &CNewMakeModelDlg::OnBnClickedBtnSavemodel)
END_MESSAGE_MAP()


// CMakeModelDlg 消息处理程序


BOOL CNewMakeModelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	SetFontSize(m_nStatusSize);
	m_comboSubject.AdjustDroppedWidth();

	InitExamData();

	SAFE_RELEASE(m_pModel);
	m_pModel = LoadSubjectModel(_pCurrSub_);
	if (m_pMakeModelDlg)
		m_pMakeModelDlg->ReInitModel(m_pModel);
	//加载模板

	InitCtrlPosition();


	InitScanner();
	m_scanThread.CreateThread();
	
	return TRUE;
}

BOOL CNewMakeModelDlg::PreTranslateMessage(MSG* pMsg)
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

void CNewMakeModelDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CNewMakeModelDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 10;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	int nBtnW = 80;
	int nBtnH = 40;

	int nCurrLeft = nLeftGap;
	int nCurrTop = nTopGap;

	if (m_bmpBtnNew.GetSafeHwnd())
	{
		m_bmpBtnNew.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (m_bmpBtnSave.GetSafeHwnd())
	{
		m_bmpBtnSave.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (m_bmpBtnUpload.GetSafeHwnd())
	{
		m_bmpBtnUpload.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (m_bmpBtnScan.GetSafeHwnd())
	{
		m_bmpBtnScan.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (m_bmpBtnDown.GetSafeHwnd())
	{
		m_bmpBtnDown.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}

	nCurrLeft = cx - nRightGap - 100;
	if (m_comboSubject.GetSafeHwnd())
	{
		m_comboSubject.MoveWindow(nCurrLeft, nCurrTop + 10, 100, nBtnH);
		nCurrLeft -= (80 + nGap);
	}
	if (GetDlgItem(IDC_STATIC_MakeModel_CurrSubject)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_MakeModel_CurrSubject)->MoveWindow(nCurrLeft, nCurrTop, 80, nBtnH);
	}

	nCurrLeft = nLeftGap;
	nCurrTop = nTopGap + nBtnH + nGap;
	m_rtChild.left = nLeftGap;
	m_rtChild.top = nCurrTop;
	m_rtChild.right = cx - nRightGap;
	m_rtChild.bottom = cy - nBottomGap;
	if (m_pNewModelDlg && m_pNewModelDlg->GetSafeHwnd())
	{
		m_pNewModelDlg->MoveWindow(m_rtChild);
	}
	if (m_pMakeModelDlg && m_pMakeModelDlg->GetSafeHwnd())
	{
		m_pMakeModelDlg->MoveWindow(m_rtChild);
	}
	Invalidate();
}

void CNewMakeModelDlg::SetFontSize(int nSize)
{
	CFont fontStatus;
	fontStatus.CreateFont(nSize - 5, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("Arial"));
	GetDlgItem(IDC_STATIC_MakeModel_CurrSubject)->SetFont(&fontStatus);
}

pTW_IDENTITY CNewMakeModelDlg::GetScanSrc(int nIndex)
{
	return _pTWAINApp->getDataSource(nIndex);
}

void CNewMakeModelDlg::InitScanner()
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

void CNewMakeModelDlg::InitUI()
{
	m_bmpBtnScan.SetStateBitmap(IDB_MakeModel_Btn_Scan_Normal, 0, IDB_MakeModel_Btn_Scan_Down); 
	m_bmpBtnScan.SetWindowText(_T("    扫描题卡"));
	m_bmpBtnSave.SetStateBitmap(IDB_MakeModel_Btn_Save_Normal, 0, IDB_MakeModel_Btn_Save_Down);
	m_bmpBtnSave.SetWindowText(_T("    保存模板"));
	m_bmpBtnUpload.SetStateBitmap(IDB_MakeModel_Btn_Upload_Normal, 0, IDB_MakeModel_Btn_Upload_Down);
	m_bmpBtnUpload.SetWindowText(_T("    上传图片"));
	m_bmpBtnDown.SetStateBitmap(IDB_MakeModel_Btn_Download_normal, 0, IDB_MakeModel_Btn_Download_down);
	m_bmpBtnDown.SetWindowText(_T("    导入图片"));

	m_pNewModelDlg = new CNewModelDlg();
	m_pNewModelDlg->Create(CNewModelDlg::IDD, this);
	m_pNewModelDlg->ShowWindow(SW_HIDE);

	m_pMakeModelDlg = new CMakeModelDlg();
	m_pMakeModelDlg->Create(CMakeModelDlg::IDD, this);
	m_pMakeModelDlg->ShowWindow(SW_SHOW);

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

void CNewMakeModelDlg::InitExamData()
{
	USES_CONVERSION;
	m_comboSubject.ResetContent();
	if (_pCurrExam_)
	{
		int i = 0;
		int nShowSubject = -1;
		for (auto pSubject : _pCurrExam_->lSubjects)
		{
			m_comboSubject.AddString(A2T(pSubject->strSubjName.c_str()));

//			m_comboSubject.AddStringWithInfo(A2T(pSubject->strSubjName.c_str()), _T("111"));
//			m_comboSubject.SetEditSel(0, 1);
//			m_comboSubject.InsertString(m_comboSubject.GetCount(), A2T(pSubject->strSubjName.c_str()));
			int nCount = m_comboSubject.GetCount();
			m_comboSubject.SetItemDataPtr(nCount - 1, pSubject);
			
			if (i == 0)
				_pCurrSub_ = pSubject;

			i++;
		}
		if (_pCurrExam_->lSubjects.size())
			m_comboSubject.SetCurSel(0);
	}
	UpdateData(FALSE);
}

pMODEL CNewMakeModelDlg::LoadSubjectModel(pEXAM_SUBJECT pSubModel)
{
	if (!pSubModel)
		return NULL;

	if (pSubModel->strModelName.empty())
		return NULL;

	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
	strModelPath = g_strModelSavePath + "\\" + pSubModel->strModelName;	//gb2312
	std::string strBaseModelName;
	try
	{
		Poco::File fileModel(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
		if (!fileModel.exists())
			return NULL;
		
		Poco::Path pathModel(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
		strBaseModelName = CMyCodeConvert::Utf8ToGb2312(pathModel.getBaseName());
	}
	catch (Poco::Exception& e)
	{
	}

	CZipObj zipObj;
	zipObj.setLogger(g_pLogger);
	zipObj.UnZipFile(A2T(strModelPath.c_str()));

	CString strModelFilePath = g_strCurrentPath + _T("Model\\") + A2T(strBaseModelName.c_str());
	pMODEL pModel = LoadModelFile(strModelFilePath);

	return pModel;
}

void CNewMakeModelDlg::OnBnClickedBtnScanpaper()
{
	if (m_comboSubject.GetCurSel() >= 0)
		_pCurrSub_ = (pEXAM_SUBJECT)m_comboSubject.GetItemDataPtr(m_comboSubject.GetCurSel());
	else
		_pCurrSub_ = NULL;

	CScanModelPaperDlg dlg(this);
	dlg.SetScanSrc(m_vecScanSrc);
	dlg.DoModal();
	m_strScanPicPath = dlg.m_strSavePath;
}


HBRUSH CNewMakeModelDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_MakeModel_CurrSubject)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

BOOL CNewMakeModelDlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 242, 250


	return TRUE;
}

void CNewMakeModelDlg::OnDestroy()
{
	CDialog::OnDestroy();

//	m_scanThread.PostThreadMessage(WM_QUIT, NULL, NULL);
	SAFE_RELEASE(m_pModel);
	if (_pTWAINApp)
	{
		_pTWAINApp->exit();
		SAFE_RELEASE(_pTWAINApp);
	}
	if (m_pNewModelDlg)
	{
		m_pNewModelDlg->DestroyWindow();
		SAFE_RELEASE(m_pNewModelDlg);
	}

	if (m_pMakeModelDlg)
	{
		m_pMakeModelDlg->DestroyWindow();
		SAFE_RELEASE(m_pMakeModelDlg);
	}

//	::WaitForSingleObject(m_scanThread.m_hThread, INFINITE);
}

LRESULT CNewMakeModelDlg::ScanDone(WPARAM wParam, LPARAM lParam)
{
	pST_SCAN_RESULT pResult = (pST_SCAN_RESULT)wParam;
	if (pResult)
	{
		TRACE("扫描完成消息。%s\n", pResult->strResult.c_str());
		g_pLogger->information(pResult->strResult);

		USES_CONVERSION;
		MODELPATH picInfo;
		picInfo.strName = A2T(pResult->strPicName.c_str());
		picInfo.strPath = A2T(pResult->strPicPath.c_str());
		m_vecModelPicPath.push_back(picInfo);
		
		if (pResult->bScanOK)	//扫描完成
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(1, 1, "扫描完成");
			dlg.DoModal();
// 			CString strSelect = _T("/root,");
// 			strSelect.Append(m_strScanPicPath);
// 			ShellExecute(NULL, _T("open"), _T("explorer.exe"), strSelect, NULL, SW_SHOWNORMAL);

			m_pMakeModelDlg->CreateNewModel(m_vecModelPicPath);
			m_pModel = m_pMakeModelDlg->m_pModel;
			m_bmpBtnNew.EnableWindow(FALSE);
		}

		delete pResult;
		pResult = NULL;
	}
	return 1;
}

LRESULT CNewMakeModelDlg::ScanErr(WPARAM wParam, LPARAM lParam)
{
	pST_SCAN_RESULT pResult = (pST_SCAN_RESULT)wParam;
	if (pResult)
	{
		TRACE("扫描错误。%s\n", pResult->strResult.c_str());
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "扫描失败");
		dlg.DoModal();

		m_vecModelPicPath.clear();
		m_bmpBtnNew.EnableWindow(TRUE);

		delete pResult;
		pResult = NULL;
	}
	return 1;
}

void CNewMakeModelDlg::OnBnClickedBtnNewmakemodel()
{
	if (m_comboSubject.GetCurSel() >= 0)
		_pCurrSub_ = (pEXAM_SUBJECT)m_comboSubject.GetItemDataPtr(m_comboSubject.GetCurSel());
	else
		_pCurrSub_ = NULL;

	if (!_pCurrSub_)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "科目信息为空");
		dlg.DoModal();
		return;
	}
	if (m_pModel)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "当前科目模板已存在，无法创建");
		dlg.DoModal();
		return;
	}

	m_vecModelPicPath.clear();

	CCreateModelDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	if (dlg.m_nSearchType == 1)
	{
		m_strScanPicPath = dlg.m_strScanSavePath;
		m_bmpBtnNew.EnableWindow(FALSE);
	}
	else if (dlg.m_nSearchType == 2)
	{
		m_pMakeModelDlg->CreateNewModel(dlg.m_vecPath);
		m_pModel = m_pMakeModelDlg->m_pModel;
		m_bmpBtnNew.EnableWindow(FALSE);
	}

}

void CNewMakeModelDlg::OnCbnSelchangeComboMakemodelSubject()
{
	if (m_comboSubject.GetCurSel() < 0)
		return;

	pEXAM_SUBJECT pSubject = (pEXAM_SUBJECT)m_comboSubject.GetItemDataPtr(m_comboSubject.GetCurSel());
	if (_pCurrSub_ != pSubject)
	{
		_pCurrSub_ = pSubject;
		m_bmpBtnNew.EnableWindow(TRUE);

		SAFE_RELEASE(m_pModel);
		m_pModel = LoadSubjectModel(_pCurrSub_);
		if (m_pMakeModelDlg)
			m_pMakeModelDlg->ReInitModel(m_pModel);
	}
}


void CNewMakeModelDlg::OnBnClickedBtnSavemodel()
{
	if (!m_pModel)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "请先创建模板！");
		dlg.DoModal();
		return;
	}

	m_pMakeModelDlg->SaveNewModel();
}

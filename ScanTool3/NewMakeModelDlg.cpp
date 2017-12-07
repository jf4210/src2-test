// MakeModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "NewMakeModelDlg.h"
#include "afxdialogex.h"
#include "ScanModelPaperDlg.h"
#include "NewMessageBox.h"
#include "CreateModelDlg.h"
#include "Net_Cmd_Protocol.h"
#include "GetModelDlg.h"
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
	DDX_Control(pDX, IDC_BTN_DownModel, m_bmpBtnDown);
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
	ON_BN_CLICKED(IDC_BTN_UploadPic, &CNewMakeModelDlg::OnBnClickedBtnUploadpic)
	ON_BN_CLICKED(IDC_BTN_DownModel, &CNewMakeModelDlg::OnBnClickedBtnDownmodel)
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

	int nBtnW = 110;
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
	if (m_bmpBtnScan.GetSafeHwnd() && m_bmpBtnScan.IsWindowVisible())
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
	m_btnFont.CreateFont(20, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("Arial"));
	GetDlgItem(IDC_STATIC_MakeModel_CurrSubject)->SetFont(&fontStatus);
	m_bmpBtnNew.SetBtnFont(m_btnFont);
	m_bmpBtnSave.SetBtnFont(m_btnFont);
	m_bmpBtnScan.SetBtnFont(m_btnFont);
	m_bmpBtnUpload.SetBtnFont(m_btnFont);
	m_bmpBtnDown.SetBtnFont(m_btnFont);
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
	m_bmpBtnNew.SetStateBitmap(IDB_MakeModel_Btn_New_normal, 0, IDB_MakeModel_Btn_New_down);
	m_bmpBtnNew.SetWindowText(_T("    新建模板"));
	m_bmpBtnScan.SetStateBitmap(IDB_MakeModel_Btn_Scan_Normal, 0, IDB_MakeModel_Btn_Scan_Down); 
	m_bmpBtnScan.SetWindowText(_T("    扫描题卡"));
	m_bmpBtnSave.SetStateBitmap(IDB_MakeModel_Btn_Save_Normal, 0, IDB_MakeModel_Btn_Save_Down);
	m_bmpBtnSave.SetWindowText(_T("    保存模板"));
	m_bmpBtnUpload.SetStateBitmap(IDB_MakeModel_Btn_Upload_Normal, 0, IDB_MakeModel_Btn_Upload_Down);
	m_bmpBtnUpload.SetWindowText(_T("    上传图片"));
	m_bmpBtnDown.SetStateBitmap(IDB_MakeModel_Btn_Download_normal, 0, IDB_MakeModel_Btn_Download_down);
	m_bmpBtnDown.SetWindowText(_T("    下载模板"));
	
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

	USES_CONVERSION;
	std::string strModelName = pSubModel->strModelName;
	if (strModelName.empty())
	{
		char szModelName[150] = { 0 };
		sprintf_s(szModelName, "%s_%s_N_%d_%d.mod", _pCurrExam_->strExamName.c_str(), pSubModel->strSubjName.c_str(), _pCurrExam_->nExamID, pSubModel->nSubjID);
		strModelName = szModelName;
		//return NULL;
	}

	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
	strModelPath = CMyCodeConvert::Utf8ToGb2312(g_strModelSavePath) + "\\" + strModelName;	//gb2312
	std::string strBaseModelName;
	try
	{
		Poco::File fileModel(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
		if (!fileModel.exists())
		{
			std::string strLog = "检查模板文件路径下模板不存在: " + strModelPath;
			g_pLogger->information(strLog);
			return NULL;
		}

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
	if (pModel)
	{
		std::string strLog = "加载模板(" + pModel->strModelName + ")完成";
		g_pLogger->information(strLog);
	}

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
		if (!pResult->bScanOK)
		{
			MODELPICPATH picInfo;
			picInfo.strName = A2T(pResult->strPicName.c_str());
			picInfo.strPath = A2T(pResult->strPicPath.c_str());
			m_vecModelPicPath.push_back(picInfo);
		}
		
		if (pResult->bScanOK)	//扫描完成
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(1, 1, "扫描完成");
			dlg.DoModal();
// 			CString strSelect = _T("/root,");
// 			strSelect.Append(m_strScanPicPath);
// 			ShellExecute(NULL, _T("open"), _T("explorer.exe"), strSelect, NULL, SW_SHOWNORMAL);

			SAFE_RELEASE(m_pModel);
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
		if (_nScanStatus_ != -4)
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "扫描失败");
			dlg.DoModal();
		}
		else
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "扫描已取消");
			dlg.DoModal();
		}

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
// 	if (m_pModel)
// 	{
// 		CNewMessageBox dlg;
// 		dlg.setShowInfo(2, 1, "当前科目模板已存在，无法创建");
// 		dlg.DoModal();
// 		return;
// 	}
	
	CCreateModelDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;
	
	m_vecModelPicPath.clear();
	if (dlg.m_nSearchType == 1)
	{
		m_strScanPicPath = dlg.m_strScanSavePath;
		m_bmpBtnNew.EnableWindow(FALSE);
	}
	else if (dlg.m_nSearchType == 2)
	{
		SAFE_RELEASE(m_pModel);
		m_vecModelPicPath = dlg.m_vecPath;
		m_pMakeModelDlg->CreateNewModel(m_vecModelPicPath);
		m_pModel = m_pMakeModelDlg->m_pModel;
		m_bmpBtnNew.EnableWindow(FALSE);
	}
	else if (dlg.m_nSearchType == 3)
	{
		//下载模板图片
		g_eGetModelPic.reset();
		_nGetModelPic_ = 1;
		_vecModelPicPath_.clear();
		m_vecModelPicPath.clear();

		std::string strModelInfo = Poco::format("%d_%d", _pCurrExam_->nExamID, _pCurrSub_->nSubjID);
		pTCP_TASK pTcpTask = new TCP_TASK;
		pTcpTask->usCmd = USER_GET_MODEL_PIC;
		pTcpTask->nPkgLen = strModelInfo.length();
		memcpy(pTcpTask->szSendBuf, (char*)strModelInfo.c_str(), strModelInfo.length());
		g_fmTcpTaskLock.lock();
		g_lTcpTask.push_back(pTcpTask);
		g_fmTcpTaskLock.unlock();

		try
		{
			g_eGetModelPic.wait(20000);
		}
		catch (Poco::TimeoutException &e)
		{
			TRACE("获取模板图片超时\n");
		}

		if (_nGetModelPic_ == 2)
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(3, 1, "下载模板图像完成");
			dlg.DoModal();

			if (_vecModelPicPath_.size() > 0)
			{
				SAFE_RELEASE(m_pModel);
				m_vecModelPicPath = _vecModelPicPath_;
				m_pMakeModelDlg->CreateNewModel(m_vecModelPicPath);
				m_pModel = m_pMakeModelDlg->m_pModel;
				m_bmpBtnNew.EnableWindow(FALSE);
			}
		}
		else if (_nGetModelPic_ == 3)
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "服务器上无模板图像");
			dlg.DoModal();
		}
		else if (_nGetModelPic_ == 4)
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "下载模板图像失败");
			dlg.DoModal();
		}
		else if (_nGetModelPic_ == 1)
		{
			CNewMessageBox dlg;
			dlg.setShowInfo(2, 1, "下载模板图像超时");
			dlg.DoModal();
		}
		_nGetModelPic_ = 0;
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
		m_vecModelPicPath.clear();
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


void CNewMakeModelDlg::OnBnClickedBtnUploadpic()
{
	USES_CONVERSION;
	if (m_pModel)
	{
		if (m_pModel->vecPaperModel.size() > 0)
			m_vecModelPicPath.clear();
		for (int i = 0; i < m_pModel->vecPaperModel.size(); i++)
		{
			CString strPicPath = g_strCurrentPath + _T("Model\\") + A2T(m_pModel->strModelName.c_str()) + _T("\\") + A2T(m_pModel->vecPaperModel[i]->strModelPicName.c_str());
			MODELPICPATH picInfo;
			picInfo.strName = A2T(m_pModel->vecPaperModel[i]->strModelPicName.c_str());
			picInfo.strPath = strPicPath;
			m_vecModelPicPath.push_back(picInfo);
		}
	}
	if (m_vecModelPicPath.size() == 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "无模板图片信息！");
		dlg.DoModal();
		return;
	}

	std::stringstream ssLog;
	bool bFailFlag = false;
	for (int i = 0; i < m_vecModelPicPath.size(); i++)
	{
		USES_CONVERSION;
		std::string strPath = T2A(m_vecModelPicPath[i].strPath);
		std::string strPicName = T2A(m_vecModelPicPath[i].strName);
		std::string strPicPath = T2A(m_vecModelPicPath[i].strPath);
		std::string strMd5;
		std::string strExtName = strPicName.substr(strPicName.rfind("."));

		try
		{
			Poco::File fileModel(CMyCodeConvert::Gb2312ToUtf8(strPath));
			if (!fileModel.exists())
			{
				bFailFlag = true;
				CNewMessageBox dlg;
				dlg.setShowInfo(2, 1, "图像路径不存在！");
				dlg.DoModal();
				continue;
			}
			
			std::string strTmpSendDir = T2A(g_strCurrentPath);
			strTmpSendDir.append("Model\\Tmp\\");

			Poco::File fileModelPath(CMyCodeConvert::Gb2312ToUtf8(strTmpSendDir));
			fileModelPath.createDirectories();

			Poco::File fileOriPic(CMyCodeConvert::Gb2312ToUtf8(strPicPath));
			fileOriPic.copyTo(strTmpSendDir);
			strPicPath = strTmpSendDir + strPicName;
		}
		catch (Poco::Exception &e)
		{
			ssLog << "上传模板图片(" << strPicPath << ")异常: " << e.displayText() << "\n";
			continue;
		}

		ssLog << "添加["<< m_pModel->nExamID << m_pModel->nSubjectID <<"]模板上传图片: " << strPicName << "(" << strPicPath << ")\n";

		strMd5 = calcFileMd5(strPath);
		
		ST_MODELPIC stModelPic;
		ZeroMemory(&stModelPic, sizeof(ST_MODELPIC));
		stModelPic.nIndex = i + 1;
		stModelPic.nExamID = m_pModel->nExamID;
		stModelPic.nSubjectID = m_pModel->nSubjectID;
		strncpy(stModelPic.szPicName, strPicName.c_str(), strPicName.length());
		strncpy(stModelPic.szPicPath, strPicPath.c_str(), strPicPath.length());
		strncpy(stModelPic.szMD5, strMd5.c_str(), strMd5.length());
		strncpy(stModelPic.szExtName, strExtName.c_str(), strExtName.length());

		pTCP_TASK pTcpTask = new TCP_TASK;
		pTcpTask->usCmd = USER_NEED_UP_MODEL_PIC;
		pTcpTask->nPkgLen = sizeof(ST_MODELPIC);
		memcpy(pTcpTask->szSendBuf, (char*)&stModelPic, sizeof(ST_MODELPIC));
		g_fmTcpTaskLock.lock();
		g_lTcpTask.push_back(pTcpTask);
		g_fmTcpTaskLock.unlock();
	}
	if (!bFailFlag)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(3, 1, "添加上传任务完成！");
		dlg.DoModal();
	}
	g_pLogger->information(ssLog.str());
}

void CNewMakeModelDlg::OnBnClickedBtnDownmodel()
{
	USES_CONVERSION;
	CGetModelDlg dlg(A2T(g_strCmdIP.c_str()), g_nCmdPort);
	dlg.DoModal();
	
	SAFE_RELEASE(m_pModel);
	m_pModel = LoadSubjectModel(_pCurrSub_);
	if (m_pMakeModelDlg && m_pModel)
		m_pMakeModelDlg->ReInitModel(m_pModel);
}

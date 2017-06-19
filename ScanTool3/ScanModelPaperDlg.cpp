// ScanModelPaperDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanModelPaperDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"
#include "MakeModelDlg.h"

// CScanModelPaperDlg 对话框

IMPLEMENT_DYNAMIC(CScanModelPaperDlg, CDialog)

CScanModelPaperDlg::CScanModelPaperDlg(CWnd* pParent /*=NULL*/)
: CTipBaseDlg(CScanModelPaperDlg::IDD, pParent)
, m_nStatusSize(25), m_bAdvancedScan(false)
{

}

CScanModelPaperDlg::~CScanModelPaperDlg()
{
}


void CScanModelPaperDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_ScanModel_Close, m_bmpBtnClose);
	DDX_Control(pDX, IDC_BTN_MakeModel_Scan, m_bmpBtnScan);
	DDX_Control(pDX, IDC_COMBO_ScanModel_Src, m_comboScanSrc);
	DDX_Control(pDX, IDC_COMBO_ScanModel_Duplex, m_comboDuplex);
	DDX_Control(pDX, IDC_COMBO_ScanModel_PaperSize, m_comboPaperSize);
	DDX_Control(pDX, IDC_BTN_Browse, m_bmpBtnBrowser);
	DDX_Text(pDX, IDC_EDIT_ScanModel_SavePath, m_strSavePath);
}


BEGIN_MESSAGE_MAP(CScanModelPaperDlg, CTipBaseDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_ScanModel_Close, &CScanModelPaperDlg::OnBnClickedBtnScanmodelClose)
	ON_BN_CLICKED(IDC_CHK_MakeModel_Advance, &CScanModelPaperDlg::OnBnClickedChkMakemodelAdvance)
	ON_BN_CLICKED(IDC_BTN_Browse, &CScanModelPaperDlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTN_MakeModel_Scan, &CScanModelPaperDlg::OnBnClickedBtnMakemodelScan)
END_MESSAGE_MAP()


// CScanModelPaperDlg 消息处理程序
BOOL CScanModelPaperDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

//	InitScanner();
	SetScanSrcInfo(m_vecScanSrc);
	InitUI();
	SetFontSize(m_nStatusSize);
	InitCtrlPosition();
	
	return TRUE;
}


void CScanModelPaperDlg::InitUI()
{
	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);
	m_bmpBtnScan.SetStateBitmap(IDB_Btn_MakeModel_Normal, 0, IDB_Btn_MakeModel_Down);
	m_bmpBtnScan.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 0);
	m_comboScanSrc.AdjustDroppedWidth(); 
	m_comboDuplex.AdjustDroppedWidth();
	m_comboPaperSize.AdjustDroppedWidth();


	int nSrc = 0;
	int nDuplex = 1;
	char* ret;
	ret = new char[20];
	ret[0] = '\0';
	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanSrc", ret) == 0)
	{
		nSrc = atoi(ret);
	}
	memset(ret, 0, 20);

	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanDuplex", ret) == 0)
	{
		nDuplex = atoi(ret);
	}
	SAFE_RELEASE_ARRY(ret);
	if (nSrc >= m_comboScanSrc.GetCount())
		nSrc = 0;
	m_comboScanSrc.SetCurSel(nSrc);

	m_comboDuplex.ResetContent();
	m_comboDuplex.AddString(_T("单面扫描"));
	m_comboDuplex.AddString(_T("双面扫描"));
	m_comboDuplex.SetCurSel(nDuplex);

	m_comboPaperSize.ResetContent();
	m_comboPaperSize.AddString(_T("A4类型"));
	m_comboPaperSize.AddString(_T("A3类型"));
	m_comboPaperSize.SetCurSel(nDuplex);

	((CButton*)GetDlgItem(IDC_CHK_MakeModel_Advance))->SetCheck(m_bAdvancedScan);
}

void CScanModelPaperDlg::InitCtrlPosition()
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

	int nBtnW = 150;
	int nBtnH = 40;
	int nStaticH = 45;

	int nCurrLeft = nLeftGap;
	int nCurrTop = 5;
	if (GetDlgItem(IDC_STATIC_ScanModel_Title)->GetSafeHwnd())
	{
		int nW = cx / 2;
		GetDlgItem(IDC_STATIC_ScanModel_Title)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
	}
	if (m_bmpBtnClose.GetSafeHwnd())
	{
		nCurrLeft = cx - 20 - 8;
		m_bmpBtnClose.MoveWindow(nCurrLeft, 2, 20, 20);
	}

	int nBaseLeft = nLeftGap + (cx - nLeftGap - nRightGap) * 0.25;
	nCurrLeft = nBaseLeft;
	nCurrTop = nTopGap + 50;
	if (GetDlgItem(IDC_STATIC_ScanModel_ScanSrc)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.2;
		GetDlgItem(IDC_STATIC_ScanModel_ScanSrc)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (m_comboScanSrc.GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.3;
		m_comboScanSrc.MoveWindow(nCurrLeft, nCurrTop + 15, nW, nStaticH);
		nCurrTop += (nStaticH + nGap);
	}
	nCurrLeft = nBaseLeft;
	if (GetDlgItem(IDC_STATIC_ScanModel_Duplex)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.2;
		GetDlgItem(IDC_STATIC_ScanModel_Duplex)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (m_comboDuplex.GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.3;
		m_comboDuplex.MoveWindow(nCurrLeft, nCurrTop + 15, nW, nStaticH);
		nCurrTop += (nStaticH + nGap);
	}
	nCurrLeft = nBaseLeft;
	if (GetDlgItem(IDC_STATIC_ScanModel_PaperSize)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.2;
		GetDlgItem(IDC_STATIC_ScanModel_PaperSize)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (m_comboPaperSize.GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.3;
		m_comboPaperSize.MoveWindow(nCurrLeft, nCurrTop + 15, nW, nStaticH);
		nCurrTop += (nStaticH + nGap);
	}
	nCurrLeft = nBaseLeft;
	if (GetDlgItem(IDC_STATIC_ScanModel_SavePath)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.2;
		GetDlgItem(IDC_STATIC_ScanModel_SavePath)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_EDIT_ScanModel_SavePath)->GetSafeHwnd())
	{
		int nW = (cx - nLeftGap - nRightGap) * 0.4;
		GetDlgItem(IDC_EDIT_ScanModel_SavePath)->MoveWindow(nCurrLeft, nCurrTop + 15, nW, 25);
		nCurrLeft += (nW + 2);
	}
	if (m_bmpBtnBrowser.GetSafeHwnd())
	{
		m_bmpBtnBrowser.MoveWindow(nCurrLeft, nCurrTop + 15, 20, 25);
		nCurrTop += (25 + nGap);
	}

	nCurrLeft = (cx - nBtnW) / 2 - 20;
	nCurrTop = cy - nBottomGap - nBtnH;
	if (m_bmpBtnScan.GetSafeHwnd())
	{
		m_bmpBtnScan.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap * 2);
	}
	if (GetDlgItem(IDC_CHK_MakeModel_Advance)->GetSafeHwnd())
	{
		int nW = 100;
		int nH = nStaticH;
		nCurrTop += (nBtnH / 2 - nH / 2);
		GetDlgItem(IDC_CHK_MakeModel_Advance)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}
}

void CScanModelPaperDlg::SetFontSize(int nSize)
{
	CFont fontStatus;
	fontStatus.CreateFont(nSize, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("宋体"));	
	m_fontBtn.DeleteObject();
	m_fontBtn.CreateFont(15, 0, 0, 0,
						 FW_BOLD, FALSE, FALSE, 0,
						 DEFAULT_CHARSET,
						 OUT_DEFAULT_PRECIS,
						 CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY,
						 DEFAULT_PITCH | FF_SWISS,
						 _T("幼圆"));
	GetDlgItem(IDC_STATIC_ScanModel_Title)->SetFont(&fontStatus);
	GetDlgItem(IDC_STATIC_ScanModel_ScanSrc)->SetFont(&fontStatus);
	GetDlgItem(IDC_STATIC_ScanModel_Duplex)->SetFont(&fontStatus);
	GetDlgItem(IDC_STATIC_ScanModel_PaperSize)->SetFont(&fontStatus);
	GetDlgItem(IDC_STATIC_ScanModel_SavePath)->SetFont(&fontStatus);
	m_bmpBtnScan.SetBtnFont(m_fontBtn);
}

// void CScanModelPaperDlg::InitScanner()
// {
// 	_pTWAINApp = new TwainApp(m_hWnd);
// 
// 	TW_IDENTITY *pAppID = _pTWAINApp->getAppIdentity();
// 
// 	pAppID->Version.MajorNum = 2;
// 	pAppID->Version.MinorNum = 1;
// 	pAppID->Version.Language = TWLG_ENGLISH_CANADIAN;
// 	pAppID->Version.Country = TWCY_CANADA;
// 	SSTRCPY(pAppID->Version.Info, sizeof(pAppID->Version.Info), "2.1.1");
// 	pAppID->ProtocolMajor = TWON_PROTOCOLMAJOR;
// 	pAppID->ProtocolMinor = TWON_PROTOCOLMINOR;
// 	pAppID->SupportedGroups = DF_APP2 | DG_IMAGE | DG_CONTROL;
// 	SSTRCPY(pAppID->Manufacturer, sizeof(pAppID->Manufacturer), "TWAIN Working Group");
// 	SSTRCPY(pAppID->ProductFamily, sizeof(pAppID->ProductFamily), "Sample");
// 	SSTRCPY(pAppID->ProductName, sizeof(pAppID->ProductName), "MFC Supported Caps");
// 
// 	_pTWAINApp->connectDSM();
// 	if (_pTWAINApp->m_DSMState >= 3)
// 	{
// 		pTW_IDENTITY pID = NULL;
// 		int   i = 0;
// 		int   index = 0;
// 		int   nDefault = -1;
// 
// 		// Emply the list the refill
// 		m_vecScanSrc.clear();
// 
// 		if (NULL != (pID = _pTWAINApp->getDefaultDataSource())) // Get Default
// 		{
// 			nDefault = pID->Id;
// 		}
// 		USES_CONVERSION;
// 		while (NULL != (pID = _pTWAINApp->getDataSource((TW_INT16)i)))
// 		{
// 			m_vecScanSrc.push_back(A2T(pID->ProductName));
// 			if (LB_ERR == index)
// 			{
// 				break;
// 			}
// 
// 			i++;
// 		}
// 		_pTWAINApp->disconnectDSM();
// 	}
// }

void CScanModelPaperDlg::SetScanSrc(std::vector<CString>& vec)
{
	m_vecScanSrc = vec;
}

void CScanModelPaperDlg::SetScanSrcInfo(std::vector<CString>& vec)
{
	m_comboScanSrc.ResetContent();

	int nDefaultItem = -1;
	for (int i = 0; i < vec.size(); i++)
	{
		nDefaultItem = 0;
		m_comboScanSrc.AddString(vec[i]);
		m_comboScanSrc.SetItemData(i, i);
	}
	m_comboScanSrc.SetCurSel(nDefaultItem);

//	InitUI();
}

HBRUSH CScanModelPaperDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CTipBaseDlg::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_ScanModel_Title)
	{
		pDC->SetTextColor(RGB(100, 100, 100));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_ScanModel_ScanSrc || CurID == IDC_STATIC_ScanModel_Duplex
			 || CurID == IDC_STATIC_ScanModel_PaperSize || CurID == IDC_STATIC_ScanModel_SavePath)
	{
		pDC->SetTextColor(RGB(150, 150, 150));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_CHK_MakeModel_Advance)
	{
		HBRUSH hMYbr = ::CreateSolidBrush(RGB(255, 255, 255));	//62, 147, 254

		pDC->SetBkMode(TRANSPARENT);

		return hMYbr;
	}
	return hbr;
}

void CScanModelPaperDlg::OnBnClickedBtnScanmodelClose()
{
	OnCancel();
}

void CScanModelPaperDlg::OnBnClickedChkMakemodelAdvance()
{
	m_bAdvancedScan = ((CButton*)GetDlgItem(IDC_CHK_MakeModel_Advance))->GetCheck();
}


void CScanModelPaperDlg::OnBnClickedBtnBrowse()
{
	USES_CONVERSION;

	LPITEMIDLIST pidlRoot = NULL;
	SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlRoot);
	BROWSEINFO	bi;
	TCHAR		szPath[MAX_PATH];
	TCHAR		szFolderPath[MAX_PATH];
	ZeroMemory(&bi, sizeof(LPBROWSEINFO));
	bi.pidlRoot = pidlRoot;
	bi.lpszTitle = _T("选择试卷文件夹路径");
	bi.lParam = NULL;
	bi.pszDisplayName = szPath;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	LPITEMIDLIST lpIDList = SHBrowseForFolder(&bi);
	if (!lpIDList)
	{
		return;
	}
	SHGetPathFromIDList(lpIDList, szFolderPath);

	m_strSavePath = szFolderPath;
	UpdateData(FALSE);
}


void CScanModelPaperDlg::OnBnClickedBtnMakemodelScan()
{
	UpdateData(TRUE);
	USES_CONVERSION;
	std::string strSavePath = T2A(m_strSavePath);
	if (m_strSavePath == _T(""))
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "图像保存路径为空！");
		dlg.DoModal();
		return;
	}

	Poco::File fileSaveDir(CMyCodeConvert::Gb2312ToUtf8(strSavePath));
	if (!fileSaveDir.exists())
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "图像保存路径不存在或非法！");
		dlg.DoModal();
		return;
	}

	int           sel = m_comboScanSrc.GetCurSel();
	TW_INT16      index = (TW_INT16)m_comboScanSrc.GetItemData(sel);
	pTW_IDENTITY  pID = NULL;

	_nScanStatus_ = 0;

	char szPicTmpPath[MAX_PATH] = { 0 };
	sprintf_s(szPicTmpPath, "%s", T2A(m_strSavePath));

	std::string strUtfPath = CMyCodeConvert::Gb2312ToUtf8(szPicTmpPath);
	try
	{
		Poco::File tmpPath(strUtfPath);
		if (tmpPath.exists())
			tmpPath.remove(true);

		Poco::File tmpPath1(strUtfPath);
		tmpPath1.createDirectories();
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "删除临时文件夹失败(" + exc.message() + "): ";
		strLog.append(szPicTmpPath);
		g_pLogger->information(strLog);
	}
	
	//获取扫描参数
	int nScanSize = 1;				//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3, TWSS_NONE-自定义
	int nScanType = 2;				//0-黑白，1-灰度，2-彩色
	int nScanDpi = 200;				//dpi: 72, 150, 200, 300
	int nAutoCut = 1;

	bool bShowScanSrcUI = g_bShowScanSrcUI ? true : m_bAdvancedScan;			//显示高级扫描界面

	int nDuplex = m_comboDuplex.GetCurSel();		//单双面,0-单面,1-双面

	int nSize = m_comboPaperSize.GetCurSel();							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3, TWSS_NONE-自定义
	if (nScanSize == 0)
		nSize = TWSS_A4LETTER;
	else if (nScanSize == 1)
		nSize = TWSS_A3;
	else
		nSize = TWSS_NONE;

	int nNum = 0;
	if (nNum == 0)
		nNum = -1;

	CMakeModelDlg* pDlg = (CMakeModelDlg*)GetParent();
	if (NULL != (pID = pDlg->GetScanSrc(index)))
	{
		SAFE_RELEASE(_pCurrPapersInfo_);
		_pCurrPapersInfo_ = new PAPERSINFO();


		_nScanStatus_ = 1;
		pST_SCANCTRL pScanCtrl = new ST_SCANCTRL();
		pScanCtrl->nScannerId = pID->Id;
		pScanCtrl->nScanCount = nNum;			//nNum
		pScanCtrl->nScanDuplexenable = nDuplex;
		pScanCtrl->nScanPixelType = nScanType;
		pScanCtrl->nScanResolution = nScanDpi;
		pScanCtrl->nScanSize = nSize;
		pScanCtrl->bShowUI = bShowScanSrcUI;	//bShowScanSrcUI;

		pDlg->m_scanThread.setNotifyDlg(pDlg);
		pDlg->m_scanThread.setModelInfo(0, strSavePath);
		pDlg->m_scanThread.resetData();
		pDlg->m_scanThread.PostThreadMessage(MSG_START_SCAN, pID->Id, (LPARAM)pScanCtrl);
	}
	else
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "获取扫描源失败");
		dlg.DoModal();
	}

	OnOK();
}

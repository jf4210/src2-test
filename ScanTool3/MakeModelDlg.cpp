// MakeModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "MakeModelDlg.h"
#include "afxdialogex.h"
#include "ScanModelPaperDlg.h"

// CMakeModelDlg 对话框

IMPLEMENT_DYNAMIC(CMakeModelDlg, CDialog)

CMakeModelDlg::CMakeModelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMakeModelDlg::IDD, pParent)
	, m_nStatusSize(30), _pTWAINApp(NULL)
{

}

CMakeModelDlg::~CMakeModelDlg()
{
}

void CMakeModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_ScanPaper, m_bmpBtnScan);
	DDX_Control(pDX, IDC_BTN_SaveModel, m_bmpBtnSave);
	DDX_Control(pDX, IDC_BTN_UploadPic, m_bmpBtnUpload);
	DDX_Control(pDX, IDC_BTN_InputPic, m_bmpBtnDown);
	DDX_Control(pDX, IDC_COMBO_MakeModel_Subject, m_comboSubject);
}


BEGIN_MESSAGE_MAP(CMakeModelDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_ScanPaper, &CMakeModelDlg::OnBnClickedBtnScanpaper)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMakeModelDlg 消息处理程序


BOOL CMakeModelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	SetFontSize(m_nStatusSize);
	m_comboSubject.AdjustDroppedWidth();
// 	InitExamData();
// 	InitChildDlg();
// 	InitCtrlPosition();
// 	m_scanThread.CreateThread();
// 
// 	SetFontSize(m_nStatusSize);
// 	m_comboSubject.AdjustDroppedWidth();

	InitCtrlPosition();


	InitScanner();
	m_scanThread.CreateThread();

	return TRUE;
}

BOOL CMakeModelDlg::PreTranslateMessage(MSG* pMsg)
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

void CMakeModelDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CMakeModelDlg::InitCtrlPosition()
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
	if (m_bmpBtnScan.GetSafeHwnd())
	{
		m_bmpBtnScan.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
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
}

void CMakeModelDlg::SetFontSize(int nSize)
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

void CMakeModelDlg::InitUI()
{
	m_bmpBtnScan.SetStateBitmap(IDB_MakeModel_Btn_Scan_Normal, 0, IDB_MakeModel_Btn_Scan_Down); 
	m_bmpBtnScan.SetWindowText(_T("    扫描题卡"));
	m_bmpBtnSave.SetStateBitmap(IDB_MakeModel_Btn_Save_Normal, 0, IDB_MakeModel_Btn_Save_Down);
	m_bmpBtnSave.SetWindowText(_T("    保存模板"));
	m_bmpBtnUpload.SetStateBitmap(IDB_MakeModel_Btn_Upload_Normal, 0, IDB_MakeModel_Btn_Upload_Down);
	m_bmpBtnUpload.SetWindowText(_T("    上传图片"));
	m_bmpBtnDown.SetStateBitmap(IDB_MakeModel_Btn_Download_normal, 0, IDB_MakeModel_Btn_Download_down);
	m_bmpBtnDown.SetWindowText(_T("    导入图片"));
}

void CMakeModelDlg::OnBnClickedBtnScanpaper()
{
	CScanModelPaperDlg dlg;
	dlg.SetScanSrc(m_vecScanSrc);
	dlg.DoModal();
}


HBRUSH CMakeModelDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


BOOL CMakeModelDlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 242, 250


	return TRUE;
}


void CMakeModelDlg::OnDestroy()
{
	CDialog::OnDestroy();

//	m_scanThread.PostThreadMessage(WM_QUIT, NULL, NULL);
	if (_pTWAINApp)
	{
		_pTWAINApp->exit();
		SAFE_RELEASE(_pTWAINApp);
	}


//	::WaitForSingleObject(m_scanThread.m_hThread, INFINITE);
}

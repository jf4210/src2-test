// GuideDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "GuideDlg.h"
#include "afxdialogex.h"

#include "ScanModleMgrDlg.h"

// CGuideDlg 对话框


IMPLEMENT_DYNAMIC(CGuideDlg, CDialog)

CGuideDlg::CGuideDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGuideDlg::IDD, pParent)
	, m_pModel(NULL), m_pScanDlg(NULL)
{

}

CGuideDlg::~CGuideDlg()
{
}

void CGuideDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGuideDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_Scan, &CGuideDlg::OnBnClickedBtnScan)
	ON_BN_CLICKED(IDC_BTN_Exit, &CGuideDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_Model, &CGuideDlg::OnBnClickedBtnModel)
	ON_BN_CLICKED(IDC_BTN_Param, &CGuideDlg::OnBnClickedBtnParam)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CGuideDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	CString strTitle = _T("");
	strTitle.Format(_T("%s %s"), SYS_BASE_NAME, SOFT_VERSION);
	SetWindowText(strTitle);

	InitConf();
	InitCtrlPosition();

//	SearchModel();

	return TRUE;
}

void CGuideDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = cy / 10;	//上边的间隔，留给控制栏
	if (nTopGap > 90)
		nTopGap = 90;
	else if (nTopGap < 30)
		nTopGap = 30;

	const int nLeftGap = 20;		//左边的空白间隔
	const int nBottomGap = 20;	//下边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nBtnW = (cx - nLeftGap - nRightGap - nGap) / 2;
	int nBtnH = (cy - nTopGap - nRightGap - nGap) / 2;

	if (nBtnW < 20)	nBtnW = 30;
	if (nBtnH < 20) nBtnH = 30;

	int nCurrentTop		= nTopGap;
	int nCurrentLeft	= nLeftGap;
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nCurrentLeft + nBtnW + nGap;
	}
	if (GetDlgItem(IDC_BTN_Model)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Model)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nBtnH + nGap;
	}
	if (GetDlgItem(IDC_BTN_Param)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Param)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nCurrentLeft + nBtnW + nGap;
	}
	if (GetDlgItem(IDC_BTN_Exit)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Exit)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nBtnH + nGap;
	}
}


// CGuideDlg 消息处理程序


void CGuideDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CGuideDlg::InitConf()
{
	wchar_t szwPath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, szwPath, MAX_PATH);
	char szPath[MAX_PATH] = { 0 };
	int nLen = WideCharToMultiByte(CP_ACP, 0, szwPath, -1, NULL, 0, NULL, NULL);
	char* pszDst = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szwPath, -1, szPath, nLen, NULL, NULL);
	szPath[nLen - 1] = 0;
	delete[] pszDst;

	CString strFile(szPath);
	strFile = strFile.Left(strFile.ReverseFind('\\') + 1);
	g_strCurrentPath = strFile;

	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
	g_strModelSavePath = CMyCodeConvert::Gb2312ToUtf8(strModelPath);
	Poco::File fileModelPath(g_strModelSavePath);
	fileModelPath.createDirectories();

	InitLog();
}

void CGuideDlg::InitLog()
{
	USES_CONVERSION;
	std::string strLogPath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath + _T("ScanTool.log")));
	Poco::AutoPtr<Poco::PatternFormatter> pFormatter(new Poco::PatternFormatter("%L%Y-%m-%d %H:%M:%S.%F %q:%t"));
	Poco::AutoPtr<Poco::FormattingChannel> pFCFile(new Poco::FormattingChannel(pFormatter));
	Poco::AutoPtr<Poco::FileChannel> pFileChannel(new Poco::FileChannel(strLogPath));
	pFCFile->setChannel(pFileChannel);
	pFCFile->open();
	pFCFile->setProperty("rotation", "1 M");
	pFCFile->setProperty("archive", "timestamp");
	pFCFile->setProperty("compress", "true");
	pFCFile->setProperty("purgeCount", "5");
	Poco::Logger& appLogger = Poco::Logger::create("ScanTool", pFCFile, Poco::Message::PRIO_INFORMATION);
	g_pLogger = &appLogger;
}
void CGuideDlg::OnBnClickedBtnScan()
{
	if (!m_pScanDlg)
	{
		m_pScanDlg = new CScanToolDlg(m_pModel);
		m_pScanDlg->Create(CScanToolDlg::IDD, this);
		m_pScanDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		m_pScanDlg->ShowWindow(SW_SHOW);
	}

// 	CScanToolDlg dlg(m_pModel);
// 	dlg.DoModal();
}


void CGuideDlg::OnBnClickedBtnExit()
{
	OnCancel();
}


void CGuideDlg::OnBnClickedBtnModel()
{
	CScanModleMgrDlg modelMgrDlg(m_pModel);
	if (modelMgrDlg.DoModal() != IDOK)
	{
		SAFE_RELEASE(modelMgrDlg.m_pModel);
		return;
	}
	if (m_pModel != modelMgrDlg.m_pModel)
	{
		SAFE_RELEASE(m_pModel);
		m_pModel = modelMgrDlg.m_pModel;
	}
}


void CGuideDlg::OnBnClickedBtnParam()
{
	
}



void CGuideDlg::OnDestroy()
{
	CDialog::OnDestroy();

	SAFE_RELEASE(m_pScanDlg);

}

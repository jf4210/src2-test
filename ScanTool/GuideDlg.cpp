// GuideDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "GuideDlg.h"
#include "afxdialogex.h"

#include "LoginDlg.h"
#include "ScanModleMgrDlg.h"
#include "ParamSetDlg.h"
#include "Net_Cmd_Protocol.h"

// CGuideDlg 对话框


IMPLEMENT_DYNAMIC(CGuideDlg, CDialog)

CGuideDlg::CGuideDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGuideDlg::IDD, pParent)
	, m_pModel(NULL), m_pScanDlg(NULL), m_bLogin(FALSE), m_nTeacherId(-1), m_nUserId(-1)
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
	ON_BN_CLICKED(IDC_BTN_Login, &CGuideDlg::OnBnClickedBtnLogin)
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

	m_pScanDlg = new CScanToolDlg(m_pModel);
	m_pScanDlg->Create(CScanToolDlg::IDD, this);
	m_pScanDlg->ShowWindow(SW_HIDE);
	CenterWindow();
//	ShowWindow(SW_SHOW);

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

	int nCurrentTop		= nTopGap;
	int nCurrentLeft	= nLeftGap;

#if 1
	int nClientW = cx - nLeftGap - nRightGap;
	int nClientH = cy - nTopGap - nRightGap;
	int nBtnW = (nClientW - nGap * 2) / 3;
	int nBtnH = (nClientH - nGap) / 2;

	if (nBtnW < 20)	nBtnW = 30;
	if (nBtnH < 20) nBtnH = 30;

	
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nCurrentLeft, nCurrentTop, nClientW - nBtnW - nGap, nBtnH);
		nCurrentLeft = nCurrentLeft + nClientW - nBtnW - nGap + nGap;
	}
	if (GetDlgItem(IDC_BTN_Login)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Login)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nBtnH + nGap;
	}
	if (GetDlgItem(IDC_BTN_Model)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Model)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nCurrentLeft + nBtnW + nGap;
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
#else
	int nBtnW = (nClientW - nGap) / 2;
	int nBtnH = (nClientH - nGap) / 2;

	if (nBtnW < 20)	nBtnW = 30;
	if (nBtnH < 20) nBtnH = 30;

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
#endif
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

		CRect rt;
		m_pScanDlg->GetWindowRect(&rt);
		SetCursorPos(rt.left + 30, rt.top + 300);
	}
	else
	{
		m_pScanDlg->InitShow(m_pModel);
		m_pScanDlg->InitScan();

		CRect rt;
		m_pScanDlg->GetWindowRect(&rt);
		SetCursorPos(rt.left + 30, rt.top + 300);
		m_pScanDlg->ShowWindow(SW_SHOW);
		ShowWindow(SW_HIDE);
	}
}

void CGuideDlg::OnBnClickedBtnExit()
{
	if (m_pModel != m_pScanDlg->m_pModel)
		SAFE_RELEASE(m_pModel);

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
//		SAFE_RELEASE(m_pModel);
		m_pModel = modelMgrDlg.m_pModel;
	}
}

void CGuideDlg::OnBnClickedBtnParam()
{
	CParamSetDlg dlg;
	dlg.m_strFileIP = g_strCmdIP.c_str();
	dlg.m_strCmdIP	= g_strCmdIP.c_str();
	dlg.m_nCmdPort	= g_nCmdPort;
	dlg.m_nFilePort = g_nFilePort;
	if (dlg.DoModal() != IDOK)
		return;


	USES_CONVERSION;
	bool bChange = false;
	if (g_strCmdIP != T2A(dlg.m_strCmdIP))		bChange = true;
	if (g_strFileIP != T2A(dlg.m_strFileIP))	bChange = true;
	if (g_nFilePort != dlg.m_nFilePort)			bChange = true;
	if (g_nCmdPort != dlg.m_nCmdPort)			bChange = true;

	g_strCmdIP	= T2A(dlg.m_strCmdIP);
	g_strFileIP = T2A(dlg.m_strFileIP);
	g_nFilePort = dlg.m_nFilePort;
	g_nCmdPort	= dlg.m_nCmdPort;

	if (bChange)
	{
		CString strConfigPath = g_strCurrentPath;
		strConfigPath.Append(_T("config.ini"));
// 		std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strConfigPath));
// 		Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
// 		pConf->setString("Server.fileIP", g_strFileIP);
// 		pConf->setString("Server.cmdIP", g_strCmdIP);
// 		pConf->setInt("Server.filePort", g_nFilePort);
// 		pConf->setInt("Server.cmdPort", g_nCmdPort);

		WritePrivateProfileString(_T("Server"), _T("fileIP"), dlg.m_strFileIP, strConfigPath);
		WritePrivateProfileString(_T("Server"), _T("cmdIP"), dlg.m_strCmdIP, strConfigPath);

		CString strVal;
		strVal.Format(_T("%d"), g_nFilePort);
		WritePrivateProfileString(_T("Server"), _T("filePort"), strVal, strConfigPath);
		strVal.Format(_T("%d"), g_nCmdPort);
		WritePrivateProfileString(_T("Server"), _T("cmdPort"), strVal, strConfigPath);
	}	
}

void CGuideDlg::OnDestroy()
{
	CDialog::OnDestroy();
	
	SAFE_RELEASE(m_pScanDlg);
}


void CGuideDlg::OnBnClickedBtnLogin()
{
	USES_CONVERSION;
	if (!m_bLogin)
	{
		CLoginDlg dlg(A2T(g_strCmdIP.c_str()), g_nCmdPort);
		if (dlg.DoModal() != IDOK)
		{
			g_lExamList.clear();
			m_bLogin = FALSE;
			m_strUserName = _T("");
			m_strNickName = _T("");
			m_strEzs = _T("");
			m_strPwd = _T("");
			m_nTeacherId = -1;
			m_nUserId = -1;
			GetDlgItem(IDC_BTN_Login)->SetWindowTextW(_T("登录"));
		}
		else
		{
			m_bLogin = TRUE;
			m_strUserName = dlg.m_strUserName;
			m_strNickName = dlg.m_strNickName;
			m_strPwd = dlg.m_strPwd;
			m_strEzs = dlg.m_strEzs;
			m_nTeacherId = dlg.m_nTeacherId;
			m_nUserId = dlg.m_nUserId;
			GetDlgItem(IDC_BTN_Login)->SetWindowTextW(_T("退出"));
		}
	}
	else
	{
		std::string strUser = T2A(m_strUserName);
		pTCP_TASK pTcpTask = new TCP_TASK;
		pTcpTask->usCmd = USER_LOGOUT;
		pTcpTask->nPkgLen = strUser.length();
		memcpy(pTcpTask->szSendBuf, (char*)strUser.c_str(), strUser.length());
		g_fmTcpTaskLock.lock();
		g_lTcpTask.push_back(pTcpTask);
		g_fmTcpTaskLock.unlock();

		g_lExamList.clear();
		m_bLogin = FALSE;
		m_strUserName = _T("");
		m_strNickName = _T("");
		m_strPwd = _T("");
		m_strEzs = _T("");
		m_nTeacherId = -1;
		m_nUserId = -1;
		GetDlgItem(IDC_BTN_Login)->SetWindowTextW(_T("登录"));
	}
	m_pScanDlg->m_pShowScannerInfoDlg->setShowInfo(m_strUserName, m_strNickName);
}

BOOL CGuideDlg::PreTranslateMessage(MSG* pMsg)
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
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

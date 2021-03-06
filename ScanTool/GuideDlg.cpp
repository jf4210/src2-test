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
	, m_pModel(NULL), m_pScanDlg(NULL), m_pShowScannerInfoDlg(NULL), m_bLogin(FALSE), m_nTeacherId(-1), m_nUserId(-1)
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
//	ON_BN_CLICKED(IDC_BTN_Exit, &CGuideDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_Model, &CGuideDlg::OnBnClickedBtnModel)
//	ON_BN_CLICKED(IDC_BTN_Param, &CGuideDlg::OnBnClickedBtnParam)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_Login, &CGuideDlg::OnBnClickedBtnLogin)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_Setting, &CGuideDlg::OnNMClickSyslinkSetting)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_Login, &CGuideDlg::OnNMClickSyslinkLogin)
	ON_MESSAGE(MSG_NOTIFY_UPDATE, CGuideDlg::MSG_UpdateNotify)
END_MESSAGE_MAP()

BOOL CGuideDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	CString strTitle = _T("");
	strTitle.Format(_T("%s %s %s"), SYS_BASE_NAME, SYS_GUIDE_NAME, SOFT_VERSION);
	SetWindowText(strTitle);

	m_pShowScannerInfoDlg = new CScanerInfoDlg(this);
	m_pShowScannerInfoDlg->Create(CScanerInfoDlg::IDD, this);
	m_pShowScannerInfoDlg->ShowWindow(SW_SHOW);

	InitConf();
	InitCtrlPosition();
	
	m_pScanDlg = new CScanToolDlg(m_pModel);
	m_pScanDlg->Create(CScanToolDlg::IDD, this);
	m_pScanDlg->ShowWindow(SW_HIDE);
	CenterWindow();
//	ShowWindow(SW_SHOW);


	if (CheckNewVersion())
		return FALSE;

	return TRUE;
}

LRESULT CGuideDlg::MSG_UpdateNotify(WPARAM wParam, LPARAM lParam)
{
	if (MessageBox(_T("有新版本可用，是否升级?"), _T("升级通知"), MB_YESNO) != IDYES)
		return FALSE;
	
	if (m_pModel != m_pScanDlg->m_pModel)
		SAFE_RELEASE(m_pModel);
	DestroyWindow();
	return TRUE;
}

BOOL CGuideDlg::CheckNewVersion()
{
	USES_CONVERSION;
	CString strVerPath = g_strCurrentPath + _T("newSetupPkg\\");
	Poco::File verPath(CMyCodeConvert::Gb2312ToUtf8(T2A(strVerPath)));
	if (verPath.exists())
	{
		CString strNewSetupFile = strVerPath + _T("setup.exe");
		Poco::File newSetupFile(CMyCodeConvert::Gb2312ToUtf8(T2A(strNewSetupFile)));
		if (newSetupFile.exists())
		{
			TCHAR buf[MAX_PATH];
			GetTempPathW(MAX_PATH, buf);

			CString strTmpPath = _T("");
			strTmpPath.Format(_T("%ssetup.exe"), buf);

			try
			{
				CString strCheckPath = g_strCurrentPath + _T("\\setup.exe");
				Poco::File checkFile(CMyCodeConvert::Gb2312ToUtf8(T2A(strCheckPath)));
				if (checkFile.exists())
					checkFile.remove(true);
				newSetupFile.copyTo(CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath)));

				newSetupFile.moveTo(CMyCodeConvert::Gb2312ToUtf8(T2A(strTmpPath)));

				SHELLEXECUTEINFOA TempInfo = { 0 };

				TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
				TempInfo.fMask = 0;
				TempInfo.hwnd = NULL;
				TempInfo.lpVerb = "runas";
				TempInfo.lpFile = T2A(strTmpPath);
				TempInfo.lpParameters = "";
				TempInfo.lpDirectory = T2A(buf);
				TempInfo.nShow = SW_NORMAL;
				TempInfo.hInstApp = NULL;

				int nResult = ::ShellExecuteExA(&TempInfo);

				//运行卸载程序
				CString strKillExe = g_strCurrentPath + _T("Uninstall.exe");
				Poco::File killFile(CMyCodeConvert::Gb2312ToUtf8(T2A(strKillExe)));
				if (killFile.exists())
				{
					SHELLEXECUTEINFOA TempInfo2 = { 0 };

					TempInfo2.cbSize = sizeof(SHELLEXECUTEINFOA);
					TempInfo2.fMask = 0;
					TempInfo2.hwnd = NULL;
					TempInfo2.lpVerb = "runas";
					TempInfo2.lpFile = T2A(strKillExe);
					TempInfo2.lpParameters = "";
					TempInfo2.lpDirectory = T2A(g_strCurrentPath);
					TempInfo2.nShow = SW_HIDE;
					TempInfo2.hInstApp = NULL;

					int nResult = ::ShellExecuteExA(&TempInfo2);
				}

				return TRUE;
			}
			catch (Poco::Exception& exc)
			{
				std::string strLog = "移动新版安装文件到系统临时目录失败";
				g_pLogger->information(strLog);
			}
		}
		else
		{
			verPath.remove(true);
		}
	}

	return FALSE;
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
	else if (nTopGap < 60)
		nTopGap = 60;

#if 1
	const int nLeftGap = 20;		//左边的空白间隔
	const int nBottomGap = 20;	//下边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	const int nGap = 40;			//普通控件的间隔

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	int nClientW = cx - nLeftGap - nRightGap;
	int nClientH = cy - nTopGap - nRightGap;
	int nMin = (nClientW - nGap > nClientH) ? nClientH : nClientW - nGap;
	int nBtnW = (nClientW - nGap) / 2;
	int nBtnH = nClientH > nBtnW ? nBtnW : nClientH;

	if (nBtnW < 30)	nBtnW = 30;
	if (nBtnH < 30) nBtnH = 30;

	int nLeft_Top = cx - nRightGap - 120;
	if (m_pShowScannerInfoDlg && m_pShowScannerInfoDlg->GetSafeHwnd())
	{
		m_pShowScannerInfoDlg->MoveWindow(5, 5, nClientW / 3, nTopGap - 10);
	}
	if (GetDlgItem(IDC_SYSLINK_Login)->GetSafeHwnd())
	{
		GetDlgItem(IDC_SYSLINK_Login)->MoveWindow(nLeft_Top, 5, 50, 30);
	}
	if (GetDlgItem(IDC_SYSLINK_Setting)->GetSafeHwnd())
	{
		GetDlgItem(IDC_SYSLINK_Setting)->MoveWindow(nLeft_Top + 60, 5, 50, 30);
	}
	
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nCurrentLeft + nBtnW + nGap;
	}
	if (GetDlgItem(IDC_BTN_Model)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Model)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nCurrentLeft + nBtnW + nGap;
	}
#else
	const int nLeftGap = 20;		//左边的空白间隔
	const int nBottomGap = 20;	//下边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nCurrentTop		= nTopGap;
	int nCurrentLeft	= nLeftGap;

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
	std::string strLogPath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath + _T("ScanTool.Log")));
	Poco::AutoPtr<Poco::PatternFormatter> pFormatter(new Poco::PatternFormatter("%L%Y-%m-%d %H:%M:%S.%F %q:%t"));
	Poco::AutoPtr<Poco::FormattingChannel> pFCFile(new Poco::FormattingChannel(pFormatter));
	Poco::AutoPtr<Poco::FileChannel> pFileChannel(new Poco::FileChannel(strLogPath));
	pFCFile->setChannel(pFileChannel);
	pFCFile->open();
	pFCFile->setProperty("rotation", "20 M");
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

// 		CRect rt;
// 		m_pScanDlg->GetWindowRect(&rt);
//		SetCursorPos(rt.left + 30, rt.top + 300);
		CPoint pt;
		GetCursorPos(&pt);
		mouse_event(MOUSEEVENTF_LEFTDOWN, pt.x, pt.y, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, pt.x, pt.y, 0, 0);
	}
	else
	{
		m_pScanDlg->InitShow(m_pModel);
		m_pScanDlg->InitScan();

// 		CRect rt;
// 		m_pScanDlg->GetWindowRect(&rt);
//		SetCursorPos(rt.left + 30, rt.top + 300);

		ShowWindow(SW_HIDE);
		m_pScanDlg->ShowWindow(SW_SHOW);
		
		CPoint pt;
		GetCursorPos(&pt);
		mouse_event(MOUSEEVENTF_LEFTDOWN, pt.x, pt.y, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, pt.x, pt.y, 0, 0);
	}
	m_pScanDlg->m_bF1Enable = TRUE;
	m_pScanDlg->m_bF2Enable = TRUE;
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
	if (g_strCmdIP != T2A(dlg.m_strCmdIP) || g_nCmdPort != dlg.m_nCmdPort)
	{
		bChange = true;
		g_bCmdNeedConnect = true;
	}
	if (g_strFileIP != T2A(dlg.m_strFileIP) || g_nFilePort != dlg.m_nFilePort)
	{
		bChange = true;
		g_bFileNeedConnect = true;
	}

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

void CGuideDlg::OnClose()
{
	if (m_pModel != m_pScanDlg->m_pModel)
		SAFE_RELEASE(m_pModel);

	CDialog::OnClose();
}

void CGuideDlg::OnDestroy()
{
	CDialog::OnDestroy();
	
	SAFE_RELEASE(m_pShowScannerInfoDlg);
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
			g_lfmExamList.lock();
			g_lExamList.clear();
			g_lfmExamList.unlock();
			m_bLogin = FALSE;
			m_strUserName = _T("");
			m_strNickName = _T("");
			m_strEzs = _T("");
			m_strPwd = _T("");
			m_nTeacherId = -1;
			m_nUserId = -1;
			GetDlgItem(IDC_SYSLINK_Login)->SetWindowTextW(_T("<a>登录</a>"));
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
			GetDlgItem(IDC_SYSLINK_Login)->SetWindowTextW(_T("<a>注销</a>"));
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

		g_lfmExamList.lock();
		g_lExamList.clear();
		g_lfmExamList.unlock();
		m_bLogin = FALSE;
		m_strUserName = _T("");
		m_strNickName = _T("");
		m_strPwd = _T("");
		m_strEzs = _T("");
		m_nTeacherId = -1;
		m_nUserId = -1;
		GetDlgItem(IDC_SYSLINK_Login)->SetWindowTextW(_T("<a>登录</a>"));
	}
	m_pShowScannerInfoDlg->setShowInfo(m_strUserName, m_strNickName);
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

void CGuideDlg::OnNMClickSyslinkSetting(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnBnClickedBtnParam();
	*pResult = 0;
}

void CGuideDlg::OnNMClickSyslinkLogin(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnBnClickedBtnLogin();
	*pResult = 0;
}


// ScanTool2.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ScanTool2Dlg.h"
#include "LoginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CScanTool2App

BEGIN_MESSAGE_MAP(CScanTool2App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CScanTool2App 构造

CScanTool2App::CScanTool2App()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CScanTool2App 对象

CScanTool2App theApp;


// CScanTool2App 初始化
bool InitConfig()
{
	USES_CONVERSION;
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

	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
	g_strModelSavePath = CMyCodeConvert::Gb2312ToUtf8(strModelPath);
	Poco::File fileModelPath(g_strModelSavePath);
	fileModelPath.createDirectories();

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


	g_strPaperSavePath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath + _T("Paper\\")));	//存放扫描试卷的路径
	g_strPaperBackupPath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath + _T("Paper\\Pkg_Backup\\")));
	Poco::File filePaperPath(g_strPaperSavePath);
	filePaperPath.createDirectories();
	Poco::File pkgBackupPath(g_strPaperBackupPath);
	pkgBackupPath.createDirectories();

	CString strConfigPath = g_strCurrentPath;
	strConfigPath.Append(_T("config.ini"));
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strConfigPath));
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
	int nRecogThreads = pConf->getInt("Recog.threads", 2);
	g_nManulUploadFile = pConf->getInt("UploadFile.manul", 0);
	g_bShowScanSrcUI = pConf->getBool("Scan.bShowUI", false);
	  = pConf->getBool("Scan.bModifySN", false);
	g_nOperatingMode = pConf->getInt("Scan.OperatingMode", 2);
	g_nZkzhNull2Issue = pConf->getInt("Scan.khNull2Issue", 0);

	std::string strFileServerIP = pConf->getString("Server.fileIP");
	int			nFileServerPort = pConf->getInt("Server.filePort", 19980);
	m_strCmdServerIP = pConf->getString("Server.cmdIP");
	m_nCmdPort = pConf->getInt("Server.cmdPort", 19980);
	g_strCmdIP = m_strCmdServerIP;
	g_nCmdPort = m_nCmdPort;
	g_strFileIP = strFileServerIP;
	g_nFilePort = nFileServerPort;
}


BOOL CScanTool2App::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO:  应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CScanTool2Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}


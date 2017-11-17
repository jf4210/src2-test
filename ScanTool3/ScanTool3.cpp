
// ScanTool2.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanTool3Dlg.h"
#include "LoginDlg.h"
#include "minidump.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CFont					g_fontBase;
// CScanTool2App

BEGIN_MESSAGE_MAP(CScanTool3App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CScanTool2App ����

CScanTool3App::CScanTool3App()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CScanTool2App ����

CScanTool3App theApp;


// CScanTool2App ��ʼ��
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


	g_strPaperSavePath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath + _T("Paper\\")));	//���ɨ���Ծ��·��
	g_strPaperBackupPath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath + _T("Paper\\Pkg_Backup\\")));
	Poco::File filePaperPath(g_strPaperSavePath);
	filePaperPath.createDirectories();
	Poco::File pkgBackupPath(g_strPaperBackupPath);
	pkgBackupPath.createDirectories();

	CString strConfigPath = g_strCurrentPath;
	strConfigPath.Append(_T("config.ini"));
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strConfigPath));
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
	_nReocgThreads_		= pConf->getInt("Recog.threads", 2);
	g_nManulUploadFile	= pConf->getInt("UploadFile.manul", 0);
	g_bShowScanSrcUI	= pConf->getBool("Scan.bShowUI", false);
	g_bModifySN			= pConf->getBool("Scan.bModifySN", false);
	g_nOperatingMode	= pConf->getInt("Scan.OperatingMode", 2);
	g_nZkzhNull2Issue	= pConf->getInt("Scan.khNull2Issue", 0);
	_nScanAnswerModel_	= pConf->getInt("Scan.scanAnswer", 0);

	g_strFileIP = pConf->getString("Server.fileIP");
	g_nFilePort = pConf->getInt("Server.filePort", 19980);
	g_strCmdIP = pConf->getString("Server.cmdIP");
	g_nCmdPort = pConf->getInt("Server.cmdPort", 19980);
	_nPicNum4Ty_ = pConf->getInt("WHTY.picNums", 2);

	_strMainTitle_ = pConf->getString("Title.mainTitle", "��ѧɨ��ͻ���");
	_strLitteTitle_ = pConf->getString("Title.littleTitle", "Tianyu big data scan tool");
	_strCopyright_ = pConf->getString("Title.copyRight", "�人���������Ƽ����޹�˾ 2017~2018 ��Ȩ����");

	try
	{
		CString strTitlePath = g_strCurrentPath;
		strTitlePath.Append(_T("title.dat"));
		std::string strTitleUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strTitlePath));
		Poco::AutoPtr<Poco::Util::IniFileConfiguration> pTitleConf(new Poco::Util::IniFileConfiguration(strTitleUtf8Path));

		_strMainTitle_ = pTitleConf->getString("Title.mainTitle", "��ѧɨ��ͻ���");
		_strLitteTitle_ = pTitleConf->getString("Title.littleTitle", "Tianyu big data scan tool");
		_strCopyright_ = pTitleConf->getString("Title.copyRight", "�人���������Ƽ����޹�˾ 2017~2018 ��Ȩ����");
	}
	catch (Poco::Exception exc)
	{
		TRACE("��ȡ������ʾ��Ϣʧ��\n");
	}

	return true;
}

BOOL CheckNewVersion()
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

				//����ж�س���
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
				std::string strLog = "�ƶ��°氲װ�ļ���ϵͳ��ʱĿ¼ʧ��";
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

BOOL CScanTool3App::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	if (!FirstInstance())
		return FALSE;
	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO:  Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("EasyTNT"));
	
	LOGFONT logfont;
	if (::SystemParametersInfo(SPI_GETICONTITLELOGFONT,
		sizeof (logfont), &logfont, 0))
	{
		VERIFY(g_fontBase.CreateFontIndirect(&logfont));
	}
	else
	{
		memset(&logfont, 0, sizeof(logfont));
		logfont.lfHeight = -11;
		logfont.lfCharSet = DEFAULT_CHARSET;
		logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		logfont.lfQuality = DEFAULT_QUALITY;
		logfont.lfPitchAndFamily = DEFAULT_PITCH;
		wcscpy_s(logfont.lfFaceName, _T("System"));

		if (!g_fontBase.CreateFontIndirect(&logfont))
		{
			ASSERT(0);
		}
	}

	RunCrashHandler();
	InitConfig();

	if (CheckNewVersion())
		return FALSE;

	USES_CONVERSION;
	CLoginDlg loginDlg(A2T(g_strCmdIP.c_str()), g_nCmdPort);
	if (loginDlg.DoModal() != IDOK)
	{
		return FALSE;
	}
	else
	{
		_bLogin_		= TRUE;
		_strUserName_	= T2A(loginDlg.m_strUserName);
		_strNickName_	= T2A(loginDlg.m_strNickName);
		_strPwd_		= T2A(loginDlg.m_strPwd);
		_strEzs_		= T2A(loginDlg.m_strEzs);
		_strPersonID_	= T2A(loginDlg.m_strPersonId);
		_strSchoolID_	= T2A(loginDlg.m_strSchoolID);
		_nTeacherId_	= loginDlg.m_nTeacherId;
		_nUserId_		= loginDlg.m_nUserId;
	}

	CScanTool3Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

BOOL CScanTool3App::FirstInstance()
{
	TCHAR strTitle[MAX_PATH] = { 0 };
	HWND hwnd = NULL;
	HWND AfterHwnd = NULL;
	while (true)
	{
		hwnd = ::FindWindowEx(NULL, AfterHwnd, _T("#32770"), NULL);
		if (!hwnd)
			break;
		else
		{
			if (::GetWindowText(hwnd, strTitle, MAX_PATH))
			{
				if (StrStr(strTitle, _T("YKLX-ScanTool GuideDlg")) != 0)	//SYS_BASE_NAME
				{
					//�ҵ����ں�Ĳ���
					GetLastActivePopup(hwnd);
					if (IsIconic(hwnd))
						ShowWindow(hwnd, SW_RESTORE);
					SetForegroundWindow(hwnd);
					return FALSE;
				}
			}
		}
		AfterHwnd = hwnd;
	}
	return TRUE;
}


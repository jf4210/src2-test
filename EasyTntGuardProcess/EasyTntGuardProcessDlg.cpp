
// EasyTntGuardProcessDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EasyTntGuardProcess.h"
#include "EasyTntGuardProcessDlg.h"
#include "afxdialogex.h"
#include "GPDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"Version.lib")

BOOL bContinue = TRUE;
CString g_strAppPath = _T("");
CString g_strInitServerIP = _T("");
int		g_nInitServerPort = 0;
CString g_strVerServerIP = _T("");
int		g_nVerServerPort = 0;
BOOL	g_bConnect = FALSE;
SOCKET	g_sock = INVALID_SOCKET;

HANDLE			g_hMutex_Conn;
CMutex			g_mutex_VSFL;
CMutex			g_mutex_LFM;
CMutex			g_mutex_DFL;
LIST_FILEINFO	g_VerServerFileList;
LIST_NEED_DOWNLOAD	g_DownLoadFileList;
MAP_FILEINFO	g_LocalFileMap;
std::string		g_strFileVersion;

BOOL		g_bShowUpdateMsg = TRUE;		//�Ƿ�֪ͨɨ�������а汾���£������һ�δ�ɨ���������ʾʱѡ�񲻸��£���ô��ɨ����������ڼ�һֱ����ʾ��ֱ������йر�
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CEasyTntGuardProcessDlg �Ի���



CEasyTntGuardProcessDlg::CEasyTntGuardProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEasyTntGuardProcessDlg::IDD, pParent)
	, m_hThread(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEasyTntGuardProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEasyTntGuardProcessDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CEasyTntGuardProcessDlg ��Ϣ�������

BOOL CEasyTntGuardProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	SetWindowText(_T("EasyTntGuardProcess"));
	g_strFileVersion = LoadFileVersion();

	//��̨����
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = WPF_RESTORETOMAXIMIZED;
	wp.showCmd = SW_HIDE;
	SetWindowPlacement(&wp);

//	ShowWindow(SW_MINIMIZE);

	InitConf();

	g_hMutex_Conn = CreateMutex(NULL, FALSE, L"VerServerFileList");

	StartUp();
	
	if (!InitNetWork())
	{
		return FALSE;
	}
	
#if 0
	CString strComm;
	strComm.Format(_T("%snewSetupPkg\\setup.exe"), g_strAppPath);
//	strComm.Format(_T("%s\\ScanTool.exe"), g_strAppPath);
	
	USES_CONVERSION;
	SHELLEXECUTEINFOA TempInfo = {0};	

	TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	TempInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	TempInfo.hwnd = NULL;
	TempInfo.lpVerb = "runas";
	TempInfo.lpFile = T2A(strComm);
	TempInfo.lpParameters = "";
	TempInfo.lpDirectory = T2A(g_strAppPath);
	TempInfo.nShow = SW_NORMAL;
	TempInfo.hInstApp = NULL;

	int nResult = ::ShellExecuteExA(&TempInfo);

	TRACE("ִ��ShellExecuteExA���1, nResult = %d\n", nResult);
	WaitForSingleObject(TempInfo.hProcess,INFINITE);
	TRACE("ִ��ShellExecuteExA���2\n");
	CString strVerExePath = g_strAppPath + _T("newSetupPkg\\");
	BOOL bDel = DeleteDirectory(T2A(strVerExePath));
	if (bDel)
		TRACE("�Ƴ��汾����ļ���: %s\n", T2A(strVerExePath));
	else
		TRACE("�Ƴ��汾����ļ���(%s)ʧ��\n", T2A(strVerExePath));
#else
	DWORD dwThreadID;
	m_hThread = CreateThread(NULL, 0, MyWork, NULL, 0, &dwThreadID);
	if (NULL == m_hThread)
	{
		TRACE0("CreateThread failed\n");
		return FALSE;
	}
#endif

	ShowWindow(FALSE);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CEasyTntGuardProcessDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CEasyTntGuardProcessDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CEasyTntGuardProcessDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEasyTntGuardProcessDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	bContinue = FALSE;

	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	g_mutex_DFL.Lock();
	g_DownLoadFileList.clear();
	g_mutex_DFL.Unlock();

	g_mutex_VSFL.Lock();
	LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
	for (; it != g_VerServerFileList.end();)
	{
		pST_FILEINFO pFileInfo = *it;
		SAFE_RELEASE(pFileInfo);
		it = g_VerServerFileList.erase(it);
	}
	g_mutex_VSFL.Unlock();

	g_mutex_LFM.Lock();
	MAP_FILEINFO::iterator itMap = g_LocalFileMap.begin();
	for (; itMap != g_LocalFileMap.end();)
	{
		pST_FILEINFO pFileInfo = itMap->second;
		SAFE_RELEASE(pFileInfo);
		itMap = g_LocalFileMap.erase(itMap);
	}
	g_mutex_LFM.Unlock();
}

void CEasyTntGuardProcessDlg::InitConf()
{
	TCHAR	szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	CString str = szPath;
	g_strAppPath = str.Left(str.ReverseFind('\\') + 1);
	CString strConfigPath = g_strAppPath + _T("config.ini");

	TCHAR szServerAddr[50] = { 0 };
	GetPrivateProfileString(_T("Server"), _T("cmdIP"), _T("116.211.105.45"), szServerAddr, 50, strConfigPath);
	g_nInitServerPort = GetPrivateProfileInt(_T("Server"), _T("cmdPort"), 19991, strConfigPath);
	g_strInitServerIP = szServerAddr;
}

std::string CEasyTntGuardProcessDlg::LoadFileVersion()
{
	DWORD dwHandle = 0;
	char* pInfoData = NULL;
	std::string strFileVersion;

	WCHAR szSelfName[MAX_PATH + 1] = { 0 };
	if (::GetModuleFileNameW(NULL, szSelfName, MAX_PATH + 1))
	{
		DWORD dwInfoSize = GetFileVersionInfoSize(szSelfName, &dwHandle);
		if (dwInfoSize > 0)
		{
			pInfoData = new char[dwInfoSize];
			memset(pInfoData, 0, dwInfoSize);
			if (GetFileVersionInfo(szSelfName, dwHandle, dwInfoSize, pInfoData))
			{
				VS_FIXEDFILEINFO* vs_file_info = NULL;
				unsigned int size = 0;
				if (VerQueryValue(pInfoData, L"\\", (LPVOID*)&vs_file_info, &size))
				{
					char szTmp[100] = { 0 };
					sprintf(szTmp, "%d.%d.%d.%d", (int)HIWORD(vs_file_info->dwFileVersionMS), (int)LOWORD(vs_file_info->dwFileVersionMS), (int)HIWORD(vs_file_info->dwFileVersionLS), (int)LOWORD(vs_file_info->dwFileVersionLS));
					strFileVersion = szTmp;
				}
#if 0	//���ð汾ʧ��
				//��ȡ����
				char* pLanValue = NULL;
				UINT nLen2 = 0;
				VerQueryValue(pInfoData, L"VarFileInfo\\Translation", (LPVOID*)&pLanValue, &nLen2);
				//�޸���Դ
				vs_file_info->dwFileVersionLS = 1234;
				vs_file_info->dwFileVersionMS = 4321;
				vs_file_info->dwProductVersionLS = 101;
				vs_file_info->dwProductVersionMS = 1010;
				HANDLE handle = BeginUpdateResource(szSelfName, FALSE);
				BOOL result = UpdateResourceW(handle
											  , RT_VERSION
											  , MAKEINTRESOURCE(1)
											  , LANG_USER_DEFAULT
											  , pInfoData
											  , dwInfoSize);	//stlang.wLanguageID
				if (result == FALSE)
				{
					AfxMessageBox(_T("Updata Resource False."));
				}
				EndUpdateResource(handle, FALSE);
#endif
			}
			SAFE_RELEASE_ARRY(pInfoData);
		}
	}
	return strFileVersion;
}

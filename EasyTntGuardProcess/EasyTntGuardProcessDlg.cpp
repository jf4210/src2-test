
// EasyTntGuardProcessDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EasyTntGuardProcess.h"
#include "EasyTntGuardProcessDlg.h"
#include "afxdialogex.h"
#include "GPDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL bContinue = TRUE;
CString g_strAppPath = _T("");
CString g_strInitServerIP = _T("");
int		g_nInitServerPort = 0;
CString g_strVerServerIP = _T("");
int		g_nVerServerPort = 0;
BOOL	g_bConnect = FALSE;
SOCKET	g_sock = INVALID_SOCKET;

HANDLE			g_hMutex_Conn;
LIST_FILEINFO	g_VerServerFileList;
LIST_NEED_DOWNLOAD	g_DownLoadFileList;
MAP_FILEINFO	g_LocalFileMap;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CEasyTntGuardProcessDlg 对话框



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


// CEasyTntGuardProcessDlg 消息处理程序

BOOL CEasyTntGuardProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	//后台运行
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = WPF_RESTORETOMAXIMIZED;
	wp.showCmd = SW_HIDE;
	SetWindowPlacement(&wp);

	ShowWindow(SW_MINIMIZE);

	InitConf();

	g_hMutex_Conn = CreateMutex(NULL, FALSE, L"VerServerFileList");

//	StartUp();
	
	if (!InitNetWork())
	{
		return FALSE;
	}
	
	DWORD dwThreadID;
	m_hThread = CreateThread(NULL, 0, MyWork, NULL, 0, &dwThreadID);
	if (NULL == m_hThread)
	{
		TRACE0("CreateThread failed\n");
		return FALSE;
	}


	ShowWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEasyTntGuardProcessDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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

	LIST_FILEINFO::iterator it = g_VerServerFileList.begin();
	for (; it != g_VerServerFileList.end();)
	{
		pST_FILEINFO pFileInfo = *it;
		SAFE_RELEASE(pFileInfo);
		it = g_VerServerFileList.erase(it);
	}

	MAP_FILEINFO::iterator itMap = g_LocalFileMap.begin();
	for (; itMap != g_LocalFileMap.end();)
	{
		pST_FILEINFO pFileInfo = itMap->second;
		SAFE_RELEASE(pFileInfo);
		itMap = g_LocalFileMap.erase(itMap);
	}
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

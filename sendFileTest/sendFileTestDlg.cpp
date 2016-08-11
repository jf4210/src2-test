
// sendFileTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "sendFileTest.h"
#include "sendFileTestDlg.h"
#include "afxdialogex.h"
#include "MyCodeConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int					g_nExitFlag = 0;				//退出标示

Poco::Logger*		g_pLogger;

Poco::FastMutex		g_fmSendLock;
SENDTASKLIST		g_lSendTask;	

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


// CsendFileTestDlg 对话框



CsendFileTestDlg::CsendFileTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CsendFileTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CsendFileTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_SendFile, m_ctrlSendFile);
}

BEGIN_MESSAGE_MAP(CsendFileTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SendFile, &CsendFileTestDlg::OnBnClickedBtnSendfile)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_SendTest, &CsendFileTestDlg::OnBnClickedBtnSendtest)
END_MESSAGE_MAP()


// CsendFileTestDlg 消息处理程序

BOOL CsendFileTestDlg::OnInitDialog()
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

	InitConfig();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CsendFileTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CsendFileTestDlg::OnPaint()
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
HCURSOR CsendFileTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CsendFileTestDlg::OnBnClickedBtnSendfile()
{
//	const int sendFileCount = 1;

// 	CString strFilePath;
// 	m_ctrlSendFile.GetWindowTextW(strFilePath);
// 
// 	if (strFilePath.IsEmpty())
// 	{
// 		AfxMessageBox(_T("未设置文件路径"));
// 		return;
// 	}
// 
// 	USES_CONVERSION;
// 	for (int i = 0; i < sendFileCount; i++)
// 	{
// 		pSENDTASK pTask = new SENDTASK;
// 		char szFilePath[200] = { 0 };
// //		sprintf_s(szFilePath, "E:\\myWorkspace\\yklx\\bin\\debug\\Paper\\%d.zip", i + 1);
// 		strcpy_s(szFilePath, "E:\\myWorkspace\\yklx\\bin\\debug\\Paper\\1.zip");
// 		pTask->strPath = T2A(strFilePath);
// //		pTask->strName = "1.zip";
// 
// 		g_lSendTask.push_back(pTask);
// 	}

	const int sendThreadCount = 50;
	std::string strIp = "192.168.1.131";
	m_pRecogThread = new Poco::Thread[sendThreadCount];
	for (int i = 0; i < sendThreadCount; i++)
	{
		CSendFileThread* pObj = new CSendFileThread(strIp, 19980);
		m_pRecogThread[i].start(*pObj);

		m_vecRecogThreadObj.push_back(pObj);
	}
}


void CsendFileTestDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	g_nExitFlag = 1;
	Sleep(1000);
	for (int i = 0; i < m_vecRecogThreadObj.size(); i++)
	{
		m_pRecogThread[i].join();
	}
	Sleep(500);
	std::vector<CSendFileThread*>::iterator itRecogObj = m_vecRecogThreadObj.begin();
	for (; itRecogObj != m_vecRecogThreadObj.end();)
	{
		CSendFileThread* pObj = *itRecogObj;
		if (pObj)
		{
			delete pObj;
			pObj = NULL;
		}
		itRecogObj = m_vecRecogThreadObj.erase(itRecogObj);
	}
}

void CsendFileTestDlg::OnBnClickedBtnSendtest()
{
	CString strFilePath;
	m_ctrlSendFile.GetWindowTextW(strFilePath);

	if (strFilePath.IsEmpty())
	{
		AfxMessageBox(_T("未设置文件路径"));
		return;
	}


	const int sendFileCount = 50;
	std::string strPath;
	std::string strExt;
	std::string strBaseName;

	int nCurrentThreadID = Poco::Thread::currentTid();

	USES_CONVERSION;
	std::string strSrcPath = T2A(strFilePath);
	try
	{
		Poco::Path filePath(CMyCodeConvert::Gb2312ToUtf8(strSrcPath));
		strPath = filePath.toString();
		strExt = filePath.getExtension();
		strBaseName = filePath.getBaseName();

		int nPos = strPath.rfind('.');
		strPath = strPath.substr(0, nPos);

		for (int i = 1; i <= sendFileCount; i++)
		{
			std::string strNewFilePath = Poco::format("%s_%d_%d.%s", strPath, i, nCurrentThreadID, strExt);

			Poco::File fileTask(CMyCodeConvert::Gb2312ToUtf8(strSrcPath));
			fileTask.copyTo(strNewFilePath);			
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "文件复制异常: " + CMyCodeConvert::Utf8ToGb2312(exc.displayText()) + "\n";
		TRACE(strLog.c_str());
	}

	for (int i = 1; i < sendFileCount; i++)
	{
		std::string strNewFilePath = Poco::format("%s_%d_%d.%s", strPath, i, nCurrentThreadID, strExt);
		std::string strNewName = Poco::format("%s_%d_%d.%s", strBaseName, i, nCurrentThreadID, strExt);

		pSENDTASK pTask = new SENDTASK;
		char szFilePath[200] = { 0 };
		pTask->strPath = strNewFilePath;
		pTask->strName = strNewName;

		g_lSendTask.push_back(pTask);
	}
}

void CsendFileTestDlg::InitConfig()
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
	
	std::string strLogPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strFile + _T("sendFile.log")));
	Poco::AutoPtr<Poco::PatternFormatter> pFormatter(new Poco::PatternFormatter("%L%Y-%m-%d %H:%M:%S.%F %q:%t"));
	Poco::AutoPtr<Poco::FormattingChannel> pFCFile(new Poco::FormattingChannel(pFormatter));
	Poco::AutoPtr<Poco::FileChannel> pFileChannel(new Poco::FileChannel(strLogPath));
	pFCFile->setChannel(pFileChannel);
	pFCFile->open();
	pFCFile->setProperty("rotation", "1 M");
	pFCFile->setProperty("archive", "timestamp");
	pFCFile->setProperty("compress", "true");
	pFCFile->setProperty("purgeCount", "5");
	Poco::Logger& appLogger = Poco::Logger::create("sendFile", pFCFile, Poco::Message::PRIO_INFORMATION);
	g_pLogger = &appLogger;
}

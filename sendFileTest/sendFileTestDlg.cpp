
// sendFileTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "sendFileTest.h"
#include "sendFileTestDlg.h"
#include "afxdialogex.h"
#include "MyCodeConvert.h"
#include "Net_Cmd_Protocol.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int					g_nExitFlag = 0;				//退出标示

Poco::Logger*		g_pLogger;
Poco::Event*		g_peStartMulticast = new Poco::Event(false);

std::string			g_strCurPath;

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
	, m_nThreads(0), m_nFileTasks(0), m_nChildProcess(0), m_nChildThreads(0), m_pMulticastServer(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CsendFileTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_SendFile, m_ctrlSendFile);
	DDX_Text(pDX, IDC_EDIT_Threads, m_nThreads);
	DDX_Text(pDX, IDC_EDIT_FileTasks, m_nFileTasks);
	DDX_Text(pDX, IDC_EDIT_Childs, m_nChildProcess);
	DDX_Text(pDX, IDC_EDIT_ChildThreads, m_nChildThreads);
}

BEGIN_MESSAGE_MAP(CsendFileTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SendFile, &CsendFileTestDlg::OnBnClickedBtnSendfile)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_SendTest, &CsendFileTestDlg::OnBnClickedBtnSendtest)
	ON_BN_CLICKED(IDC_BTN_Multicast, &CsendFileTestDlg::OnBnClickedBtnMulticast)
	ON_BN_CLICKED(IDC_BTN_StartChild, &CsendFileTestDlg::OnBnClickedBtnStartchild)
	ON_BN_CLICKED(IDC_BTN_StartChildThread, &CsendFileTestDlg::OnBnClickedBtnStartchildthread)
	ON_BN_CLICKED(IDC_BTN_Test, &CsendFileTestDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_Test2, &CsendFileTestDlg::OnBnClickedBtnTest2)
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
	m_pMulticastServer = new MulticastServer(m_strMulticastIP, m_nMulticastPort);

	UpdateData(FALSE);

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
	UpdateData(TRUE);
	if (m_nThreads <= 0)
	{
		AfxMessageBox(_T("设置线程数量"));
		return;
	}

	m_pRecogThread = new Poco::Thread[m_nThreads];
	for (int i = 0; i < m_nThreads; i++)
	{
		CSendFileThread* pObj = new CSendFileThread(m_strServerIP, m_nServerPort);
		m_pRecogThread[i].start(*pObj);

		m_vecRecogThreadObj.push_back(pObj);
	}
}


void CsendFileTestDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	g_nExitFlag = 1;
	if (g_peStartMulticast)
	{
		delete g_peStartMulticast;
		g_peStartMulticast = NULL;
	}
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

	UpdateData(TRUE);
	if (m_nFileTasks <= 0)
	{
		AfxMessageBox(_T("设置文件任务数量"));
		return;
	}

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

		for (int i = 1; i <= m_nFileTasks; i++)
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

	for (int i = 1; i <= m_nFileTasks; i++)
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
	g_strCurPath = T2A(strFile);
	
	std::string strName = Poco::format("sendFile_%d.log", (int)Poco::Thread::currentTid());

	std::string strLogPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strFile));	//_T("sendFile.log")
	strLogPath.append(strName);
	Poco::AutoPtr<Poco::PatternFormatter> pFormatter(new Poco::PatternFormatter("%L%Y-%m-%d %H:%M:%S.%F %q:%t"));
	Poco::AutoPtr<Poco::FormattingChannel> pFCFile(new Poco::FormattingChannel(pFormatter));
	Poco::AutoPtr<Poco::FileChannel> pFileChannel(new Poco::FileChannel(strLogPath));
	pFCFile->setChannel(pFileChannel);
	pFCFile->open();
	pFCFile->setProperty("rotation", "1 M");
	pFCFile->setProperty("archive", "timestamp");
	pFCFile->setProperty("compress", "true");
	pFCFile->setProperty("purgeCount", "5");

//	std::string strName = Poco::format("sendFile_%d", Poco::Thread::currentTid());

	Poco::Logger& appLogger = Poco::Logger::create(strName, pFCFile, Poco::Message::PRIO_INFORMATION);
	g_pLogger = &appLogger;

	CString strConfigPath = strFile;
	strConfigPath.Append(_T("config_sendFileTest.ini"));
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strConfigPath));
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
	m_strServerIP = pConf->getString("SERVER.IP");
	m_nServerPort = pConf->getInt("SERVER.Port", 19980);

	m_strMulticastIP = pConf->getString("Multicast.IP");
	m_nMulticastPort = pConf->getInt("Multicast.Port", 19980);
}


void CsendFileTestDlg::OnBnClickedBtnMulticast()
{
	ST_CMD_HEADER stHead;
	stHead.usCmd = MULTICAST_START;
	stHead.uPackSize = 0;
	char szSendBuf[1024] = { 0 };
	memcpy(szSendBuf, &stHead, HEAD_SIZE);

	Poco::Net::MulticastSocket ms;
	int n = ms.sendTo(szSendBuf, HEAD_SIZE, m_pMulticastServer->group());
}


void CsendFileTestDlg::OnBnClickedBtnStartchild()
{
	UpdateData(TRUE);
	if (m_nChildProcess <= 0)
	{
		AfxMessageBox(_T("设置启动的子进程数量"));
		return;
	}

	ST_CMD_HEADER stHead;
	stHead.usCmd = MULTICAST_INIT_PROCESS;
	stHead.uPackSize = 0;
	stHead.usResult = m_nChildProcess;
	char szSendBuf[1024] = { 0 };
	memcpy(szSendBuf, &stHead, HEAD_SIZE);

	Poco::Net::MulticastSocket ms;
	int n = ms.sendTo(szSendBuf, HEAD_SIZE, m_pMulticastServer->group());
}


void CsendFileTestDlg::OnBnClickedBtnStartchildthread()
{
	UpdateData(TRUE);
	if (m_nChildThreads <= 0)
	{
		AfxMessageBox(_T("设置启动的子线程数量"));
		return;
	}

	ST_CMD_HEADER stHead;
	stHead.usCmd = MULTICAST_INIT_THREAD;
	stHead.uPackSize = 0;
	stHead.usResult = m_nChildThreads;
	char szSendBuf[1024] = { 0 };
	memcpy(szSendBuf, &stHead, HEAD_SIZE);

	Poco::Net::MulticastSocket ms;
	int n = ms.sendTo(szSendBuf, HEAD_SIZE, m_pMulticastServer->group());
}


void CsendFileTestDlg::OnBnClickedBtnTest()
{
	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("All Files (*.*)|*.*;)||"),
					NULL);
	if (dlg.DoModal() != IDOK)
		return;

	USES_CONVERSION;
	std::string strJsnModel = T2A(dlg.GetPathName());

	std::string strJsnData;
	std::ifstream in(strJsnModel);
	if (!in)
		return ;

	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);					//不过滤空格
		strJsnData.append(strJsnLine);
	}
	in.close();

	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strJsnData);		//strJsnData
		Poco::JSON::Object::Ptr objData = result.extract<Poco::JSON::Object::Ptr>();



	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("加载模板文件解析json失败: ");
		strErrInfo.append(jsone.message());
		g_pLogger->information(strErrInfo);
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("加载模板文件解析json失败2: ");
		strErrInfo.append(exc.message());
		g_pLogger->information(strErrInfo);
	}
}


void CsendFileTestDlg::OnBnClickedBtnTest2()
{
	std::string strPath = "E:\\myWorkspace\\yklx\\bin\\debug\\fileRecv\\";

	static int i = 1;

	USES_CONVERSION;
	std::string strPaperPath = CMyCodeConvert::Gb2312ToUtf8(g_strCurPath);
	Poco::DirectoryIterator it(strPaperPath);
	Poco::DirectoryIterator end;
	while (it != end)
	{
		Poco::Path p(it->path());
		if (it->isFile())
		{
			Poco::Path pSubFile(it->path());
			Poco::File file(it->path());
			std::string strOldFileName = pSubFile.getFileName();

			if (strOldFileName.find("_Child") != std::string::npos)
			{
				file.remove();
			}
		}
		it++;
	}
}

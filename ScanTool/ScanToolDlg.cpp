
// ScanToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanToolDlg.h"
#include "afxdialogex.h"
#include "LoginDlg.h"
#include "GetModelDlg.h"
#include "ScanModleMgrDlg.h"
#include "ShowFileTransferDlg.h"
#include "Net_Cmd_Protocol.h"
#include "GuideDlg.h"
#include <windows.h>
//#include "minidump.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace cv;

int					g_nManulUploadFile = 0;		//手动上传文件，通过qq这类的
bool				g_bCmdConnect = false;		//命令通道连接
bool				g_bFileConnect = false;		//文件通道连接

bool				g_bCmdNeedConnect = false;	//命令通道是否需要重连，用于通道地址信息修改的情况
bool				g_bFileNeedConnect = false;	//文件通道是否需要重连，用于通道地址信息修改的情况

bool				g_bShowScanSrcUI = false;	//是否显示原始扫描界面

int					g_nExitFlag = 0;
CString				g_strCurrentPath;
std::string			g_strPaperSavePath;	//试卷扫描后保存的总路径
std::string			g_strModelSavePath;
Poco::Logger*		g_pLogger;
Poco::FastMutex		g_fmRecog;		//识别线程获取任务锁
RECOGTASKLIST		g_lRecogTask;	//识别任务列表

Poco::FastMutex		g_fmPapers;		//操作试卷袋列表的任务锁
PAPERS_LIST			g_lPapers;		//所有的试卷袋信息

Poco::FastMutex		g_fmSendLock;	//上传文件列表的锁
SENDTASKLIST		g_lSendTask;	//上传文件任务列表

Poco::FastMutex		g_fmTcpTaskLock;
TCP_TASKLIST		g_lTcpTask;		//tcp任务列表

EXAM_LIST			g_lExamList;	//当前账号对应的考试列表

Poco::FastMutex		g_fmCompressLock;		//压缩文件列表锁
COMPRESSTASKLIST	g_lCompressTask;		//解压文件列表

Poco::Event			g_eTcpThreadExit;
Poco::Event			g_eSendFileThreadExit;
Poco::Event			g_eCompressThreadExit;

double	_dCompThread_Fix_ = 1.2;
double	_dDiffThread_Fix_ = 0.2;
double	_dDiffExit_Fix_ = 0.3;
double	_dCompThread_Head_ = 1.0;
double	_dDiffThread_Head_ = 0.085;
double	_dDiffExit_Head_ = 0.15;

int		_nGauseKernel_ = 5;			//高斯变换核因子
int		_nSharpKernel_ = 5;			//锐化核因子
int		_nCannyKernel_ = 90;		//轮廓化核因子
int		_nDilateKernel_ = 6;		//膨胀核因子	//Size(6, 6)	普通空白框可识别
int		_nErodeKernel_ = 2;			//腐蚀核因子

int		g_nRecogGrayMin = 0;			//灰度点(除空白点,OMR外)计算灰度的最小考试范围
int		g_nRecogGrayMax_White = 255;	//空白点校验点计算灰度的最大考试范围
int		g_nRecogGrayMin_OMR = 0;		//OMR计算灰度的最小考试范围
int		g_RecogGrayMax_OMR = 235;		//OMR计算灰度的最大考试范围

std::string			g_strEncPwd = "yklxTest";				//文件加密解密密码
std::string			g_strCmdIP;
std::string			g_strFileIP;
int					g_nCmdPort;
int					g_nFilePort;

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


// CScanToolDlg 对话框



CScanToolDlg::CScanToolDlg(pMODEL pModel, CWnd* pParent /*=NULL*/)
	: CDialogEx(CScanToolDlg::IDD, pParent)
	, m_pModel(pModel), m_ncomboCurrentSel(-1), m_pRecogThread(NULL), m_pCurrentPicShow(NULL), m_nModelPicNums(1)
	, m_bTwainInit(FALSE), m_nCurrTabSel(0), m_nScanCount(0), m_nScanStatus(0)
	, m_pPapersInfo(NULL), m_pPaper(NULL), m_colorStatus(RGB(0, 0, 255)), m_nStatusSize(30), m_pCurrentShowPaper(NULL)
	, m_pSendFileObj(NULL), m_SendFileThread(NULL), m_bLogin(FALSE), m_pTcpCmdObj(NULL), m_TcpCmdThread(NULL)
	, m_nTeacherId(-1), m_nUserId(-1), m_nCurrItemPaperList(-1)
	, m_pShowModelInfoDlg(NULL), m_pShowScannerInfoDlg(NULL)
	, m_nDuplex(1), m_bF1Enable(FALSE), m_bF2Enable(FALSE)
	, m_pCompressObj(NULL), m_pCompressThread(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScanToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Picture, m_lcPicture);
	DDX_Control(pDX, IDC_LIST_Paper, m_lcPaper);
	DDX_Control(pDX, IDC_COMBO_Model, m_comboModel);
	DDX_Control(pDX, IDC_TAB_PicShow, m_tabPicShowCtrl);
}

BEGIN_MESSAGE_MAP(CScanToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_Scan, &CScanToolDlg::OnBnClickedBtnScan)
	ON_BN_CLICKED(IDC_BTN_ScanModule, &CScanToolDlg::OnBnClickedBtnScanmodule)
	ON_CBN_SELCHANGE(IDC_COMBO_Model, &CScanToolDlg::OnCbnSelchangeComboModel)
	ON_BN_CLICKED(IDC_BTN_InputPaper, &CScanToolDlg::OnBnClickedBtnInputpaper)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PicShow, &CScanToolDlg::OnTcnSelchangeTabPicshow)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Picture, &CScanToolDlg::OnNMDblclkListPicture)
	ON_MESSAGE(MSG_ERR_RECOG, &CScanToolDlg::MsgRecogErr)
	ON_MESSAGE(WM_CV_LBTNDOWN, &CScanToolDlg::RoiLBtnDown)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_UpLoadPapers, &CScanToolDlg::OnBnClickedBtnUploadpapers)
	ON_BN_CLICKED(IDC_BTN_Login, &CScanToolDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(IDC_BTN_ModelMgr, &CScanToolDlg::OnBnClickedBtnModelmgr)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Picture, &CScanToolDlg::OnNMHoverListPicture)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_Picture, &CScanToolDlg::OnLvnKeydownListPicture)
	ON_BN_CLICKED(IDC_BTN_UploadMgr, &CScanToolDlg::OnBnClickedBtnUploadmgr)
	ON_BN_CLICKED(IDC_BTN_ScanAll, &CScanToolDlg::OnBnClickedBtnScanall)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_ReBack, &CScanToolDlg::OnBnClickedBtnReback)
	ON_WM_HOTKEY()
END_MESSAGE_MAP()


// CScanToolDlg 消息处理程序

BOOL CScanToolDlg::OnInitDialog()
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

//	RunCrashHandler();

	USES_CONVERSION;
	CString strTitle = _T("");
	strTitle.Format(_T("%s %s"), SYS_BASE_NAME, SOFT_VERSION);
	SetWindowText(strTitle);

	InitUI();
	InitConfig();
	InitParam();
	InitFileUpLoadList();
	
// 	Poco::LocalDateTime dtNow;
// 	std::string strData;
// 	Poco::format(strData, "%4d-%2d-%2d %2d:%2d", dtNow.year(), dtNow.month(), dtNow.day(), dtNow.hour(), dtNow.minute());
// 	Poco::LocalDateTime now;
// 	char szTime[50] = { 0 };
// 	sprintf_s(szTime, "%d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
// 	TRACE(szTime);
	
#ifdef SHOW_COMBOLIST_MAINDLG
	SearchModel();
	if (m_ncomboCurrentSel != -1)
	{
		m_comboModel.SetCurSel(m_ncomboCurrentSel);
		if (m_comboModel.GetCount())
		{
//			m_ncomboCurrentSel = m_comboModel.GetCurSel();
// 			CString strModelName;
// 			m_comboModel.GetLBText(m_ncomboCurrentSel, strModelName);
// 			CString strModelPath = g_strCurrentPath + _T("Model\\") + strModelName;
// 			CString strModelFullPath = strModelPath + _T(".mod");
// 			UnZipFile(strModelFullPath);
// 			m_pModel = LoadModelFile(strModelPath);
		}
	}
	else
	{
		if (m_comboModel.GetCount())
		{
			m_ncomboCurrentSel = 0;
			CString strModelName;
			m_comboModel.GetLBText(m_ncomboCurrentSel, strModelName);
			CString strModelPath = g_strCurrentPath + _T("Model\\") + strModelName;
			CString strModelFullPath = strModelPath + _T(".mod");
			UnZipFile(strModelFullPath);
			m_pModel = LoadModelFile(strModelPath);
		}
		m_comboModel.SetCurSel(m_ncomboCurrentSel);
	}
#endif
	if (m_pModel != NULL)
		m_nModelPicNums = m_pModel->nPicNum;
	else
	{
#ifndef TO_WHTY
		m_nModelPicNums = 1;
#endif
	}
	InitTab();

	m_pShowModelInfoDlg->ShowModelInfo(m_pModel);

	// 调用TWAIN 初始化扫描设置
	ReleaseTwain();
	m_bTwainInit = FALSE;
	if (!m_bTwainInit)
	{
		m_bTwainInit = InitTwain(m_hWnd);
		if (!IsValidDriver())
		{
			AfxMessageBox(_T("Unable to load Twain Driver."));
		}

// 		memset(&m_Source, 0, sizeof(m_Source));
// 		if (!SourceSelected())
// 		{
// 			SelectDefaultSource();
// 		}
// 
// 		if (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_USERSELECT, &m_Source))
// 		{
// 			m_bSourceSelected = TRUE;
// 		}

		ScanSrcInit();
	}

	RegisterHotKey(GetSafeHwnd(), 1001, NULL, VK_F1);//F1键
	RegisterHotKey(GetSafeHwnd(), 1002, NULL, VK_F2);//F2键  

	StartGuardProcess();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CScanToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CScanToolDlg::OnPaint()
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
HCURSOR CScanToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CScanToolDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	USES_CONVERSION;

	UnregisterHotKey(GetSafeHwnd(), 1001);//注销F2键  
	UnregisterHotKey(GetSafeHwnd(), 1002);//注销Alt+1键  

	g_lExamList.clear();
	g_nExitFlag = 1;
	ReleaseTwain();
	g_pLogger->information("ReleaseTwain() complete.");

	SAFE_RELEASE(m_pShowModelInfoDlg);
	SAFE_RELEASE(m_pShowScannerInfoDlg);

	std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
	for (; itPic != m_vecPicShow.end();)
	{
		CPicShow* pModelPicShow = *itPic;
		SAFE_RELEASE(pModelPicShow);
		itPic = m_vecPicShow.erase(itPic);
	}

	SAFE_RELEASE(m_pModel);

	MODELLIST::iterator it = m_lModel.begin();
	for (; it != m_lModel.end();)
	{
		pMODEL pModel = *it;
		SAFE_RELEASE(pModel);
		it = m_lModel.erase(it);
	}

	g_pLogger->information("模板列表释放完毕.");

	g_fmRecog.lock();			//释放未处理完的识别任务列表
	RECOGTASKLIST::iterator itRecog = g_lRecogTask.begin();
	for (; itRecog != g_lRecogTask.end();)
	{
		pRECOGTASK pRecogTask = *itRecog;
		SAFE_RELEASE(pRecogTask);
		itRecog = g_lRecogTask.erase(itRecog);
	}
	g_fmRecog.unlock();

	for (int i = 0; i < m_vecRecogThreadObj.size(); i++)
	{
		m_vecRecogThreadObj[i]->eExit.wait();
		m_pRecogThread[i].join();
	}
	std::vector<CRecognizeThread*>::iterator itRecogObj = m_vecRecogThreadObj.begin();
	for (; itRecogObj != m_vecRecogThreadObj.end();)
	{
		CRecognizeThread* pObj = *itRecogObj;
		SAFE_RELEASE(pObj);
		itRecogObj = m_vecRecogThreadObj.erase(itRecogObj);
	}
	if (m_pRecogThread)
	{
		delete[] m_pRecogThread;
		m_pRecogThread = NULL;
	}
	g_pLogger->information("识别线程释放完毕.");

#if 1
	ReleaseUploadFileList();
#else
	Poco::JSON::Object jsnFileList;
	Poco::JSON::Array jsnSendTaskArry;
	g_fmSendLock.lock();
	SENDTASKLIST::iterator itSendTask = g_lSendTask.begin();
	for (; itSendTask != g_lSendTask.end();)
	{
		if ((*itSendTask)->nSendState != 2)
		{
			//发送失败的文件暂存信息，下次登录时提示是否重新上传
			Poco::JSON::Object objTask;
			objTask.set("name", CMyCodeConvert::Gb2312ToUtf8((*itSendTask)->strFileName));
			objTask.set("path", CMyCodeConvert::Gb2312ToUtf8((*itSendTask)->strPath));
			jsnSendTaskArry.add(objTask);
		}
		pSENDTASK pTask = *itSendTask;
		itSendTask = g_lSendTask.erase(itSendTask);
		SAFE_RELEASE(pTask);
	}
	g_fmSendLock.unlock();
	if (jsnSendTaskArry.size())
	{
		Poco::LocalDateTime dtNow;
		std::string strData;
		Poco::format(strData, "%4d-%02d-%02d %02d:%02d", dtNow.year(), dtNow.month(), dtNow.day(), dtNow.hour(), dtNow.minute());
		jsnFileList.set("time", strData);
		jsnFileList.set("fileList", jsnSendTaskArry);

		std::stringstream jsnString;
		jsnFileList.stringify(jsnString);

		std::string strJsnFile = T2A(g_strCurrentPath + _T("tmpFileList.dat"));
		ofstream out(strJsnFile);
		if (out)
		{
			out << jsnString.str().c_str();
			out.close();
		}
	}
#endif

	m_SendFileThread->join();
	SAFE_RELEASE(m_pSendFileObj);
	g_eSendFileThreadExit.wait();
	SAFE_RELEASE(m_SendFileThread);
	g_pLogger->information("发送文件线程释放完毕.");

	g_fmTcpTaskLock.lock();
	TCP_TASKLIST::iterator itCmd = g_lTcpTask.begin();
	for (; itCmd != g_lTcpTask.end(); itCmd++)
	{
		pTCP_TASK pTask = *itCmd;
		itCmd = g_lTcpTask.erase(itCmd);
		SAFE_RELEASE(pTask);
	}
	g_fmTcpTaskLock.unlock();
	m_TcpCmdThread->join();
	SAFE_RELEASE(m_pTcpCmdObj);
	g_eTcpThreadExit.wait();
	SAFE_RELEASE(m_TcpCmdThread);
	g_pLogger->information("tcp命令处理线程释放完毕.");

	g_fmCompressLock.lock();
	COMPRESSTASKLIST::iterator itCompress = g_lCompressTask.begin();
	for (; itCompress != g_lCompressTask.end();)
	{
		pCOMPRESSTASK pTask = *itCompress;
		itCompress = g_lCompressTask.erase(itCompress);
		SAFE_RELEASE(pTask);
	}
	g_fmCompressLock.unlock();
	m_pCompressThread->join();
	SAFE_RELEASE(m_pCompressObj);
	g_eCompressThreadExit.wait();
	SAFE_RELEASE(m_pCompressThread);
	g_pLogger->information("压缩处理线程释放完毕.");
	

	g_fmPapers.lock();			//释放试卷袋列表
	PAPERS_LIST::iterator itPapers = g_lPapers.begin();
	for (; itPapers != g_lPapers.end();)
	{
		pPAPERSINFO pPapersTask = *itPapers;
		SAFE_RELEASE(pPapersTask);
		itPapers = g_lPapers.erase(itPapers);
	}
	g_fmPapers.unlock();

	SAFE_RELEASE(m_pPapersInfo);
	g_pLogger->information("试卷袋列表释放完毕.");
}

void CScanToolDlg::InitConfig()
{
	USES_CONVERSION;
#ifndef SHOW_GUIDEDLG
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
	pFCFile->setProperty("rotation", "5 M");
	pFCFile->setProperty("archive", "timestamp");
	pFCFile->setProperty("compress", "true");
	pFCFile->setProperty("purgeCount", "5");
	Poco::Logger& appLogger = Poco::Logger::create("ScanTool", pFCFile, Poco::Message::PRIO_INFORMATION);
	g_pLogger = &appLogger;
#endif

	g_strPaperSavePath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath + _T("Paper\\")));	//存放扫描试卷的路径
	Poco::File filePaperPath(g_strPaperSavePath);
	filePaperPath.createDirectories();

	CString strConfigPath = g_strCurrentPath;
	strConfigPath.Append(_T("config.ini"));
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strConfigPath));
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
	int nRecogThreads = pConf->getInt("Recog.threads", 2);
	g_nManulUploadFile = pConf->getInt("UploadFile.manul", 0);
	g_bShowScanSrcUI = pConf->getBool("Scan.bShowUI", false);
	std::string strFileServerIP	= pConf->getString("Server.fileIP");
	int			nFileServerPort	= pConf->getInt("Server.filePort", 19980);
	m_strCmdServerIP				= pConf->getString("Server.cmdIP");
	m_nCmdPort						= pConf->getInt("Server.cmdPort", 19980);
	g_strCmdIP	= m_strCmdServerIP;
	g_nCmdPort	= m_nCmdPort;
	g_strFileIP = strFileServerIP;
	g_nFilePort = nFileServerPort;

#ifdef TO_WHTY
	m_nModelPicNums = pConf->getInt("WHTY.picNums", 2);
	m_bF1Enable = TRUE;
	m_bF2Enable = TRUE;
#endif

	m_pRecogThread = new Poco::Thread[nRecogThreads];
	for (int i = 0; i < nRecogThreads; i++)
	{
		CRecognizeThread* pRecogObj = new CRecognizeThread;
		m_pRecogThread[i].start(*pRecogObj);

		m_vecRecogThreadObj.push_back(pRecogObj);
	}
	m_SendFileThread = new Poco::Thread;
	m_pSendFileObj = new CSendFileThread(strFileServerIP, nFileServerPort);
	m_SendFileThread->start(*m_pSendFileObj);
	m_TcpCmdThread = new Poco::Thread;
	m_pTcpCmdObj = new CTcpClient(m_strCmdServerIP, m_nCmdPort);
	m_TcpCmdThread->start(*m_pTcpCmdObj);
	m_pCompressThread = new Poco::Thread;
	m_pCompressObj = new CCompressThread(this);
	m_pCompressThread->start(*m_pCompressObj);
}

void CScanToolDlg::InitTab()
{
	if (m_pModel)
	{
		std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
		for (; itPic != m_vecPicShow.end();)
		{
			CPicShow* pModelPicShow = *itPic;
			if (pModelPicShow)
			{
				delete pModelPicShow;
				pModelPicShow = NULL;
			}
			itPic = m_vecPicShow.erase(itPic);
		}
	}
	m_tabPicShowCtrl.DeleteAllItems();

	USES_CONVERSION;
	CRect rtTab;
	m_tabPicShowCtrl.GetClientRect(&rtTab);
	for (int i = 0; i < m_nModelPicNums; i++)
	{
		char szTabHeadName[20] = { 0 };
		sprintf_s(szTabHeadName, "第%d页", i + 1);

		m_tabPicShowCtrl.InsertItem(i, A2T(szTabHeadName));

		CPicShow* pPicShow = new CPicShow(this);
		pPicShow->Create(CPicShow::IDD, &m_tabPicShowCtrl);
		pPicShow->ShowWindow(SW_HIDE);
		pPicShow->MoveWindow(&rtTab);
		m_vecPicShow.push_back(pPicShow);
	}
	m_tabPicShowCtrl.SetCurSel(0);
	if (m_vecPicShow.size())
	{
		m_vecPicShow[0]->ShowWindow(SW_SHOW);
		m_pCurrentPicShow = m_vecPicShow[0];
	}	

	if (m_tabPicShowCtrl.GetSafeHwnd())
	{
		CRect rtTab;
		m_tabPicShowCtrl.GetClientRect(&rtTab);
		int nTabHead_H = 24;		//tab控件头的高度
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
}

void CScanToolDlg::InitUI()
{
	m_lcPicture.SetExtendedStyle(m_lcPicture.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcPicture.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 40);
	m_lcPicture.InsertColumn(1, _T("学生信息"), LVCFMT_CENTER, 150);

	m_lcPaper.SetExtendedStyle(m_lcPaper.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcPaper.InsertColumn(0, _T("试卷名"), LVCFMT_CENTER, 80);
	m_lcPaper.InsertColumn(1, _T("上传状态"), LVCFMT_CENTER, 70);

	SetFontSize(m_nStatusSize);

	USES_CONVERSION;

	m_nCurrTabSel = 0;

	m_pShowModelInfoDlg = new CShowModelInfoDlg(this);
	m_pShowModelInfoDlg->Create(CShowModelInfoDlg::IDD, this);
	m_pShowModelInfoDlg->ShowWindow(SW_SHOW);

	m_pShowScannerInfoDlg = new CScanerInfoDlg(this);
	m_pShowScannerInfoDlg->Create(CScanerInfoDlg::IDD, this);
	m_pShowScannerInfoDlg->ShowWindow(SW_SHOW);

#ifdef TO_WHTY
	m_pShowScannerInfoDlg->ShowWindow(SW_HIDE);
#endif
#ifndef SHOW_SCANALL_MAINDLG
	GetDlgItem(IDC_BTN_ScanAll)->ShowWindow(SW_HIDE);
#endif
#ifndef SHOW_MODELMAKE_MAINDLG
	GetDlgItem(IDC_BTN_ScanModule)->ShowWindow(SW_HIDE);
#endif
#ifndef SHOW_COMBOLIST_MAINDLG
	GetDlgItem(IDC_STATIC_Model)->ShowWindow(SW_HIDE);
	m_comboModel.ShowWindow(SW_HIDE);
#endif
#ifndef SHOW_LOGIN_MAINDLG
	GetDlgItem(IDC_BTN_Login)->ShowWindow(SW_HIDE);
#endif
#ifndef SHOW_MODELMGR_MAINDLG
	GetDlgItem(IDC_BTN_ModelMgr)->ShowWindow(SW_HIDE);
#endif
#ifndef SHOW_PAPERINPUT_MAINDLG
	GetDlgItem(IDC_BTN_InputPaper)->ShowWindow(SW_HIDE);
#endif
#ifndef SHOW_GUIDEDLG
	GetDlgItem(IDC_BTN_ReBack)->ShowWindow(SW_HIDE);
#endif
	//++ 后期可以删除
	GetDlgItem(IDC_STATIC_PaperList)->ShowWindow(SW_HIDE);
	m_lcPaper.ShowWindow(SW_HIDE);
	//

#if 1
	CRect rc;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	int sx = rc.Width();
	int sy = rc.Height();
	if (sx > MAX_DLG_WIDTH)
		sx = MAX_DLG_WIDTH;
	if (sy > MAX_DLG_HEIGHT)
		sy = MAX_DLG_HEIGHT;
#else
	int sx = MAX_DLG_WIDTH;
	int sy = MAX_DLG_HEIGHT;
#endif
	MoveWindow(0, 0, sx, sy);

	CenterWindow();
	InitCtrlPosition();
}

void CScanToolDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

HBRUSH CScanToolDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();

	if (IDC_STATIC_STATUS == CurID)
	{
		pDC->SetTextColor(m_colorStatus);
// 		HBRUSH hbrsh = (HBRUSH)GetStockObject(NULL_BRUSH);
// 		pDC->SetBkMode(TRANSPARENT);

		return hbr;	// hbrsh;
	}
	return hbr;
}

void CScanToolDlg::SetFontSize(int nSize)
{
	m_fontStatus.DeleteObject();
	m_fontStatus.CreateFont(nSize, 0, 0, 0,
		FW_BOLD, FALSE, FALSE, 0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("Arial"));
	GetDlgItem(IDC_STATIC_STATUS)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_SCANCOUNT)->SetFont(&m_fontStatus);
}

void CScanToolDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = cy / 10;	//上边的间隔，留给控制栏
	if (nTopGap > 90)
		nTopGap = 90;
	else if (nTopGap < 50)
		nTopGap = 50;

	const int nLeftGap = 2;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nGap = 2;			//普通控件的间隔

	int nListCtrlWidth = 200;	//图片列表控件宽度
	int nStaticTip = 15;		//列表提示static控件高度
	int nStatusHeight = nStaticTip * 2;		//底层状态提示栏的高度
	int nComboBoxHeith = 25;	//模板下拉列表控件的高度
	int nPaperListHeigth = cy * 0.3;	//试卷袋列表的控件高度
	if (nPaperListHeigth > 300)
		nPaperListHeigth = 300;
#ifdef SHOW_COMBOLIST_MAINDLG
	int nPicListHeight = cy - nTopGap - nStaticTip - nGap - nComboBoxHeith - nGap - nStaticTip - nGap /*- nGap - nStaticTip*/ - nGap - nPaperListHeigth - nGap - nStatusHeight - nBottomGap;		//图片列表控件高度
#else
	int nPicListHeight = cy - nTopGap /*- nStaticTip - nGap - nComboBoxHeith - nGap*/ - nStaticTip - nGap /*- nGap - nStaticTip*/ - nGap - nPaperListHeigth - nGap - nStatusHeight - nBottomGap;		//图片列表控件高度
#endif

	int nCurrentTop = nTopGap;
#ifdef SHOW_COMBOLIST_MAINDLG
	if (GetDlgItem(IDC_STATIC_Model)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Model)->MoveWindow(nLeftGap, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop = nCurrentTop + nStaticTip + nGap;
	}
	if (m_comboModel.GetSafeHwnd())
	{
		m_comboModel.MoveWindow(nLeftGap, nCurrentTop, nListCtrlWidth, nComboBoxHeith);
		nCurrentTop = nCurrentTop + nComboBoxHeith + nGap;
	}
#endif
	if (GetDlgItem(IDC_STATIC_PicList)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PicList)->MoveWindow(nLeftGap, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop = nCurrentTop + nStaticTip + nGap;
	}
	if (m_lcPicture.GetSafeHwnd())
	{
		m_lcPicture.MoveWindow(nLeftGap, nCurrentTop, nListCtrlWidth, nPicListHeight);
		nCurrentTop = nCurrentTop + nPicListHeight + nGap;
	}
#if 1
	if (m_pShowModelInfoDlg && m_pShowModelInfoDlg->GetSafeHwnd())
	{
		m_pShowModelInfoDlg->MoveWindow(nLeftGap, nCurrentTop, nListCtrlWidth, nPaperListHeigth);
		nCurrentTop = nCurrentTop + nPaperListHeigth + nGap;
	}
#else
	if (GetDlgItem(IDC_STATIC_PaperList)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PaperList)->MoveWindow(nLeftGap, nCurrentTop, nListCtrlWidth, nStaticTip);
		nCurrentTop = nCurrentTop + nStaticTip + nGap;
	}
	if (m_lcPaper.GetSafeHwnd())
	{
		m_lcPaper.MoveWindow(nLeftGap, nCurrentTop, nListCtrlWidth, nPaperListHeigth);
		nCurrentTop = nCurrentTop + nPaperListHeigth + nGap;
	}
#endif
	if (GetDlgItem(IDC_STATIC_SCANCOUNT)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_SCANCOUNT)->MoveWindow(nLeftGap, cy - nBottomGap - nStatusHeight, 150, nStatusHeight);
	}
	if (GetDlgItem(IDC_STATIC_STATUS)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_STATUS)->MoveWindow(nLeftGap + 150, cy - nBottomGap - nStatusHeight, cx - nLeftGap - nRightGap, nStatusHeight);
	}

	int nPicShowTabCtrlWidth = cx - nLeftGap - nRightGap - nListCtrlWidth - nGap - nGap;
	if (m_tabPicShowCtrl.GetSafeHwnd())
	{
		m_tabPicShowCtrl.MoveWindow(nLeftGap + nListCtrlWidth + nGap, nTopGap, nPicShowTabCtrlWidth, cy - nTopGap - nGap - nGap - nStatusHeight - nBottomGap);

		CRect rtTab;
		m_tabPicShowCtrl.GetClientRect(&rtTab);
		int nTabHead_H = 24;		//tab控件头的高度
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}

	//控制栏按钮位置
	int nBtnWidth = nTopGap - nGap - nGap;
	int nBtnCurrLeft = nLeftGap;
#ifdef SHOW_LOGIN_MAINDLG
	if (GetDlgItem(IDC_BTN_Login)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Login)->MoveWindow(nBtnCurrLeft, nGap, nBtnWidth, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nBtnWidth + nGap;
	}
#endif
	int nScanBtnW = (nTopGap - nGap - nGap) * 1.5;
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nBtnCurrLeft, nGap, nScanBtnW, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nScanBtnW + nGap;
	}
#ifdef SHOW_SCANALL_MAINDLG
	if (GetDlgItem(IDC_BTN_ScanAll)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ScanAll)->MoveWindow(nBtnCurrLeft, nGap, nBtnWidth, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nBtnWidth + nGap;
	}
#endif
#ifdef SHOW_MODELMAKE_MAINDLG
	if (GetDlgItem(IDC_BTN_ScanModule)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ScanModule)->MoveWindow(nBtnCurrLeft, nGap, nBtnWidth, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nBtnWidth + nGap;
	}
#endif
#ifdef SHOW_MODELMGR_MAINDLG
	if (GetDlgItem(IDC_BTN_ModelMgr)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ModelMgr)->MoveWindow(nBtnCurrLeft, nGap, nBtnWidth, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nBtnWidth + nGap;
	}
#endif
	if (GetDlgItem(IDC_BTN_UpLoadPapers)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_UpLoadPapers)->MoveWindow(nBtnCurrLeft, nGap, nScanBtnW, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nScanBtnW + nGap;
	}
	if (GetDlgItem(IDC_BTN_UploadMgr)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_UploadMgr)->MoveWindow(nBtnCurrLeft, nGap, nBtnWidth, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nBtnWidth + nGap;
	}
#ifdef SHOW_PAPERINPUT_MAINDLG
	if (GetDlgItem(IDC_BTN_InputPaper)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_InputPaper)->MoveWindow(nBtnCurrLeft, nGap, nBtnWidth, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nBtnWidth + nGap;
	}
#endif
#ifdef SHOW_GUIDEDLG
	if (GetDlgItem(IDC_BTN_ReBack)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ReBack)->MoveWindow(nBtnCurrLeft, nGap, nBtnWidth, nTopGap - nGap - nGap);
		nBtnCurrLeft = nBtnCurrLeft + nBtnWidth + nGap;
	}
#endif
	if (m_pShowScannerInfoDlg && m_pShowScannerInfoDlg->GetSafeHwnd())
	{
		m_pShowScannerInfoDlg->MoveWindow(cx - nRightGap - 220, nGap, 220, nTopGap - nGap - nGap);	//cy - nRightGap - 10, nGap, 150, nTopGap - nGap - nGap
	}

	Invalidate();
}

BOOL CScanToolDlg::ScanSrcInit()
{
	USES_CONVERSION;
	if (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &m_Source))
	{
		TW_IDENTITY temp_Source = m_Source;
		m_scanSourceArry.Add(temp_Source);
		while (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &m_Source))
		{
			TW_IDENTITY temp_Source = m_Source;
			m_scanSourceArry.Add(temp_Source);
		}
		m_bSourceSelected = TRUE;
	}
	else
	{
		m_bSourceSelected = FALSE;
	}
	return m_bSourceSelected;
}

void CScanToolDlg::OnTcnSelchangeTabPicshow(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	int nIndex = m_tabPicShowCtrl.GetCurSel();
	m_nCurrTabSel = nIndex;

	m_pCurrentPicShow = m_vecPicShow[nIndex];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != nIndex)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}
void CScanToolDlg::OnBnClickedBtnLogin()
{
	USES_CONVERSION;
	if (!m_bLogin)
	{
		CLoginDlg dlg(A2T(m_strCmdServerIP.c_str()), m_nCmdPort);
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
			GetDlgItem(IDC_BTN_Login)->SetWindowTextW(_T("注销"));
		}
	}
	else
	{
		std::string strUser = T2A(m_strUserName);
		pTCP_TASK pTcpTask	= new TCP_TASK;
		pTcpTask->usCmd		= USER_LOGOUT;
		pTcpTask->nPkgLen	= strUser.length();
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
	m_pShowScannerInfoDlg->setShowInfo(m_strUserName, m_strNickName);
}
void CScanToolDlg::OnBnClickedBtnScan()
{
	if (m_nScanStatus == 1)	//扫描中，不能操作
		return;

	m_bF1Enable = FALSE;

	if (!m_bTwainInit)
	{
		m_bTwainInit = InitTwain(m_hWnd);
		if (!IsValidDriver())
		{
			AfxMessageBox(_T("Unable to load Twain Driver."));
		}
		m_scanSourceArry.RemoveAll();
		ScanSrcInit();
	}

#ifndef TO_WHTY
	BOOL bLogin = FALSE;
#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();
	bLogin = pDlg->m_bLogin;
#else
	bLogin = m_bLogin;
#endif
	if (!bLogin)
	{
		if (MessageBox(_T("未登录，图像不能保存，是否继续扫描？"), _T("警告"), MB_YESNO) != IDYES)
		{
			m_bF1Enable = TRUE;
			return;
		}
	}

	if (!m_pModel)
	{
		AfxMessageBox(_T("未设置扫描模板，请在模板设置界面选择扫描模板"));	//模板解析错误
		m_bF1Enable = TRUE;
		return;
	}
#endif	

	int nScanSrc = 0;
	int nDuplexDef = 1;
	int nScanCount = 0;

	CScanCtrlDlg dlg(m_scanSourceArry);
	if (dlg.DoModal() != IDOK)
	{
		m_bF1Enable = TRUE;
		return;
	}

	nScanSrc = dlg.m_nCurrScanSrc;
	nDuplexDef = dlg.m_nCurrDuplex;
	nScanCount = dlg.m_nStudentNum;
	
	m_comboModel.EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_Scan)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ScanAll)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ScanModule)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ModelMgr)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BTN_InputPaper)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_UpLoadPapers)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_UploadMgr)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ReBack)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(_T(""));

	USES_CONVERSION;
	char szPicTmpPath[MAX_PATH] = { 0 };

	m_pCurrentShowPaper = NULL;

	if (m_nScanStatus == 2)		//扫描异常结束后，删除前一份的试卷袋信息
	{
		SAFE_RELEASE(m_pPapersInfo);
	}
	sprintf_s(szPicTmpPath, "%sPaper\\Tmp", T2A(g_strCurrentPath));
	if (!m_pPapersInfo)
	{
		m_lcPicture.DeleteAllItems();
		GetDlgItem(IDC_STATIC_SCANCOUNT)->SetWindowText(_T(""));

		std::string strUtfPath = CMyCodeConvert::Gb2312ToUtf8(szPicTmpPath);
		try
		{
			Poco::File tmpPath(strUtfPath);
			if (tmpPath.exists())
				tmpPath.remove(true);

			Poco::File tmpPath1(strUtfPath);
			tmpPath1.createDirectories();
		}
		catch (Poco::Exception& exc)
		{
			std::string strLog = "删除临时文件夹失败(" + exc.message() + "): ";
			strLog.append(szPicTmpPath);
			g_pLogger->information(strLog);
		}

		m_strCurrPicSavePath = szPicTmpPath;
		m_pPapersInfo = new PAPERSINFO();
		m_nScanCount = 0;					//已扫描数量清0
	}
	m_nScanStatus = 1;


	m_Source = m_scanSourceArry.GetAt(nScanSrc);


	bool bShowScanSrcUI = g_bShowScanSrcUI;

	if (dlg.m_bAdvancedSetting)
		bShowScanSrcUI = true;

	int nDuplex = nDuplexDef;		//单双面,0-单面,1-双面
	int nSize = 1;							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3
	int nPixel = 2;							//0-黑白，1-灰度，2-彩色
	int nResolution = 200;					//dpi: 72, 150, 200, 300
	
	int nNum = 0;
	if (nDuplex == 0)
	{
		nNum = nScanCount * m_nModelPicNums;
	}
	else
	{
		int nModelPics = m_nModelPicNums;
		if (nModelPics % 2)
			nModelPics++;

		nNum = nScanCount * nModelPics;
	}
	m_nDuplex = nDuplex;

	if (nNum == 0)
		nNum = TWCPP_ANYCOUNT;

	if (!Acquire(nNum, nDuplex, nSize, nPixel, nResolution, bShowScanSrcUI))	//TWCPP_ANYCOUNT
	{
		m_comboModel.EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_Scan)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ScanAll)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ScanModule)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ModelMgr)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_InputPaper)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_UpLoadPapers)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_UploadMgr)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ReBack)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(_T(""));
		m_nScanStatus = 2;
	}

	m_bF1Enable = TRUE;
}

void CScanToolDlg::OnBnClickedBtnScanall()
{
	if (m_nScanStatus == 1)	//扫描中，不能操作
		return;

	if (!m_bTwainInit)
	{
		m_bTwainInit = InitTwain(m_hWnd);
		if (!IsValidDriver())
		{
			AfxMessageBox(_T("Unable to load Twain Driver."));
		}
		m_scanSourceArry.RemoveAll();
		ScanSrcInit();
	}

	BOOL bLogin = FALSE;
#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();
	bLogin = pDlg->m_bLogin;
#else
	bLogin = m_bLogin;
#endif
	if (!bLogin)
	{
		if (MessageBox(_T("未登录，图像不能保存，是否继续扫描？"), _T("警告"), MB_YESNO) != IDYES)
			return;
	}
	if (!m_pModel)
	{
		AfxMessageBox(_T("未设置扫描模板，请在模板设置界面选择扫描模板"));	//模板解析错误
		return;
	}

	CScanCtrlDlg dlg(m_scanSourceArry);
	if (dlg.DoModal() != IDOK)
		return;

	m_comboModel.EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_Scan)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ScanAll)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ScanModule)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ModelMgr)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_InputPaper)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_UpLoadPapers)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_UploadMgr)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ReBack)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(_T(""));

	USES_CONVERSION;
	char szPicTmpPath[MAX_PATH] = { 0 };

	m_pCurrentShowPaper = NULL;

	SAFE_RELEASE(m_pPapersInfo);	//整袋重扫，删除前一袋试卷的信息
	
	sprintf_s(szPicTmpPath, "%sPaper\\Tmp", T2A(g_strCurrentPath));
	if (!m_pPapersInfo)
	{
		m_lcPicture.DeleteAllItems();
		GetDlgItem(IDC_STATIC_SCANCOUNT)->SetWindowText(_T(""));

		std::string strUtfPath = CMyCodeConvert::Gb2312ToUtf8(szPicTmpPath);
		try
		{
			Poco::File tmpPath(strUtfPath);
			if (tmpPath.exists())
				tmpPath.remove(true);

			Poco::File tmpPath1(strUtfPath);
			tmpPath1.createDirectories();
		}
		catch (Poco::Exception& exc)
		{
			std::string strLog = "删除临时文件夹失败(" + exc.message() + "): ";
			strLog.append(szPicTmpPath);
			g_pLogger->information(strLog);
		}

		m_strCurrPicSavePath = szPicTmpPath;
		m_pPapersInfo = new PAPERSINFO();
		m_nScanCount = 0;					//已扫描数量清0
	}
	m_nScanStatus = 1;

	m_Source = m_scanSourceArry.GetAt(dlg.m_nCurrScanSrc);
	int nDuplex = dlg.m_nCurrDuplex;		//单双面,0-单面,1-双面
	int nSize = 1;							//1-A4
	int nPixel = 2;							//0-黑白，1-灰度，2-彩色
	int nResolution = 200;					//dpi: 72, 150, 200, 300

// 	int nNum = dlg.m_nStudentNum;
// 
// 	if (nDuplex == 1)
// 		nNum *= 2;

	int nNum = 0;
	if (nDuplex == 0)
	{
		nNum = dlg.m_nStudentNum * m_nModelPicNums;
	}
	else
	{
		int nModelPics = m_nModelPicNums;
		if (nModelPics % 2)
			nModelPics++;

		nNum = dlg.m_nStudentNum * nModelPics;
	}
	m_nDuplex = nDuplex;

	if (nNum == 0)
		nNum = TWCPP_ANYCOUNT;
	if (!Acquire(nNum, nDuplex, nSize, nPixel, nResolution, g_bShowScanSrcUI))	//TWCPP_ANYCOUNT
	{
		m_comboModel.EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_Scan)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ScanAll)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ScanModule)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ModelMgr)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_InputPaper)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_UpLoadPapers)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_UploadMgr)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ReBack)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(_T(""));
		m_nScanStatus = 2;
	}
}

void CScanToolDlg::OnBnClickedBtnScanmodule()
{
	int nCoutn = m_comboModel.GetCount();
	if (nCoutn == 0)
		m_pModel = NULL;

	ReleaseTwain();
	m_bTwainInit = FALSE;

	CMakeModelDlg dlg(m_pModel);
	dlg.DoModal();

	if (!m_pModel)	//如果模板不为空，说明之前已经有模板了，不需要使用新模板
		m_pModel = dlg.m_pModel;

	if (m_pModel != dlg.m_pModel)
	{
		SAFE_RELEASE(dlg.m_pModel);
	}

	USES_CONVERSION;
	if (m_pModel)
	{
		m_comboModel.ResetContent();
		int n = m_comboModel.GetCount();
		SearchModel();
		for (int i = 0; i < m_comboModel.GetCount(); i++)
		{
			CString strItemName;
			m_comboModel.GetLBText(i, strItemName);
			if (strItemName == A2T(m_pModel->strModelName.c_str()))
			{
				m_comboModel.SetCurSel(i);
				m_ncomboCurrentSel = i;
				break;
			}
		}

		m_nModelPicNums = m_pModel->nPicNum;
	}
	
	InitTab();
}

void CScanToolDlg::SearchModel()
{
	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
//	g_strModelSavePath = CMyCodeConvert::Gb2312ToUtf8(strModelPath);

	std::string strLog;
	try
	{
		std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strModelPath);
		Poco::DirectoryIterator it(strUtf8Path);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			if (it->isFile() && p.getExtension() == "mod")
			{
//				std::string strModelName = p.getBaseName();
				std::string strModelName = CMyCodeConvert::Utf8ToGb2312(p.getBaseName());
				m_comboModel.AddString(A2T(strModelName.c_str()));

				if (m_pModel && m_pModel->strModelName.compare(strModelName) == 0)
				{
					m_ncomboCurrentSel = m_comboModel.GetCount() - 1;
				}
			}
			it++;
		}
		strLog = "搜索模板完成";
//		m_comboModel.SetCurSel(0);
	}
	catch (Poco::FileException& exc)
	{
		strLog = "搜索模板失败: " + exc.displayText();
	}
	catch (Poco::Exception& exc)
	{
		strLog = "搜索模板失败2: " + exc.displayText();
	}
	g_pLogger->information(strLog);
}

void CScanToolDlg::OnCbnSelchangeComboModel()
{
	if (m_ncomboCurrentSel == m_comboModel.GetCurSel())
		return;

	m_ncomboCurrentSel = m_comboModel.GetCurSel();

	CString strModelName;
	m_comboModel.GetLBText(m_comboModel.GetCurSel(), strModelName);
	CString strModelPath = g_strCurrentPath + _T("Model\\") + strModelName;
	CString strModelFullPath = strModelPath + _T(".mod");
	UnZipFile(strModelFullPath);

	SAFE_RELEASE(m_pModel);
	m_pModel = LoadModelFile(strModelPath);
	m_pShowModelInfoDlg->ShowModelInfo(m_pModel);
	if (!m_pModel)
		return;

	m_nModelPicNums = m_pModel->nPicNum;
	InitTab();

	//模板切换后之前的扫描试卷需要清除
	m_pCurrentShowPaper = NULL;
	m_lcPicture.DeleteAllItems();
	SAFE_RELEASE(m_pPapersInfo);
}

void CScanToolDlg::OnBnClickedBtnInputpaper()
{
	m_bF1Enable = FALSE;
	m_bF2Enable = FALSE;
	CPaperInputDlg dlg(m_pModel);
	if (dlg.DoModal() != IDOK)
	{
		m_bF1Enable = TRUE;
		m_bF2Enable = TRUE;
		return;
	}

	m_bF1Enable = TRUE;
	m_bF2Enable = TRUE;
}

// 
// void CopyData(char *dest, const char *src, int dataByteSize, bool isConvert, int height) 
// {
// 	char * p = dest;
// 	if (!isConvert) 
// 	{
// 		memcpy(dest, src, dataByteSize);
// 		return;
// 	}
// 	if (height <= 0) return;
// 	//int height = dataByteSize/rowByteSize;
// 	int rowByteSize = dataByteSize / height;
// 	src = src + dataByteSize - rowByteSize;
// 	for (int i = 0; i < height; i++) 
// 	{
// 		memcpy(dest, src, rowByteSize);
// 		dest += rowByteSize;
// 		src -= rowByteSize;
// 	}
// }

IplImage* CBitmap2IplImage(CBitmap* pBitmap)
{
	HBITMAP   hBmp = (HBITMAP)(*pBitmap);//
	BITMAP bmp;
	::GetBitmapBits(hBmp, sizeof(BITMAP), &bmp);


	int nChannels = 1;
	int depth = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;
	IplImage* pImg = cvCreateImage(cvSize(bmp.bmWidth, bmp.bmHeight), depth, nChannels);

//	pImg->origin = IPL_ORIGIN_TL;

	BYTE *pBuffer = new BYTE[bmp.bmHeight*bmp.bmWidth*nChannels];//创建缓冲区 
	GetBitmapBits(hBmp, bmp.bmHeight*bmp.bmWidth*nChannels, pBuffer);//将位图信息复制到缓冲区
	memcpy(pImg->imageData, pBuffer, bmp.bmHeight*bmp.bmWidth*nChannels);//将缓冲区信息复制给IplImage
	delete[] pBuffer;
	return pImg;
}

IplImage* DIB2IplImage(CDIB& m_dib)
{
	/////////////////////////////////////////////////////////// DIB2CBitmap
// 	unsigned char *pBits = (unsigned char *)malloc(m_dib.height*m_dib.bytes);
// 	memcpy(pBits, m_dib.m_pBits, m_dib.height*m_dib.bytes);
	CBitmap* bitMap = new CBitmap;
	bitMap->CreateBitmap(m_dib.width, m_dib.height, 1, m_dib.m_nBits, m_dib.m_pVoid);	//m_dib.m_pVoid
//	free(pBits);
	//////////////////////////////////////////////////////
	IplImage* img = CBitmap2IplImage(bitMap);
	delete[] bitMap;

//	free(pBits2);

	return img;
}

IplImage* CreateImgFormData(BYTE* pImgData, int nWidth, int nHeight, int nChannel)
{
	IplImage *pImg = cvCreateImage(cvSize(nWidth, nHeight), 8, nChannel);
	if (!pImg)
	{
		return NULL;
	}

	for (int h = 0; h < nHeight; h++)
	{
		BYTE* pRowData = pImgData + nWidth * nChannel * (nHeight - 1 - h); //如果图像会上下颠倒就 将(nHeight-1-h) 改为 h 就可以 

		memcpy(pImg->imageData + pImg->widthStep * nChannel * h, pRowData, nWidth);
	}

	return pImg;
}


void CScanToolDlg::CopyImage(HANDLE hBitmap, TW_IMAGEINFO& info)
{
	if (_bTwainContinue == FALSE)	//如果已经设置了扫描停止标识，则不进行当前图像的处理
		return;

	SetImage(hBitmap, info.BitsPerPixel);
}

void CScanToolDlg::SetImage(HANDLE hBitmap, int bits)
{
// 	clock_t start, end;
// 	start = clock();

	
	if (m_nDuplex)	//如果是双面扫描，需要判断模板为奇数时舍弃最后一张图片的情况
	{
		if (m_nModelPicNums % 2 != 0)
		{
			static int i = 1;
			if (i % (m_nModelPicNums + 1) == 0)
			{
				std::string strLog = Poco::format("abandon image, i = %d", i);
				g_pLogger->information(strLog);
				TRACE("%s\n", strLog.c_str());
				i = 1;
				return;
			}
			i++;
		}		
	}


	CDIB dib;
	dib.CreateFromHandle(hBitmap, bits);

	BITMAPFILEHEADER bFile;
	::ZeroMemory(&bFile, sizeof(bFile));
	memcpy((void *)&bFile.bfType, "BM", 2);
	bFile.bfSize = dib.GetDIBSize() + sizeof(bFile);
	bFile.bfOffBits = sizeof(BITMAPINFOHEADER) + dib.GetPaletteSize()*sizeof(RGBQUAD) + sizeof(BITMAPFILEHEADER);

	bool bTmp = false;
	USES_CONVERSION;
	unsigned char *pBits = NULL;
	try
	{
		pBits = (unsigned char *)malloc(bFile.bfSize);
	}
	catch (...)
	{
		_bTwainContinue = FALSE;
		bTmp = true;
	}
	if (bTmp)
	{
		char szErrInfo[100] = { 0 };
		sprintf_s(szErrInfo, "获取内存失败。");
		SetStatusShowInfo(A2T(szErrInfo), TRUE);
		g_pLogger->information(szErrInfo);
	}

	memcpy(pBits, &bFile, sizeof(BITMAPFILEHEADER));
	memcpy(pBits + sizeof(BITMAPFILEHEADER), dib.m_pVoid, dib.GetDIBSize());

	BYTE *p = pBits;
	BITMAPFILEHEADER fheader;
	memcpy(&fheader, p, sizeof(BITMAPFILEHEADER));
	BITMAPINFOHEADER bmphdr;
	p += sizeof(BITMAPFILEHEADER);
	memcpy(&bmphdr, p, sizeof(BITMAPINFOHEADER));
	int w = bmphdr.biWidth;
	int h = bmphdr.biHeight;
	p = pBits + fheader.bfOffBits;

	int nChannel = (bmphdr.biBitCount == 1) ? 1 : bmphdr.biBitCount / 8;
	int depth = (bmphdr.biBitCount == 1) ? IPL_DEPTH_1U : IPL_DEPTH_8U;
	try
	{
		IplImage *pIpl2 = cvCreateImage(cvSize(w, h), depth, nChannel);

		int height;
		bool isLowerLeft = bmphdr.biHeight > 0;
		height = (bmphdr.biHeight > 0) ? bmphdr.biHeight : -bmphdr.biHeight;
		CopyData(pIpl2->imageData, (char*)p, bmphdr.biSizeImage, isLowerLeft, height);
		free(pBits);
		pBits = NULL;

		// 	IplImage* pIpl = DIB2IplImage(dib);
		// 	cv::Mat matTest = cv::cvarrToMat(pIpl);

		int nStudentId = m_nScanCount / m_nModelPicNums + 1;
		int nOrder = m_nScanCount % m_nModelPicNums + 1;

		char szPicName[50] = { 0 };
		char szPicPath[MAX_PATH] = { 0 };
		sprintf_s(szPicName, "S%d_%d.jpg", nStudentId, nOrder);
		sprintf_s(szPicPath, "%s\\S%d_%d.jpg", m_strCurrPicSavePath.c_str(), nStudentId, nOrder);

		m_nScanCount++;

	
		cv::Mat matTest2 = cv::cvarrToMat(pIpl2);

		//++ 2016.8.26 判断扫描图片方向，并进行旋转
		if (m_pModel && m_pModel->nType)	//只针对使用制卷工具自动生成的模板使用旋转检测功能，因为制卷工具的图片方向固定
		{
			int nResult = CheckOrientation(matTest2, nOrder - 1);
			switch (nResult)	//1:针对模板图像需要进行的旋转，正向，不需要旋转，2：右转90(模板图像旋转), 3：左转90(模板图像旋转), 4：右转180(模板图像旋转)
			{
				case 1:	break;
				case 2:
				{
						  Mat dst;
						  transpose(matTest2, dst);	//左旋90，镜像 
						  flip(dst, matTest2, 0);		//左旋90，模板图像需要右旋90，原图即需要左旋90
				}
					break;
				case 3:
				{
						  Mat dst;
						  transpose(matTest2, dst);	//左旋90，镜像 
						  flip(dst, matTest2, 1);		//右旋90，模板图像需要左旋90，原图即需要右旋90
				}
					break;
				case 4:
				{
						  Mat dst;
						  transpose(matTest2, dst);	//左旋90，镜像 
						  Mat dst2;
						  flip(dst, dst2, 1);
						  Mat dst5;
						  transpose(dst2, dst5);
						  flip(dst5, matTest2, 1);	//右旋180
				}
					break;
				default: break;
			}
		}
		//--

		cv::Mat matTest3 = matTest2.clone();

		std::string strPicName = szPicPath;
		imwrite(strPicName, matTest2);

		cvReleaseImage(&pIpl2);

#if 1
		m_pCurrentPicShow->ShowPic(matTest3);
#endif
		
		std::string strLog = "Get image: " + strPicName;
		g_pLogger->information(strLog);

		pST_PicInfo pPic = new ST_PicInfo;
		pPic->strPicName = szPicName;
		pPic->strPicPath = szPicPath;
		//试卷列表显示扫描试卷
		if (nOrder == 1)	//第一页的时候创建新的试卷信息
		{
			char szStudentName[30] = { 0 };
			sprintf_s(szStudentName, "S%d", nStudentId);
			m_pPaper = new ST_PaperInfo;
			m_pPaper->strStudentInfo = szStudentName;
			m_pPaper->pModel = m_pModel;
			m_pPaper->pPapers = m_pPapersInfo;
			m_pPaper->pSrcDlg = this;
			m_pPaper->lPic.push_back(pPic);

			m_pPapersInfo->fmlPaper.lock();
			m_pPapersInfo->lPaper.push_back(m_pPaper);
			m_pPapersInfo->fmlPaper.unlock();

			//添加进试卷列表控件
			int nCount = m_lcPicture.GetItemCount();
			char szCount[10] = { 0 };
			sprintf_s(szCount, "%d", nCount + 1);
			m_lcPicture.InsertItem(nCount, NULL);
			m_lcPicture.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
			m_lcPicture.SetItemText(nCount, 1, (LPCTSTR)A2T(szStudentName));
			m_lcPicture.SetItemData(nCount, (DWORD_PTR)m_pPaper);
		}
		else
		{
			m_pPaper->lPic.push_back(pPic);
		}
		pPic->pPaper = m_pPaper;

		//添加到识别任务列表
		if (m_pModel)
		{
			pRECOGTASK pTask = new RECOGTASK;
			pTask->pPaper = m_pPaper;
			g_lRecogTask.push_back(pTask);
		}

		m_pCurrentShowPaper = m_pPaper;

		CString strMsg = _T("");
		strMsg.Format(_T("已扫描%d张"), m_nScanCount);
		GetDlgItem(IDC_STATIC_SCANCOUNT)->SetWindowText(strMsg);
	}
	catch (cv::Exception& exc)
	{
		_bTwainContinue = FALSE;
		m_nScanStatus = 2;

		free(pBits);
		pBits = NULL;

		int nStudentId = m_nScanCount / m_nModelPicNums + 1;
		int nOrder = m_nScanCount % m_nModelPicNums + 1;

		char szPicName[50] = { 0 };
		sprintf_s(szPicName, "S%d_%d.jpg", nStudentId, nOrder);

		char szErrInfo[200] = { 0 };
		sprintf_s(szErrInfo, "生成图片(%s)失败", szPicName);
		CString strShow = _T("");
		strShow.Format(_T("生成图片(%s)失败"), szPicName);
		SetStatusShowInfo(strShow, TRUE);
		g_pLogger->information(szErrInfo);
	}
}

void CScanToolDlg::ScanDone(int nStatus)
{
#ifndef TO_WHTY
	if (!m_pModel)
		return;
#endif

	m_pPapersInfo->nPaperCount = m_nScanCount / m_nModelPicNums;	//计算扫描试卷数量

	m_comboModel.EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_Scan)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_ScanAll)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_ScanModule)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_ModelMgr)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_InputPaper)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_UpLoadPapers)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_UploadMgr)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_ReBack)->EnableWindow(TRUE);

	bool bWarn = false;
	if (nStatus != 1)
		bWarn = true;

	USES_CONVERSION;
	CString strMsg = _T("");
	if (nStatus == 1)
	{
		strMsg = _T("扫描完成");
		m_nScanStatus = 3;
	}
	else
	{
		strMsg = _T("扫描异常结束");
//		SAFE_RELEASE(m_pPapersInfo);
		m_nScanCount = 0;
		m_nScanStatus = 2;
	}
	SetStatusShowInfo(strMsg, bWarn);
}

void CScanToolDlg::SetStatusShowInfo(CString strMsg, BOOL bWarn /*= FALSE*/)
{
	if (bWarn)
		m_colorStatus = RGB(255, 0, 0);
	else
		m_colorStatus = RGB(0, 0, 255);
	GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(strMsg);
	TRACE("\n----------------\n");
	TRACE(strMsg);
	TRACE("\n----------------\n");
}

BOOL CScanToolDlg::PreTranslateMessage(MSG* pMsg)
{
// 	if (pMsg->message == WM_SYSCHAR)		//WM_SYSCHAR
// 	{
// 		switch (pMsg->wParam)
// 		{
// 		case 's':
// 		case 'S':
// 			OnBnClickedBtnScan();
// 			return TRUE;
// 		}
// 	}

	ProcessMessage(*pMsg);

#ifdef SHOW_GUIDEDLG
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			if (m_nScanStatus == 1)
			{
				AfxMessageBox(_T("扫描进行中, 请稍后操作!"));
				return TRUE;
			}
			if (m_pPapersInfo)
			{
				if (MessageBox(_T("存在未保存的试卷，是否退出？"), _T("警告"), MB_YESNO) != IDYES)
					return TRUE;
			}
			ReleaseScan();
			((CGuideDlg*)AfxGetMainWnd())->m_pModel = m_pModel;
			((CGuideDlg*)AfxGetMainWnd())->ShowWindow(SW_SHOW);
			this->ShowWindow(SW_HIDE);
			m_bF1Enable = FALSE;
			m_bF2Enable = FALSE;
			return TRUE;
		}
	}
#endif
	return CDialog::PreTranslateMessage(pMsg);
}

void CScanToolDlg::OnNMDblclkListPicture(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (pNMItemActivate->iItem < 0)
		return;

	m_nCurrItemPaperList = pNMItemActivate->iItem;
#if 1
	ShowPaperByItem(m_nCurrItemPaperList);
#else
	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcPicture.GetItemData(pNMItemActivate->iItem);


	m_pCurrentShowPaper = pPaper;
	if (pPaper->bIssuePaper)
		PaintIssueRect(pPaper);
	else
		PaintRecognisedRect(pPaper);

	m_nCurrTabSel = 0;

	m_tabPicShowCtrl.SetCurSel(0);
	m_pCurrentPicShow = m_vecPicShow[0];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != 0)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
#endif
}

void CScanToolDlg::PaintRecognisedRect(pST_PaperInfo pPaper)
{
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		Mat matSrc = imread((*itPic)->strPicPath);
#ifdef PIC_RECTIFY_TEST
		Mat dst;
		Mat rotMat;
		PicRectify(matSrc, dst, rotMat);
		Mat matImg;
		if (dst.channels() == 1)
			cvtColor(dst, matImg, CV_GRAY2BGR);
		else
			matImg = dst;
// #ifdef WarpAffine_TEST
// 		cv::Mat	inverseMat(2, 3, CV_32FC1);
// 		PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
// #endif
#else
		Mat matImg = matSrc;
#endif

#ifdef WarpAffine_TEST
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		if (pPaper->pModel)
			PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
#endif

#ifdef Test_ShowOriPosition
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		GetInverseMat((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
#endif
		Mat tmp = matImg;	// matSrc.clone();
		Mat tmp2 = matImg.clone();

		if (pPaper->pModel)
		{
			RECTLIST::iterator itHTracker = pPaper->pModel->vecPaperModel[i]->lSelHTracker.begin();
			for (int j = 0; itHTracker != pPaper->pModel->vecPaperModel[i]->lSelHTracker.end(); itHTracker++, j++)
			{
				cv::Rect rt = (*itHTracker).rt;
				//			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				rectangle(tmp, rt, CV_RGB(25, 200, 20), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			RECTLIST::iterator itVTracker = pPaper->pModel->vecPaperModel[i]->lSelVTracker.begin();
			for (int j = 0; itVTracker != pPaper->pModel->vecPaperModel[i]->lSelVTracker.end(); itVTracker++, j++)
			{
				cv::Rect rt = (*itVTracker).rt;
				//			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				rectangle(tmp, rt, CV_RGB(25, 200, 20), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
		
		RECTLIST::iterator itNormal = (*itPic)->lNormalRect.begin();													//显示识别正常的点
		for (int j = 0; itNormal != (*itPic)->lNormalRect.end(); itNormal++, j++)
		{
			cv::Rect rt = (*itNormal).rt;
//			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

			char szCP[20] = { 0 };
			if (itNormal->eCPType == SN || itNormal->eCPType == OMR)
			{
				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			else
			{
				rectangle(tmp, rt, CV_RGB(50, 255, 55), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
		if (pPaper->pModel)
		{
			RECTLIST::iterator itSelRoi = pPaper->pModel->vecPaperModel[i]->lSelFixRoi.begin();													//显示识别定点的选择区
			for (int j = 0; itSelRoi != pPaper->pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++, j++)
			{
				cv::Rect rt = (*itSelRoi).rt;
				//			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				char szCP[20] = { 0 };
				// 				sprintf_s(szCP, "FIX%d", j);
				// 				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, rt, CV_RGB(0, 0, 255), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
		

		RECTLIST::iterator itPicFix = (*itPic)->lFix.begin();														//显示识别出来的定点
		for (int j = 0; itPicFix != (*itPic)->lFix.end(); itPicFix++, j++)
		{
			cv::Rect rt = (*itPicFix).rt;
//			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

			char szCP[20] = { 0 };
			sprintf_s(szCP, "FIX%d", j);
			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp, rt, CV_RGB(0, 255, 0), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
		if (pPaper->pModel)
		{
			RECTLIST::iterator itFixRect = pPaper->pModel->vecPaperModel[i]->lFix.begin();								//显示模板上的定点对应到此试卷上的新定点
			for (int j = 0; itFixRect != pPaper->pModel->vecPaperModel[i]->lFix.end(); itFixRect++, j++)
			{
				cv::Rect rt = (*itFixRect).rt;
				//			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				char szCP[20] = { 0 };
				sprintf_s(szCP, "FIX%d", j);
				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			// 		RECTLIST::iterator itHRect = pPaper->pModel->vecPaperModel[i].lH_Head.begin();
			// 		for (int j = 0; itHRect != pPaper->pModel->vecPaperModel[i].lH_Head.end(); itHRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itHRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "H%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(55, 0, 255), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itVRect = pPaper->pModel->vecPaperModel[i].lV_Head.begin();
			// 		for (int j = 0; itVRect != pPaper->pModel->vecPaperModel[i].lV_Head.end(); itVRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itVRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "V%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(55, 0, 255), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itABRect = pPaper->pModel->vecPaperModel[i].lABModel.begin();
			// 		for (int j = 0; itABRect != pPaper->pModel->vecPaperModel[i].lABModel.end(); itABRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itABRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "AB%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itCourseRect = pPaper->pModel->vecPaperModel[i].lCourse.begin();
			// 		for (int j = 0; itCourseRect != pPaper->pModel->vecPaperModel[i].lCourse.end(); itCourseRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itCourseRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "C%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itQKRect = pPaper->pModel->vecPaperModel[i].lQK_CP.begin();
			// 		for (int j = 0; itQKRect != pPaper->pModel->vecPaperModel[i].lQK_CP.end(); itQKRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itQKRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "QK%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itGrayRect = pPaper->pModel->vecPaperModel[i].lGray.begin();
			// 		for (int j = 0; itGrayRect != pPaper->pModel->vecPaperModel[i].lGray.end(); itGrayRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itGrayRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "G%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}
			// 		RECTLIST::iterator itWhiteRect = pPaper->pModel->vecPaperModel[i].lWhite.begin();
			// 		for (int j = 0; itWhiteRect != pPaper->pModel->vecPaperModel[i].lWhite.end(); itWhiteRect++, j++)
			// 		{
			// 			cv::Rect rt = (*itWhiteRect).rt;
			// 			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
			// 
			// 			char szCP[20] = { 0 };
			// 			sprintf_s(szCP, "W%d", j);
			// 			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			// 			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			// 			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			// 		}

			//打印OMR、SN位置
#ifdef PaintOmrSnRect
// 			SNLIST::iterator itSN = pPaper->pModel->vecPaperModel[i]->lSNInfo.begin();
// 			for (; itSN != pPaper->pModel->vecPaperModel[i]->lSNInfo.end(); itSN++)
// 			{
// 				pSN_ITEM pSnItem = *itSN;
// 				RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
// 				for (; itSnItem != pSnItem->lSN.end(); itSnItem++)
// 				{
// 					cv::Rect rt = (*itSnItem).rt;
// 	#ifdef Test_ShowOriPosition
// 
// 					GetPosition2(inverseMat, rt, rt);
// 	#else
// 					GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
// 	#endif
// 					rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
// 					rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
// 				}
// 			}
// 
// 			OMRLIST::iterator itOmr = pPaper->pModel->vecPaperModel[i]->lOMR2.begin();
// 			for (; itOmr != pPaper->pModel->vecPaperModel[i]->lOMR2.end(); itOmr++)
// 			{
// 				pOMR_QUESTION pOmrQuestion = &(*itOmr);
// 				RECTLIST::iterator itOmrItem = pOmrQuestion->lSelAnswer.begin();
// 				for (; itOmrItem != pOmrQuestion->lSelAnswer.end(); itOmrItem++)
// 				{
// 					cv::Rect rt = (*itOmrItem).rt;
// 	#ifdef Test_ShowOriPosition
// 
// 					GetPosition2(inverseMat, rt, rt);
// 	#else
// 					GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
// 	#endif
// 					rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
// 					rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
// 				}
// 			}
#endif
		}
		
		addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
		m_vecPicShow[i]->ShowPic(tmp);
	}
}

int CScanToolDlg::PaintIssueRect(pST_PaperInfo pPaper)
{
	int nResult = -1;
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		if ((*itPic)->bFindIssue)
		{
			Point pt(0, 0);
			Mat matSrc = imread((*itPic)->strPicPath);
#ifdef PIC_RECTIFY_TEST
			Mat dst;
			Mat rotMat;
			PicRectify(matSrc, dst, rotMat);
			Mat matImg;
			if (dst.channels() == 1)
				cvtColor(dst, matImg, CV_GRAY2BGR);
			else
				matImg = dst;
// #ifdef WarpAffine_TEST
// 			cv::Mat	inverseMat(2, 3, CV_32FC1);
// 			PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
// #endif
#else
			Mat matImg = matSrc;
#endif

#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			if (pPaper->pModel)
				PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
#endif

			Mat tmp = matImg;	// matSrc.clone();
			Mat tmp2 = matImg.clone();

			if (pPaper->pModel)
			{
				RECTLIST::iterator itSelRoi = pPaper->pModel->vecPaperModel[i]->lSelFixRoi.begin();													//显示识别定点的选择区
				for (int j = 0; itSelRoi != pPaper->pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++, j++)
				{
					cv::Rect rt = (*itSelRoi).rt;
					//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);

					char szCP[20] = { 0 };
					// 				sprintf_s(szCP, "FIX%d", j);
					// 				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
					rectangle(tmp, rt, CV_RGB(0, 0, 255), 2);
					rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
				}
			}

			RECTLIST::iterator itPicFix = (*itPic)->lFix.begin();													//显示识别出来的定点
			for (int j = 0; itPicFix != (*itPic)->lFix.end(); itPicFix++, j++)
			{
				cv::Rect rt = (*itPicFix).rt;
//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);

				char szCP[20] = { 0 };
				sprintf_s(szCP, "FIX%d", j);
				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, rt, CV_RGB(0, 255, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}

			if (pPaper->pModel)
			{
				RECTLIST::iterator itFixRect = pPaper->pModel->vecPaperModel[i]->lFix.begin();							//显示模板上的定点对应到此试卷上的新定点
				for (int j = 0; itFixRect != pPaper->pModel->vecPaperModel[i]->lFix.end(); itFixRect++, j++)
				{
					cv::Rect rt = (*itFixRect).rt;
					//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

					char szCP[20] = { 0 };
					sprintf_s(szCP, "FIX%d", j);
					putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
					rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
					rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
				}

				// 			RECTLIST::iterator itHRect = pPaper->pModel->vecPaperModel[i].lH_Head.begin();
				// 			for (int j = 0; itHRect != pPaper->pModel->vecPaperModel[i].lH_Head.end(); itHRect++, j++)
				// 			{
				// 				cv::Rect rt = (*itHRect).rt;
				// 				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
				// 
				// 				char szCP[20] = { 0 };
				// 				sprintf_s(szCP, "H%d", j);
				// 				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				// 				rectangle(tmp, rt, CV_RGB(55, 0, 255), 2);
				// 				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
				// 			}
				// 			RECTLIST::iterator itVRect = pPaper->pModel->vecPaperModel[i].lV_Head.begin();
				// 			for (int j = 0; itVRect != pPaper->pModel->vecPaperModel[i].lV_Head.end(); itVRect++, j++)
				// 			{
				// 				cv::Rect rt = (*itVRect).rt;
				// 				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
				// 
				// 				char szCP[20] = { 0 };
				// 				sprintf_s(szCP, "V%d", j);
				// 				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				// 				rectangle(tmp, rt, CV_RGB(55, 0, 255), 2);
				// 				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
				// 			}

				RECTLIST::iterator itHTracker = pPaper->pModel->vecPaperModel[i]->lSelHTracker.begin();
				for (int j = 0; itHTracker != pPaper->pModel->vecPaperModel[i]->lSelHTracker.end(); itHTracker++, j++)
				{
					cv::Rect rt = (*itHTracker).rt;
					//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

					rectangle(tmp, rt, CV_RGB(25, 200, 20), 2);
					rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
				}
				RECTLIST::iterator itVTracker = pPaper->pModel->vecPaperModel[i]->lSelVTracker.begin();
				for (int j = 0; itVTracker != pPaper->pModel->vecPaperModel[i]->lSelVTracker.end(); itVTracker++, j++)
				{
					cv::Rect rt = (*itVTracker).rt;
					//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

					rectangle(tmp, rt, CV_RGB(25, 200, 20), 2);
					rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
				}
			}
			

			RECTLIST::iterator itNormal = (*itPic)->lNormalRect.begin();
			for (int j = 0; itNormal != (*itPic)->lNormalRect.end(); itNormal++, j++)
			{
				cv::Rect rt = (*itNormal).rt;

				char szCP[20] = { 0 };
				rectangle(tmp, rt, CV_RGB(50, 255, 55), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}

			nResult = i;
			RECTLIST::iterator itIssueRect = (*itPic)->lIssueRect.begin();
			for (int j = 0; itIssueRect != (*itPic)->lIssueRect.end(); itIssueRect++, j++)
			{
				cv::Rect rt = (*itIssueRect).rt;			
//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i].lFix, rt);
				if (j == 0)
				{
					pt = (*itIssueRect).rt.tl();	// +(*itPic)->ptFix;		//(*itIssueRect).rt.tl()
					pt.x = pt.x - 100;
					pt.y = pt.y - 100;
				}
				char szCP[20] = { 0 };
				sprintf_s(szCP, "Err%d", j);
#if 1
				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 200, 100), -1);
#else
				putText(tmp, szCP, Point((*itIssueRect).rt.x, (*itIssueRect).rt.y + (*itIssueRect).rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, (*itIssueRect).rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, (*itIssueRect).rt, CV_RGB(255, 200, 100), -1);
#endif
			}

			addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
			m_vecPicShow[i]->ShowPic(tmp, pt);
		}
	}
	return nResult;
}

LRESULT CScanToolDlg::MsgRecogErr(WPARAM wParam, LPARAM lParam)
{
	pST_PaperInfo pPaper	= (pST_PaperInfo)wParam;
	pPAPERSINFO   pPapers	= (pPAPERSINFO)lParam;

	TRACE("\nMsgRecogErr ==> 识别出错，需要停止扫描仪\n");
	_bTwainContinue = FALSE;
//	CancelTransfer();
//	ReleaseScan();
// 	if (DisableSource())
// 	{
// 		TRACE("nMsgRecogErr ==> DisableSource() success\n");
// 	}

	int nIssuePaper = 0;
	int nCount = m_lcPicture.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pPaperData = (pST_PaperInfo)m_lcPicture.GetItemData(i);
		if (pPaper == pPaperData)
		{
			nIssuePaper = PaintIssueRect(pPaperData);

			m_lcPicture.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			m_lcPicture.SetSelectionMark(i);
			m_lcPicture.SetFocus();
			break;
		}
	}
	if (nIssuePaper < 0)
		return FALSE;

	m_nCurrTabSel = nIssuePaper;

	m_tabPicShowCtrl.SetCurSel(nIssuePaper);
	m_pCurrentPicShow = m_vecPicShow[nIssuePaper];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != nIssuePaper)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
	m_pCurrentShowPaper = pPaper;
	m_nScanStatus = 2;
	USES_CONVERSION;
	char szErrInfo[200] = { 0 };
	sprintf_s(szErrInfo, "考生 %s 第%d页识别出错误校验点", pPaper->strStudentInfo.c_str(), nIssuePaper + 1);
	SetStatusShowInfo(A2T(szErrInfo), TRUE);
	return TRUE;
}

void CScanToolDlg::OnBnClickedBtnUploadpapers()
{
	m_bF2Enable = FALSE;
	if (!m_pPapersInfo)
	{
		AfxMessageBox(_T("没有试卷袋信息"));
		m_bF2Enable = TRUE;
		return;
	}

	int nUnRecogCount = g_lRecogTask.size();
	if (nUnRecogCount > 0)
	{
		AfxMessageBox(_T("请稍后，图像正在识别！"));
		return;
	}

	BOOL bLogin = FALSE;
	CString strUser = _T("");
	CString strEzs = _T("");
	int nTeacherId = -1;
	int nUserId = -1;
#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();
	bLogin = pDlg->m_bLogin;
	strUser = pDlg->m_strUserName;
	strEzs = pDlg->m_strEzs;
	nTeacherId = pDlg->m_nTeacherId;
	nUserId = pDlg->m_nUserId;
#else
	bLogin = m_bLogin;
	strUser = m_strUserName;
	strEzs = m_strEzs;
	nTeacherId = m_nTeacherId;
	nUserId = m_nUserId;
#endif

#ifndef TO_WHTY
	if (!bLogin)
	{
		AfxMessageBox(_T("没有登录，不能上传，请先登录!"));
		m_bF2Enable = TRUE;
		return;
	}
#endif
	if (m_pPapersInfo->lIssue.size() > 0)
	{
		AfxMessageBox(_T("存在识别异常试卷，不能上传，请先处理异常试卷"));
		m_bF2Enable = TRUE;
		return;
	}

	int nExamID = 0;
	int nSubjectID = 0;
#ifndef TO_WHTY
	CPapersInfoSaveDlg dlg(m_pPapersInfo, m_pModel);
	if (dlg.DoModal() != IDOK)
	{
		m_bF2Enable = TRUE;
		return;
	}

	nExamID = dlg.m_nExamID;
	nSubjectID = dlg.m_SubjectID;
#else
	if (MessageBox(_T("是否上传当前扫描卷?"), _T("提示"), MB_YESNO) != IDYES)
	{
		m_bF2Enable = TRUE;
		return;
	}

	Poco::LocalDateTime nowTime;
	Poco::Random rm;
	rm.seed();
	char szPapersName[50] = { 0 };
	sprintf_s(szPapersName, "%d%02d%02d%02d%02d%02d-%05d", nowTime.year(), nowTime.month(), nowTime.day(), nowTime.hour(), nowTime.minute(), nowTime.second(), rm.next(99999));
	m_pPapersInfo->strPapersName = szPapersName;
#endif

	clock_t start, end;
	start = clock();

	USES_CONVERSION;
	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itNomarlPaper = m_pPapersInfo->lPaper.begin();
	for (; itNomarlPaper != m_pPapersInfo->lPaper.end(); itNomarlPaper++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itNomarlPaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itNomarlPaper)->strSN);
		jsnPaper.set("qk", (*itNomarlPaper)->nQKFlag);
		
		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itNomarlPaper)->lSnResult.begin();
		for (; itSn != (*itNomarlPaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				jsnSnPosition.set("x", itRect->rt.x);
				jsnSnPosition.set("y", itRect->rt.y);
				jsnSnPosition.set("w", itRect->rt.width);
				jsnSnPosition.set("h", itRect->rt.height);
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaper.set("snDetail", jsnSnDetailArry);

		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itNomarlPaper)->lOmrResult.begin();
		for (; itOmr != (*itNomarlPaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value2", itOmr->strRecogVal2);
			jsnOmr.set("doubt", itOmr->nDoubt);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);

		Poco::JSON::Array jsnElectOmrArry;
		ELECTOMR_LIST::iterator itElectOmr = (*itNomarlPaper)->lElectOmrResult.begin();
		for (; itElectOmr != (*itNomarlPaper)->lElectOmrResult.end(); itElectOmr++)
		{
			Poco::JSON::Object jsnElectOmr;
			jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnElectOmr.set("value", itElectOmr->strRecogResult);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
			for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnElectOmr.set("position", jsnPositionArry);
			jsnElectOmrArry.add(jsnElectOmr);
		}
		jsnPaper.set("electOmr", jsnElectOmrArry);		//选做题结果
		jsnPaperArry.add(jsnPaper);
	}
	PAPER_LIST::iterator itIssuePaper = m_pPapersInfo->lIssue.begin();
	for (; itIssuePaper != m_pPapersInfo->lIssue.end(); itIssuePaper++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itIssuePaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itIssuePaper)->strSN);
		jsnPaper.set("qk", (*itIssuePaper)->nQKFlag);

		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itIssuePaper)->lSnResult.begin();
		for (; itSn != (*itIssuePaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				jsnSnPosition.set("x", itRect->rt.x);
				jsnSnPosition.set("y", itRect->rt.y);
				jsnSnPosition.set("w", itRect->rt.width);
				jsnSnPosition.set("h", itRect->rt.height);
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaper.set("snDetail", jsnSnDetailArry);

		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itIssuePaper)->lOmrResult.begin();
		for (; itOmr != (*itIssuePaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value2", itOmr->strRecogVal2);
			jsnOmr.set("doubt", itOmr->nDoubt);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);
		
		Poco::JSON::Array jsnElectOmrArry;
		ELECTOMR_LIST::iterator itElectOmr = (*itIssuePaper)->lElectOmrResult.begin();
		for (; itElectOmr != (*itIssuePaper)->lElectOmrResult.end(); itElectOmr++)
		{
			Poco::JSON::Object jsnElectOmr;
			jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnElectOmr.set("value", itElectOmr->strRecogResult);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
			for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
			{
				Poco::JSON::Object jsnItem;
				char szVal[5] = { 0 };
				sprintf_s(szVal, "%c", itRect->nAnswer + 65);
				jsnItem.set("val", szVal);
				jsnItem.set("x", itRect->rt.x);
				jsnItem.set("y", itRect->rt.y);
				jsnItem.set("w", itRect->rt.width);
				jsnItem.set("h", itRect->rt.height);
				jsnPositionArry.add(jsnItem);
			}
			jsnElectOmr.set("position", jsnPositionArry);
			jsnElectOmrArry.add(jsnElectOmr);
		}
		jsnPaper.set("electOmr", jsnElectOmrArry);		//选做题结果
		jsnPaperArry.add(jsnPaper);
	}
	//写试卷袋信息到文件
	std::string strUploader = CMyCodeConvert::Gb2312ToUtf8(T2A(strUser));
	std::string sEzs = T2A(strEzs);
	Poco::JSON::Object jsnFileData;
	jsnFileData.set("examId", nExamID);
	jsnFileData.set("subjectId", nSubjectID);
	jsnFileData.set("uploader", strUploader);
	jsnFileData.set("ezs", sEzs);
	jsnFileData.set("nTeacherId", nTeacherId);
	jsnFileData.set("nUserId", nUserId);
	jsnFileData.set("scanNum", m_pPapersInfo->nPaperCount);		//扫描的学生数量
	jsnFileData.set("detail", jsnPaperArry);

	jsnFileData.set("nOmrDoubt", m_pPapersInfo->nOmrDoubt);
	jsnFileData.set("nOmrNull", m_pPapersInfo->nOmrNull);
	jsnFileData.set("nSnNull", m_pPapersInfo->nSnNull);
	std::stringstream jsnString;
	jsnFileData.stringify(jsnString, 0);

	std::string strFileData;
#ifdef USES_FILE_ENC
	if(!encString(jsnString.str(), strFileData))
		strFileData = jsnString.str();
#else
	strFileData = jsnString.str();
#endif

	char szExamInfoPath[MAX_PATH] = { 0 };
	sprintf_s(szExamInfoPath, "%s\\papersInfo.dat", m_strCurrPicSavePath.c_str());
	ofstream out(szExamInfoPath);
	out << strFileData.c_str();
	out.close();
	//

	//试卷袋压缩
	char szPapersSavePath[MAX_PATH] = { 0 };
	char szZipName[50] = { 0 };
	if (bLogin)
	{
		Poco::LocalDateTime now;
		char szTime[50] = { 0 };
		sprintf_s(szTime, "%d%02d%02d%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

		sprintf_s(szPapersSavePath, "%sPaper\\%s_%d-%d_%s", T2A(g_strCurrentPath), T2A(strUser), nExamID, nSubjectID, szTime);
		sprintf_s(szZipName, "%s_%d-%d_%s%s", T2A(strUser), nExamID, nSubjectID, szTime, T2A(PAPERS_EXT_NAME));	//%s_%s.pkg
	}
	else
	{
		sprintf_s(szPapersSavePath, "%sPaper\\%s", T2A(g_strCurrentPath), m_pPapersInfo->strPapersName.c_str());
		sprintf_s(szZipName, "%s%s", m_pPapersInfo->strPapersName.c_str(), T2A(PAPERS_EXT_NAME));
	}
	CString strInfo;
	bool bWarn = false;
	strInfo.Format(_T("正在保存%s..."), A2T(szZipName));
	SetStatusShowInfo(strInfo, bWarn);
#if 1
	pCOMPRESSTASK pTask = new COMPRESSTASK;
	pTask->strCompressFileName = szZipName;
	pTask->strExtName = T2A(PAPERS_EXT_NAME);
	pTask->strSavePath = szPapersSavePath;
	pTask->strSrcFilePath = m_strCurrPicSavePath;
	g_fmCompressLock.lock();
	g_lCompressTask.push_back(pTask);
	g_fmCompressLock.unlock();
#else
	if (!ZipFile(A2T(m_strCurrPicSavePath.c_str()), A2T(szPapersSavePath), PAPERS_EXT_NAME))
	{
		bWarn = true;
		strInfo.Format(_T("保存%s失败"), A2T(szZipName));
	}
	else
	{
		end = clock();
		strInfo.Format(_T("保存%s成功"), A2T(szZipName));
		SAFE_RELEASE(m_pPapersInfo);
		m_lcPicture.DeleteAllItems();
		m_pCurrentShowPaper = NULL;
	}
	SetStatusShowInfo(strInfo, bWarn);

	if (bWarn)
	{
		m_bF2Enable = TRUE;
		return;
	}

	//添加上传列表，	******************		需要进行鉴权操作	***************	
	#if 1
		char szFileFullPath[300] = { 0 };
		sprintf_s(szFileFullPath, "%s%s", szPapersSavePath, T2A(PAPERS_EXT_NAME));
		pSENDTASK pTask = new SENDTASK;
		pTask->strFileName	= szZipName;
		pTask->strPath = szFileFullPath;
		g_fmSendLock.lock();
		g_lSendTask.push_back(pTask);
		g_fmSendLock.unlock();
	#endif
	m_bF2Enable = TRUE;
#endif
}

LRESULT CScanToolDlg::RoiLBtnDown(WPARAM wParam, LPARAM lParam)
{
	cv::Point pt = *(cv::Point*)(wParam);
	ShowRectByPoint(pt, m_pCurrentShowPaper);
	return TRUE;
}

void CScanToolDlg::ShowRectByPoint(cv::Point pt, pST_PaperInfo pPaper)
{
	if (!pPaper || !pPaper->pModel || pPaper->pModel->vecPaperModel.size() < m_nCurrTabSel)
		return;

// 	if (!pPaper->bIssuePaper)		//当前卷面没有问题点，不进行显示
// 		return;

	int nFind = -1;
	RECTINFO* pRc = NULL;
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		if (i == m_nCurrTabSel)
			break;
	}
// 	pt.x = pt.x - (*itPic)->ptFix.x;
// 	pt.y = pt.y - (*itPic)->ptFix.y;
	cv::Rect rt;
	rt.x = pt.x;
	rt.y = pt.y;
//	GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[m_nCurrTabSel].lFix, rt);
// 	GetPosition(pPaper->pModel->vecPaperModel[m_nCurrTabSel].lFix, (*itPic)->lFix, rt);
// 	pt.x = rt.x;
// 	pt.y = rt.y;
	nFind = GetRectInfoByPoint(pt, *itPic, pRc);
	if (nFind < 0)
		return;

	if (!pRc)
		return;

	CString strInfo;
	strInfo.Format(_T("阀值: %d, 要求比例: %.3f, 实际: %.3f"), pRc->nThresholdValue, pRc->fStandardValuePercent, pRc->fRealValuePercent);
	if (pPaper->bIssuePaper)
		SetStatusShowInfo(strInfo, TRUE);
	else
		SetStatusShowInfo(strInfo);
}

int CScanToolDlg::GetRectInfoByPoint(cv::Point pt, pST_PicInfo pPic, RECTINFO*& pRc)
{
	int  nFind = -1;
	RECTLIST::iterator itIssueRectInfo = pPic->lIssueRect.begin();
	for (int i = 0; itIssueRectInfo != pPic->lIssueRect.end(); itIssueRectInfo++, i++)
	{
		if (itIssueRectInfo->rt.contains(pt))
		{
			nFind = i;
			pRc = &(*itIssueRectInfo);
			break;
		}
	}
	
	if (nFind < 0)
	{
		RECTLIST::iterator itRectInfo = pPic->lNormalRect.begin();
		for (int i = 0; itRectInfo != pPic->lNormalRect.end(); itRectInfo++, i++)
		{
			if (itRectInfo->rt.contains(pt))
			{
				nFind = i;
				pRc = &(*itRectInfo);
				break;
			}
		}
	}
	return nFind;
}

void CScanToolDlg::OnBnClickedBtnModelmgr()
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
		m_pShowModelInfoDlg->ShowModelInfo(m_pModel);
		if (!m_pModel)
			return;

		m_nModelPicNums = m_pModel->nPicNum;
		InitTab();

		//模板切换后之前的扫描试卷需要清除
		m_pCurrentShowPaper = NULL;
		m_lcPicture.DeleteAllItems();
		SAFE_RELEASE(m_pPapersInfo);
	}
}


void CScanToolDlg::OnNMHoverListPicture(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;		//**********	这里如果不响应，同时返回结果值不为1的话，	****************
						//**********	就会产生产生TRACK SELECT，也就是鼠标悬停	****************
						//**********	一段时间后，所在行自动被选中
}


void CScanToolDlg::OnLvnKeydownListPicture(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	*pResult = 0;

	if (pLVKeyDow->wVKey == VK_UP)
	{
		m_nCurrItemPaperList--;
		if (m_nCurrItemPaperList <= 0)
			m_nCurrItemPaperList = 0;

		ShowPaperByItem(m_nCurrItemPaperList);
	}
	else if (pLVKeyDow->wVKey == VK_DOWN)
	{

		m_nCurrItemPaperList++;
		if (m_nCurrItemPaperList >= m_lcPicture.GetItemCount() - 1)
			m_nCurrItemPaperList = m_lcPicture.GetItemCount() - 1;

		ShowPaperByItem(m_nCurrItemPaperList);
	}
}

void CScanToolDlg::ShowPaperByItem(int nItem)
{
	if (nItem < 0)
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lcPicture.GetItemData(nItem);
//	m_nCurrItemPaperList = pNMItemActivate->iItem;

	m_pCurrentShowPaper = pPaper;
	if (pPaper->bIssuePaper)
		PaintIssueRect(pPaper);
	else
		PaintRecognisedRect(pPaper);

	m_nCurrTabSel = 0;

	m_tabPicShowCtrl.SetCurSel(0);
	m_pCurrentPicShow = m_vecPicShow[0];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != 0)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}


void CScanToolDlg::OnBnClickedBtnUploadmgr()
{
	CShowFileTransferDlg dlg;
	dlg.DoModal();
}

void CScanToolDlg::InitFileUpLoadList()
{
	USES_CONVERSION;
	std::string strFilePath = T2A(g_strCurrentPath + _T("tmpFileList.dat"));
	
	std::ifstream in(strFilePath);
	if (!in)	return ;

	std::string strJsnData;
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

		std::string strDate = objData->get("time").convert<std::string>();
		Poco::JSON::Array::Ptr objArry = objData->getArray("fileList");
		if (objArry->size())
		{
			CString strMsg = _T("");
			strMsg.Format(_T("存在未上传成功的文件(%d个)，是否继续上传?(上次上传时间: %s)"), objArry->size(), A2T(strDate.c_str()));
			if (MessageBox(strMsg, _T(""), MB_YESNO) != IDYES)
			{
				Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(strFilePath));
				if (fileList.exists())
					fileList.remove();

				return;
			}
		}
		for (int i = 0; i < objArry->size(); i++)
		{
			Poco::JSON::Object::Ptr objFileTask = objArry->getObject(i);

			pSENDTASK pTask = new SENDTASK;
			pTask->strFileName	= CMyCodeConvert::Utf8ToGb2312(objFileTask->get("name").convert<std::string>());
			pTask->strPath		= CMyCodeConvert::Utf8ToGb2312(objFileTask->get("path").convert<std::string>());
			g_fmSendLock.lock();
			g_lSendTask.push_back(pTask);
			g_fmSendLock.unlock();
		}
		Poco::File fileList(CMyCodeConvert::Gb2312ToUtf8(strFilePath));
		if (fileList.exists())
			fileList.remove();
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("解析临时文件发送列表失败: ");
		strErrInfo.append(exc.message());
		g_pLogger->information(strErrInfo);
	}
}

void CScanToolDlg::InitParam()
{
	std::string strLog;
	std::string strFile = g_strCurrentPath + "param.dat";
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strFile);
	try
	{
		Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));

		g_nRecogGrayMin		= pConf->getInt("RecogGray.gray_Min", 0);
		g_nRecogGrayMax_White = pConf->getInt("RecogGray.white_Max", 255);
		g_nRecogGrayMin_OMR = pConf->getInt("RecogGray.omr_Min", 0);
		g_RecogGrayMax_OMR	= pConf->getInt("RecogGray.omr_Max", 235);

		_dCompThread_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fCompTread", 1.2);
		_dDiffThread_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fDiffThread", 0.2);
		_dDiffExit_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fDiffExit", 0.3);
		_dCompThread_Head_ = pConf->getDouble("RecogOmrSn_Head.fCompTread", 1.2);
		_dDiffThread_Head_ = pConf->getDouble("RecogOmrSn_Head.fDiffThread", 0.085);
		_dDiffExit_Head_ = pConf->getDouble("RecogOmrSn_Head.fDiffExit", 0.15);
		
		strLog = "读取识别灰度参数完成";
	}
	catch (Poco::Exception& exc)
	{
		strLog = "读取参数失败，使用默认参数 " + CMyCodeConvert::Utf8ToGb2312(exc.displayText());
		g_nRecogGrayMin		= 0;
		g_nRecogGrayMax_White = 255;
		g_nRecogGrayMin_OMR = 0;
		g_RecogGrayMax_OMR	= 235;
	}
	g_pLogger->information(strLog);
}

void CScanToolDlg::OnClose()
{
#ifdef SHOW_GUIDEDLG
	if (m_nScanStatus == 1)
	{
		AfxMessageBox(_T("扫描进行中, 请稍后操作!"));
		return;
	}
	if (m_pPapersInfo)
	{
		if (MessageBox(_T("存在未保存的试卷，是否退出？"), _T("警告"), MB_YESNO) != IDYES)
			return;
	}
	ReleaseScan();
	((CGuideDlg*)AfxGetMainWnd())->m_pModel = m_pModel;
	((CGuideDlg*)AfxGetMainWnd())->ShowWindow(SW_SHOW);
	this->ShowWindow(SW_HIDE);
	m_bF1Enable = FALSE;
	m_bF2Enable = FALSE;
	return;
#endif
	__super::OnClose();
}

void CScanToolDlg::InitShow(pMODEL pModel)
{
	if (m_pModel != pModel)
	{
		SAFE_RELEASE(m_pModel);
		m_pModel = pModel;
	}
	if (pModel != NULL)
		m_nModelPicNums = pModel->nPicNum;
	else
		m_nModelPicNums = 1;
	InitTab();
	m_pShowModelInfoDlg->ShowModelInfo(m_pModel);

	SAFE_RELEASE(m_pPapersInfo);
	m_lcPicture.DeleteAllItems();
	m_pCurrentShowPaper = NULL;

	m_comboModel.ResetContent();
	m_ncomboCurrentSel = -1;
	SearchModel();
	m_comboModel.SetCurSel(m_ncomboCurrentSel);
}

void CScanToolDlg::InitScan()
{
	if (!m_bTwainInit)
	{
		if (!m_bTwainInit)
		{
			m_bTwainInit = InitTwain(m_hWnd);
			if (!IsValidDriver())
			{
				AfxMessageBox(_T("Unable to load Twain Driver."));
			}
			m_scanSourceArry.RemoveAll();
			ScanSrcInit();
		}
	}
	m_nScanStatus = 0;
}

void CScanToolDlg::ReleaseScan()
{
	ReleaseTwain();
	m_bTwainInit = FALSE;
}

void CScanToolDlg::OnBnClickedBtnReback()
{
	if (m_nScanStatus == 1)
	{
		AfxMessageBox(_T("扫描进行中, 请稍后操作!"));
		return;
	}
	if (m_pPapersInfo)
	{
		if (MessageBox(_T("存在未保存的试卷，是否退出？"), _T("警告"), MB_YESNO) != IDYES)
			return;
	}
	ReleaseScan();
	((CGuideDlg*)AfxGetMainWnd())->m_pModel = m_pModel;
	((CGuideDlg*)AfxGetMainWnd())->ShowWindow(SW_SHOW);
	this->ShowWindow(SW_HIDE);
	m_bF1Enable = FALSE;
	m_bF2Enable = FALSE;
}

void sharpenImage1(const cv::Mat &image, cv::Mat &result)
{
	//创建并初始化滤波模板
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = 5;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//对图像进行滤波
	cv::filter2D(image, result, image.depth(), kernel);
}

int GetRects(cv::Mat& matSrc, cv::Rect rt, pMODEL pModel, int nPic, int nOrientation, int nHead)
{
	int nResult = 0;
	std::vector<Rect>RectCompList;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matSrc.cols)
		{
			rt.width = matSrc.cols - rt.x;
		}
		if (rt.br().y > matSrc.rows)
		{
			rt.height = matSrc.rows - rt.y;
		}

		Mat matCompRoi;
		matCompRoi = matSrc(rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
		sharpenImage1(matCompRoi, matCompRoi);

#ifdef USES_GETTHRESHOLD_ZTFB
		const int channels[1] = { 0 };
		const int histSize[1] = { 150 };
		float hranges[2] = { 0, 150 };
		const float* ranges[1];
		ranges[0] = hranges;
		MatND hist;
		calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		float fMean = (float)nSum / nCount;		//均值

		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nDevSum += pow(h - fMean, 2)*binVal;
		}
		float fStdev = sqrt(nDevSum / nCount);	//标准差
		int nThreshold = fMean + 2 * fStdev;
		if (fStdev > fMean)
			nThreshold = fMean + fStdev;

		if (nThreshold > 150) nThreshold = 150;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);

// 		int blockSize = 25;		//25
// 		int constValue = 10;
// 		cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
#else
		threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
		cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));	//Size(6, 6)	普通空白框可识别
		dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//提取轮廓  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
#if 1
		//模板图像的水平同步头平均长宽
		RECTLIST::iterator itBegin;
		if (nHead == 1)	//检测水平同步头
			itBegin = pModel->vecPaperModel[nPic]->lH_Head.begin();
		else
			itBegin = pModel->vecPaperModel[nPic]->lV_Head.begin();
		RECTINFO rcFist = *itBegin;
		RECTINFO rcSecond = *(++itBegin);

		int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
		int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

		float fPer_W, fPer_H;	//模板第二个点与第一个点的宽、高的比例，用于最小值控制
		cv::Rect rtFirst, rtSecond;
		if (nOrientation == 1 || nOrientation == 4)
		{
			rtSecond = rcSecond.rt;
			rtFirst = rcFist.rt;
			fPer_W = 0.5;
			fPer_H = 0.25;
		}
		else if (nOrientation == 2 || nOrientation == 3)
		{
			rtSecond.width = rcSecond.rt.height;
			rtSecond.height = rcSecond.rt.width;

			rtFirst.width = rcFist.rt.height;
			rtFirst.height = rcFist.rt.width;
			fPer_W = 0.25;
			fPer_H = 0.5;
		}

		if (pModel->nType == 1)
		{
			int nMid_modelW = rcSecond.rt.width;
			int nMid_modelH = rcSecond.rt.height;
			int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
			nMidInterW = 3;
			nMidInterH = 3;
			nHeadInterW = 4;
			nHeadInterH = 4;
			nMid_minW = nMid_modelW - nMidInterW;
			nMid_maxW = nMid_modelW + nMidInterW;
			nMid_minH = nMid_modelH - nMidInterH;
			nMid_maxH = nMid_modelH + nMidInterH;

			nHead_minW = rcFist.rt.width - nHeadInterW;
			nHead_maxW = rcFist.rt.width + nHeadInterW;
			nHead_minH = rcFist.rt.height - nHeadInterH;
			nHead_maxH = rcFist.rt.height + nHeadInterH;

// 			float fOffset = 0.1;
// 			int nMid_modelW = rtSecond.width + 2;		//加2是因为制卷模板框框没有经过查边框运算，经过查边框后，外框会包含整个矩形，需要加上上下各1个单位的线宽
// 			int nMid_modelH = rtSecond.height + 2;
// 			if (nMid_modelW < rtFirst.width * fPer_W + 0.5)	nMid_modelW = rtFirst.width * fPer_W + 0.5;
// 			if (nMid_modelH < rtFirst.height * fPer_H + 0.5)	nMid_modelH = rtFirst.height * fPer_H + 0.5;
// 			nMid_minW = nMid_modelW * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 			nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 			nMid_minH = nMid_modelH * (1 - fOffset);				//同上
// 			nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//同上
// 
// 			nHead_minW = rtFirst.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
// 			nHead_maxW = rtFirst.width * (1 + fOffset * 4) + 0.5;		//同上
// 			nHead_minH = rtFirst.height * (1 - fOffset);				//同上
// 			nHead_maxH = rtFirst.height * (1 + fOffset * 4) + 0.5;		//同上
		}
		else
		{
			float fOffset = 0.2;
			nMid_minW = rtSecond.width * (1 - fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_maxW = rtSecond.width * (1 + fOffset);		//中间同步头宽度与模板中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nMid_minH = rtSecond.height * (1 - fOffset);		//同上
			nMid_maxH = rtSecond.height * (1 + fOffset);		//同上

			nHead_minW = rtFirst.width * (1 - fOffset);		//两端同步头(第一个或最后一个)宽度与两端中间同步头宽度的偏差不超过模板同步头宽度的0.2
			nHead_maxW = rtFirst.width * (1 + fOffset);		//同上
			nHead_minH = rtFirst.height * (1 - fOffset);		//同上
			nHead_maxH = rtFirst.height * (1 + fOffset);		//同上
		}
		TRACE("w(%d, %d), h(%d, %d)，最大点:w(%d, %d), h(%d, %d)\n", nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, \
			  nHead_minW, nHead_maxW, nHead_minH, nHead_maxH);

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;

			if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
			{
				if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//排除第一个或最后一个大的同步头
				{
					TRACE("过滤同步头(%d,%d,%d,%d), 要求范围W:[%d,%d], H[%d,%d], 参考大小(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
					continue;
				}
				else
				{
					TRACE("首尾同步头(即定位点同步头)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
				}
			}
			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
#else
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*更新迭代索引*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;
			if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50 || rm.area() < 70)	//********** 需要寻找一种新的方法来过滤矩形	********
			{
//				TRACE("过滤矩形:(%d,%d,%d,%d), 面积: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
				continue;
			}
			RectCompList.push_back(rm);
		}
#endif
		nResult = RectCompList.size();
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "识别同步头异常: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		nResult = -1;
	}
	return nResult;
}

cv::Rect GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Rect rtResult;
	if (nOrientation == 1)	//matSrc正向
	{
		rtResult = rt;
	}
	else if (nOrientation == 2)	//matSrc右转90度
	{
		cv::Point pt1, pt2;
		pt1.x = nH - rt.tl().y;
		pt1.y = rt.tl().x;
		pt2.x = nH - rt.br().y;
		pt2.y = rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 3)	//matSrc左转90度
	{
		cv::Point pt1, pt2;
		pt1.x = rt.tl().y;
		pt1.y = nW - rt.tl().x;
		pt2.x = rt.br().y;
		pt2.y = nW - rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 4)	//matSrc右转180度
	{
		cv::Point pt1, pt2;
		pt1.x = nW - rt.tl().x;
		pt1.y = nH - rt.tl().y;
		pt2.x = nW - rt.br().x;
		pt2.y = nH - rt.br().y;
		rtResult = cv::Rect(pt1, pt2);
	}
	return rtResult;
}

int CScanToolDlg::CheckOrientation(cv::Mat& matSrc, int n)
{
	clock_t start, end;
	start = clock();

	bool bFind = false;
	int nResult = 1;	//1:针对模板图像需要进行的旋转，正向，不需要旋转，2：右转90(模板图像旋转), 3：左转90(模板图像旋转), 4：右转180(模板图像旋转)
	
	if (!m_pModel->nHasHead)
		return nResult;

	const float fMinPer = 0.5;		//识别矩形数/模板矩形数 低于最小值，认为不合格
	const float fMaxPer = 1.5;		//识别矩形数/模板矩形数 超过最大值，认为不合格
	const float fMidPer = 0.8;

	cv::Rect rtModelPic;
	rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
	rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matSrc.cols;
	rtSrcPic.height = matSrc.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matSrc.cols / matSrc.rows;

	cv::Rect rt1 = m_pModel->vecPaperModel[n]->rtHTracker;
	cv::Rect rt2 = m_pModel->vecPaperModel[n]->rtVTracker;
	TRACE("水平橡皮筋:(%d,%d,%d,%d), 垂直橡皮筋(%d,%d,%d,%d)\n", rt1.x, rt1.y, rt1.width, rt1.height, rt2.x, rt2.y, rt2.width, rt2.height);

	float fFirst_H, fFirst_V, fSecond_H, fSecond_V;
	fFirst_H = fFirst_V = fSecond_H = fSecond_V = 0.0;
	if (nModelPicPersent == nSrcPicPercent)	//与模板图片方向一致，需判断正向还是反向一致
	{
		TRACE("与模板图片方向一致\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			TRACE("查水平同步头\n");
			cv::Rect rtH = GetRectByOrientation(rtModelPic, m_pModel->vecPaperModel[n]->rtHTracker, i);
			int nHead_H = GetRects(matSrc, rtH, m_pModel, n, i, 1);		//查水平同步头数量
			int nSum_H = m_pModel->vecPaperModel[n]->lH_Head.size();

			float fSimilarity_H = (float)nHead_H / nSum_H;
			if (fSimilarity_H < fMinPer || fSimilarity_H > fMaxPer)
				continue;

			if (i == 1)
				fFirst_H = fSimilarity_H;
			else
				fSecond_H = fSimilarity_H;

			TRACE("查垂直同步头\n");
			cv::Rect rtH2 = GetRectByOrientation(rtModelPic, m_pModel->vecPaperModel[n]->rtVTracker, i);
			int nHead_V = GetRects(matSrc, rtH2, m_pModel, n, i, 2);	//查垂直同步头数量
			int nSum_V = m_pModel->vecPaperModel[n]->lV_Head.size();

			float fSimilarity_V = (float)nHead_V / nSum_V;

			char szLog[300] = { 0 };
			sprintf_s(szLog, "rtH = (%d,%d,%d,%d), rtH2 = (%d,%d,%d,%d),\nnHead_H = %d, nHead_V = %d, nSum_H = %d, nSum_V = %d, H=%.2f, V=%.2f\n", rtH.tl().x, rtH.tl().y, rtH.width, rtH.height, rtH2.tl().x, rtH2.tl().y, rtH2.width, rtH2.height, \
					  nHead_H, nHead_V, nSum_H, nSum_V, fSimilarity_H, fSimilarity_V);
			g_pLogger->information(szLog);
			TRACE(szLog);

			if (fSimilarity_H > fMidPer)
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (fSimilarity_V > fMidPer)
				{
					bFind = true;
					nResult = i;
					break;
				}
				else    //fSimilarity_V in [0.5,0.8]	有可能，再进行进一步判断
				{
					if (i == 1)
						fFirst_V = fSimilarity_V;
					else
						fSecond_V = fSimilarity_V;
				}
			}
			else	//fSimilarity_H in [0.5,0.8]	有可能，再进行进一步判断
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (i == 1)
					fFirst_V = fSimilarity_V;
				else
					fSecond_V = fSimilarity_V;
			}
		}

		if (!bFind)
		{
			if (fFirst_H > fSecond_H && fFirst_V > fSecond_V)
			{
				nResult = 1;
			}
			else if (fFirst_H < fSecond_H && fFirst_V < fSecond_V)
			{
				nResult = 4;
			}
			else
			{
				TRACE("无法判断图片方向\n");
				g_pLogger->information("无法判断图片方向");
				nResult = 1;
			}
		}
	}
	else	//与模板图片方向不一致，需判断向右旋转90还是向左旋转90
	{
		TRACE("与模板图片方向不一致\n");
		for (int i = 2; i <= 3; i++)
		{
			TRACE("查水平同步头\n");
			cv::Rect rtH = GetRectByOrientation(rtModelPic, m_pModel->vecPaperModel[n]->rtHTracker, i);
			int nHead_H = GetRects(matSrc, rtH, m_pModel, n, i, 1);		//查水平同步头数量
			int nSum_H = m_pModel->vecPaperModel[n]->lH_Head.size();

			float fSimilarity_H = (float)nHead_H / nSum_H;
			if (fSimilarity_H < fMinPer || fSimilarity_H > fMaxPer)
				continue;

			if (i == 2)
				fFirst_H = fSimilarity_H;
			else
				fSecond_H = fSimilarity_H;

			TRACE("查垂直同步头\n");
			cv::Rect rtH2 = GetRectByOrientation(rtModelPic, m_pModel->vecPaperModel[n]->rtVTracker, i);
			int nHead_V = GetRects(matSrc, rtH2, m_pModel, n, i, 2);		//查垂直同步头数量
			int nSum_V = m_pModel->vecPaperModel[n]->lV_Head.size();

			float fSimilarity_V = (float)nHead_V / nSum_V;

			char szLog[300] = { 0 };
			sprintf_s(szLog, "rtH = (%d,%d,%d,%d), rtH2 = (%d,%d,%d,%d),\nnHead_H = %d, nHead_V = %d, nSum_H = %d, nSum_V = %d, H=%.2f, V=%.2f\n", rtH.tl().x, rtH.tl().y, rtH.width, rtH.height, rtH2.tl().x, rtH2.tl().y, rtH2.width, rtH2.height, \
					  nHead_H, nHead_V, nSum_H, nSum_V, fSimilarity_H, fSimilarity_V);
			g_pLogger->information(szLog);
			TRACE(szLog);

			if (fSimilarity_H > fMidPer)
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (fSimilarity_V > fMidPer)
				{
					bFind = true;
					nResult = i;
					break;
				}
				else    //fSimilarity_V in [0.5,0.8]	有可能，再进行进一步判断
				{
					if (i == 2)
						fFirst_V = fSimilarity_V;
					else
						fSecond_V = fSimilarity_V;
				}
			}
			else	//fSimilarity_H in [0.5,0.8]	有可能，再进行进一步判断
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (i == 2)
					fFirst_V = fSimilarity_V;
				else
					fSecond_V = fSimilarity_V;
			}
		}

		if (!bFind)
		{
			if (fFirst_H > fSecond_H && fFirst_V > fSecond_V)
			{
				nResult = 2;
			}
			else if (fFirst_H < fSecond_H && fFirst_V < fSecond_V)
			{
				nResult = 3;
			}
			else
			{
				TRACE("无法判断图片方向\n");
				g_pLogger->information("无法判断图片方向");
				nResult = 1;
			}
		}
	}

	end = clock();
	TRACE("判断旋转方向时间: %dms\n", end - start);

	std::string strDirection;
	switch (nResult)
	{
		case 1: strDirection = "正向，不需要旋转"; break;
		case 2: strDirection = "右旋90"; break;
		case 3: strDirection = "左旋90"; break;
		case 4: strDirection = "右旋180"; break;
	}
	std::string strLog = "方向判断结果：" + strDirection;
	g_pLogger->information(strLog);
	TRACE("%s\n", strLog.c_str());

	return nResult;
}


void CScanToolDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (nHotKeyId == 1001)	//F1
	{
		if (m_bF1Enable && m_nScanStatus != 1)		//扫描中不进行快捷键响应
			OnBnClickedBtnScan();
	}
	else if (nHotKeyId == 1002)	//F2
	{
		if (m_bF2Enable && m_nScanStatus != 1)		//扫描中不进行快捷键响应
			OnBnClickedBtnUploadpapers();
	}
	__super::OnHotKey(nHotKeyId, nKey1, nKey2);
}

void CScanToolDlg::ReleaseUploadFileList()
{
	USES_CONVERSION;
	Poco::JSON::Object jsnFileList;
	Poco::JSON::Array jsnSendTaskArry;
	g_fmSendLock.lock();
	SENDTASKLIST::iterator itSendTask = g_lSendTask.begin();
	for (; itSendTask != g_lSendTask.end();)
	{
		if ((*itSendTask)->nSendState != 2)
		{
			//发送失败的文件暂存信息，下次登录时提示是否重新上传
			Poco::JSON::Object objTask;
			objTask.set("name", CMyCodeConvert::Gb2312ToUtf8((*itSendTask)->strFileName));
			objTask.set("path", CMyCodeConvert::Gb2312ToUtf8((*itSendTask)->strPath));
			jsnSendTaskArry.add(objTask);
		}
		pSENDTASK pTask = *itSendTask;
		itSendTask = g_lSendTask.erase(itSendTask);
		SAFE_RELEASE(pTask);
	}
	g_fmSendLock.unlock();
	if (jsnSendTaskArry.size())
	{
		Poco::LocalDateTime dtNow;
		std::string strData;
		Poco::format(strData, "%4d-%02d-%02d %02d:%02d", dtNow.year(), dtNow.month(), dtNow.day(), dtNow.hour(), dtNow.minute());
		jsnFileList.set("time", strData);
		jsnFileList.set("fileList", jsnSendTaskArry);

		std::stringstream jsnString;
		jsnFileList.stringify(jsnString);

		std::string strJsnFile = T2A(g_strCurrentPath + _T("tmpFileList.dat"));
		ofstream out(strJsnFile);
		if (out)
		{
			out << jsnString.str().c_str();
			out.close();
		}
	}
}

BOOL CScanToolDlg::StartGuardProcess()
{
	CString strProcessName = _T("");
	strProcessName.Format(_T("EasyTntGuardProcess.exe"));
	if (!CheckProcessExist(strProcessName))
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		CString strComm;
		char szWrkDir[MAX_PATH];
		strComm.Format(_T("%sEasyTntGuardProcess.exe"), g_strCurrentPath);
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		if (!CreateProcessW(NULL, (LPTSTR)(LPCTSTR)strComm, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			int nErrorCode = GetLastError();
			USES_CONVERSION;
			std::string strLog = Poco::format("CreateProcess %s failed. ErrorCode = %d", T2A(strComm), nErrorCode);
			g_pLogger->information(strLog);
			return FALSE;
		}
	}
	return TRUE;
}

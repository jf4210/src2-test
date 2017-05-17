
// ScanTool2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanTool3Dlg.h"
#include "afxdialogex.h"
#include "global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CScanTool2Dlg 对话框

//++全局变量
using namespace std;
using namespace cv;
int					g_nManulUploadFile = 0;		//手动上传文件，通过qq这类的
bool				g_bCmdConnect = false;		//命令通道连接
bool				g_bFileConnect = false;		//文件通道连接

bool				g_bCmdNeedConnect = false;	//命令通道是否需要重连，用于通道地址信息修改的情况
bool				g_bFileNeedConnect = false;	//文件通道是否需要重连，用于通道地址信息修改的情况

bool				g_bShowScanSrcUI = false;	//是否显示原始扫描界面
int					g_nOperatingMode = 2;		//操作模式，1--简易模式(遇到问题点不停止扫描)，2-严格模式(遇到问题点时立刻停止扫描)
int					g_nZkzhNull2Issue = 0;		//识别到准考证号未空时，是否认为是问题试卷

int					g_nExitFlag = 0;
CString				g_strCurrentPath;
std::string			g_strPaperSavePath;	//试卷扫描后保存的总路径
std::string			g_strModelSavePath;
std::string			g_strPaperBackupPath;	//试卷发送完成后的备份路径
Poco::Logger*		g_pLogger;
Poco::FastMutex		g_fmRecog;		//识别线程获取任务锁
RECOGTASKLIST		g_lRecogTask;	//识别任务列表

Poco::FastMutex		g_fmPapers;		//操作试卷袋列表的任务锁
PAPERS_LIST			g_lPapers;		//所有的试卷袋信息

Poco::FastMutex		g_fmSendLock;	//上传文件列表的锁
SENDTASKLIST		g_lSendTask;	//上传文件任务列表

Poco::FastMutex		g_fmTcpTaskLock;
TCP_TASKLIST		g_lTcpTask;		//tcp任务列表

Poco::FastMutex		g_lfmExamList;
EXAM_LIST			g_lExamList;	//当前账号对应的考试列表

Poco::FastMutex		g_fmCompressLock;		//压缩文件列表锁
COMPRESSTASKLIST	g_lCompressTask;		//解压文件列表

//++线程退出完成事件
Poco::Event			g_eTcpThreadExit;
Poco::Event			g_eSendFileThreadExit;
Poco::Event			g_eCompressThreadExit;
//--
//++事件定义
Poco::Event			g_eGetExamList;		//获取考试列表事件
Poco::Event			g_eDownLoadModel;	//下载模板完成状态
//--

int					g_nDownLoadModelStatus = 0;		//下载模板的状态	0-未下载，初始化，1-模板下载中，2-下载成功，3-本地存在此文件，不需要下载, -1-服务器此科目模板不存在, -2-服务器读取文件失败

STUDENT_LIST		g_lBmkStudent;	//报名库学生列表


double	_dCompThread_Fix_ = 1.2;
double	_dDiffThread_Fix_ = 0.2;
double	_dDiffExit_Fix_ = 0.3;
double	_dCompThread_Head_ = 1.0;
double	_dDiffThread_Head_ = 0.085;
double	_dDiffExit_Head_ = 0.15;
int		_nThreshold_Recog2_ = 240;	//第2中识别方法的二值化阀值
double	_dCompThread_3_ = 170;		//第三种识别方法
double	_dDiffThread_3_ = 20;
double	_dDiffExit_3_ = 50;
double	_dAnswerSure_ = 100;		//可以确认是答案的最大灰度

int		_nAnticlutterKernel_ = 4;	//识别同步头时防干扰膨胀腐蚀的核因子
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

int				_nReocgThreads_ = 3;		//识别线程数量

//++登录信息
bool	_bLogin_ = false;		//是否已经登录
std::string _strUserName_;		//登录用户名
std::string _strNickName_;		//用户昵称
std::string _strPwd_;			//密码
std::string _strEzs_;			//后端需要的EZS
int _nTeacherId_ = 0;			//教师ID
int _nUserId_ = 0;				//用户ID
//--
//++扫描相关
pEXAMINFO			_pCurrExam_= NULL;		//当前考试
pEXAM_SUBJECT		_pCurrSub_ = NULL;		//当前考试科目
pMODEL				_pModel_ = NULL;		//当前扫描使用的模板
//--
//--

CScanTool3Dlg::CScanTool3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScanTool3Dlg::IDD, pParent)
	, m_pExamInfoMgrDlg(NULL), m_pScanMgrDlg(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScanTool3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CScanTool3Dlg::InitThreads()
{
	m_pRecogThread = new Poco::Thread[_nReocgThreads_];
	for (int i = 0; i < _nReocgThreads_; i++)
	{
		CRecognizeThread* pRecogObj = new CRecognizeThread;
		m_pRecogThread[i].start(*pRecogObj);

		m_vecRecogThreadObj.push_back(pRecogObj);
	}
// 	m_SendFileThread = new Poco::Thread;
// 	m_pSendFileObj = new CSendFileThread(strFileServerIP, nFileServerPort);
// 	m_SendFileThread->start(*m_pSendFileObj);
	m_TcpCmdThread = new Poco::Thread;
	m_pTcpCmdObj = new CTcpClient(g_strCmdIP, g_nCmdPort);
	m_TcpCmdThread->start(*m_pTcpCmdObj);
	m_pCompressThread = new Poco::Thread;
	m_pCompressObj = new CCompressThread(this);
	m_pCompressThread->start(*m_pCompressObj);

// 	m_pScanThread = new Poco::Thread;
// 	m_pScanThreadObj = new CScanThread;
// 	m_pScanThread->start(*m_pScanThreadObj);
}

void CScanTool3Dlg::ReleaseThreads()
{
	//识别线程
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

	//文件发送线程
// 	m_SendFileThread->join();
// 	SAFE_RELEASE(m_pSendFileObj);
// 	g_eSendFileThreadExit.wait();
// 	SAFE_RELEASE(m_SendFileThread);

	//tcp命令线程
	m_TcpCmdThread->join();
	SAFE_RELEASE(m_pTcpCmdObj);
	g_eTcpThreadExit.wait();
	SAFE_RELEASE(m_TcpCmdThread);

	//压缩线程
	m_pCompressThread->join();
	SAFE_RELEASE(m_pCompressObj);
	g_eCompressThreadExit.wait();
	SAFE_RELEASE(m_pCompressThread);
}

void CScanTool3Dlg::InitCtrlPositon()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	if (m_pExamInfoMgrDlg && m_pExamInfoMgrDlg->GetSafeHwnd())
		m_pExamInfoMgrDlg->MoveWindow(rcClient);
	if (m_pScanMgrDlg && m_pScanMgrDlg->GetSafeHwnd())
		m_pScanMgrDlg->MoveWindow(rcClient);
}

void CScanTool3Dlg::ReleaseData()
{
	g_lfmExamList.lock();
	EXAM_LIST::iterator itExam = g_lExamList.begin();
	for (; itExam != g_lExamList.end();)
	{
		pEXAMINFO pExam = *itExam;
		itExam = g_lExamList.erase(itExam);
		SAFE_RELEASE(pExam);
	}
	g_lfmExamList.unlock();

	SAFE_RELEASE(_pModel_);
}

void CScanTool3Dlg::ReleaseDlg()
{
	if (m_pExamInfoMgrDlg)
	{
		m_pExamInfoMgrDlg->DestroyWindow();
		SAFE_RELEASE(m_pExamInfoMgrDlg);
	}
	if (m_pScanMgrDlg)
	{
		m_pScanMgrDlg->DestroyWindow();
		SAFE_RELEASE(m_pScanMgrDlg);
	}
}

void CScanTool3Dlg::InitUI()
{
	CRect rc;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	int sx = rc.Width();
	int sy = rc.Height();
	if (sx > MAX_DLG_WIDTH)
		sx = MAX_DLG_WIDTH;
	if (sy > MAX_DLG_HEIGHT)
		sy = MAX_DLG_HEIGHT;
	MoveWindow(0, 0, sx, sy);
	InitCtrlPositon();
}

void CScanTool3Dlg::SwitchDlg(int nDlg)
{
	if (nDlg == 0)
	{
		m_pExamInfoMgrDlg->ShowWindow(SW_SHOW);
		m_pScanMgrDlg->ShowWindow(SW_HIDE);
	}
	else if (nDlg == 1)
	{
		m_pExamInfoMgrDlg->ShowWindow(SW_HIDE);
		m_pScanMgrDlg->ShowWindow(SW_SHOW);
		m_pScanMgrDlg->DownLoadModel();
		m_pScanMgrDlg->SearchModel();		//加载模板
		m_pScanMgrDlg->UpdateInfo();
	}
}

BEGIN_MESSAGE_MAP(CScanTool3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CScanTool2Dlg 消息处理程序

BOOL CScanTool3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	USES_CONVERSION;
	CString strTitle = _T("");
	strTitle.Format(_T("%s %s"), SYS_BASE_NAME, SOFT_VERSION);
	SetWindowText(strTitle);

	InitThreads();
	m_pExamInfoMgrDlg = new CExamInfoMgrDlg(this);
	m_pExamInfoMgrDlg->Create(CExamInfoMgrDlg::IDD, this);
	m_pExamInfoMgrDlg->ShowWindow(SW_SHOW);

	m_pScanMgrDlg = new CScanMgrDlg(this);
	m_pScanMgrDlg->Create(CScanMgrDlg::IDD, this);
	m_pScanMgrDlg->ShowWindow(SW_HIDE);

	try
	{
		g_eGetExamList.wait(5000);
	}
	catch (Poco::TimeoutException &e)
	{
	}
	InitUI();
	m_pExamInfoMgrDlg->InitShowData();
//	m_pExamInfoMgrDlg->Invalidate();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CScanTool3Dlg::OnPaint()
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
HCURSOR CScanTool3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CScanTool3Dlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	g_nExitFlag = 1;
	ReleaseData();
	ReleaseDlg();
	ReleaseThreads();
}


void CScanTool3Dlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	InitCtrlPositon();
}

void CScanTool3Dlg::DumpReleaseTwain()
{
	m_pScanMgrDlg->m_scanThread.exit();
}


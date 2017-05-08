
// ScanTool2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ScanTool2Dlg.h"
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

Poco::Event			g_eTcpThreadExit;
Poco::Event			g_eSendFileThreadExit;
Poco::Event			g_eCompressThreadExit;

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
//--

CScanTool2Dlg::CScanTool2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScanTool2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScanTool2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CScanTool2Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CScanTool2Dlg 消息处理程序

BOOL CScanTool2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CScanTool2Dlg::OnPaint()
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
HCURSOR CScanTool2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


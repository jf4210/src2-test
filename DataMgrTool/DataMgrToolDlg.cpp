
// DataMgrToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"
#include "afxdialogex.h"
#include "RecogParamDlg.h"
#include "ShowPapersDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//------------------------------------
Poco::Event		_E_StartSearch_;
std::string		_strPkgSearchPath_;		//试卷袋搜索路径
std::string		_strJpgSearchPath_;		//试卷图像文件搜索路径

Poco::FastMutex	_fmSearchPathList_;
L_SearchTask	_SearchPathList_;

Poco::FastMutex	_fmAddPkg_;
L_PKGINFO		_PkgInfoList_;


std::string		_strComparePkgInfo_;
//------------------------------------

//答案容器
std::map<std::string, std::string> answerMap;

Poco::FastMutex			g_fmReSult;		//发送线程获取任务锁
RECOG_RESULT_LIST		g_lResultTask;	//发送任务列表

Poco::FastMutex		g_fmRecog;		//识别线程获取任务锁
RECOGTASKLIST		g_lRecogTask;	//识别任务列表


Poco::FastMutex			g_fmHttpSend;
LIST_SEND_HTTP			g_lHttpSend;			//发送HTTP任务列表

std::string		g_strUploadUri;		//识别结果提交的uri地址
int				g_nRecogMode = 1;		//识别模式，0-严格模式，1-简单模式
int				g_nRecogChkRotation = 0;	//识别图像时检测并调整图像方向
int				g_nRecogEasyModel = 0;	//识别结果宽容模式(3选2且非空)，第2种识别方法准确度不高
int				g_nRecogWithShowPkg = 0;	//在重新识别试卷袋后显示此试卷袋信息
int				g_nRecogWithContract = 0;	//重识别时使用自动对比度调节，即检查试卷的灰度与模板的平均灰度差，根据此差值调节Omr、Sn的对比度和亮度
int				g_nFastRecogModel = 0;		//进行快速识别模式，识别时减少写日志操作

CLog g_Log;
int	g_nExitFlag;
CString				g_strCurrentPath;
std::string		_strEncryptPwd_ = "yklxTest";
std::string		_strSessionName_;

Poco::FastMutex			g_fmDecompressLock;		//解压文件列表锁
DECOMPRESSTASKLIST		g_lDecompressTask;		//解压文件列表

Poco::FastMutex			g_fmCompressLock;		//压缩文件列表锁
COMPRESSTASKLIST		g_lCompressTask;		//解压文件列表

Poco::Event				g_eCompressThreadExit;

Poco::FastMutex		g_fmPapers;		//操作试卷袋列表的任务锁
PAPERS_LIST			g_lPapers;		//所有的试卷袋信息

pMODEL _pModel_ = NULL;

bool	_nUseNewParam_ = false;			//是否使用新的参数重新识别模板

int		_nAnticlutterKernel_ = 4;	//识别同步头时防干扰膨胀腐蚀的核因子
int		_nCannyKernel_ = 90;		//轮廓化核因子

int		g_nRecogGrayMin = 0;			//灰度点(除空白点,OMR外)计算灰度的最小考试范围
int		g_nRecogGrayMax_White = 255;	//空白点校验点计算灰度的最大考试范围
int		g_nRecogGrayMin_OMR = 0;		//OMR计算灰度的最小考试范围
int		g_RecogGrayMax_OMR = 235;		//OMR计算灰度的最大考试范围

double	_dOmrThresholdPercent_Fix_;		//定点模式OMR识别可认为是选中的标准百分比
double	_dSnThresholdPercent_Fix_;		//定点模式SN识别可认为是选中的标准百分比
double	_dQKThresholdPercent_Fix_;		//定点模式QK识别可认为是选中的标准百分比
double	_dOmrThresholdPercent_Head_;	//同步头模式OMR识别可认为是选中的标准百分比
double	_dSnThresholdPercent_Head_;		//同步头模式SN识别可认为是选中的标准百分比
double	_dQKThresholdPercent_Head_;		//同步头模式QK识别可认为是选中的标准百分比

double  _dAnswerSure_DensityFix_ = 1.5;	//密度算法确定为答案的比例
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


int		_nOMR_ = 230;		//重新识别模板时，用来识别OMR的密度值的阀值
int		_nSN_ = 200;		//重新识别模板时，用来识别ZKZH的密度值的阀值

//统计信息
Poco::FastMutex _fmErrorStatistics_;
int		_nErrorStatistics1_ = 0;	//第一种方法识别错误数
int		_nErrorStatistics2_ = 0;	//第二种方法识别错误数
int		_nOmrDoubtStatistics_ = 0;	//识别怀疑总数
int		_nOmrNullStatistics_ = 0;	//识别为空总数
int		_nSnNullStatistics_ = 0;	//SN识别为空总数
int		_nAllOmrStatistics_ = 0;		//OMR统计总数
int		_nAllSnStatistics_ = 0;			//SN统计总数

int		_nOmrDoubtSnCount_ = 0;		//存在omr怀疑的考生数
int		_nOmrNullSnCount_ = 0;		//存在omr为空的考生数
int		_nOmrSingleToMulti_ = 0;	//存在单选题识别为多选且无怀疑的考生数
int		_nOmrIssueSnCount_ = 0;		//属于Omr问题卷的考生数量

int		_nPkgDoubtStatistics_ = 0;		//原始试卷包识别怀疑总数
int		_nPkgOmrNullStatistics_ = 0;	//原始试卷包识别为空总数
int		_nPkgSnNullStatistics_ = 0;		//原始试卷包中SN识别为空总数
std::string _strDetailStatisTics_;		//详细的试卷袋识别统计信息


int		_nDecompress_ = 0;	//解压试卷袋数量
int		_nRecog_ = 0;		//识别试卷数量
int		_nRecogPapers_ = 0;	//识别试卷袋
int		_nCompress_ = 0;	//压缩试卷袋数量
Poco::FastMutex _fmDecompress_;	
Poco::FastMutex _fmRecog_;
Poco::FastMutex _fmRecogPapers_;
Poco::FastMutex _fmCompress_;

STUDENT_LIST		g_lBmkStudent;					//报名库学生列表

double	_dDoubtPer_ = 0.0;	//重新识别时，新识别的试卷袋的怀疑率超过此阀值时，将此试卷袋压缩到另一个目录

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


// CDataMgrToolDlg 对话框



CDataMgrToolDlg::CDataMgrToolDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CDataMgrToolDlg::IDD, pParent)
, /*m_pCompressObj(NULL),*/ m_pCompressThread(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDataMgrToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_RAR_Src, m_strRarPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_RAR_Src, m_mfcEdit_RAR);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_DecompressDir, m_strDecompressPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_DecompressDir, m_mfcEdit_Decompress);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_Encrypt_Src, m_strEncryptPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_Encrypt_Src, m_mfcEdit_Encrypt);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_RePkg, m_mfcEdit_RePkg);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_RePkg, m_strRePkgPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_RePkgSavePath, m_mfcEdit_RePkg_SavePath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_RePkgSavePath, m_strRePkg_SavePath);
	DDX_Text(pDX, IDC_EDIT_Msg, m_strMsg);
	DDX_Control(pDX, IDC_EDIT_Msg, m_edit_Msg);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_Pkg_DIR, m_strPkgPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_Pkg_DIR, m_mfcEdit_PkgDir);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_JpgPath_ShowPapers, m_mfcEdit_JpgPath);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_Pkg_PapersDIR, m_strWatchPaper_PapersDir);
	DDX_Text(pDX, IDC_EDIT_DecompressPaper, m_strWatchPaper_PaperInfo);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_JpgPath_ShowPapers, m_strJpgPath);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_RecogDir, m_strRecogPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_RecogDir, m_mfcEdit_RecogDir);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_ModelPath, m_strModelPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_ModelPath, m_mfcEdit_ModelPath);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_ModelPath_ShowPapers, m_strModelPath_showPapersDlg);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_ModelPath_ShowPapers, m_mfcEdit_ModelPath_showPapersDlg);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_RAR_Src_showPapers, m_strPkgPath_showPapersDlg);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_RAR_Src_showPapers, m_mfcEdit_PkgPath_showPapersDlg);

	DDX_Text(pDX, IDC_EDIT_Per, _dDoubtPer_);
}

BEGIN_MESSAGE_MAP(CDataMgrToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_MFCBUTTON_Decompress, &CDataMgrToolDlg::OnBnClickedMfcbuttonDecompress)
	ON_BN_CLICKED(IDC_MFCBUTTON_Decrypt, &CDataMgrToolDlg::OnBnClickedMfcbuttonDecrypt)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_Clear, &CDataMgrToolDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_RecogPKG, &CDataMgrToolDlg::OnBnClickedBtnRecogpkg)
	ON_BN_CLICKED(IDC_BTN_ReRecogPKG, &CDataMgrToolDlg::OnBnClickedBtnRerecogpkg)
	ON_MESSAGE(MSG_ERR_RECOG, &CDataMgrToolDlg::MsgRecogErr)
	ON_MESSAGE(MSG_RECOG_COMPLETE, &CDataMgrToolDlg::MsgRecogComplete)
	ON_MESSAGE(MSG_SENDRESULT_STATE, &CDataMgrToolDlg::MsgSendResultState)
	ON_BN_CLICKED(IDC_BTN_StudentAnswer, &CDataMgrToolDlg::OnBnClickedBtnStudentanswer)
	ON_BN_CLICKED(IDC_BTN_Statistics, &CDataMgrToolDlg::OnBnClickedBtnStatistics)
	ON_BN_CLICKED(IDC_BTN_StatisticsResult, &CDataMgrToolDlg::OnBnClickedBtnStatisticsresult)
	ON_BN_CLICKED(IDC_BTN_ClearStatistics, &CDataMgrToolDlg::OnBnClickedBtnClearstatistics)
	ON_BN_CLICKED(IDC_CHK_ReadParam, &CDataMgrToolDlg::OnBnClickedChkReadparam)
	ON_BN_CLICKED(IDC_BTN_LoadParam, &CDataMgrToolDlg::OnBnClickedBtnLoadparam)
	ON_BN_CLICKED(IDC_MFCBUTTON_RePkg, &CDataMgrToolDlg::OnBnClickedMfcbuttonRepkg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_STOP, &CDataMgrToolDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_DecompressLook, &CDataMgrToolDlg::OnBnClickedBtnDecompresslook)
	ON_BN_CLICKED(IDC_BTN_WatchPic, &CDataMgrToolDlg::OnBnClickedBtnWatchpic)
	ON_BN_CLICKED(IDC_BTN_WatchPapers, &CDataMgrToolDlg::OnBnClickedBtnWatchpapers)
	ON_BN_CLICKED(IDC_BTN_WatchPaper, &CDataMgrToolDlg::OnBnClickedBtnWatchpaper)
	ON_BN_CLICKED(IDC_CHK_AutoContract, &CDataMgrToolDlg::OnBnClickedChkAutocontract)
	ON_BN_CLICKED(IDC_BTN_ExportStatisticsDetail, &CDataMgrToolDlg::OnBnClickedBtnExportstatisticsdetail)
END_MESSAGE_MAP()


// CDataMgrToolDlg 消息处理程序

BOOL CDataMgrToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
// 		BOOL bNameValid;
// 		CString strAboutMenu;
// 		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
// 		ASSERT(bNameValid);
// 		if (!strAboutMenu.IsEmpty())
// 		{
// 			pSysMenu->AppendMenu(MF_SEPARATOR);
// 			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
// 		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	USES_CONVERSION;
	CString strTitle = _T(""); 
#ifdef PUBLIC_TOOL
	strTitle.Format(_T("%s %s \t限制版"), SYS_BASE_NAME, SOFT_VERSION);
#else
	strTitle.Format(_T("%s %s"), SYS_BASE_NAME, SOFT_VERSION);
#endif
	SetWindowText(strTitle);
	
	InitConfig();
	InitParam();
	
	_nUseNewParam_ = false;
	((CButton*)GetDlgItem(IDC_CHK_ReadParam))->SetCheck(0);
	GetDlgItem(IDC_BTN_LoadParam)->EnableWindow(FALSE);

	((CButton*)GetDlgItem(IDC_CHK_AutoContract))->SetCheck(g_nRecogWithContract);

	SetTimer(TIMER_UPDATE_STARTBAR, 1000, NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDataMgrToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDataMgrToolDlg::OnPaint()
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
HCURSOR CDataMgrToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDataMgrToolDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(TIMER_UPDATE_STARTBAR);

	g_nExitFlag = 1;

	//搜索pkg文件线程退出
	_fmSearchPathList_.lock();
	L_SearchTask::iterator itSearch = _SearchPathList_.begin();
	for (; itSearch != _SearchPathList_.end();)
	{
		pST_SEARCH pTask = *itSearch;
		itSearch = _SearchPathList_.erase(itSearch);
		SAFE_RELEASE(pTask);
	}
	_fmSearchPathList_.unlock();
	m_pSearchPkgThread->join();
	SAFE_RELEASE(m_pSearchPkgObj);
	SAFE_RELEASE(m_pSearchPkgThread);

	//发送识别结果线程退出
	g_fmHttpSend.lock();
	LIST_SEND_HTTP::iterator itHttp = g_lHttpSend.begin();
	for (; itHttp != g_lHttpSend.end();)
	{
		pSEND_HTTP_TASK pTask = *itHttp;
		itHttp = g_lHttpSend.erase(itHttp);
		SAFE_RELEASE(pTask);
	}
	g_fmHttpSend.unlock();
	m_pHttpThread->join();
	SAFE_RELEASE(m_pHttpObj);
	SAFE_RELEASE(m_pHttpThread);

	g_fmDecompressLock.lock();
	DECOMPRESSTASKLIST::iterator it = g_lDecompressTask.begin();
	for (; it != g_lDecompressTask.end();)
	{
		pDECOMPRESSTASK pTask = *it;
		it = g_lDecompressTask.erase(it);
		SAFE_RELEASE(pTask);
	}
	g_fmDecompressLock.unlock();
	//解压线程退出
	for (int i = 0; i < m_vecDecompressThreadObj.size(); i++)
	{
		m_vecDecompressThreadObj[i]->eExit.wait();
		m_pDecompressThread[i].join();
	}
	std::vector<CDecompressThread*>::iterator itDecObj = m_vecDecompressThreadObj.begin();
	for (; itDecObj != m_vecDecompressThreadObj.end();)
	{
		CDecompressThread* pObj = *itDecObj;
		if (pObj)
		{
			delete pObj;
			pObj = NULL;
		}
		itDecObj = m_vecDecompressThreadObj.erase(itDecObj);
	}
	delete[] m_pDecompressThread;

	//识别线程退出
	g_fmRecog.lock();			//释放未处理完的识别任务列表
	RECOGTASKLIST::iterator itRecog = g_lRecogTask.begin();
	for (; itRecog != g_lRecogTask.end();)
	{
		pRECOGTASK pRecogTask = *itRecog;
		itRecog = g_lRecogTask.erase(itRecog);
		SAFE_RELEASE(pRecogTask);
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
		itRecogObj = m_vecRecogThreadObj.erase(itRecogObj);
		SAFE_RELEASE(pObj);
	}
	if (m_pRecogThread)
	{
		delete[] m_pRecogThread;
		m_pRecogThread = NULL;
	}
	g_Log.LogOut("识别线程释放完毕.");

	if (_pModel_)
	{
		delete _pModel_;
		_pModel_ = NULL;
	}

	//压缩pkg文件线程退出
	g_fmCompressLock.lock();
	COMPRESSTASKLIST::iterator itCompress = g_lCompressTask.begin();
	for (; itCompress != g_lCompressTask.end();)
	{
		pCOMPRESSTASK pTask = *itCompress;
		itCompress = g_lCompressTask.erase(itCompress);
		SAFE_RELEASE(pTask);
	}
	g_fmCompressLock.unlock();

	for (int i = 0; i < m_vecCompressThreadObj.size(); i++)
	{
		m_vecCompressThreadObj[i]->eExit.wait();
		m_pCompressThread[i].join();
	}
	std::vector<CCompressThread*>::iterator itCompObj = m_vecCompressThreadObj.begin();
	for (; itCompObj != m_vecCompressThreadObj.end();)
	{
		CCompressThread* pObj = *itCompObj;
		if (pObj)
		{
			delete pObj;
			pObj = NULL;
		}
		itCompObj = m_vecCompressThreadObj.erase(itCompObj);
	}
	delete[] m_pCompressThread;
	g_Log.LogOut("压缩处理线程释放完毕.");

	g_fmPapers.lock();			//释放试卷袋列表
	PAPERS_LIST::iterator itPapers = g_lPapers.begin();
	for (; itPapers != g_lPapers.end();)
	{
		pPAPERSINFO pPapersTask = *itPapers;
		itPapers = g_lPapers.erase(itPapers);
		SAFE_RELEASE(pPapersTask);
	}
	g_fmPapers.unlock();
}


void CDataMgrToolDlg::InitParam()
{
	USES_CONVERSION;
	std::string strLog;
// 	std::string strFile = g_strCurrentPath + "param.dat";
// 	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strFile);

// 	std::string strFile = T2A(g_strCurrentPath);
// 	strFile.append("param.dat");
	CString strFile = g_strCurrentPath;
	strFile.Append(_T("param.dat"));
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strFile));
	try
	{
		Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));

		g_nRecogGrayMin = pConf->getInt("RecogGray.gray_Min", 0);
		g_nRecogGrayMax_White = pConf->getInt("RecogGray.white_Max", 255);
		g_nRecogGrayMin_OMR = pConf->getInt("RecogGray.omr_Min", 0);
		g_RecogGrayMax_OMR = pConf->getInt("RecogGray.omr_Max", 235);

		_dAnswerSure_DensityFix_ = pConf->getDouble("RecogOmrSn_Fix.fAnswerSure", 1.5);
		_dCompThread_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fCompTread", 1.2);
		_dDiffThread_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fDiffThread", 0.2);
		_dDiffExit_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fDiffExit", 0.3);
		_dCompThread_Head_ = pConf->getDouble("RecogOmrSn_Head.fCompTread", 1.2);
		_dDiffThread_Head_ = pConf->getDouble("RecogOmrSn_Head.fDiffThread", 0.085);
		_dDiffExit_Head_ = pConf->getDouble("RecogOmrSn_Head.fDiffExit", 0.15);

		_nThreshold_Recog2_ = pConf->getInt("RecogOmrSn_Fun2.nThreshold_Fun2", 240);

		_dCompThread_3_ = pConf->getDouble("RecogOmrSn_Fun3.fCompTread", 170);
		_dDiffThread_3_ = pConf->getDouble("RecogOmrSn_Fun3.fDiffThread", 20);
		_dDiffExit_3_ = pConf->getDouble("RecogOmrSn_Fun3.fDiffExit", 50);
		_dAnswerSure_ = pConf->getDouble("RecogOmrSn_Fun3.fAnswerSure", 100);


		_nOMR_ = pConf->getInt("MakeModel_Threshold.omr", 230);
		_nSN_ = pConf->getInt("MakeModel_Threshold.sn", 200);


		_dQKThresholdPercent_Fix_ = pConf->getDouble("MakeModel_RecogPercent_Fix.qk", 1.5);
		_dOmrThresholdPercent_Fix_ = pConf->getDouble("MakeModel_RecogPercent_Fix.omr", 1.5);
		_dSnThresholdPercent_Fix_ = pConf->getDouble("MakeModel_RecogPercent_Fix.sn", 1.5);

		_dQKThresholdPercent_Head_ = pConf->getDouble("MakeModel_RecogPercent_Head.qk", 1.5);
		_dOmrThresholdPercent_Head_ = pConf->getDouble("MakeModel_RecogPercent_Head.omr", 1.5);
		_dSnThresholdPercent_Head_ = pConf->getDouble("MakeModel_RecogPercent_Head.sn", 1.5);

		strLog = "读取识别灰度参数完成";
	}
	catch (Poco::Exception& exc)
	{
		strLog = "读取参数失败，使用默认参数 " + CMyCodeConvert::Utf8ToGb2312(exc.displayText());
		g_nRecogGrayMin = 0;
		g_nRecogGrayMax_White = 255;
		g_nRecogGrayMin_OMR = 0;
		g_RecogGrayMax_OMR = 235;
	}
	g_Log.LogOut(strLog);
}

void CDataMgrToolDlg::InitConfig()
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

	std::string strLogPath = CMyCodeConvert::Gb2312ToUtf8(T2A(g_strCurrentPath)) + "DMT.Log";
	g_Log.SetFileName(strLogPath);

#ifdef DATE_LIMIT
	Poco::LocalDateTime tNow;
	if (tNow.year() > 2017 || tNow.month() > 2)
		return;
#endif


	CString strConfigPath = g_strCurrentPath;
	strConfigPath.Append(_T("DMT_config.ini"));
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strConfigPath));
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
	int nRecogThreads = pConf->getInt("Recog.threads", 2);
	int nDecompressThreads = pConf->getInt("Decompress.threads", 1);
	int nCompressThreads = pConf->getInt("Compress.threads", 1);
	g_strUploadUri = pConf->getString("UpLoadResult.Uri");

	m_pSearchPkgThread = new Poco::Thread;
	m_pSearchPkgObj = new CSearchThread(this);
	m_pSearchPkgThread->start(*m_pSearchPkgObj);
	m_pSearchPkgThread->setPriority(Poco::Thread::PRIO_HIGH);

#ifdef Test_SendRecogResult
	m_pHttpThread = new Poco::Thread;
	m_pHttpObj = new CSendToHttpThread(this);
	m_pHttpThread->start(*m_pHttpObj);
	m_pHttpThread->setPriority(Poco::Thread::PRIO_HIGH);
#endif
	
#ifdef WIN_THREAD_TEST

#else
	m_pDecompressThread = new Poco::Thread[nDecompressThreads];
	for (int i = 0; i < nDecompressThreads; i++)
	{
		CDecompressThread* pObj = new CDecompressThread(this);
		m_pDecompressThread[i].start(*pObj);
		m_pDecompressThread[i].setPriority(Poco::Thread::PRIO_HIGH);
		m_vecDecompressThreadObj.push_back(pObj);
	}

	m_pCompressThread = new Poco::Thread[nCompressThreads];
	for (int i = 0; i < nCompressThreads; i++)
	{
		CCompressThread* pObj = new CCompressThread(this);
		m_pCompressThread[i].start(*pObj); 
		m_vecCompressThreadObj.push_back(pObj);
	}

	m_pRecogThread = new Poco::Thread[nRecogThreads];
	for (int i = 0; i < nRecogThreads; i++)
	{
		CRecognizeThread* pRecogObj = new CRecognizeThread;
		m_pRecogThread[i].start(*pRecogObj);
		m_pRecogThread[i].setPriority(Poco::Thread::PRIO_HIGH);
		m_vecRecogThreadObj.push_back(pRecogObj);
	}
#endif

	//statusBar
#if 0
	static UINT indicators[] =
	{
		ID_INDICATOR_CAPS,             //CAP lock indicator.
		ID_INDICATOR_NUM,              //NUM lock indicator.
		ID_INDICATOR_CAPS,             //CAP lock indicator.
		ID_INDICATOR_NUM,              //NUM lock indicator.
		ID_INDICATOR_CAPS,             //CAP lock indicator.
		ID_INDICATOR_NUM,              //NUM lock indicator.
		ID_INDICATOR_CAPS,             //CAP lock indicator.
		ID_INDICATOR_NUM,              //NUM lock indicator.
	};

	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create statusbarn");
		return;         // fail to create
	}
	UINT nID = 0;        //控制状态栏里面的分栏
	m_wndStatusBar.SetPaneInfo(0, nID, SBPS_NORMAL, 100);              //返回值存nID中
	m_wndStatusBar.SetPaneText(0, _T("解压试卷袋"));
	m_wndStatusBar.SetPaneInfo(1, nID, SBPS_NORMAL, 50);
	m_wndStatusBar.SetPaneText(1, _T(""));
	m_wndStatusBar.SetPaneInfo(2, nID, SBPS_NORMAL, 100);
	m_wndStatusBar.SetPaneText(2, _T("识别试卷"));
	m_wndStatusBar.SetPaneInfo(3, nID, SBPS_POPOUT, 50);
	m_wndStatusBar.SetPaneText(3, _T(""));
	m_wndStatusBar.SetPaneInfo(4, nID, SBPS_NORMAL, 100);
	m_wndStatusBar.SetPaneText(4, _T("识别试卷袋"));
	m_wndStatusBar.SetPaneInfo(5, nID, SBPS_POPOUT, 50);
	m_wndStatusBar.SetPaneText(5, _T(""));
	m_wndStatusBar.SetPaneInfo(6, nID, SBPS_POPOUT, 100);
	m_wndStatusBar.SetPaneText(6, _T("压缩试卷袋"));
	m_wndStatusBar.SetPaneInfo(7, nID, SBPS_POPOUT, 50);
	m_wndStatusBar.SetPaneText(7, _T(""));
	//               SetPaneInfo()函数的第三个参数的可选项如下：
	//               The following indicator styles are supported:
	//                           SBPS_NOBORDERS               No 3-D border around the pane.
	//                           SBPS_POPOUT                        Reverse border so that text "pops out."
	//                           SBPS_DISABLED                     Do not draw text.
	//                           SBPS_STRETCH                      Stretch pane to fill unused space. Only one pane per status bar can have thisstyle.
	//                           SBPS_NORMAL                        No stretch, borders, or pop-out.
	//----------------让这个状态栏最终显示在对话框中-------------
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
#else
	m_statusBar.Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0, 0, 0, 0), this, 0);
	int strPartDim[8] = { 80, 170, 240, 340, 420, 500, 580, -1 }; //分割数量
	m_statusBar.SetParts(8, strPartDim);

	//设置状态栏文本
	m_statusBar.SetText(_T("解压试卷袋:"), 0, 0);
	m_statusBar.SetText(_T("识别试卷:"), 2, 0);
	m_statusBar.SetText(_T("识别试卷袋:"), 4, 0);
	m_statusBar.SetText(_T("压缩试卷袋:"), 6, 0);
	//下面是在状态栏中加入图标
//	m_statusBar.SetIcon(1, SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE));//为第二个分栏中加的图标
#endif
}

unsigned __stdcall CDataMgrToolDlg::ReleaseDirThread(void* pArguments)
{
	pPAPERSINFO   pPapers = (pPAPERSINFO)pArguments;

	//删除源文件夹
	try
	{
		Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
		if (srcFileDir.exists())
			srcFileDir.remove(true);
	}
	catch (Poco::Exception& exc)
	{
		std::string strErr = "删除文件夹(" + pPapers->strPapersPath + ")失败: " + exc.message();
		g_Log.LogOutError(strErr);
	}
	return 0;
}

void CDataMgrToolDlg::OnBnClickedMfcbuttonDecompress()
{
	UpdateData(TRUE);
	USES_CONVERSION;

	std::string strFilePath = T2A(m_strRarPath);
	try
	{
		Poco::Path filePath(CMyCodeConvert::Gb2312ToUtf8(strFilePath));

		pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
		pDecompressTask->strFilePath = strFilePath;
		pDecompressTask->strFileBaseName = CMyCodeConvert::Utf8ToGb2312(filePath.getBaseName());
		pDecompressTask->strSrcFileName = CMyCodeConvert::Utf8ToGb2312(filePath.getFileName());
		pDecompressTask->strDecompressDir = T2A(m_strDecompressPath);

		g_fmDecompressLock.lock();
		g_lDecompressTask.push_back(pDecompressTask);
		g_fmDecompressLock.unlock();
	}
	catch (Poco::Exception& exc)
	{
	}	
}


void CDataMgrToolDlg::OnBnClickedMfcbuttonDecrypt()
{
#ifdef PUBLIC_TOOL
	return;
#endif
	UpdateData(TRUE);
	USES_CONVERSION;

	std::string strFilePath = T2A(m_strEncryptPath);
	std::string strJsnData;
	std::ifstream in(strFilePath);
	if (!in)
	{
		CString strMsg = _T("");
		strMsg.Format(_T("打开文件(%s)失败\r\n"), A2T(strFilePath.c_str()));
		showMsg(strMsg);
		return;
	}
	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);
		strJsnData.append(strJsnLine);
	}
	in.close();

	std::string strFileData;
	if (!decString(strJsnData, strFileData))
		strFileData = strJsnData;

	int n = strFileData.length();
	CString strMsg = _T("");
	strMsg.Format(_T("-------------------\r\n%s\r\n-------------------\r\n"), A2T(strFileData.c_str()));
	showMsg(strMsg);
}

void CDataMgrToolDlg::OnBnClickedMfcbuttonRepkg()
{
#ifdef PUBLIC_TOOL
	return;
#endif
	UpdateData(TRUE);
	USES_CONVERSION;

	CString _strPkgName = m_strRePkgPath.Right(m_strRePkgPath.GetLength() - m_strRePkgPath.ReverseFind('\\') - 1);
	string strPkgName = T2A(_strPkgName);
	string strSavePath = T2A(m_strRePkg_SavePath);

	std::string strNewPapersSavePath = strSavePath + "\\" + strPkgName;
	pCOMPRESSTASK pTask = new COMPRESSTASK;
	pTask->strCompressFileName = strPkgName;
	pTask->strExtName = ".pkg";
	pTask->strSavePath = strNewPapersSavePath;
	pTask->strSrcFilePath = T2A(m_strRePkgPath);
	g_fmCompressLock.lock();
	g_lCompressTask.push_back(pTask);
	g_fmCompressLock.unlock();
}

void CDataMgrToolDlg::showMsg(CString& strMsg)
{
	if (m_strMsg.GetLength() > 10000)
		m_strMsg.Empty();

	m_strMsg.Append(strMsg);
	m_edit_Msg.SetWindowTextW(m_strMsg);


	int nLineCount = m_edit_Msg.GetLineCount();
	m_edit_Msg.LineScroll(nLineCount);
}

void CDataMgrToolDlg::showPapers(pPAPERSINFO pPapers)
{
	CShowPapersDlg dlg;
	dlg.setShowPapers(pPapers);
	dlg.DoModal();
}

void CDataMgrToolDlg::OnBnClickedBtnClear()
{
	UpdateData(TRUE);
	m_strMsg.Empty();
	UpdateData(FALSE);
}

void CDataMgrToolDlg::OnBnClickedBtnRecogpkg()
{
#ifdef PUBLIC_TOOL
	return;
#endif
	UpdateData(TRUE);
	USES_CONVERSION;

	std::string strModelPath = T2A(m_strModelPath);

	int nPos1 = strModelPath.rfind('\\');
	int nPos2 = strModelPath.rfind('.');

	std::string strBaseName = strModelPath.substr(nPos1 + 1, nPos2 - nPos1 - 1);
	std::string strSrcName = strModelPath.substr(nPos1 + 1, strModelPath.length() - nPos1 - 1);

	std::string strBasePath = strModelPath.substr(0, nPos1);

	pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
	pDecompressTask->nTaskType = 4;
	pDecompressTask->nExcuteTask = 3;	//加载模板后，不用搜索Pkg目录，后面手动加载
	pDecompressTask->strFilePath = T2A(m_strModelPath);
	pDecompressTask->strFileBaseName = strBaseName;
	pDecompressTask->strSrcFileName = strSrcName;
	pDecompressTask->strDecompressDir = strBasePath;

	_fmDecompress_.lock();
	_nDecompress_++;
	_fmDecompress_.unlock();

	g_fmDecompressLock.lock();
	g_lDecompressTask.push_back(pDecompressTask);
	g_fmDecompressLock.unlock();

	//*******************	识别试卷袋文件夹的试卷
	_strJpgSearchPath_ = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strRecogPath));
}

void CDataMgrToolDlg::OnBnClickedBtnRerecogpkg()
{
#ifdef PUBLIC_TOOL
	return;
#endif
	UpdateData(TRUE);
	USES_CONVERSION;

	CRecogParamDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;

	CString strDecompressDir = m_strPkgPath + "\\tmpDecompress";
	try
	{
		CString strNewPkgSavePath = m_strPkgPath + "\\newPkg";
		CString strNewPkg_HighErrSavePath = m_strPkgPath + "\\newPkg_HighError";
		//先将目录删除


		//

		std::string strModelPath = T2A(m_strModelPath);

		int nPos1 = strModelPath.rfind('\\');
		int nPos2 = strModelPath.rfind('.');

		std::string strBaseName = strModelPath.substr(nPos1 + 1, nPos2 - nPos1 - 1);
		std::string strSrcName = strModelPath.substr(nPos1 + 1, strModelPath.length() - nPos1 - 1);

		std::string strBasePath = strModelPath.substr(0, nPos1);

		pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
		pDecompressTask->nTaskType = 4;
		pDecompressTask->strFilePath = T2A(m_strModelPath);
		pDecompressTask->strFileBaseName = strBaseName;
		pDecompressTask->strSrcFileName = strSrcName;
		pDecompressTask->strDecompressDir = strBasePath;
		pDecompressTask->bRecogOmr = dlg.m_nChkOmr;				//进行参数传递
		pDecompressTask->bRecogZkzh = dlg.m_nChkSN;				//进行参数传递
		pDecompressTask->bRecogElectOmr = dlg.m_nChkElectOmr;	//进行参数传递
		pDecompressTask->nSendEzs = dlg.m_nHandleResult;		//参数传递
		pDecompressTask->nNoNeedRecogVal = dlg.m_nNoRecogVal;	//....
		g_nRecogMode = dlg.m_nRecogMode;
		g_nRecogChkRotation = dlg.m_nRecogChkRotation;
		g_nRecogEasyModel = dlg.m_nRecogEasyModel;
		g_nRecogWithContract = dlg.m_nAutoContract;
		g_nFastRecogModel = dlg.m_nFastRecogModel;

		_fmDecompress_.lock();
		_nDecompress_++;
		_fmDecompress_.unlock();

		g_fmDecompressLock.lock();
		g_lDecompressTask.push_back(pDecompressTask);
		g_fmDecompressLock.unlock();



		std::string strPkgPath = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strPkgPath));
		_strPkgSearchPath_ = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strPkgPath));

		m_strWatchPaper_PapersDir = m_strPkgPath;
		UpdateData(FALSE);
	}
	catch (Poco::Exception& exc)
	{
	}
}

LRESULT CDataMgrToolDlg::MsgRecogErr(WPARAM wParam, LPARAM lParam)
{
	pST_PaperInfo pPaper = (pST_PaperInfo)wParam;
	pPAPERSINFO   pPapers = (pPAPERSINFO)lParam;

	USES_CONVERSION;
	CString strMsg;
	strMsg.Format(_T("%s:%s识别出异常点\r\n"), A2T(pPapers->strPapersName.c_str()), A2T(pPaper->strStudentInfo.c_str()));
	showMsg(strMsg);
	return TRUE;
}

LRESULT CDataMgrToolDlg::MsgRecogComplete(WPARAM wParam, LPARAM lParam)
{
	pST_PaperInfo pPaper = (pST_PaperInfo)wParam;
	pPAPERSINFO   pPapers = (pPAPERSINFO)lParam;


	USES_CONVERSION;
	CString strMsg;
	std::stringstream ss;

	if (answerMap.size() > 0)
	{
		int nOmrCount = 0;
		PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
		for (; itPaper != pPapers->lPaper.end(); itPaper++)
		{
			pST_PaperInfo pPaper = *itPaper;
			nOmrCount += pPaper->lOmrResult.size();
		}
		PAPER_LIST::iterator itPaper2 = pPapers->lIssue.begin();
		for (; itPaper2 != pPapers->lIssue.end(); itPaper2++)
		{
			pST_PaperInfo pPaper = *itPaper2;
			nOmrCount += pPaper->lOmrResult.size();
		}
		ss.str("");
		char szStatisticsInfo[300] = { 0 };
		sprintf_s(szStatisticsInfo, "\n识别错误信息统计: omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\n", (float)pPapers->nOmrError_1 / nOmrCount * 100, pPapers->nOmrError_1, nOmrCount, \
				  (float)pPapers->nOmrError_2 / nOmrCount * 100, pPapers->nOmrError_2, nOmrCount);

		ss << "\r\n\t----------\r\n\t" << pPapers->strPapersName << "结果正确率统计完成:\r\n\t" << szStatisticsInfo << "\r\n\t----------\r\n";
	}
	

	if (pPapers->lIssue.size() == 0)
	{
		int nModelOmrCount = 0;
		for (int k = 0; k < _pModel_->vecPaperModel.size(); k++)
			nModelOmrCount += _pModel_->vecPaperModel[k]->lOMR2.size();

		int nPapersCount = pPapers->lPaper.size() + pPapers->lIssue.size();
		int nOmrCount = nModelOmrCount * nPapersCount;

		char szStatisticsInfo[300] = { 0 };
		sprintf_s(szStatisticsInfo, "新识别信息: omrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), zkzhNull = %.2f%%(%d/%d)\r\n存在omr怀疑、omr为空、单选识别成多选的试卷(%d + %d + %d = %d)份, 总的Omr问题卷(%d)份\r\n", (float)pPapers->nOmrDoubt / nOmrCount * 100, pPapers->nOmrDoubt, nOmrCount, \
				  (float)pPapers->nOmrNull / nOmrCount * 100, pPapers->nOmrNull, nOmrCount, \
				  (float)pPapers->nSnNull / nPapersCount * 100, pPapers->nSnNull, nPapersCount, \
				  pPapers->nOmrDoubtSnCounts, pPapers->nOmrNullSnCounts, pPapers->nSingleToMultiSnCounts, \
				  pPapers->nOmrDoubtSnCounts + pPapers->nOmrNullSnCounts + pPapers->nSingleToMultiSnCounts, pPapers->nOmrIssueSnCounts);

		char szPkgStatisticsInfo[300] = { 0 };
		sprintf_s(szPkgStatisticsInfo, "原始包信息: omrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), zkzhNull = %.2f%%(%d/%d)\n", (float)pPapers->nPkgOmrDoubt / nOmrCount * 100, pPapers->nPkgOmrDoubt, nOmrCount, \
				  (float)pPapers->nPkgOmrNull / nOmrCount * 100, pPapers->nPkgOmrNull, nOmrCount, \
				  (float)pPapers->nPkgSnNull / nPapersCount * 100, pPapers->nPkgSnNull, nPapersCount);

		strMsg.Format(_T("\r\n==================\r\n%s识别完成\r\n%s\r\n%s\r\n%s\r\n\r\n"), A2T(pPapers->strPapersName.c_str()), A2T(szStatisticsInfo), A2T(ss.str().c_str()), A2T(szPkgStatisticsInfo));

		ss.str("");
		ss << pPapers->strPapersName << "\tomrDoubt = " << (float)pPapers->nOmrDoubt / nOmrCount * 100 << "%%(" << pPapers->nOmrDoubt << "/" << nOmrCount \
			<< "), omrNull = " << (float)pPapers->nOmrNull / nOmrCount * 100 << "%%(" << pPapers->nOmrNull << "/" << nOmrCount << "), zkzhNull = " << (float)pPapers->nSnNull / nPapersCount * 100 \
			<< "%%(" << pPapers->nSnNull << "/" << nPapersCount << "), 存在omr怀疑、omr为空、单选识别成多选的试卷(" << pPapers->nOmrDoubtSnCounts << "+" << pPapers->nOmrNullSnCounts << "+" \
			<< pPapers->nSingleToMultiSnCounts << "=" << pPapers->nOmrDoubtSnCounts + pPapers->nOmrNullSnCounts + pPapers->nSingleToMultiSnCounts << ")份, 总的Omr问题卷(" \
			<< pPapers->nOmrIssueSnCounts << ")份\n";
		_strDetailStatisTics_.append(ss.str());
	}
	else
		strMsg.Format(_T("\r\n****************\r\n%s识别出问题试卷, 问题卷数量=%d\r\n"), A2T(pPapers->strPapersName.c_str()), pPapers->lIssue.size());
	showMsg(strMsg);

	if (pPapers->nSendEzs == 3)
	{
		showPapers(pPapers);


		unsigned threadID;
		_beginthreadex(NULL, 0, ReleaseDirThread, pPapers, 0, &threadID);

		//删除源文件夹
// 		try
// 		{
// 			Poco::File srcFileDir(CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersPath));
// 			if (srcFileDir.exists())
// 				srcFileDir.remove(true);
// 		}
// 		catch (Poco::Exception& exc)
// 		{
// 			std::string strErr = "删除文件夹(" + pPapers->strPapersPath + ")失败: " + exc.message();
// 			g_Log.LogOutError(strErr);
// 		}

// 		g_fmPapers.lock();			//释放试卷袋列表
// 		PAPERS_LIST::iterator itPapers = g_lPapers.begin();
// 		for (; itPapers != g_lPapers.end(); itPapers++)
// 		{
// 			pPAPERSINFO pPapersTask = *itPapers;
// 			if (pPapersTask == pPapers)
// 			{
// 				itPapers = g_lPapers.erase(itPapers);
// 				SAFE_RELEASE(pPapersTask);
// 				break;
// 			}
// 		}
// 		g_fmPapers.unlock();
	}

	return TRUE;
}


LRESULT CDataMgrToolDlg::MsgSendResultState(WPARAM wParam, LPARAM lParam)
{
//	pST_PaperInfo pPaper = (pST_PaperInfo)wParam;
	int nState = (int)wParam;
	pPAPERSINFO   pPapers = (pPAPERSINFO)lParam;

	USES_CONVERSION;
	CString strMsg;
	if (nState == 0)
		strMsg.Format(_T("(%s)发送识别结果给后端服务器失败\r\n"), A2T(pPapers->strPapersName.c_str()));
	else
		strMsg.Format(_T("(%s)发送识别结果给后端服务器成功\r\n"), A2T(pPapers->strPapersName.c_str()));
	showMsg(strMsg);
	return TRUE;
}


void CDataMgrToolDlg::OnBnClickedBtnStudentanswer()
{
	CString strFile = _T("");
	CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"), NULL);

	if (dlgFile.DoModal())
	{
		strFile = dlgFile.GetPathName();
	}

	std::ifstream fin(strFile/*szPath*/, std::ios::in);
	if (!fin)
	{
		TRACE("Open file %s failed.\n", strFile);
		return;
	}

	if (answerMap.size())
		answerMap.clear();

	int nCount = 0;
	std::string strLine;
	std::stringstream ss;
	while (getline(fin, strLine))
	{
		ss.clear();
		ss.str(strLine);


		std::string strPkgName;
		std::string strStudentName;
		std::string strItem;
		std::string strAnswer;
		ss >> strPkgName >> strStudentName >> strItem >> strAnswer;
		answerMap.insert(std::pair<std::string, std::string>(strPkgName + ":" + strStudentName + ":" + strItem, strAnswer));
		nCount++;
	}
	CString strInfo = _T("\r\n加载学生答案完成\r\n");
	showMsg(strInfo);
}


void CDataMgrToolDlg::OnBnClickedBtnStatistics()
{
	UpdateData(TRUE);
	USES_CONVERSION;

	CString strDecompressDir = m_strPkgPath + "\\tmpDecompress";
	try
	{
		std::string strPkgPath = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strPkgPath));
		Poco::DirectoryIterator it(strPkgPath);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			if (it->isFile() && p.getExtension() == "pkg")
			{
				std::string strFileName = p.getFileName();
				std::string strExtion = p.getExtension();

				pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
				pDecompressTask->nTaskType = 5;
				pDecompressTask->strFilePath = CMyCodeConvert::Utf8ToGb2312(p.toString());
				pDecompressTask->strFileBaseName = CMyCodeConvert::Utf8ToGb2312(p.getBaseName());
				pDecompressTask->strSrcFileName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());
				pDecompressTask->strDecompressDir = T2A(strDecompressDir);

				g_fmDecompressLock.lock();
				g_lDecompressTask.push_back(pDecompressTask);
				g_fmDecompressLock.unlock();
			}
			it++;
		}
	}
	catch (Poco::Exception& exc)
	{
	}
}


void CDataMgrToolDlg::OnBnClickedBtnStatisticsresult()
{
	USES_CONVERSION;
	CString strMsg;

	char szStatisticsInfo[500] = { 0 };
	sprintf_s(szStatisticsInfo, "\n所有试卷袋识别错误信息统计:\r\nomrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), snNull = %.2f%%(%d/%d), omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\r\n存在omr怀疑、omr为空、单选识别成多选的试卷(%d + %d + %d = %d)份,总的Omr问题卷(%d)份\r\n原始试卷袋统计: omrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), snNull = %.2f%%(%d/%d)\n", (float)_nOmrDoubtStatistics_ / _nAllOmrStatistics_ * 100, _nOmrDoubtStatistics_, _nAllOmrStatistics_, \
			  (float)_nOmrNullStatistics_ / _nAllOmrStatistics_ * 100, _nOmrNullStatistics_, _nAllOmrStatistics_, \
			  (float)_nSnNullStatistics_ / _nAllSnStatistics_ * 100, _nSnNullStatistics_, _nAllSnStatistics_, \
			  (float)_nErrorStatistics1_ / _nAllOmrStatistics_ * 100, _nErrorStatistics1_, _nAllOmrStatistics_, \
			  (float)_nErrorStatistics2_ / _nAllOmrStatistics_ * 100, _nErrorStatistics2_, _nAllOmrStatistics_, \
			  _nOmrDoubtSnCount_, _nOmrNullSnCount_, _nOmrSingleToMulti_, _nOmrDoubtSnCount_ + _nOmrNullSnCount_ + _nOmrSingleToMulti_, _nOmrIssueSnCount_, \
			  (float)_nPkgDoubtStatistics_ / _nAllOmrStatistics_ * 100, _nPkgDoubtStatistics_, _nAllOmrStatistics_, \
			  (float)_nPkgOmrNullStatistics_ / _nAllOmrStatistics_ * 100, _nPkgOmrNullStatistics_, _nAllOmrStatistics_, \
			  (float)_nPkgSnNullStatistics_ / _nAllSnStatistics_ * 100, _nPkgSnNullStatistics_, _nAllSnStatistics_);

	_strDetailStatisTics_.append("=====================================");
	_strDetailStatisTics_.append(szStatisticsInfo);
	_strDetailStatisTics_.append("=====================================");

	strMsg.Format(_T("\r\n***********************\r\n%s\r\n***********************\r\n"), A2T(szStatisticsInfo));
	showMsg(strMsg);
	g_Log.LogOut(T2A(strMsg));
}


void CDataMgrToolDlg::OnBnClickedBtnClearstatistics()
{
	_fmErrorStatistics_.lock();
	_nErrorStatistics1_ = 0;
	_nErrorStatistics2_ = 0;
	_nOmrDoubtStatistics_ = 0;
	_nOmrNullStatistics_ = 0;
	_nAllOmrStatistics_ = 0;
	_nSnNullStatistics_ = 0;
	_nAllSnStatistics_ = 0;

	_nOmrDoubtSnCount_ = 0;
	_nOmrNullStatistics_ = 0;
	_nOmrSingleToMulti_ = 0;
	_nOmrIssueSnCount_ = 0;

	_nPkgDoubtStatistics_ = 0;
	_nPkgOmrNullStatistics_ = 0;
	_nPkgSnNullStatistics_ = 0;
	_fmErrorStatistics_.unlock();

	_strDetailStatisTics_ = "";

	_fmDecompress_.lock();
	_nDecompress_ = 0;
	_fmDecompress_.unlock();
	_fmRecog_.lock();
	_nRecog_ = 0;
	_fmRecog_.unlock();
	_fmRecogPapers_.lock();
	_nRecogPapers_ = 0;
	_fmRecogPapers_.unlock();
	_fmCompress_.lock();
	_nCompress_ = 0;
	_fmCompress_.unlock();

	CString strMsg;
	strMsg.Format(_T("\r\n统计结果已清零\r\n"));
	showMsg(strMsg);
}


void CDataMgrToolDlg::OnBnClickedChkReadparam()
{
	if (((CButton*)GetDlgItem(IDC_CHK_ReadParam))->GetCheck())
	{
		_nUseNewParam_ = true;
//		((CButton*)GetDlgItem(IDC_CHK_ReadParam))->SetCheck(0);
		GetDlgItem(IDC_BTN_LoadParam)->EnableWindow(TRUE);
	}
	else
	{
		_nUseNewParam_ = false;
//		((CButton*)GetDlgItem(IDC_CHK_ReadParam))->SetCheck(1);
		GetDlgItem(IDC_BTN_LoadParam)->EnableWindow(FALSE);
	}
}


void CDataMgrToolDlg::OnBnClickedBtnLoadparam()
{
	InitParam();
	CString strMsg = _T("加载参数完成\r\n");
	showMsg(strMsg);
}



void CDataMgrToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_UPDATE_STARTBAR)
	{
		CString strTmp;
		if (m_wndStatusBar.GetSafeHwnd())
		{
			int nUnDecompressCount	= g_lDecompressTask.size();
			int nUnRecogCount		= g_lRecogTask.size();
			int nUnCompressCount	= g_lCompressTask.size();
			strTmp.Format(_T("%d / %d"), _nDecompress_, nUnDecompressCount);
			m_wndStatusBar.SetPaneText(1, strTmp);
			strTmp.Format(_T("%d / %d"), _nRecog_, nUnRecogCount);
			m_wndStatusBar.SetPaneText(3, strTmp);
			strTmp.Format(_T("%d"), _nRecogPapers_);
			m_wndStatusBar.SetPaneText(5, strTmp);
			strTmp.Format(_T("%d / %d"), _nCompress_, nUnCompressCount);
			m_wndStatusBar.SetPaneText(7, strTmp);
		}
		else
		{
			int nUnDecompressCount = g_lDecompressTask.size();
			int nUnRecogCount = g_lRecogTask.size();
			int nUnCompressCount = g_lCompressTask.size();
			strTmp.Format(_T("%d / %d"), _nDecompress_, nUnDecompressCount);
			m_statusBar.SetText(strTmp, 1, 0);
			strTmp.Format(_T("%d / %d"), _nRecog_, nUnRecogCount);
			m_statusBar.SetText(strTmp, 3, 0);
			strTmp.Format(_T("%d"), _nRecogPapers_);
			m_statusBar.SetText(strTmp, 5, 0);
			strTmp.Format(_T("%d / %d"), _nCompress_, nUnCompressCount);
			m_statusBar.SetText(strTmp, 7, 0);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CDataMgrToolDlg::OnBnClickedBtnStop()
{
	pST_SEARCH pSearchPathTask = NULL;
	_fmSearchPathList_.lock();
	L_SearchTask::iterator itSearchPathItem = _SearchPathList_.begin();
	for (; itSearchPathItem != _SearchPathList_.end();)
	{
		pSearchPathTask = *itSearchPathItem;
		itSearchPathItem = _SearchPathList_.erase(itSearchPathItem);
		SAFE_RELEASE(pSearchPathTask);
	}
	_fmSearchPathList_.unlock();

	pDECOMPRESSTASK pDecompressTask = NULL;
	g_fmDecompressLock.lock();
	DECOMPRESSTASKLIST::iterator itDecompressItem = g_lDecompressTask.begin();
	for (; itDecompressItem != g_lDecompressTask.end();)
	{
		pDecompressTask = *itDecompressItem;
		itDecompressItem = g_lDecompressTask.erase(itDecompressItem);
		SAFE_RELEASE(pDecompressTask);
	}
	g_fmDecompressLock.unlock();

	g_fmRecog.lock();
	pRECOGTASK pRecogTask = NULL;
	RECOGTASKLIST::iterator itRecogItem = g_lRecogTask.begin();
	for (; itRecogItem != g_lRecogTask.end();)
	{
		pRecogTask = *itRecogItem;
		itRecogItem = g_lRecogTask.erase(itRecogItem);
		SAFE_RELEASE(pRecogTask);
	}
	g_fmRecog.unlock();

	pCOMPRESSTASK pCompressTask = NULL;
	g_fmCompressLock.lock();
	COMPRESSTASKLIST::iterator itCompressItem = g_lCompressTask.begin();
	for (; itCompressItem != g_lCompressTask.end();)
	{
		pCompressTask = *itCompressItem;
		itCompressItem = g_lCompressTask.erase(itCompressItem);
		SAFE_RELEASE(pCompressTask);
	}
	g_fmCompressLock.unlock();
}


void CDataMgrToolDlg::OnBnClickedBtnDecompresslook()
{
	UpdateData(TRUE);
	USES_CONVERSION;

	std::string strPkgPath = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strWatchPaper_PapersDir));
	std::string strPaperInfo = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strWatchPaper_PaperInfo));

	int nPos = strPaperInfo.find(":");
	if (nPos == std::string::npos)
	{
		AfxMessageBox(_T("试卷信息格式不正确，格式为：试卷包名称:考生名称，如demojs001_100-200_20170421140955_13:S2"));
		return;
	}

	//++创建文件解压路径
	CString strTmp = m_strWatchPaper_PapersDir + _T("\\查看试卷-解压路径");
	std::string strDecompressPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strTmp));
	try
	{
		Poco::File dir(strDecompressPath);
		dir.createDirectories();
	}
	catch (Poco::Exception& e)
	{
	}
	//--

	std::string strPkgName;
	std::string strPaperName; 
	strPkgName = strPaperInfo.substr(0, nPos);
	strPaperName = strPaperInfo.substr(nPos + 1, strPaperInfo.length());

	pST_SEARCH pDirTask = new ST_SEARCH;
	pDirTask->nSearchType = 2;
	pDirTask->strSearchPath = strPkgPath;
	pDirTask->strSearchName = strPkgName;
	pDirTask->strPaperName = strPaperName;
	pDirTask->strDecompressPath = CMyCodeConvert::Utf8ToGb2312(strDecompressPath);

	_fmSearchPathList_.lock();
	_SearchPathList_.push_back(pDirTask);
	_fmSearchPathList_.unlock();
}


void CDataMgrToolDlg::OnBnClickedBtnWatchpic()
{
	UpdateData(TRUE);
	USES_CONVERSION;

	std::string strPkgPath = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strWatchPaper_PapersDir));
	std::string strPaperInfo = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strWatchPaper_PaperInfo));

	std::string strStudentInfo = strPaperInfo;
	Poco::MD5Engine md5;
	Poco::DigestOutputStream outstr(md5);
	outstr << strStudentInfo;
	outstr.flush();
	const Poco::DigestEngine::Digest& digest = md5.digest();
	std::string strMd5 = Poco::DigestEngine::digestToHex(digest);
	CString strMsg = _T("");
	strMsg.Format(_T("MD5(%s)=\r\n%s\r\n"), A2T(strPaperInfo.c_str()), A2T(strMd5.c_str()));
	showMsg(strMsg);

#if 0
	int nPos = strPaperInfo.find(":");
	if (nPos == std::string::npos)
	{
		AfxMessageBox(_T("试卷信息格式不正确，格式为：试卷包名称:考生名称，如demojs001_100-200_20170421140955_13:S2"));
		return;
	}

	CString strTmp = m_strWatchPaper_PapersDir + _T("\\查看试卷-解压路径\\");
	std::string strDecompressPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strTmp));

	std::string strPkgName;
	std::string strPaperName;
	strPkgName = strPaperInfo.substr(0, nPos);
	strPaperName = strPaperInfo.substr(nPos + 1, strPaperInfo.length());
	std::string strPicPath = strDecompressPath + strPkgName + "\\" + strPaperName + "_1.jpg";
	try
	{
		Poco::File picFile(strPicPath);
		if (!picFile.exists())
		{
			CString str = _T("");
			str.Format(_T("试卷袋图片不存在%s"), A2T(CMyCodeConvert::Utf8ToGb2312(strPicPath).c_str()));
			AfxMessageBox(str);
			return;
		}
	}
	catch (Poco::Exception& e)
	{
	}
	std::string strPicPath_gb = CMyCodeConvert::Utf8ToGb2312(strPicPath);
	cv::Mat matPic = cv::imread(strPicPath_gb);
	cv::namedWindow("试卷袋图像", 0);
	cv::imshow("试卷袋图像", matPic);
#endif
}


void CDataMgrToolDlg::OnBnClickedBtnWatchpapers()
{
#ifdef PUBLIC_TOOL
	return;
#endif
	UpdateData(TRUE);
	USES_CONVERSION;
	std::string strPkgPath = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strPkgPath_showPapersDlg));
//	std::string strModelPath = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strModelPath_showPapersDlg));

	if (MessageBox(_T("模板文件属于全局变量，确保不要在识别过程中来操作这里!!!"), _T(""), MB_OKCANCEL) != IDOK)
		return;
	
	std::string strModelPath = T2A(m_strModelPath_showPapersDlg);

	int nPos1 = strModelPath.rfind('\\');
	int nPos2 = strModelPath.rfind('.');

	std::string strBaseName = strModelPath.substr(nPos1 + 1, nPos2 - nPos1 - 1);
	std::string strSrcName = strModelPath.substr(nPos1 + 1, strModelPath.length() - nPos1 - 1);

	std::string strBasePath = strModelPath.substr(0, nPos1);

	pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
	pDecompressTask->nTaskType = 4;
	pDecompressTask->nExcuteTask = 0;	//加载模板后，不用搜索Pkg目录，后面手动加载
	pDecompressTask->strFilePath = T2A(m_strModelPath_showPapersDlg);
	pDecompressTask->strFileBaseName = strBaseName;
	pDecompressTask->strSrcFileName = strSrcName;
	pDecompressTask->strDecompressDir = strBasePath;

	_fmDecompress_.lock();
	_nDecompress_++;
	_fmDecompress_.unlock();

	g_fmDecompressLock.lock();
	g_lDecompressTask.push_back(pDecompressTask);
	g_fmDecompressLock.unlock();
	
	//++创建文件解压路径
	std::string strPkgBasePath = T2A(m_strPkgPath_showPapersDlg);
	strPkgBasePath = strPkgBasePath.substr(0, strPkgBasePath.rfind('\\'));

	std::string strTmpPath = strPkgBasePath + "\\查看试卷-解压路径";
	CString strTmp = A2T(strTmpPath.c_str());
	std::string strDecompressPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strTmp));
	try
	{
		Poco::File dir(strDecompressPath);
		dir.createDirectories();
	}
	catch (Poco::Exception& e)
	{
	}
	//--
	Poco::Path filePath(strPkgPath);

	pDECOMPRESSTASK pDecompressTask1 = new DECOMPRESSTASK;
	pDecompressTask1->nTaskType = 6;
	pDecompressTask1->strFilePath = T2A(m_strPkgPath_showPapersDlg);
	pDecompressTask1->strFileBaseName = CMyCodeConvert::Utf8ToGb2312(filePath.getBaseName());
	pDecompressTask1->strSrcFileName = CMyCodeConvert::Utf8ToGb2312(filePath.getFileName());
	pDecompressTask1->strDecompressDir = T2A(strTmp);		//解压到的路径

	_fmDecompress_.lock();
	_nDecompress_++;
	_fmDecompress_.unlock();

	g_fmDecompressLock.lock();
	g_lDecompressTask.push_back(pDecompressTask1);
	g_fmDecompressLock.unlock();

	g_nRecogChkRotation = 1;	//识别时检测方向
	if (((CButton*)GetDlgItem(IDC_CHK_NeedRecogPkg))->GetCheck())
		g_nRecogWithShowPkg = 1;
	else
		g_nRecogWithShowPkg = 0;
//	g_nRecogEasyModel = 1;

}

void CDataMgrToolDlg::OnBnClickedBtnWatchpaper()
{
#ifdef PUBLIC_TOOL
	return;
#endif
	UpdateData(TRUE);
	USES_CONVERSION;

	std::string strModelPath = T2A(m_strModelPath_showPapersDlg);

	int nPos1 = strModelPath.rfind('\\');
	int nPos2 = strModelPath.rfind('.');

	std::string strBaseName = strModelPath.substr(nPos1 + 1, nPos2 - nPos1 - 1);
	std::string strSrcName = strModelPath.substr(nPos1 + 1, strModelPath.length() - nPos1 - 1);

	std::string strBasePath = strModelPath.substr(0, nPos1);

	pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
	pDecompressTask->nTaskType = 4;
	pDecompressTask->nExcuteTask = 2;	//加载模板后，不用搜索Pkg目录，后面手动加载
	pDecompressTask->strFilePath = T2A(m_strModelPath_showPapersDlg);
	pDecompressTask->strFileBaseName = strBaseName;
	pDecompressTask->strSrcFileName = strSrcName;
	pDecompressTask->strDecompressDir = strBasePath;

	_fmDecompress_.lock();
	_nDecompress_++;
	_fmDecompress_.unlock();

	g_fmDecompressLock.lock();
	g_lDecompressTask.push_back(pDecompressTask);
	g_fmDecompressLock.unlock();

	//*******************	识别试卷袋文件夹的试卷
	_strJpgSearchPath_ = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strJpgPath));

	g_nRecogChkRotation = 1;	//识别时检测方向
}

void CDataMgrToolDlg::OnBnClickedChkAutocontract()
{
	if (((CButton*)GetDlgItem(IDC_CHK_AutoContract))->GetCheck())
		g_nRecogWithContract = 1;
	else
		g_nRecogWithContract = 0;
}


void CDataMgrToolDlg::OnBnClickedBtnExportstatisticsdetail()
{
	if (_strDetailStatisTics_ == "")
	{
		AfxMessageBox(_T("当前无试卷袋的识别统计数据"));
		return;
	}
	TCHAR szFilter[] = { _T("TXT Files (*.txt)|*.txt|Excel Files (*.xls)|*.xls||") };
	CString fileName;
	fileName = "*.*";

	CFileDialog dlg(FALSE, _T("*.txt"), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);
	dlg.m_ofn.lpstrDefExt = _T(".txt");

	if (dlg.DoModal() == IDOK)
	{
		fileName = dlg.GetFolderPath() + _T("\\") + dlg.GetFileName();
		CFile file;
		file.Open(fileName, CFile::modeCreate | CFile::modeReadWrite);
		//file.Write(fileName.GetBuffer(fileName.GetLength()),fileName.GetLength());  

		//将编辑框中的内容写到文件中  
		file.Write(_strDetailStatisTics_.c_str(), _strDetailStatisTics_.length());
		file.Close();
	}
}


// DataMgrToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//答案容器
std::map<std::string, std::string> answerMap;

Poco::FastMutex		g_fmRecog;		//识别线程获取任务锁
RECOGTASKLIST		g_lRecogTask;	//识别任务列表

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

int		_nCannyKernel_ = 90;		//轮廓化核因子

int		g_nRecogGrayMin = 0;			//灰度点(除空白点,OMR外)计算灰度的最小考试范围
int		g_nRecogGrayMax_White = 255;	//空白点校验点计算灰度的最大考试范围
int		g_nRecogGrayMin_OMR = 0;		//OMR计算灰度的最小考试范围
int		g_RecogGrayMax_OMR = 235;		//OMR计算灰度的最大考试范围


double	_dCompThread_Fix_ = 1.2;
double	_dDiffThread_Fix_ = 0.2;
double	_dDiffExit_Fix_ = 0.3;
double	_dCompThread_Head_ = 1.0;
double	_dDiffThread_Head_ = 0.085;
double	_dDiffExit_Head_ = 0.15;

int		_nOMR_ = 230;		//重新识别模板时，用来识别OMR的密度值的阀值
int		_nSN_ = 200;		//重新识别模板时，用来识别ZKZH的密度值的阀值

//统计信息
Poco::FastMutex _fmErrorStatistics_;
int		_nErrorStatistics1_ = 0;	//第一种方法识别错误数
int		_nErrorStatistics2_ = 0;	//第二种方法识别错误数
int		_nDoubtStatistics = 0;		//识别怀疑总数
int		_nNullStatistics = 0;		//识别为空总数
int		_nAllStatistics_ = 0;		//统计总数

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
, m_pCompressObj(NULL), m_pCompressThread(NULL)
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
	DDX_Text(pDX, IDC_EDIT_Msg, m_strMsg);
	DDX_Control(pDX, IDC_EDIT_Msg, m_edit_Msg);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_Pkg_DIR, m_strPkgPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_Pkg_DIR, m_mfcEdit_PkgDir);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_RecogDir, m_strRecogPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_RecogDir, m_mfcEdit_RecogDir);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_ModelPath, m_strModelPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_ModelPath, m_mfcEdit_ModelPath);
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
	ON_BN_CLICKED(IDC_BTN_StudentAnswer, &CDataMgrToolDlg::OnBnClickedBtnStudentanswer)
	ON_BN_CLICKED(IDC_BTN_Statistics, &CDataMgrToolDlg::OnBnClickedBtnStatistics)
	ON_BN_CLICKED(IDC_BTN_StatisticsResult, &CDataMgrToolDlg::OnBnClickedBtnStatisticsresult)
	ON_BN_CLICKED(IDC_BTN_ClearStatistics, &CDataMgrToolDlg::OnBnClickedBtnClearstatistics)
	ON_BN_CLICKED(IDC_CHK_ReadParam, &CDataMgrToolDlg::OnBnClickedChkReadparam)
	ON_BN_CLICKED(IDC_BTN_LoadParam, &CDataMgrToolDlg::OnBnClickedBtnLoadparam)
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
	InitParam();
	
	_nUseNewParam_ = false;
	((CButton*)GetDlgItem(IDC_CHK_ReadParam))->SetCheck(0);
	GetDlgItem(IDC_BTN_LoadParam)->EnableWindow(FALSE);

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

	g_nExitFlag = 1;
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

	for (int i = 0; i < m_vecDecompressThreadObj.size(); i++)
	{
		m_pDecompressThread[i].join();
	}
	delete[] m_pDecompressThread;


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
	g_Log.LogOut("识别线程释放完毕.");

	if (_pModel_)
	{
		delete _pModel_;
		_pModel_ = NULL;
	}


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
	g_Log.LogOut("压缩处理线程释放完毕.");

	g_fmPapers.lock();			//释放试卷袋列表
	PAPERS_LIST::iterator itPapers = g_lPapers.begin();
	for (; itPapers != g_lPapers.end();)
	{
		pPAPERSINFO pPapersTask = *itPapers;
		SAFE_RELEASE(pPapersTask);
		itPapers = g_lPapers.erase(itPapers);
	}
	g_fmPapers.unlock();
}


void CDataMgrToolDlg::InitParam()
{
	std::string strLog;
	std::string strFile = g_strCurrentPath + "param.dat";
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strFile);
	try
	{
		Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));

		g_nRecogGrayMin = pConf->getInt("RecogGray.gray_Min", 0);
		g_nRecogGrayMax_White = pConf->getInt("RecogGray.white_Max", 255);
		g_nRecogGrayMin_OMR = pConf->getInt("RecogGray.omr_Min", 0);
		g_RecogGrayMax_OMR = pConf->getInt("RecogGray.omr_Max", 235);

		_dCompThread_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fCompTread", 1.2);
		_dDiffThread_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fDiffThread", 0.2);
		_dDiffExit_Fix_ = pConf->getDouble("RecogOmrSn_Fix.fDiffExit", 0.3);
		_dCompThread_Head_ = pConf->getDouble("RecogOmrSn_Head.fCompTread", 1.2);
		_dDiffThread_Head_ = pConf->getDouble("RecogOmrSn_Head.fDiffThread", 0.085);
		_dDiffExit_Head_ = pConf->getDouble("RecogOmrSn_Head.fDiffExit", 0.15);
		
		_nOMR_ = pConf->getInt("MakeModel_Threshold.omr", 230);
		_nSN_ = pConf->getInt("MakeModel_Threshold.sn", 200);

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

	std::string strLogPath = g_strCurrentPath + "DMT.Log";
	g_Log.SetFileName(strLogPath);

	CString strConfigPath = g_strCurrentPath;
	strConfigPath.Append(_T("DMT_config.ini"));
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strConfigPath));
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
	int nRecogThreads = pConf->getInt("Recog.threads", 2);
	int nDecompressThreads = pConf->getInt("Decompress.threads", 1);


	m_pDecompressThread = new Poco::Thread[nDecompressThreads];
	for (int i = 0; i < nDecompressThreads; i++)
	{
		CDecompressThread* pObj = new CDecompressThread(this);
		m_pDecompressThread[i].start(*pObj);
		m_vecDecompressThreadObj.push_back(pObj);
	}

	m_pCompressThread = new Poco::Thread;
	m_pCompressObj = new CCompressThread(this);
	m_pCompressThread->start(*m_pCompressObj);

	m_pRecogThread = new Poco::Thread[nRecogThreads];
	for (int i = 0; i < nRecogThreads; i++)
	{
		CRecognizeThread* pRecogObj = new CRecognizeThread;
		m_pRecogThread[i].start(*pRecogObj);

		m_vecRecogThreadObj.push_back(pRecogObj);
	}
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

	CString strMsg = _T("");
	strMsg.Format(_T("-------------------\r\n%s\r\n-------------------\r\n"), A2T(strFileData.c_str()));
	showMsg(strMsg);
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



void CDataMgrToolDlg::OnBnClickedBtnClear()
{
	m_strMsg.Empty();
	UpdateData(FALSE);
}


void CDataMgrToolDlg::OnBnClickedBtnRecogpkg()
{
	UpdateData(TRUE);
	USES_CONVERSION;

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
				pDecompressTask->nTaskType = 2;
				pDecompressTask->strFilePath = CMyCodeConvert::Utf8ToGb2312(p.toString());
				pDecompressTask->strFileBaseName = CMyCodeConvert::Utf8ToGb2312(p.getBaseName());
				pDecompressTask->strSrcFileName = CMyCodeConvert::Utf8ToGb2312(p.getFileName());
				pDecompressTask->strDecompressDir = T2A(m_strRecogPath);

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

void CDataMgrToolDlg::OnBnClickedBtnRerecogpkg()
{
	UpdateData(TRUE);
	USES_CONVERSION;

	CString strDecompressDir = m_strPkgPath + "\\tmpDecompress";
	try
	{
		int nPos1 = m_strModelPath.ReverseFind('\\');
		int nPos2 = m_strModelPath.ReverseFind('.');
		std::string strModelPath = T2A(m_strModelPath);
		std::string strBaseName = strModelPath.substr(nPos1 + 1, nPos2 - nPos1 - 1);
		std::string strSrcName = strModelPath.substr(nPos1 + 1, strModelPath.length() - nPos1 - 1);

		std::string strBasePath = strModelPath.substr(0, nPos1);

		pDECOMPRESSTASK pDecompressTask = new DECOMPRESSTASK;
		pDecompressTask->nTaskType = 4;
		pDecompressTask->strFilePath = T2A(m_strModelPath);
		pDecompressTask->strFileBaseName = strBaseName;
		pDecompressTask->strSrcFileName = strSrcName;
		pDecompressTask->strDecompressDir = strBasePath;

		g_fmDecompressLock.lock();
		g_lDecompressTask.push_back(pDecompressTask);
		g_fmDecompressLock.unlock();



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
				pDecompressTask->nTaskType = 3;
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

		ss << "\r\n----------\r\n" << pPapers->strPapersName << "结果正确率统计完成:\r\n" << szStatisticsInfo << "\r\n----------\r\n";
	}
	

	if (pPapers->lIssue.size() == 0)
	{
		int nModelOmrCount = 0;
		for (int k = 0; k < _pModel_->vecPaperModel.size(); k++)
			nModelOmrCount += _pModel_->vecPaperModel[k]->lOMR2.size();

		int nPapersCount = pPapers->lPaper.size() + pPapers->lIssue.size();
		int nOmrCount = nModelOmrCount * nPapersCount;

		char szStatisticsInfo[300] = { 0 };
		sprintf_s(szStatisticsInfo, "\n统计信息: omrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), zkzhNull = %.2f%%(%d/%d)\n", (float)pPapers->nOmrDoubt / nOmrCount * 100, pPapers->nOmrDoubt, nOmrCount, \
				  (float)pPapers->nOmrNull / nOmrCount * 100, pPapers->nOmrNull, nOmrCount, \
				  (float)pPapers->nSnNull / nPapersCount * 100, pPapers->nSnNull, nPapersCount);

		strMsg.Format(_T("\r\n=======>>>>>>>>>>\r\n%s识别完成\r\n%s\r\n%s=======<<<<<<<<<<\r\n"), A2T(pPapers->strPapersName.c_str()), A2T(szStatisticsInfo), A2T(ss.str().c_str()));
	}
	else
		strMsg.Format(_T("\r\n%s识别出问题试卷\r\n"), A2T(pPapers->strPapersName.c_str()));
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

	char szStatisticsInfo[300] = { 0 };
	sprintf_s(szStatisticsInfo, "\n所有试卷袋识别错误信息统计:\r\ndoubt = %.2f%%(%d/%d), null = %.2f%%(%d/%d), omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\n", (float)_nDoubtStatistics / _nAllStatistics_ * 100, _nDoubtStatistics, _nAllStatistics_, \
			  (float)_nNullStatistics / _nAllStatistics_ * 100, _nNullStatistics, _nAllStatistics_, \
			  (float)_nErrorStatistics1_ / _nAllStatistics_ * 100, _nErrorStatistics1_, _nAllStatistics_, \
			  (float)_nErrorStatistics2_ / _nAllStatistics_ * 100, _nErrorStatistics2_, _nAllStatistics_);
	strMsg.Format(_T("\r\n***********************\r\n%s\r\n***********************\r\n"), A2T(szStatisticsInfo));
	showMsg(strMsg);
	g_Log.LogOut(T2A(strMsg));
}


void CDataMgrToolDlg::OnBnClickedBtnClearstatistics()
{
	_fmErrorStatistics_.lock();
	_nErrorStatistics1_ = 0;
	_nErrorStatistics2_ = 0;
	_nDoubtStatistics = 0;
	_nNullStatistics = 0;
	_nAllStatistics_ = 0;
	_fmErrorStatistics_.unlock();

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

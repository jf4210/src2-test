
// DataMgrToolDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"
#include "afxdialogex.h"
#include "RecogParamDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//------------------------------------
Poco::Event		_E_StartSearch_;
std::string		_strPkgSearchPath_;		//�Ծ������·��

Poco::FastMutex	_fmSearchPathList_;
L_SearchTask	_SearchPathList_;

Poco::FastMutex	_fmAddPkg_;
L_PKGINFO		_PkgInfoList_;


std::string		_strComparePkgInfo_;
//------------------------------------

//������
std::map<std::string, std::string> answerMap;

Poco::FastMutex			g_fmReSult;		//�����̻߳�ȡ������
RECOG_RESULT_LIST		g_lResultTask;	//���������б�

Poco::FastMutex		g_fmRecog;		//ʶ���̻߳�ȡ������
RECOGTASKLIST		g_lRecogTask;	//ʶ�������б�


Poco::FastMutex			g_fmHttpSend;
LIST_SEND_HTTP			g_lHttpSend;			//����HTTP�����б�

std::string		g_strUploadUri;		//ʶ�����ύ��uri��ַ

CLog g_Log;
int	g_nExitFlag;
CString				g_strCurrentPath;
std::string		_strEncryptPwd_ = "yklxTest";
std::string		_strSessionName_;

Poco::FastMutex			g_fmDecompressLock;		//��ѹ�ļ��б���
DECOMPRESSTASKLIST		g_lDecompressTask;		//��ѹ�ļ��б�

Poco::FastMutex			g_fmCompressLock;		//ѹ���ļ��б���
COMPRESSTASKLIST		g_lCompressTask;		//��ѹ�ļ��б�

Poco::Event				g_eCompressThreadExit;

Poco::FastMutex		g_fmPapers;		//�����Ծ���б��������
PAPERS_LIST			g_lPapers;		//���е��Ծ����Ϣ

pMODEL _pModel_ = NULL;

bool	_nUseNewParam_ = false;			//�Ƿ�ʹ���µĲ�������ʶ��ģ��

int		_nAnticlutterKernel_ = 4;	//ʶ��ͬ��ͷʱ���������͸�ʴ�ĺ�����
int		_nCannyKernel_ = 90;		//������������

int		g_nRecogGrayMin = 0;			//�Ҷȵ�(���հ׵�,OMR��)����Ҷȵ���С���Է�Χ
int		g_nRecogGrayMax_White = 255;	//�հ׵�У������Ҷȵ�����Է�Χ
int		g_nRecogGrayMin_OMR = 0;		//OMR����Ҷȵ���С���Է�Χ
int		g_RecogGrayMax_OMR = 235;		//OMR����Ҷȵ�����Է�Χ

double	_dOmrThresholdPercent_Fix_;		//����ģʽOMRʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
double	_dSnThresholdPercent_Fix_;		//����ģʽSNʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
double	_dQKThresholdPercent_Fix_;		//����ģʽQKʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
double	_dOmrThresholdPercent_Head_;	//ͬ��ͷģʽOMRʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
double	_dSnThresholdPercent_Head_;		//ͬ��ͷģʽSNʶ�����Ϊ��ѡ�еı�׼�ٷֱ�
double	_dQKThresholdPercent_Head_;		//ͬ��ͷģʽQKʶ�����Ϊ��ѡ�еı�׼�ٷֱ�

double	_dCompThread_Fix_ = 1.2;
double	_dDiffThread_Fix_ = 0.2;
double	_dDiffExit_Fix_ = 0.3;
double	_dCompThread_Head_ = 1.0;
double	_dDiffThread_Head_ = 0.085;
double	_dDiffExit_Head_ = 0.15;
int		_nThreshold_Recog2_ = 240;	//��2��ʶ�𷽷��Ķ�ֵ����ֵ
double	_dCompThread_3_ = 170;		//������ʶ�𷽷�
double	_dDiffThread_3_ = 20;
double	_dDiffExit_3_ = 50;
double	_dAnswerSure_ = 100;		//����ȷ���Ǵ𰸵����Ҷ�


int		_nOMR_ = 230;		//����ʶ��ģ��ʱ������ʶ��OMR���ܶ�ֵ�ķ�ֵ
int		_nSN_ = 200;		//����ʶ��ģ��ʱ������ʶ��ZKZH���ܶ�ֵ�ķ�ֵ

//ͳ����Ϣ
Poco::FastMutex _fmErrorStatistics_;
int		_nErrorStatistics1_ = 0;	//��һ�ַ���ʶ�������
int		_nErrorStatistics2_ = 0;	//�ڶ��ַ���ʶ�������
int		_nOmrDoubtStatistics_ = 0;	//ʶ��������
int		_nOmrNullStatistics_ = 0;	//ʶ��Ϊ������
int		_nSnNullStatistics_ = 0;	//SNʶ��Ϊ������
int		_nAllOmrStatistics_ = 0;		//OMRͳ������
int		_nAllSnStatistics_ = 0;			//SNͳ������
int		_nPkgDoubtStatistics_ = 0;		//ԭʼ�Ծ��ʶ��������
int		_nPkgOmrNullStatistics_ = 0;	//ԭʼ�Ծ��ʶ��Ϊ������
int		_nPkgSnNullStatistics_ = 0;		//ԭʼ�Ծ����SNʶ��Ϊ������

int		_nDecompress_ = 0;	//��ѹ�Ծ������
int		_nRecog_ = 0;		//ʶ���Ծ�����
int		_nRecogPapers_ = 0;	//ʶ���Ծ��
int		_nCompress_ = 0;	//ѹ���Ծ������
Poco::FastMutex _fmDecompress_;	
Poco::FastMutex _fmRecog_;
Poco::FastMutex _fmRecogPapers_;
Poco::FastMutex _fmCompress_;

double	_dDoubtPer_ = 0.0;	//����ʶ��ʱ����ʶ����Ծ���Ļ����ʳ����˷�ֵʱ�������Ծ��ѹ������һ��Ŀ¼

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


// CDataMgrToolDlg �Ի���



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

	DDX_Text(pDX, IDC_MFCEDITBROWSE_RecogDir, m_strRecogPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_RecogDir, m_mfcEdit_RecogDir);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_ModelPath, m_strModelPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_ModelPath, m_mfcEdit_ModelPath);

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
END_MESSAGE_MAP()


// CDataMgrToolDlg ��Ϣ�������

BOOL CDataMgrToolDlg::OnInitDialog()
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

	USES_CONVERSION;
	CString strTitle = _T("");
	strTitle.Format(_T("%s %s"), SYS_BASE_NAME, SOFT_VERSION);
	SetWindowText(strTitle);

	InitConfig();
	InitParam();
	
	_nUseNewParam_ = false;
	((CButton*)GetDlgItem(IDC_CHK_ReadParam))->SetCheck(0);
	GetDlgItem(IDC_BTN_LoadParam)->EnableWindow(FALSE);

	SetTimer(TIMER_UPDATE_STARTBAR, 1000, NULL);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CDataMgrToolDlg::OnPaint()
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
HCURSOR CDataMgrToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDataMgrToolDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(TIMER_UPDATE_STARTBAR);

	g_nExitFlag = 1;

	//����pkg�ļ��߳��˳�
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

	//����ʶ�����߳��˳�
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

	//��ѹ�߳��˳�
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

	//ʶ���߳��˳�
	g_fmRecog.lock();			//�ͷ�δ�������ʶ�������б�
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
	g_Log.LogOut("ʶ���߳��ͷ����.");

	if (_pModel_)
	{
		delete _pModel_;
		_pModel_ = NULL;
	}

	//ѹ��pkg�ļ��߳��˳�
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
	g_Log.LogOut("ѹ�������߳��ͷ����.");

	g_fmPapers.lock();			//�ͷ��Ծ���б�
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

		strLog = "��ȡʶ��ҶȲ������";
	}
	catch (Poco::Exception& exc)
	{
		strLog = "��ȡ����ʧ�ܣ�ʹ��Ĭ�ϲ��� " + CMyCodeConvert::Utf8ToGb2312(exc.displayText());
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
	UINT nID = 0;        //����״̬������ķ���
	m_wndStatusBar.SetPaneInfo(0, nID, SBPS_NORMAL, 100);              //����ֵ��nID��
	m_wndStatusBar.SetPaneText(0, _T("��ѹ�Ծ��"));
	m_wndStatusBar.SetPaneInfo(1, nID, SBPS_NORMAL, 50);
	m_wndStatusBar.SetPaneText(1, _T(""));
	m_wndStatusBar.SetPaneInfo(2, nID, SBPS_NORMAL, 100);
	m_wndStatusBar.SetPaneText(2, _T("ʶ���Ծ�"));
	m_wndStatusBar.SetPaneInfo(3, nID, SBPS_POPOUT, 50);
	m_wndStatusBar.SetPaneText(3, _T(""));
	m_wndStatusBar.SetPaneInfo(4, nID, SBPS_NORMAL, 100);
	m_wndStatusBar.SetPaneText(4, _T("ʶ���Ծ��"));
	m_wndStatusBar.SetPaneInfo(5, nID, SBPS_POPOUT, 50);
	m_wndStatusBar.SetPaneText(5, _T(""));
	m_wndStatusBar.SetPaneInfo(6, nID, SBPS_POPOUT, 100);
	m_wndStatusBar.SetPaneText(6, _T("ѹ���Ծ��"));
	m_wndStatusBar.SetPaneInfo(7, nID, SBPS_POPOUT, 50);
	m_wndStatusBar.SetPaneText(7, _T(""));
	//               SetPaneInfo()�����ĵ����������Ŀ�ѡ�����£�
	//               The following indicator styles are supported:
	//                           SBPS_NOBORDERS               No 3-D border around the pane.
	//                           SBPS_POPOUT                        Reverse border so that text "pops out."
	//                           SBPS_DISABLED                     Do not draw text.
	//                           SBPS_STRETCH                      Stretch pane to fill unused space. Only one pane per status bar can have thisstyle.
	//                           SBPS_NORMAL                        No stretch, borders, or pop-out.
	//----------------�����״̬��������ʾ�ڶԻ�����-------------
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
#else
	m_statusBar.Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0, 0, 0, 0), this, 0);
	int strPartDim[8] = { 80, 130, 210, 260, 340, 390, 480, -1 }; //�ָ�����
	m_statusBar.SetParts(8, strPartDim);

	//����״̬���ı�
	m_statusBar.SetText(_T("��ѹ�Ծ��:"), 0, 0);
	m_statusBar.SetText(_T("ʶ���Ծ�:"), 2, 0);
	m_statusBar.SetText(_T("ʶ���Ծ��:"), 4, 0);
	m_statusBar.SetText(_T("ѹ���Ծ��:"), 6, 0);
	//��������״̬���м���ͼ��
//	m_statusBar.SetIcon(1, SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE));//Ϊ�ڶ��������мӵ�ͼ��
#endif
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
		strMsg.Format(_T("���ļ�(%s)ʧ��\r\n"), A2T(strFilePath.c_str()));
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

	CRecogParamDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;

	CString strDecompressDir = m_strPkgPath + "\\tmpDecompress";
	try
	{
		CString strNewPkgSavePath = m_strPkgPath + "\\newPkg";
		CString strNewPkg_HighErrSavePath = m_strPkgPath + "\\newPkg_HighError";
		//�Ƚ�Ŀ¼ɾ��


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
		pDecompressTask->bRecogOmr = dlg.m_nChkOmr;				//���в�������
		pDecompressTask->bRecogZkzh = dlg.m_nChkSN;				//���в�������
		pDecompressTask->bRecogElectOmr = dlg.m_nChkElectOmr;	//���в�������
		pDecompressTask->nSendEzs = dlg.m_nHandleResult;		//��������
		pDecompressTask->nNoNeedRecogVal = dlg.m_nNoRecogVal;	//....


		_fmDecompress_.lock();
		_nDecompress_++;
		_fmDecompress_.unlock();

		g_fmDecompressLock.lock();
		g_lDecompressTask.push_back(pDecompressTask);
		g_fmDecompressLock.unlock();



		std::string strPkgPath = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strPkgPath));
		_strPkgSearchPath_ = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strPkgPath));
	#if 1
// 		pST_SEARCH pTask = new ST_SEARCH;
// 		pTask->strSearchPath = CMyCodeConvert::Gb2312ToUtf8(strPkgPath);
// 		_SearchPathList_.push_back(pTask);
	#else
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
	#endif
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
	strMsg.Format(_T("%s:%sʶ����쳣��\r\n"), A2T(pPapers->strPapersName.c_str()), A2T(pPaper->strStudentInfo.c_str()));
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
		sprintf_s(szStatisticsInfo, "\nʶ�������Ϣͳ��: omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\n", (float)pPapers->nOmrError_1 / nOmrCount * 100, pPapers->nOmrError_1, nOmrCount, \
				  (float)pPapers->nOmrError_2 / nOmrCount * 100, pPapers->nOmrError_2, nOmrCount);

		ss << "\r\n\t----------\r\n\t" << pPapers->strPapersName << "�����ȷ��ͳ�����:\r\n\t" << szStatisticsInfo << "\r\n\t----------\r\n";
	}
	

	if (pPapers->lIssue.size() == 0)
	{
		int nModelOmrCount = 0;
		for (int k = 0; k < _pModel_->vecPaperModel.size(); k++)
			nModelOmrCount += _pModel_->vecPaperModel[k]->lOMR2.size();

		int nPapersCount = pPapers->lPaper.size() + pPapers->lIssue.size();
		int nOmrCount = nModelOmrCount * nPapersCount;

		char szStatisticsInfo[300] = { 0 };
		sprintf_s(szStatisticsInfo, "��ʶ����Ϣ: omrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), zkzhNull = %.2f%%(%d/%d)\n", (float)pPapers->nOmrDoubt / nOmrCount * 100, pPapers->nOmrDoubt, nOmrCount, \
				  (float)pPapers->nOmrNull / nOmrCount * 100, pPapers->nOmrNull, nOmrCount, \
				  (float)pPapers->nSnNull / nPapersCount * 100, pPapers->nSnNull, nPapersCount);

		char szPkgStatisticsInfo[300] = { 0 };
		sprintf_s(szPkgStatisticsInfo, "ԭʼ����Ϣ: omrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), zkzhNull = %.2f%%(%d/%d)\n", (float)pPapers->nPkgOmrDoubt / nOmrCount * 100, pPapers->nPkgOmrDoubt, nOmrCount, \
				  (float)pPapers->nPkgOmrNull / nOmrCount * 100, pPapers->nPkgOmrNull, nOmrCount, \
				  (float)pPapers->nPkgSnNull / nPapersCount * 100, pPapers->nPkgSnNull, nPapersCount);

		strMsg.Format(_T("\r\n==================\r\n%sʶ�����\r\n%s\r\n%s\r\n%s\r\n\r\n"), A2T(pPapers->strPapersName.c_str()), A2T(szStatisticsInfo), A2T(ss.str().c_str()), A2T(szPkgStatisticsInfo));
	}
	else
		strMsg.Format(_T("\r\n****************\r\n%sʶ��������Ծ�, ���������=%d\r\n"), A2T(pPapers->strPapersName.c_str()), pPapers->lIssue.size());
	showMsg(strMsg);
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
		strMsg.Format(_T("(%s)����ʶ��������˷�����ʧ��\r\n"), A2T(pPapers->strPapersName.c_str()));
	else
		strMsg.Format(_T("(%s)����ʶ��������˷������ɹ�\r\n"), A2T(pPapers->strPapersName.c_str()));
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
	CString strInfo = _T("\r\n����ѧ�������\r\n");
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
	sprintf_s(szStatisticsInfo, "\n�����Ծ��ʶ�������Ϣͳ��:\r\nomrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), snNull = %.2f%%(%d/%d), omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\r\nԭʼ�Ծ��ͳ��: omrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), snNull = %.2f%%(%d/%d)\n", (float)_nOmrDoubtStatistics_ / _nAllOmrStatistics_ * 100, _nOmrDoubtStatistics_, _nAllOmrStatistics_, \
			  (float)_nOmrNullStatistics_ / _nAllOmrStatistics_ * 100, _nOmrNullStatistics_, _nAllOmrStatistics_, \
			  (float)_nSnNullStatistics_ / _nAllSnStatistics_ * 100, _nSnNullStatistics_, _nAllSnStatistics_, \
			  (float)_nErrorStatistics1_ / _nAllOmrStatistics_ * 100, _nErrorStatistics1_, _nAllOmrStatistics_, \
			  (float)_nErrorStatistics2_ / _nAllOmrStatistics_ * 100, _nErrorStatistics2_, _nAllOmrStatistics_, \
			  (float)_nPkgDoubtStatistics_ / _nAllOmrStatistics_ * 100, _nPkgDoubtStatistics_, _nAllOmrStatistics_, \
			  (float)_nPkgOmrNullStatistics_ / _nAllOmrStatistics_ * 100, _nPkgOmrNullStatistics_, _nAllOmrStatistics_, \
			  (float)_nPkgSnNullStatistics_ / _nAllSnStatistics_ * 100, _nPkgSnNullStatistics_, _nAllSnStatistics_);
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
	_nPkgDoubtStatistics_ = 0;
	_nPkgOmrNullStatistics_ = 0;
	_nPkgSnNullStatistics_ = 0;
	_fmErrorStatistics_.unlock();

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
	strMsg.Format(_T("\r\nͳ�ƽ��������\r\n"));
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
	CString strMsg = _T("���ز������\r\n");
	showMsg(strMsg);
}



void CDataMgrToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_UPDATE_STARTBAR)
	{
		CString strTmp;
		if (m_wndStatusBar.GetSafeHwnd())
		{
			strTmp.Format(_T("%d"), _nDecompress_);
			m_wndStatusBar.SetPaneText(1, strTmp);
			strTmp.Format(_T("%d"), _nRecog_);
			m_wndStatusBar.SetPaneText(3, strTmp);
			strTmp.Format(_T("%d"), _nRecogPapers_);
			m_wndStatusBar.SetPaneText(5, strTmp);
			strTmp.Format(_T("%d"), _nCompress_);
			m_wndStatusBar.SetPaneText(7, strTmp);
		}
		else
		{
			strTmp.Format(_T("%d"), _nDecompress_);
			m_statusBar.SetText(strTmp, 1, 0);
			strTmp.Format(_T("%d"), _nRecog_);
			m_statusBar.SetText(strTmp, 3, 0);
			strTmp.Format(_T("%d"), _nRecogPapers_);
			m_statusBar.SetText(strTmp, 5, 0);
			strTmp.Format(_T("%d"), _nCompress_);
			m_statusBar.SetText(strTmp, 7, 0);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

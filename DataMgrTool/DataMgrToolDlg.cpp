
// DataMgrToolDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "DataMgrTool.h"
#include "DataMgrToolDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//������
std::map<std::string, std::string> answerMap;

Poco::FastMutex		g_fmRecog;		//ʶ���̻߳�ȡ������
RECOGTASKLIST		g_lRecogTask;	//ʶ�������б�

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

int		_nCannyKernel_ = 90;		//������������

int		g_nRecogGrayMin = 0;			//�Ҷȵ�(���հ׵�,OMR��)����Ҷȵ���С���Է�Χ
int		g_nRecogGrayMax_White = 255;	//�հ׵�У������Ҷȵ�����Է�Χ
int		g_nRecogGrayMin_OMR = 0;		//OMR����Ҷȵ���С���Է�Χ
int		g_RecogGrayMax_OMR = 235;		//OMR����Ҷȵ�����Է�Χ


double	_dCompThread_Fix_ = 1.2;
double	_dDiffThread_Fix_ = 0.2;
double	_dDiffExit_Fix_ = 0.3;
double	_dCompThread_Head_ = 1.0;
double	_dDiffThread_Head_ = 0.085;
double	_dDiffExit_Head_ = 0.15;

int		_nOMR_ = 230;		//����ʶ��ģ��ʱ������ʶ��OMR���ܶ�ֵ�ķ�ֵ
int		_nSN_ = 200;		//����ʶ��ģ��ʱ������ʶ��ZKZH���ܶ�ֵ�ķ�ֵ

//ͳ����Ϣ
Poco::FastMutex _fmErrorStatistics_;
int		_nErrorStatistics1_ = 0;	//��һ�ַ���ʶ�������
int		_nErrorStatistics2_ = 0;	//�ڶ��ַ���ʶ�������
int		_nDoubtStatistics = 0;		//ʶ��������
int		_nNullStatistics = 0;		//ʶ��Ϊ������
int		_nAllStatistics_ = 0;		//ͳ������

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

	InitConfig();
	InitParam();
	
	_nUseNewParam_ = false;
	((CButton*)GetDlgItem(IDC_CHK_ReadParam))->SetCheck(0);
	GetDlgItem(IDC_BTN_LoadParam)->EnableWindow(FALSE);

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


	g_fmRecog.lock();			//�ͷ�δ�������ʶ�������б�
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
	g_Log.LogOut("ʶ���߳��ͷ����.");

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
	g_Log.LogOut("ѹ�������߳��ͷ����.");

	g_fmPapers.lock();			//�ͷ��Ծ���б�
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

		ss << "\r\n----------\r\n" << pPapers->strPapersName << "�����ȷ��ͳ�����:\r\n" << szStatisticsInfo << "\r\n----------\r\n";
	}
	

	if (pPapers->lIssue.size() == 0)
	{
		int nModelOmrCount = 0;
		for (int k = 0; k < _pModel_->vecPaperModel.size(); k++)
			nModelOmrCount += _pModel_->vecPaperModel[k]->lOMR2.size();

		int nPapersCount = pPapers->lPaper.size() + pPapers->lIssue.size();
		int nOmrCount = nModelOmrCount * nPapersCount;

		char szStatisticsInfo[300] = { 0 };
		sprintf_s(szStatisticsInfo, "\nͳ����Ϣ: omrDoubt = %.2f%%(%d/%d), omrNull = %.2f%%(%d/%d), zkzhNull = %.2f%%(%d/%d)\n", (float)pPapers->nOmrDoubt / nOmrCount * 100, pPapers->nOmrDoubt, nOmrCount, \
				  (float)pPapers->nOmrNull / nOmrCount * 100, pPapers->nOmrNull, nOmrCount, \
				  (float)pPapers->nSnNull / nPapersCount * 100, pPapers->nSnNull, nPapersCount);

		strMsg.Format(_T("\r\n=======>>>>>>>>>>\r\n%sʶ�����\r\n%s\r\n%s=======<<<<<<<<<<\r\n"), A2T(pPapers->strPapersName.c_str()), A2T(szStatisticsInfo), A2T(ss.str().c_str()));
	}
	else
		strMsg.Format(_T("\r\n%sʶ��������Ծ�\r\n"), A2T(pPapers->strPapersName.c_str()));
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
	sprintf_s(szStatisticsInfo, "\n�����Ծ��ʶ�������Ϣͳ��:\r\ndoubt = %.2f%%(%d/%d), null = %.2f%%(%d/%d), omrError1 = %.2f%%(%d/%d), omrError2 = %.2f%%(%d/%d)\n", (float)_nDoubtStatistics / _nAllStatistics_ * 100, _nDoubtStatistics, _nAllStatistics_, \
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

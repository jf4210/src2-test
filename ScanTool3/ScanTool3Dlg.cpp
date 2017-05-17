
// ScanTool2Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanTool3Dlg.h"
#include "afxdialogex.h"
#include "global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CScanTool2Dlg �Ի���

//++ȫ�ֱ���
using namespace std;
using namespace cv;
int					g_nManulUploadFile = 0;		//�ֶ��ϴ��ļ���ͨ��qq�����
bool				g_bCmdConnect = false;		//����ͨ������
bool				g_bFileConnect = false;		//�ļ�ͨ������

bool				g_bCmdNeedConnect = false;	//����ͨ���Ƿ���Ҫ����������ͨ����ַ��Ϣ�޸ĵ����
bool				g_bFileNeedConnect = false;	//�ļ�ͨ���Ƿ���Ҫ����������ͨ����ַ��Ϣ�޸ĵ����

bool				g_bShowScanSrcUI = false;	//�Ƿ���ʾԭʼɨ�����
int					g_nOperatingMode = 2;		//����ģʽ��1--����ģʽ(��������㲻ֹͣɨ��)��2-�ϸ�ģʽ(���������ʱ����ֹͣɨ��)
int					g_nZkzhNull2Issue = 0;		//ʶ��׼��֤��δ��ʱ���Ƿ���Ϊ�������Ծ�

int					g_nExitFlag = 0;
CString				g_strCurrentPath;
std::string			g_strPaperSavePath;	//�Ծ�ɨ��󱣴����·��
std::string			g_strModelSavePath;
std::string			g_strPaperBackupPath;	//�Ծ�����ɺ�ı���·��
Poco::Logger*		g_pLogger;
Poco::FastMutex		g_fmRecog;		//ʶ���̻߳�ȡ������
RECOGTASKLIST		g_lRecogTask;	//ʶ�������б�

Poco::FastMutex		g_fmPapers;		//�����Ծ���б��������
PAPERS_LIST			g_lPapers;		//���е��Ծ����Ϣ

Poco::FastMutex		g_fmSendLock;	//�ϴ��ļ��б����
SENDTASKLIST		g_lSendTask;	//�ϴ��ļ������б�

Poco::FastMutex		g_fmTcpTaskLock;
TCP_TASKLIST		g_lTcpTask;		//tcp�����б�

Poco::FastMutex		g_lfmExamList;
EXAM_LIST			g_lExamList;	//��ǰ�˺Ŷ�Ӧ�Ŀ����б�

Poco::FastMutex		g_fmCompressLock;		//ѹ���ļ��б���
COMPRESSTASKLIST	g_lCompressTask;		//��ѹ�ļ��б�

//++�߳��˳�����¼�
Poco::Event			g_eTcpThreadExit;
Poco::Event			g_eSendFileThreadExit;
Poco::Event			g_eCompressThreadExit;
//--
//++�¼�����
Poco::Event			g_eGetExamList;		//��ȡ�����б��¼�
Poco::Event			g_eDownLoadModel;	//����ģ�����״̬
//--

int					g_nDownLoadModelStatus = 0;		//����ģ���״̬	0-δ���أ���ʼ����1-ģ�������У�2-���سɹ���3-���ش��ڴ��ļ�������Ҫ����, -1-�������˿�Ŀģ�岻����, -2-��������ȡ�ļ�ʧ��

STUDENT_LIST		g_lBmkStudent;	//������ѧ���б�


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

int		_nAnticlutterKernel_ = 4;	//ʶ��ͬ��ͷʱ���������͸�ʴ�ĺ�����
int		_nGauseKernel_ = 5;			//��˹�任������
int		_nSharpKernel_ = 5;			//�񻯺�����
int		_nCannyKernel_ = 90;		//������������
int		_nDilateKernel_ = 6;		//���ͺ�����	//Size(6, 6)	��ͨ�հ׿��ʶ��
int		_nErodeKernel_ = 2;			//��ʴ������

int		g_nRecogGrayMin = 0;			//�Ҷȵ�(���հ׵�,OMR��)����Ҷȵ���С���Է�Χ
int		g_nRecogGrayMax_White = 255;	//�հ׵�У������Ҷȵ�����Է�Χ
int		g_nRecogGrayMin_OMR = 0;		//OMR����Ҷȵ���С���Է�Χ
int		g_RecogGrayMax_OMR = 235;		//OMR����Ҷȵ�����Է�Χ

std::string			g_strEncPwd = "yklxTest";				//�ļ����ܽ�������
std::string			g_strCmdIP;
std::string			g_strFileIP;
int					g_nCmdPort;
int					g_nFilePort;

int				_nReocgThreads_ = 3;		//ʶ���߳�����

//++��¼��Ϣ
bool	_bLogin_ = false;		//�Ƿ��Ѿ���¼
std::string _strUserName_;		//��¼�û���
std::string _strNickName_;		//�û��ǳ�
std::string _strPwd_;			//����
std::string _strEzs_;			//�����Ҫ��EZS
int _nTeacherId_ = 0;			//��ʦID
int _nUserId_ = 0;				//�û�ID
//--
//++ɨ�����
pEXAMINFO			_pCurrExam_= NULL;		//��ǰ����
pEXAM_SUBJECT		_pCurrSub_ = NULL;		//��ǰ���Կ�Ŀ
pMODEL				_pModel_ = NULL;		//��ǰɨ��ʹ�õ�ģ��
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
	//ʶ���߳�
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

	//�ļ������߳�
// 	m_SendFileThread->join();
// 	SAFE_RELEASE(m_pSendFileObj);
// 	g_eSendFileThreadExit.wait();
// 	SAFE_RELEASE(m_SendFileThread);

	//tcp�����߳�
	m_TcpCmdThread->join();
	SAFE_RELEASE(m_pTcpCmdObj);
	g_eTcpThreadExit.wait();
	SAFE_RELEASE(m_TcpCmdThread);

	//ѹ���߳�
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
		m_pScanMgrDlg->SearchModel();		//����ģ��
		m_pScanMgrDlg->UpdateInfo();
	}
}

BEGIN_MESSAGE_MAP(CScanTool3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CScanTool2Dlg ��Ϣ�������

BOOL CScanTool3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

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

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CScanTool3Dlg::OnPaint()
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


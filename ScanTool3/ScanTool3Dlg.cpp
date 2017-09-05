
// ScanTool2Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanTool3Dlg.h"
#include "afxdialogex.h"
#include "global.h"
#include "NewMessageBox.h"
#include "Net_Cmd_Protocol.h"
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
bool				g_bModifySN = false;		//�Ƿ������޸�׼��֤��
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
Poco::Event			g_eGetBmk;			//��ȡ�������¼�
Poco::Event			g_eDownLoadModel;	//����ģ�����״̬
Poco::Event			g_eGetModelPic;		//����ģ��ͼƬ״̬
//--

pPAPERSINFO			_pCurrPapersInfo_ = NULL;
int					g_nDownLoadModelStatus = 0;		//����ģ���״̬	0-δ���أ���ʼ����1-ģ�������У�2-���سɹ���3-���ش��ڴ��ļ�������Ҫ����, -1-�������˿�Ŀģ�岻����, -2-��������ȡ�ļ�ʧ��
int					_nScanStatus_ = 0;				//ɨ����� 0-δɨ�裬1-����ɨ�裬2-ɨ�����, 3-ɨ����ֹ, -1--����ɨ����ʧ��, -2--����ɨ����ʧ��, -3--ɨ��ʧ��, -4-ɨ��ȡ��
STUDENT_LIST		g_lBmkStudent;					//������ѧ���б�
int					_nGetModelPic_ = 0;				//��ȡģ��ͼƬ״̬��0-δ��ȡ��1-���ڻ�ȡ(����)�� 2-��ȡģ��ͼƬ�ɹ���3-��������ģ��ͼƬ��4-��������ȡʧ��
int					_nDoubleScan_ = 1;				//�Ƿ�˫��ɨ�裬0-���棬 1-˫�棬����˫��ͼ�����ʱ����

std::vector<MODELPICPATH> _vecModelPicPath_;		//���ڴӷ�����������ͼ���ֱ�Ӵ���ģ��
#ifdef NewBmkTest
ALLSTUDENT_LIST		g_lBmkAllStudent;		//�������������п�Ŀ�ı�����ѧ���б�
EXAMBMK_MAP			g_mapBmkMgr;			//���Ա���������ϣ��
#endif


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
std::string			g_strFileIp4HandModel;		//����ģʽʱ�ļ���������ַ
int					g_nFilePort4HandModel;		//����ģʽʱ�ļ��������˿�

int				_nReocgThreads_ = 3;		//ʶ���߳�����

//++������Ϣ
std::string _strCopyright_;
std::string _strMainTitle_;
std::string _strLitteTitle_;
//--

//++��¼��Ϣ
bool	_bHandModel_ = false;	//�Ƿ�������ģʽ������ģʽ��������
bool	_bLogin_ = false;		//�Ƿ��Ѿ���¼
std::string _strUserName_;		//��¼�û���
std::string _strNickName_;		//�û��ǳ�
std::string _strPwd_;			//����
std::string _strEzs_;			//�����Ҫ��EZS
int _nTeacherId_ = 0;			//��ʦID
int _nUserId_ = 0;				//�û�ID
std::string _strPersonID_;		//����ģʽ������ר��
std::string _strSchoolID_;		//����ģʽ������ר��
int _nPicNum4Ty_ = 2;			//����ģʽ��ģ��ͼƬ����
//--

bool				_bGetBmk_ = false;			//�Ƿ��õ�ǰ��Ŀ������
//++ɨ�����
pEXAMINFO			_pCurrExam_= NULL;		//��ǰ����
pEXAM_SUBJECT		_pCurrSub_ = NULL;		//��ǰ���Կ�Ŀ
pMODEL				_pModel_ = NULL;		//��ǰɨ��ʹ�õ�ģ��
int					_nScanCount_ = 0;		//ɨ�����������ǰ��ɨ����ٷ�
int					_nScanPaperCount_ = 0;	//��ǰ�Ѿ�ɨ�������������������ʼ����
int					_nScanAnswerModel_ = 0;	//0-ɨ���Ծ� 1-ɨ��Omr�𰸣� 2-ɨ���������
//--
E_DLG_TYPE			_eCurrDlgType_ = DLG_Login;	//��ǰ��ʾ�Ĵ��ڣ��������ڲ���
//--
//�ϴ���������ַ���
#ifdef TEST_MULTI_SENDER
Poco::FastMutex _fmMapSender_;
MAP_FILESENDER	_mapSender_;		//�ϴ�����������
#endif
//--

CScanTool3Dlg::CScanTool3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScanTool3Dlg::IDD, pParent)
	, m_pExamInfoMgrDlg(NULL), m_pScanMgrDlg(NULL), m_pModifyZkzhDlg(NULL), m_nStatusSize(25)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScanTool3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_bmpBtnExit);
	DDX_Control(pDX, IDC_BTN_Min, m_bmpBtnMin);
	DDX_Control(pDX, IDC_BTN_UserPic, m_bmpBtnUserPic);
	DDX_Text(pDX, IDC_STATIC_Title, m_strTitle);
	DDX_Text(pDX, IDC_STATIC_MinTitle, m_strVersion);
	DDX_Text(pDX, IDC_STATIC_UserName, m_strUserName);
}

void CScanTool3Dlg::InitFileUpLoadList()
{
	USES_CONVERSION;
	if (g_nManulUploadFile != 1)
	{
		CString strSearchPath = A2T(CMyCodeConvert::Utf8ToGb2312(g_strPaperSavePath).c_str());
		CFileFind ff;
		BOOL bFind = ff.FindFile(strSearchPath + _T("*"), 0);
		while (bFind)
		{
			bFind = ff.FindNextFileW();
			if (ff.GetFileName() == _T(".") || ff.GetFileName() == _T(".."))
				continue;
			else if (ff.IsArchived())
			{
				if (ff.GetFileName().Find(PAPERS_EXT_NAME) >= 0 || ff.GetFileName().Find(PAPERS_EXT_NAME_4TY) >= 0)
				{
					pSENDTASK pTask = new SENDTASK;
					pTask->strFileName = T2A(ff.GetFileName());
					pTask->strPath = T2A(ff.GetFilePath());
					g_fmSendLock.lock();
					g_lSendTask.push_back(pTask);
					g_fmSendLock.unlock();
				}
			}
		}
	}
}

void CScanTool3Dlg::InitCompressList()
{
	USES_CONVERSION;

	CString strSearchPath = A2T(CMyCodeConvert::Utf8ToGb2312(g_strPaperSavePath).c_str());
	CFileFind ff;
	BOOL bFind = ff.FindFile(strSearchPath + _T("*"), 0);
	while (bFind)
	{
		bFind = ff.FindNextFileW();
		if (ff.GetFileName() == _T(".") || ff.GetFileName() == _T(".."))
			continue;
		else if (ff.IsDirectory())
		{
			int nPos = -1;
			if ((nPos = ff.GetFileName().Find(_T("_ToCompress_UnPwd"))) >= 0)
			{
				CString strFileName = ff.GetFileName();
				CString strBaseZipName = strFileName.Left(nPos);
				CString strZipName = strBaseZipName;
				strZipName.Append(PAPERS_EXT_NAME_4TY);
				CString strSrcDirPath = ff.GetFilePath();
				CString strSavePath = g_strCurrentPath + _T("Paper\\") + strBaseZipName;

				char szZipName[100] = { 0 };
				pCOMPRESSTASK pTask = new COMPRESSTASK;
				pTask->strCompressFileName = T2A(strZipName);
				pTask->strExtName = T2A(PAPERS_EXT_NAME_4TY);
				pTask->strSavePath = T2A(strSavePath);
				pTask->strSrcFilePath = T2A(strSrcDirPath);
				g_fmCompressLock.lock();
				g_lCompressTask.push_back(pTask);
				g_fmCompressLock.unlock();
			}
			if ((nPos = ff.GetFileName().Find(_T("_ToCompress"))) >= 0 && (ff.GetFileName().Find(_T("_UnPwd")) == std::string::npos))
			{
				CString strFileName = ff.GetFileName();
				CString strBaseZipName = strFileName.Left(nPos);
				CString strZipName = strBaseZipName;
				strZipName.Append(PAPERS_EXT_NAME);
				CString strSrcDirPath = ff.GetFilePath();
				CString strSavePath = g_strCurrentPath + _T("Paper\\") + strBaseZipName;

				char szZipName[100] = { 0 };
				pCOMPRESSTASK pTask = new COMPRESSTASK;
				pTask->strCompressFileName = T2A(strZipName);
				pTask->strExtName = T2A(PAPERS_EXT_NAME);
				pTask->strSavePath = T2A(strSavePath);
				pTask->strSrcFilePath = T2A(strSrcDirPath);
				g_fmCompressLock.lock();
				g_lCompressTask.push_back(pTask);
				g_fmCompressLock.unlock();
			}
		}
	}
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
	m_SendFileThread = new Poco::Thread;
	m_pSendFileObj = new CSendFileThread(g_strFileIP, g_nFilePort);
	m_SendFileThread->start(*m_pSendFileObj);
	m_TcpCmdThread = new Poco::Thread;
	m_pTcpCmdObj = new CTcpClient(g_strCmdIP, g_nCmdPort);
	m_pTcpCmdObj->SetMainWnd(this);
	m_TcpCmdThread->start(*m_pTcpCmdObj);

	m_pCompressThread = new Poco::Thread;
	m_pCompressObj = new CCompressThread(m_pScanMgrDlg);
	m_pCompressThread->start(*m_pCompressObj);

// 	m_pScanThread = new Poco::Thread;
// 	m_pScanThreadObj = new CScanThread;
// 	m_pScanThread->start(*m_pScanThreadObj);
}

void CScanTool3Dlg::InitCtrlPositon()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTitleH = 100;
	BITMAP bmp;
	if (m_bmpTitle.GetSafeHandle())
	{
		m_bmpTitle.GetBitmap(&bmp);
		nTitleH = bmp.bmHeight;
	}
	rcClient.top = nTitleH;
// 	rcClient.left = rcClient.left + 10;
// 	rcClient.right = rcClient.right - 10;
// 	rcClient.bottom = rcClient.bottom + 10;

	int nGap = 5;
	int nBtnW = 20;
	int nBtnH = 20;
	int nCurrLeft = cx - nBtnW * 2;
	int nCurrTop = 0;
	if (m_bmpBtnMin.GetSafeHwnd())
	{
		m_bmpBtnMin.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW);
	}
	if (m_bmpBtnExit.GetSafeHwnd())
	{
		m_bmpBtnExit.MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
	}
	if (m_bmpBtnUserPic.GetSafeHwnd())
	{
		int nW = 38;
		int nH = 38;
		nCurrLeft = cx - nW - 20;
		nCurrTop = nBtnH + (nTitleH - nBtnH) / 2 - nH / 2;
		m_bmpBtnUserPic.MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}
	if (GetDlgItem(IDC_STATIC_UserName)->GetSafeHwnd())
	{
		int nW = 100;
		int nH = 38;	//��m_bmpBtnUserPic�߶�һ��
		nCurrLeft -= (nW + nGap);
		GetDlgItem(IDC_STATIC_UserName)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}

	nCurrLeft = 110;
	nCurrTop = nTitleH / 4;
	if (GetDlgItem(IDC_STATIC_Title)->GetSafeHwnd())
	{
		int nH = nTitleH / 2 * 0.7;
		int nW = rcClient.Width() / 2;
		GetDlgItem(IDC_STATIC_Title)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrTop += nH;
	}
	if (GetDlgItem(IDC_STATIC_Title)->GetSafeHwnd())
	{
		int nH = nTitleH / 2 * 0.7;
		int nW = rcClient.Width() / 2;
		GetDlgItem(IDC_STATIC_MinTitle)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrTop += nH;
	}

	if (m_pExamInfoMgrDlg && m_pExamInfoMgrDlg->GetSafeHwnd())
		m_pExamInfoMgrDlg->MoveWindow(rcClient);
	if (m_pScanMgrDlg && m_pScanMgrDlg->GetSafeHwnd())
		m_pScanMgrDlg->MoveWindow(rcClient);
	if (m_pModifyZkzhDlg && m_pModifyZkzhDlg->GetSafeHwnd())
		m_pModifyZkzhDlg->MoveWindow(rcClient);
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
	m_SendFileThread->join();
	SAFE_RELEASE(m_pSendFileObj);
	g_eSendFileThreadExit.wait();
	SAFE_RELEASE(m_SendFileThread);

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


	SAFE_RELEASE(_pModel_);
	SAFE_RELEASE(_pCurrPapersInfo_);
}

void CScanTool3Dlg::ReleaseData()
{
	//�����б�
	g_lfmExamList.lock();
	EXAM_LIST::iterator itExam = g_lExamList.begin();
	for (; itExam != g_lExamList.end();)
	{
		pEXAMINFO pExam = *itExam;
		itExam = g_lExamList.erase(itExam);
		SAFE_RELEASE(pExam);
	}
	g_lfmExamList.unlock();

	//�����ļ��б�
	g_fmSendLock.lock();
	SENDTASKLIST::iterator itSendTask = g_lSendTask.begin();
	for (; itSendTask != g_lSendTask.end();)
	{
		pSENDTASK pTask = *itSendTask;
		itSendTask = g_lSendTask.erase(itSendTask);
		SAFE_RELEASE(pTask);
	}
	g_fmSendLock.unlock();

	//�������߳�
#ifdef TEST_MULTI_SENDER
	_fmMapSender_.lock();
	MAP_FILESENDER::iterator itSender = _mapSender_.begin();
	for (; itSender != _mapSender_.end();)
	{
		pST_SENDER pObjSender = itSender->second;
		CFileUpLoad* pUpLoad = pObjSender->pUpLoad;
		itSender = _mapSender_.erase(itSender);
		SAFE_RELEASE(pUpLoad);
		SAFE_RELEASE(pObjSender);
	}
	_fmMapSender_.unlock();
#endif

	//�ͷ�δ�������ʶ�������б�
	g_fmRecog.lock();
	RECOGTASKLIST::iterator itRecog = g_lRecogTask.begin();
	for (; itRecog != g_lRecogTask.end();)
	{
		pRECOGTASK pRecogTask = *itRecog;
		SAFE_RELEASE(pRecogTask);
		itRecog = g_lRecogTask.erase(itRecog);
	}
	g_fmRecog.unlock();

// 	SAFE_RELEASE(_pModel_);
// 	SAFE_RELEASE(_pCurrPapersInfo_);
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
	if (m_pModifyZkzhDlg)
	{
		m_pModifyZkzhDlg->DestroyWindow();
		SAFE_RELEASE(m_pModifyZkzhDlg);
	}
}

void CScanTool3Dlg::InitUI()
{
	m_bmpBtnExit.SetStateBitmap(IDB_Main_BtnClose, 0, IDB_Main_BtnClose_Hover);
	m_bmpBtnMin.SetStateBitmap(IDB_Main_BtnMin, 0, IDB_Main_BtnMin_Hover);
	m_bmpBtnUserPic.SetStateBitmap(IDB_Pic_User, 0, 0);
	
	m_bmpTitle.LoadBitmap(IDB_Main_TitleBK);
	m_bmpBkg.LoadBitmap(IDB_Main_Bk);

	SetFontSize(m_nStatusSize);

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

BOOL CScanTool3Dlg::StartGuardProcess()
{
	CString strProcessName = _T("");
	strProcessName.Format(_T("EasyTntGuardProcess.exe"));
	int nProcessID = 0;
#if 0
	if (CheckProcessExist(strProcessName, nProcessID))
	{
		HANDLE hProcess = 0;
		DWORD dwExitCode = 0;
		hProcess = ::OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_CREATE_THREAD, FALSE, nProcessID);

		LPVOID Param = VirtualAllocEx(hProcess, NULL, sizeof(DWORD), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		WriteProcessMemory(hProcess, Param, (LPVOID)&dwExitCode, sizeof(DWORD), NULL);

		HANDLE hThread = CreateRemoteThread(hProcess,
											NULL,
											NULL,
											(LPTHREAD_START_ROUTINE)ExitProcess,
											Param,
											NULL,
											NULL);
	}
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
#else
	if (!CheckProcessExist(strProcessName, nProcessID))
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
#endif
	return TRUE;
}

LRESULT CScanTool3Dlg::MsgCmdDlModel(WPARAM wParam, LPARAM lParam)
{
	HandleModel();
	return 1;
}

LRESULT CScanTool3Dlg::MsgCmdGetBmk(WPARAM wParam, LPARAM lParam)
{
	int nType = wParam;		//0--��ȡ��Ŀ��������ɣ�1--��ȡ���Ա��������
	int nGetBmkResult = lParam;
#ifdef NewBmkTest
	if (nType == 0)
	{
		if (g_lBmkStudent.size() == 0)
		{
			_bGetBmk_ = false;
			CNewMessageBox	dlg;
			dlg.setShowInfo(1, 2, "��ȡ����������ʧ��, �Ƿ������");
			dlg.DoModal();
			if (dlg.m_nResult != IDYES)
			{
				SwitchDlg(0);
				return 0;
			}
		}
	}
	else
	{
		bool bWarn = false;
		EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
		if (itFindExam != g_mapBmkMgr.end())
		{
			if (itFindExam->second.size() == 0 || nGetBmkResult == 1)	//���Ա�����Ϊ��
			{
				_bGetBmk_ = false;
				bWarn = true;
			}
		}
		else
			bWarn = true;

		if (bWarn)	// && (MessageBox(_T("��ȡ����������ʧ��, �Ƿ����?"), _T("��ʾ"), MB_YESNO) != IDYES)
		{
			CNewMessageBox	dlg;
			dlg.setShowInfo(1, 2, "��ȡ����������ʧ��, �Ƿ������");
			dlg.DoModal();
			if (dlg.m_nResult != IDYES)
			{
				SwitchDlg(0);
				return 0;
			}
		}
	}
#else
	if (g_lBmkStudent.size() == 0)
	{
		_bGetBmk_ = false;
		if (MessageBox(_T("��ȡ����������ʧ��, �Ƿ����?"), _T("��ʾ"), MB_YESNO) != IDYES)
		{
			SwitchDlg(0);
			return 0;
		}
	}
#endif

	if (_eCurrDlgType_ == Dlg_ScanRecordMgr)	//�����ǰ��ɨ���¼�����ڣ��Ͳ���Ҫ��ȥ����ģ���ˣ��Ǵ�������ֱ����ת����ǰҳ��ģ�ֻ��ȡ������ͺ�
	{
		m_pScanMgrDlg->UpdateChildDlgInfo();
		return 0;
	}
	
	if (!m_pScanMgrDlg->DownLoadModel())
	{
		CNewMessageBox	dlg;
		if (!_pCurrExam_ || !_pCurrSub_)
			dlg.setShowInfo(2, 1, "���Ի��Ŀ��ϢΪ��");
		else if (_pCurrSub_->strModelName.empty())
			dlg.setShowInfo(2, 1, "δ����ɨ��ģ��");
		dlg.DoModal();
		//�������Թ���ҳ��
		SwitchDlg(0);
	}
	return 1;
}

LRESULT CScanTool3Dlg::MsgCmdConnectFail(WPARAM wParam, LPARAM lParam)
{
	m_pScanMgrDlg->ConnectServerFail();
	return 1;
}

LRESULT CScanTool3Dlg::MSG_UpdateNotify(WPARAM wParam, LPARAM lParam)
{
	CNewMessageBox dlg;
	dlg.setShowInfo(1, 2, "���°汾���ã��Ƿ�����?");
	dlg.DoModal();
	if (dlg.m_nResult != IDYES)
		return FALSE;

	DestroyWindow();
	return TRUE;
}

void CScanTool3Dlg::SwitchDlg(int nDlg, int nChildID /*= 1*/)
{
	if (nDlg == 0)
	{
		m_pExamInfoMgrDlg->ShowWindow(SW_SHOW);
		m_pScanMgrDlg->ShowWindow(SW_HIDE);

		_eCurrDlgType_ = DLG_ExamMgr;
	}
	else if (nDlg == 1)
	{
		m_pExamInfoMgrDlg->ShowWindow(SW_HIDE);
		m_pScanMgrDlg->ShowWindow(SW_SHOW);
		m_pScanMgrDlg->InitExamData();
		if (nChildID == 4)
			m_pScanMgrDlg->SetReturnDlg(1);
		m_pScanMgrDlg->ShowChildDlg(nChildID);
	}
	else if (nDlg == 2)
	{
		m_pExamInfoMgrDlg->ShowWindow(SW_HIDE);
		m_pScanMgrDlg->ShowWindow(SW_SHOW);
		m_pScanMgrDlg->UpdateChildDlgInfo(1);

		m_pModifyZkzhDlg->ShowWindow(SW_HIDE);
	}
}

void CScanTool3Dlg::SwitchModifyZkzkDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper)
{
	m_pExamInfoMgrDlg->ShowWindow(SW_HIDE);
	m_pScanMgrDlg->ShowWindow(SW_HIDE);

	m_pModifyZkzhDlg->ReInitData(_pModel_, _pCurrPapersInfo_, pStuMgr, pShowPaper);
	m_pModifyZkzhDlg->ShowWindow(SW_SHOW);
}

BEGIN_MESSAGE_MAP(CScanTool3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_MESSAGE(MSG_CMD_DL_MODEL_OK, &CScanTool3Dlg::MsgCmdDlModel)
	ON_MESSAGE(MSG_CMD_GET_BMK_OK, &CScanTool3Dlg::MsgCmdGetBmk)
	ON_MESSAGE(MSG_CMD_CONNECTION_FAIL, &CScanTool3Dlg::MsgCmdConnectFail)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CScanTool3Dlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_Min, &CScanTool3Dlg::OnBnClickedBtnMin)
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(MSG_NOTIFY_UPDATE, CScanTool3Dlg::MSG_UpdateNotify)
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
// 	m_strTitle = SYS_BASE_NAME;
// 	m_strVersion.Format(_T("Tianyu big data scan tool %s"), SOFT_VERSION);
	m_strTitle = A2T(_strMainTitle_.c_str());
	m_strVersion.Format(_T("%s %s"), A2T(_strLitteTitle_.c_str()), SOFT_VERSION);

	m_strUserName = A2T(_strNickName_.c_str());

	SetWindowText(_T("YKLX-ScanTool GuideDlg"));

	m_pExamInfoMgrDlg = new CExamInfoMgrDlg(this);
	m_pExamInfoMgrDlg->Create(CExamInfoMgrDlg::IDD, this);
	m_pExamInfoMgrDlg->ShowWindow(SW_SHOW);

	m_pScanMgrDlg = new CScanMgrDlg(this);
	m_pScanMgrDlg->Create(CScanMgrDlg::IDD, this);
	m_pScanMgrDlg->ShowWindow(SW_HIDE);

	m_pModifyZkzhDlg = new CModifyZkzhDlg(_pModel_, _pCurrPapersInfo_, NULL);
	m_pModifyZkzhDlg->Create(CModifyZkzhDlg::IDD, this);
	m_pModifyZkzhDlg->ShowWindow(SW_HIDE);

	InitThreads();

	try
	{
		g_eGetExamList.wait(10000);
	}
	catch (Poco::TimeoutException &e)
	{
		TRACE("��ȡ�����ⳬʱ\n");
	}
	InitUI();
	m_pExamInfoMgrDlg->InitShowData();
//	GetFileAddrs();

	InitFileUpLoadList();
	InitCompressList();
//#ifndef _DEBUG
	StartGuardProcess();
//#endif
	UpdateData(FALSE);
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
	ReleaseThreads();
	ReleaseDlg();
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

bool CScanTool3Dlg::HandleModel()
{
	int nResult = m_pScanMgrDlg->SearchModel();
	if (nResult == 1)
		m_pScanMgrDlg->ShowChildDlg(2);
	else
	{
		CNewMessageBox	dlg;
		if(nResult == -1)
			dlg.setShowInfo(2, 1, "����ģ��ʧ��");
		else if(nResult == -2)
			dlg.setShowInfo(2, 1, "��ȡģ��ʧ��");
		else
			dlg.setShowInfo(2, 1, "δ����ģ��");
		dlg.DoModal();
//		AfxMessageBox(_T("��ȡģ��ʧ��"));
		SwitchDlg(0);
	}
	return nResult > 0;
}

void CScanTool3Dlg::OnBnClickedBtnClose()
{
	CNewMessageBox dlg;
	dlg.setShowInfo(1, 2, "�Ƿ�ȷ���˳�?");
	dlg.DoModal();
	if (dlg.m_nResult != IDYES)
		return ;

	SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
}

void CScanTool3Dlg::OnBnClickedBtnMin()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

LRESULT CScanTool3Dlg::OnNcHitTest(CPoint point)
{
	CRect rcWndRect;
	GetWindowRect(rcWndRect);
	//	rcWndRect.bottom = rcWndRect.top + 40;	//40
	if (rcWndRect.PtInRect(point))
		return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}

void CScanTool3Dlg::DrawBorder(CDC *pDC)
{
	CPen *pOldPen = NULL;
	CPen pPen;
	CRect rcClient(0, 0, 0, 0);
	GetClientRect(&rcClient);
	pPen.CreatePen(PS_SOLID, 1, RGB(118, 190, 254));

	pDC->SelectStockObject(NULL_BRUSH);
	pOldPen = pDC->SelectObject(&pPen);
	pDC->Rectangle(&rcClient);
	pDC->SelectObject(pOldPen);
	pPen.Detach();
}

BOOL CScanTool3Dlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	int iX, iY;
	CDC memDC;
	CRect rectClient;
	BITMAP bmp;

	iX = iY = 0;
	GetClientRect(&rectClient);

	if (memDC.CreateCompatibleDC(pDC))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_bmpTitle);
		m_bmpTitle.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, rectClient.Width(), bmp.bmHeight, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);

		int nTmpX = bmp.bmWidth;
		int nTitleH = bmp.bmHeight;

		iX = 0;
		iY = iY + nTitleH;
		pOldBmp = memDC.SelectObject(&m_bmpBkg);
		m_bmpBkg.GetBitmap(&bmp);

		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, rectClient.Width(), rectClient.Height() - nTitleH, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);
	}
	memDC.DeleteDC();

	DrawBorder(pDC);

	ReleaseDC(pDC);
	return TRUE;
}

void CScanTool3Dlg::SetFontSize(int nSize)
{
	m_fontTitle.DeleteObject();
	m_fontTitle.CreateFont(nSize, 0, 0, 0,
						   FW_BOLD, FALSE, FALSE, 0,
						   DEFAULT_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH | FF_SWISS,
						   _T("��Բ"));	//Arial
	m_fontVersion.DeleteObject();
	m_fontVersion.CreateFont(12, 0, 0, 0,
							 FW_BOLD, FALSE, FALSE, 0,
							 DEFAULT_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,
							 DEFAULT_PITCH | FF_SWISS,
							 _T("��Բ"));	//Arial
	m_fontUserName.DeleteObject();
	m_fontUserName.CreateFont(15, 0, 0, 0,
							 FW_BOLD, FALSE, FALSE, 0,
							 DEFAULT_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,
							 DEFAULT_PITCH | FF_SWISS,
							 _T("��Բ"));	//Arial
	GetDlgItem(IDC_STATIC_Title)->SetFont(&m_fontTitle);
	GetDlgItem(IDC_STATIC_MinTitle)->SetFont(&m_fontVersion);
	GetDlgItem(IDC_STATIC_UserName)->SetFont(&m_fontUserName);
}

HBRUSH CScanTool3Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_MinTitle)
	{
		pDC->SetTextColor(RGB(221, 236, 254));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_Title || CurID == IDC_STATIC_UserName)
	{
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
// 	else if (CurID == IDC_STATIC_UserName)
// 	{
// 		pDC->SetBkMode(TRANSPARENT);
// 		return (HBRUSH)GetStockObject(NULL_BRUSH);
// 	}
	return hbr;
}

void CScanTool3Dlg::GetFileAddrs()
{
	TRACE("����������ʽ���ļ��ϴ���ַ\n");

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_GET_FILE_UPLOAD_ADDR;
	pTcpTask->nPkgLen = 0;
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
}

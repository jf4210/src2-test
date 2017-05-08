
// ScanTool2Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ScanTool2Dlg.h"
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

Poco::Event			g_eTcpThreadExit;
Poco::Event			g_eSendFileThreadExit;
Poco::Event			g_eCompressThreadExit;

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


// CScanTool2Dlg ��Ϣ�������

BOOL CScanTool2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CScanTool2Dlg::OnPaint()
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
HCURSOR CScanTool2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


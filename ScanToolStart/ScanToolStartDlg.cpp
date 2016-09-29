
// ScanToolStartDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanToolStart.h"
#include "ScanToolStartDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CScanToolStartDlg �Ի���



CScanToolStartDlg::CScanToolStartDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScanToolStartDlg::IDD, pParent)
	, m_nStatusSize(24)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScanToolStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CScanToolStartDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_Simple, &CScanToolStartDlg::OnBnClickedBtnSimple)
	ON_BN_CLICKED(IDC_BTN_Specialty, &CScanToolStartDlg::OnBnClickedBtnSpecialty)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CScanToolStartDlg ��Ϣ�������

BOOL CScanToolStartDlg::OnInitDialog()
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

	TCHAR	szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	CString str = szPath;
	m_strAppPath = str.Left(str.ReverseFind('\\') + 1);

	m_colorStatus = RGB(0, 100, 0);
	SetFontSize(m_nStatusSize);
	InitCtrlPosition();

	CenterWindow();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CScanToolStartDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CScanToolStartDlg::OnPaint()
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
HCURSOR CScanToolStartDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CScanToolStartDlg::OnBnClickedBtnSimple()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	CString strComm;
	strComm.Format(_T("%s\\ScanTool_S.exe"), m_strAppPath);
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strComm, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		int nErrorCode = GetLastError();
		TRACE0("CreateProcess %s failed. ErrorCode = %d", strComm, nErrorCode);
		AfxMessageBox(_T("����ʧ�ܣ�"));
		return ;
	}
	OnOK();
}


void CScanToolStartDlg::OnBnClickedBtnSpecialty()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	CString strComm;
	strComm.Format(_T("%s\\ScanTool.exe"), m_strAppPath);
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strComm, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		int nErrorCode = GetLastError();
		TRACE0("CreateProcess %s failed. ErrorCode = %d", strComm, nErrorCode);
		AfxMessageBox(_T("����ʧ�ܣ�"));
		return;
	}
	OnOK();
}

void CScanToolStartDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nTopGap = cy / 10;	//�ϱߵļ��������������
	if (nTopGap > 50)
		nTopGap = 50;
	else if (nTopGap < 20)
		nTopGap = 20;

	const int nLeftGap = 20;		//��ߵĿհ׼��
	const int nBottomGap = 20;	//�±ߵĿհ׼��
	const int nRightGap = 20;	//�ұߵĿհ׼��
	const int nGap = 10;			//��ͨ�ؼ��ļ��
	
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	int nStaticH = 20;
	int nBtnW = (cx - nLeftGap - nRightGap - nGap) / 2;
	int nBtnH = cy - nBottomGap - nTopGap - nStaticH - nGap;

	if (GetDlgItem(IDC_STATIC_Tip)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Tip)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW * 2, nStaticH);
		nCurrentTop = nCurrentTop + nStaticH + nGap;
	}

	if (GetDlgItem(IDC_BTN_Simple)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Simple)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nCurrentLeft + nBtnW + nGap;
	}
	if (GetDlgItem(IDC_BTN_Specialty)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Specialty)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
	}
}


void CScanToolStartDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CScanToolStartDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_STATIC_Tip)->SetFont(&m_fontStatus);
}


HBRUSH CScanToolStartDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();

	if (IDC_STATIC_Tip == CurID)
	{
		pDC->SetTextColor(m_colorStatus);

		return hbr;	// hbrsh;
	}
	return hbr;
}

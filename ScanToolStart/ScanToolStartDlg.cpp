
// ScanToolStartDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanToolStart.h"
#include "ScanToolStartDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CScanToolStartDlg 对话框



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


// CScanToolStartDlg 消息处理程序

BOOL CScanToolStartDlg::OnInitDialog()
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

	TCHAR	szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	CString str = szPath;
	m_strAppPath = str.Left(str.ReverseFind('\\') + 1);

	m_colorStatus = RGB(0, 100, 0);
	SetFontSize(m_nStatusSize);
	InitCtrlPosition();

	CenterWindow();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CScanToolStartDlg::OnPaint()
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
		AfxMessageBox(_T("启动失败！"));
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
		AfxMessageBox(_T("启动失败！"));
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

	int nTopGap = cy / 10;	//上边的间隔，留给控制栏
	if (nTopGap > 50)
		nTopGap = 50;
	else if (nTopGap < 20)
		nTopGap = 20;

	const int nLeftGap = 20;		//左边的空白间隔
	const int nBottomGap = 20;	//下边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	const int nGap = 10;			//普通控件的间隔
	
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

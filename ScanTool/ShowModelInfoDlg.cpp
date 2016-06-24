// ShowModelInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ShowModelInfoDlg.h"
#include "afxdialogex.h"


// CShowModelInfoDlg 对话框

IMPLEMENT_DYNAMIC(CShowModelInfoDlg, CDialog)

CShowModelInfoDlg::CShowModelInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowModelInfoDlg::IDD, pParent)
	, m_nStatusSize(27), m_strModelName(_T("")), m_strModelDesc(_T(""))
{

}

CShowModelInfoDlg::~CShowModelInfoDlg()
{
}

void CShowModelInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_ModelName, m_strModelName);
	DDX_Text(pDX, IDC_EDIT_ModelDesc, m_strModelDesc);
}


BEGIN_MESSAGE_MAP(CShowModelInfoDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void CShowModelInfoDlg::InitUI()
{
	SetFontSize(m_nStatusSize);

	InitCtrlPosition();
}

void CShowModelInfoDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nLeftGap = 2;		//左边的空白间隔
	const int nBottomGap = 2;	//下边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nTopGap = 2;
	const int nGap = 2;			//普通控件的间隔

	int nCurrentTop = 0;
	int nStaticW = 40;
	int nStaticH = 25;
	if (GetDlgItem(IDC_STATIC_ModelTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ModelTips)->MoveWindow(nLeftGap, nTopGap, nStaticW, nStaticH);
	}
	if (GetDlgItem(IDC_STATIC_ModelName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ModelName)->MoveWindow(nLeftGap + nGap + nStaticW, nTopGap, cx - nLeftGap - nStaticW - nGap, nStaticH);
		nCurrentTop = nTopGap + nStaticH + nGap;
	}
	if (GetDlgItem(IDC_STATIC_DescTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_DescTips)->MoveWindow(nLeftGap, nCurrentTop, nStaticW, nStaticH);
		nCurrentTop = nCurrentTop + nStaticH + nGap;
	}
	int nDescH = cy - nCurrentTop - nBottomGap;
	if (GetDlgItem(IDC_EDIT_ModelDesc)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_ModelDesc)->MoveWindow(nLeftGap, nCurrentTop, cx - nLeftGap - nRightGap, nDescH);
	}
}


// CShowModelInfoDlg 消息处理程序

BOOL CShowModelInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	m_colorStatus = RGB(0, 0, 255);
	return TRUE;
}

void CShowModelInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
//	Invalidate();
}

void CShowModelInfoDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_STATIC_ModelName)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_EDIT_ModelDesc)->SetFont(&m_fontStatus);
}

HBRUSH CShowModelInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();

	if (IDC_STATIC_ModelName == CurID)
	{
		pDC->SetTextColor(m_colorStatus);

		return hbr;	// hbrsh;
	}
	if (IDC_EDIT_ModelDesc == CurID)
	{
		pDC->SetTextColor(m_colorStatus);

		return hbr;	// hbrsh;
	}
	return hbr;
}

void CShowModelInfoDlg::ShowModelInfo(pMODEL pModel)
{
	if (!pModel)
	{
		m_strModelName = _T("");
		m_strModelDesc = _T("");
		UpdateData(FALSE);
		return;
	}

	m_strModelName = pModel->strModelName;
	m_strModelDesc = pModel->strModelDesc;
	UpdateData(FALSE);
}

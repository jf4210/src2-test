// SystemSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "SystemSetDlg.h"
#include "afxdialogex.h"
#include "AdanceSetMgrDlg.h"

// CSystemSetDlg 对话框

IMPLEMENT_DYNAMIC(CSystemSetDlg, CDialog)

CSystemSetDlg::CSystemSetDlg(CWnd* pParent /*=NULL*/)
	: CBaseTabDlg(IDD_SYSTEMSETDLG, pParent)
	, m_nChkLostCorner(0), m_nRectFitInterval(4), m_nUseRectFit(0)
{

}

CSystemSetDlg::~CSystemSetDlg()
{
}

void CSystemSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseTabDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_LostCorner, m_chkLostCorner);
	DDX_Control(pDX, IDC_Chk_UseRectFit, m_chkUseRectFit);
	DDX_Text(pDX, IDC_EDIT_RectFit, m_nRectFitInterval);
}


BEGIN_MESSAGE_MAP(CSystemSetDlg, CBaseTabDlg)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHK_LostCorner, &CSystemSetDlg::OnBnClickedChkLostcorner)
	ON_BN_CLICKED(IDC_Chk_UseRectFit, &CSystemSetDlg::OnBnClickedChkUserectfit)
END_MESSAGE_MAP()


// CSystemSetDlg 消息处理程序

BOOL CSystemSetDlg::OnInitDialog()
{
	CBaseTabDlg::OnInitDialog();

	InitUI();

	return TRUE;
}

BOOL CSystemSetDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CBaseTabDlg::PreTranslateMessage(pMsg);
}

void CSystemSetDlg::InitUI()
{

}

BOOL CSystemSetDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));
	ReleaseDC(pDC);

	return CBaseTabDlg::OnEraseBkgnd(pDC);
}

HBRUSH CSystemSetDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CBaseTabDlg::OnCtlColor(pDC, pWnd, nCtlColor);
	
	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_CHK_LostCorner || CurID == IDC_Chk_UseRectFit)
	{
		HBRUSH hMYbr = ::CreateSolidBrush(RGB(255, 255, 255));	//62, 147, 254

		pDC->SetBkMode(TRANSPARENT);

		return hMYbr;
	}
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

void CSystemSetDlg::InitData(AdvanceParam& stParam)
{
	m_nChkLostCorner = stParam.nChkLostCorner;
	m_chkLostCorner.SetCheck(m_nChkLostCorner);
	m_nUseRectFit = stParam.nUseRectFit;
	m_chkUseRectFit.SetCheck(m_nUseRectFit);
	m_nRectFitInterval = stParam.nRectFitInterval;
	GetDlgItem(IDC_EDIT_RectFit)->EnableWindow(m_nUseRectFit);

	UpdateData(FALSE);
}

BOOL CSystemSetDlg::SaveParamData(AdvanceParam& stParam)
{
	UpdateData(TRUE);
	stParam.nChkLostCorner = m_nChkLostCorner;
	stParam.nUseRectFit = m_nUseRectFit;
	stParam.nRectFitInterval = m_nRectFitInterval;
	return TRUE;
}

void CSystemSetDlg::OnBnClickedChkLostcorner()
{
	m_nChkLostCorner = m_chkLostCorner.GetCheck();
}


void CSystemSetDlg::OnBnClickedChkUserectfit()
{
	m_nUseRectFit = m_chkUseRectFit.GetCheck();
	GetDlgItem(IDC_EDIT_RectFit)->EnableWindow(m_nUseRectFit);
}

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
	, m_nChkLostCorner(0), m_nRectFitInterval_SN(4), m_nUseRectFit_SN(0), m_nUseRectFit_Omr(0), m_nRectFitInterval_Omr(2)
{

}

CSystemSetDlg::~CSystemSetDlg()
{
}

void CSystemSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseTabDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_LostCorner, m_chkLostCorner);
	DDX_Control(pDX, IDC_Chk_UseRectFit_SN, m_chkUseRectFit_SN);
	DDX_Text(pDX, IDC_EDIT_RectFit_SN, m_nRectFitInterval_SN);
	DDX_Control(pDX, IDC_Chk_UseRectFit_Omr, m_chkUseRectFit_Omr);
	DDX_Text(pDX, IDC_EDIT_RectFit_Omr, m_nRectFitInterval_Omr);
	DDX_Control(pDX, IDC_SPIN_RectFit_ZKZH, m_Spin_ZkzhFit);
	DDX_Control(pDX, IDC_SPIN_RectFit_Omr, m_Spin_OmrFit);
}


BEGIN_MESSAGE_MAP(CSystemSetDlg, CBaseTabDlg)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHK_LostCorner, &CSystemSetDlg::OnBnClickedChkLostcorner)
	ON_BN_CLICKED(IDC_Chk_UseRectFit_SN, &CSystemSetDlg::OnBnClickedChkUserectfit)
	ON_BN_CLICKED(IDC_Chk_UseRectFit_Omr, &CSystemSetDlg::OnBnClickedChkUserectfitOmr)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RectFit_ZKZH, &CSystemSetDlg::OnDeltaposSpinRectfitZkzh)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RectFit_Omr, &CSystemSetDlg::OnDeltaposSpinRectfitOmr)
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
	m_Spin_ZkzhFit.SetBuddy(GetDlgItem(IDC_EDIT_RectFit_SN));
	m_Spin_ZkzhFit.SetRange(1, 10);
	m_Spin_OmrFit.SetBuddy(GetDlgItem(IDC_EDIT_RectFit_Omr));
	m_Spin_OmrFit.SetRange(1, 10);
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
	if (CurID == IDC_CHK_LostCorner || CurID == IDC_Chk_UseRectFit_SN || CurID == IDC_Chk_UseRectFit_Omr)
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
	m_nUseRectFit_SN = stParam.nUseRectFit_SN;
	m_chkUseRectFit_SN.SetCheck(m_nUseRectFit_SN);
	m_nUseRectFit_Omr = stParam.nUseRectFit_Omr;
	m_chkUseRectFit_Omr.SetCheck(m_nUseRectFit_Omr);
	m_nRectFitInterval_SN = stParam.nRectFitInterval_SN;
	m_nRectFitInterval_Omr = stParam.nRectFitInterval_Omr;
	GetDlgItem(IDC_EDIT_RectFit_SN)->EnableWindow(m_nUseRectFit_SN);
	GetDlgItem(IDC_EDIT_RectFit_Omr)->EnableWindow(m_nUseRectFit_Omr);

	UpdateData(FALSE);
}

BOOL CSystemSetDlg::SaveParamData(AdvanceParam& stParam)
{
	UpdateData(TRUE);
	stParam.nChkLostCorner = m_nChkLostCorner;
	stParam.nUseRectFit_SN = m_nUseRectFit_SN;
	stParam.nRectFitInterval_SN = m_nRectFitInterval_SN;
	stParam.nUseRectFit_Omr = m_nUseRectFit_Omr;
	stParam.nRectFitInterval_Omr = m_nRectFitInterval_Omr;
	return TRUE;
}

void CSystemSetDlg::OnBnClickedChkLostcorner()
{
	m_nChkLostCorner = m_chkLostCorner.GetCheck();
}


void CSystemSetDlg::OnBnClickedChkUserectfit()
{
	m_nUseRectFit_SN = m_chkUseRectFit_SN.GetCheck();
	GetDlgItem(IDC_EDIT_RectFit_SN)->EnableWindow(m_nUseRectFit_SN);
}


void CSystemSetDlg::OnBnClickedChkUserectfitOmr()
{
	m_nUseRectFit_Omr = m_chkUseRectFit_Omr.GetCheck();
	GetDlgItem(IDC_EDIT_RectFit_Omr)->EnableWindow(m_nUseRectFit_Omr);
}


void CSystemSetDlg::OnDeltaposSpinRectfitZkzh(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	UpdateData(true);
	if (pNMUpDown->iDelta == -1) // 如果此值为-1 , 说明点击了Spin的往下的箭头  
	{
		m_nRectFitInterval_SN--;
		if (m_nRectFitInterval_SN < 1)
			m_nRectFitInterval_SN = 1;
	}
	else if (pNMUpDown->iDelta == 1) // 如果此值为1, 说明点击了Spin的往上的箭头  
	{
		m_nRectFitInterval_SN++;
		if (m_nRectFitInterval_SN > 10)
			m_nRectFitInterval_SN = 10;
	}
	UpdateData(false);
}


void CSystemSetDlg::OnDeltaposSpinRectfitOmr(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	UpdateData(true);
	if (pNMUpDown->iDelta == -1) // 如果此值为-1 , 说明点击了Spin的往下的箭头  
	{
		m_nRectFitInterval_Omr--;
		if (m_nRectFitInterval_Omr < 1)
			m_nRectFitInterval_Omr = 1;
	}
	else if (pNMUpDown->iDelta == 1) // 如果此值为1, 说明点击了Spin的往上的箭头  
	{
		m_nRectFitInterval_Omr++;
		if (m_nRectFitInterval_Omr > 10)
			m_nRectFitInterval_Omr = 10;
	}
	UpdateData(false);
}

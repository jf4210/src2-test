// RecogParamSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "RecogParamSetDlg.h"
#include "afxdialogex.h"
#include "AdanceSetMgrDlg.h"
#include "NewMessageBox.h"

// CRecogParamSetDlg 对话框

IMPLEMENT_DYNAMIC(CRecogParamSetDlg, CDialog)

CRecogParamSetDlg::CRecogParamSetDlg(CWnd* pParent /*=NULL*/)
	: CBaseTabDlg(IDD_RECOGPARAMSETDLG, pParent)
{

}

CRecogParamSetDlg::~CRecogParamSetDlg()
{
}

void CRecogParamSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseTabDlg::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Threshold_ZKZH, m_nThresholdZkzh);
	DDX_Text(pDX, IDC_EDIT_Threshold_OMR, m_nThresholdOmr);
	DDX_Control(pDX, IDC_SPIN_Zkzh, m_Spin_Zkzh);
	DDX_Control(pDX, IDC_SPIN_Omr, m_Spin_Omr);
	DDX_Text(pDX, IDC_EDIT_Sensitivity_ZKZH, m_nSensitiveZkzh);
	DDX_Text(pDX, IDC_EDIT_Sensitivity_Omr, m_nSensitiveOmr);
	DDX_Text(pDX, IDC_EDIT_CharacterAnchorPoints, m_nCharacterAnchorPt);
	DDX_Control(pDX, IDC_SPIN_CharacterAnchorPt, m_Spin_CharacterAnchorPt);
	DDX_Control(pDX, IDC_STATIC_Sensitivity, m_groupSensitivity);
	DDX_Control(pDX, IDC_STATIC_Threshold, m_groupThreshold);
	DDX_Control(pDX, IDC_STATIC_Other, m_groupOther);
	DDX_Text(pDX, IDC_EDIT_CharacterConfidence, m_nCharacterConfidence);
	DDX_Text(pDX, IDC_EDIT_Threshold_Fix, m_nCurrentFixThreshold);
	DDX_Text(pDX, IDC_EDIT_Threshold_Gray, m_nCurrentGrayThreshold);
	DDX_Text(pDX, IDC_EDIT_Threshold_QkWj, m_nThresholdQkWj);
	DDX_Text(pDX, IDC_EDIT_Persent_Fix, m_nPersentFix);
	DDX_Text(pDX, IDC_EDIT_Persent_Gray, m_nPersentGray);
	DDX_Text(pDX, IDC_EDIT_Persent_QkWj, m_nPersentQkWj);
	DDX_Text(pDX, IDC_EDIT_Persent_ZKZH, m_nPersentZkzh);
	DDX_Text(pDX, IDC_EDIT_Persent_OMR, m_nPersentOmr);
}


BEGIN_MESSAGE_MAP(CRecogParamSetDlg, CBaseTabDlg)
	ON_BN_CLICKED(IDC_BTN_Def_Param, &CRecogParamSetDlg::OnBnClickedBtnDefParam)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Zkzh, &CRecogParamSetDlg::OnDeltaposSpinZkzh)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Omr, &CRecogParamSetDlg::OnDeltaposSpinOmr)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CharacterAnchorPt, &CRecogParamSetDlg::OnDeltaposSpinCharacteranchorpt)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CRecogParamSetDlg 消息处理程序
BOOL CRecogParamSetDlg::OnInitDialog()
{
	CBaseTabDlg::OnInitDialog();

	m_groupSensitivity.SetCatptionTextColor(RGB(0, 0, 0))
		.SetBackgroundColor(RGB(255, 255, 255));
	m_groupSensitivity.SetAlignment(SS_CENTER);
	m_groupThreshold.SetCatptionTextColor(RGB(0, 0, 0))
		.SetBackgroundColor(RGB(255, 255, 255));
	m_groupThreshold.SetAlignment(SS_CENTER);
	m_groupOther.SetCatptionTextColor(RGB(0, 0, 0))
		.SetBackgroundColor(RGB(255, 255, 255));
	m_groupOther.SetAlignment(SS_CENTER);

	m_Spin_Zkzh.SetBuddy(GetDlgItem(IDC_EDIT_Sensitivity_ZKZH));
	m_Spin_Zkzh.SetRange(1, 50);
	m_Spin_Omr.SetBuddy(GetDlgItem(IDC_EDIT_Sensitivity_Omr));
	m_Spin_Omr.SetRange(1, 50);
	m_Spin_CharacterAnchorPt.SetBuddy(GetDlgItem(IDC_EDIT_CharacterAnchorPoints));
	m_Spin_CharacterAnchorPt.SetRange(1, 20);
	
	UpdateData(FALSE);
	return TRUE;
}

BOOL CRecogParamSetDlg::PreTranslateMessage(MSG* pMsg)
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
	return CDialog::PreTranslateMessage(pMsg);
}

void CRecogParamSetDlg::InitData(AdvanceParam& stParam)
{
	m_nSensitiveZkzh	= stParam.nCurrentZkzhSensitivity;
	m_nSensitiveOmr		= stParam.nCurrentOmrSensitivity;
	m_nDefSensitiveZkzh = stParam.nDefZkzhSensitivity;
	m_nDefSensitiveOmr	= stParam.nDefOmrSensitivity;

	m_nCurrentFixThreshold = stParam.nCurrentFixThreshold;
	m_nCurrentGrayThreshold = stParam.nCurrentGrayThreshold;
	m_nThresholdQkWj	= stParam.nCurrentQkWjThreshold;
	m_nThresholdZkzh	= stParam.nCurrentZkzhThreshold;
	m_nThresholdOmr		= stParam.nCurrentOmrThreshold;
	m_nDefFixThreshold	= stParam.nDefFixThreshold;
	m_nDefGrayThreshold = stParam.nDefGrayThreshold;
	m_nDefThresholdQkWj = stParam.nDefQkWjThreshold;
	m_nDefThresholdZkzh = stParam.nDefZkzhThreshold;
	m_nDefThresholdOmr	= stParam.nDefOmrThreshold;

	m_nPersentFix	= stParam.nPersentFix;
	m_nPersentGray	= stParam.nPersentGray;
	m_nPersentQkWj	= stParam.nPersentQkWj;
	m_nPersentZkzh	= stParam.nPersentZkzh;
	m_nPersentOmr	= stParam.nPersentOmr;
	m_nDefPersentFix	= stParam.nDefPersentFix;
	m_nDefPersentGray	= stParam.nDefPersentGray;
	m_nDefPersentQkWj	= stParam.nDefPersentQkWj;
	m_nDefPersentZkzh	= stParam.nDefPersentZkzh;
	m_nDefPersentOmr	= stParam.nDefPersentOmr;


	m_nCharacterAnchorPt	= stParam.nCharacterAnchorPoint;
	m_nDefCharacterAnchorPt = stParam.nDefCharacterAnchorPoint;
	m_nCharacterConfidence	= stParam.nCharacterConfidence;
	m_nDefCharacterConfidence = stParam.nDefCharacterConfidence;

	UpdateData(FALSE);
}

BOOL CRecogParamSetDlg::SaveParamData(AdvanceParam& stParam)
{
	UpdateData(TRUE);
	if (m_nCurrentFixThreshold > 255 || m_nCurrentFixThreshold < 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "定点的识别阀值设置非法!");
		dlg.DoModal();
		return FALSE;
	}
	if (m_nCurrentGrayThreshold > 255 || m_nCurrentGrayThreshold < 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "校验点的识别阀值设置非法!");
		dlg.DoModal();
		return FALSE;
	}
	if (m_nThresholdQkWj > 255 || m_nThresholdQkWj < 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "缺考/违纪的识别阀值设置非法!");
		dlg.DoModal();
		return FALSE;
	}
	if (m_nThresholdZkzh > 255 || m_nThresholdZkzh < 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "考号的识别阀值设置非法!");
		dlg.DoModal();
		return FALSE;
	}
	if (m_nThresholdOmr > 255 || m_nThresholdOmr < 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "选择题的识别阀值设置非法!");
		dlg.DoModal();
		return FALSE;
	}
	if (m_nCharacterAnchorPt > 20 || m_nCharacterAnchorPt < 1)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "文字定位点个数设置非法!");
		dlg.DoModal();
		return FALSE;
	}

	stParam.nCurrentZkzhSensitivity = m_nSensitiveZkzh;
	stParam.nCurrentOmrSensitivity	= m_nSensitiveOmr;
	
	stParam.nCurrentFixThreshold	= m_nCurrentFixThreshold;
	stParam.nCurrentGrayThreshold	= m_nCurrentGrayThreshold;
	stParam.nCurrentQkWjThreshold	= m_nThresholdQkWj;
	stParam.nCurrentZkzhThreshold	= m_nThresholdZkzh;
	stParam.nCurrentOmrThreshold	= m_nThresholdOmr;

	stParam.nPersentFix		= m_nPersentFix;
	stParam.nPersentGray	= m_nPersentGray;
	stParam.nPersentQkWj	= m_nPersentQkWj;
	stParam.nPersentZkzh	= m_nPersentZkzh;
	stParam.nPersentOmr		= m_nPersentOmr;

	stParam.nCharacterAnchorPoint	= m_nCharacterAnchorPt;
	stParam.nCharacterConfidence	= m_nCharacterConfidence;
	return TRUE;
}

void CRecogParamSetDlg::OnBnClickedBtnDefParam()
{
	m_nSensitiveZkzh = m_nDefSensitiveZkzh;
	m_nSensitiveOmr = m_nDefSensitiveOmr;

	m_nCurrentFixThreshold = m_nDefFixThreshold;
	m_nCurrentGrayThreshold = m_nDefGrayThreshold;
	m_nThresholdZkzh	= m_nDefThresholdZkzh;
	m_nThresholdOmr		= m_nDefThresholdOmr;

	m_nPersentFix	= m_nDefPersentFix;
	m_nPersentGray	= m_nDefPersentGray;
	m_nPersentZkzh	= m_nDefPersentZkzh;
	m_nPersentOmr	= m_nDefPersentOmr;

	m_nCharacterAnchorPt = m_nDefCharacterAnchorPt;
	m_nCharacterConfidence = m_nDefCharacterConfidence;
	UpdateData(FALSE);
}

void CRecogParamSetDlg::OnDeltaposSpinZkzh(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	UpdateData(true);
	if (pNMUpDown->iDelta == -1) // 如果此值为-1 , 说明点击了Spin的往下的箭头  
	{
		m_nSensitiveZkzh--;
		if (m_nSensitiveZkzh < 1)
			m_nSensitiveZkzh = 1;
	}
	else if (pNMUpDown->iDelta == 1) // 如果此值为1, 说明点击了Spin的往上的箭头  
	{
		m_nSensitiveZkzh++;
		if (m_nSensitiveZkzh > 50)
			m_nSensitiveZkzh = 50;
	}
	UpdateData(false);
}


void CRecogParamSetDlg::OnDeltaposSpinOmr(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	UpdateData(true);
	if (pNMUpDown->iDelta == -1) // 如果此值为-1 , 说明点击了Spin的往下的箭头  
	{
		m_nSensitiveOmr--;
		if (m_nSensitiveOmr < 1)
			m_nSensitiveOmr = 1;
	}
	else if (pNMUpDown->iDelta == 1) // 如果此值为1, 说明点击了Spin的往上的箭头  
	{
		m_nSensitiveOmr++;
		if (m_nSensitiveOmr > 50)
			m_nSensitiveOmr = 50;
	}
	UpdateData(false);
}


void CRecogParamSetDlg::OnDeltaposSpinCharacteranchorpt(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	UpdateData(true);
	if (pNMUpDown->iDelta == -1) // 如果此值为-1 , 说明点击了Spin的往下的箭头  
	{
		m_nCharacterAnchorPt--;
		if (m_nCharacterAnchorPt < 1)
			m_nCharacterAnchorPt = 1;
	}
	else if (pNMUpDown->iDelta == 1) // 如果此值为1, 说明点击了Spin的往上的箭头  
	{
		m_nCharacterAnchorPt++;
		if (m_nCharacterAnchorPt > 20)
			m_nCharacterAnchorPt = 20;
	}
	UpdateData(false);
}


BOOL CRecogParamSetDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));
	ReleaseDC(pDC);

	return CBaseTabDlg::OnEraseBkgnd(pDC);
}


HBRUSH CRecogParamSetDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CBaseTabDlg::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}


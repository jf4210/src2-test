// RecogParamSetDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "RecogParamSetDlg.h"
#include "afxdialogex.h"
#include "AdanceSetMgrDlg.h"
#include "NewMessageBox.h"

// CRecogParamSetDlg �Ի���

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
}


BEGIN_MESSAGE_MAP(CRecogParamSetDlg, CBaseTabDlg)
	ON_BN_CLICKED(IDC_BTN_Def_Param, &CRecogParamSetDlg::OnBnClickedBtnDefParam)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Zkzh, &CRecogParamSetDlg::OnDeltaposSpinZkzh)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Omr, &CRecogParamSetDlg::OnDeltaposSpinOmr)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CharacterAnchorPt, &CRecogParamSetDlg::OnDeltaposSpinCharacteranchorpt)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CRecogParamSetDlg ��Ϣ��������
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

void CRecogParamSetDlg::InitData(AdvanceParam& stParam)
{
//	CAdanceSetMgrDlg* pDlg = (CAdanceSetMgrDlg*)GetParent();
	m_nSensitiveZkzh	= stParam.nCurrentZkzhSensitivity;
	m_nSensitiveOmr		= stParam.nCurrentOmrSensitivity;
	m_nDefSensitiveZkzh = stParam.nDefZkzhSensitivity;
	m_nDefSensitiveOmr	= stParam.nDefOmrSensitivity;

	m_nThresholdZkzh	= stParam.nCurrentZkzhThreshold;
	m_nThresholdOmr		= stParam.nCurrentOmrThreshold;
	m_nDefThresholdZkzh = stParam.nDefZkzhThreshold;
	m_nDefThresholdOmr	= stParam.nDefOmrThreshold;

	m_nCharacterAnchorPt	= stParam.nCharacterAnchorPoint;
	m_nDefCharacterAnchorPt = stParam.nDefCharacterAnchorPoint;
	m_nCharacterConfidence	= stParam.nCharacterConfidence;
	m_nDefCharacterConfidence = stParam.nDefCharacterConfidence;

	UpdateData(FALSE);
}

BOOL CRecogParamSetDlg::SaveParamData(AdvanceParam& stParam)
{
	UpdateData(TRUE);
	if (m_nThresholdZkzh > 255 || m_nThresholdZkzh < 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "���ŵ�ʶ��ֵ���÷Ƿ�!");
		dlg.DoModal();
		return FALSE;
	}
	if (m_nThresholdOmr > 255 || m_nThresholdOmr < 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "ѡ�����ʶ��ֵ���÷Ƿ�!");
		dlg.DoModal();
		return FALSE;
	}
	if (m_nCharacterAnchorPt > 20 || m_nCharacterAnchorPt < 1)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 1, "���ֶ�λ��������÷Ƿ�!");
		dlg.DoModal();
		return FALSE;
	}

	stParam.nCurrentZkzhSensitivity = m_nSensitiveZkzh;
	stParam.nCurrentOmrSensitivity	= m_nSensitiveOmr;
	stParam.nCurrentZkzhThreshold	= m_nThresholdZkzh;
	stParam.nCurrentOmrThreshold	= m_nThresholdOmr;

	stParam.nCharacterAnchorPoint	= m_nCharacterAnchorPt;
//	stParam.nDefCharacterAnchorPoint = m_nDefThresholdOmr;
	stParam.nCharacterConfidence	= m_nCharacterConfidence;
	return TRUE;
}

void CRecogParamSetDlg::OnBnClickedBtnDefParam()
{
	m_nSensitiveZkzh = m_nDefSensitiveZkzh;
	m_nSensitiveOmr = m_nDefSensitiveOmr;
	m_nThresholdZkzh = m_nDefThresholdZkzh;
	m_nThresholdOmr = m_nDefThresholdOmr;
	m_nCharacterAnchorPt = m_nDefCharacterAnchorPt;
	m_nCharacterConfidence = m_nDefCharacterConfidence;
	UpdateData(FALSE);
}

void CRecogParamSetDlg::OnDeltaposSpinZkzh(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	UpdateData(true);
	if (pNMUpDown->iDelta == -1) // �����ֵΪ-1 , ˵�������Spin�����µļ�ͷ  
	{
		m_nSensitiveZkzh--;
		if (m_nSensitiveZkzh < 1)
			m_nSensitiveZkzh = 1;
	}
	else if (pNMUpDown->iDelta == 1) // �����ֵΪ1, ˵�������Spin�����ϵļ�ͷ  
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
	if (pNMUpDown->iDelta == -1) // �����ֵΪ-1 , ˵�������Spin�����µļ�ͷ  
	{
		m_nSensitiveOmr--;
		if (m_nSensitiveOmr < 1)
			m_nSensitiveOmr = 1;
	}
	else if (pNMUpDown->iDelta == 1) // �����ֵΪ1, ˵�������Spin�����ϵļ�ͷ  
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
	if (pNMUpDown->iDelta == -1) // �����ֵΪ-1 , ˵�������Spin�����µļ�ͷ  
	{
		m_nCharacterAnchorPt--;
		if (m_nCharacterAnchorPt < 1)
			m_nCharacterAnchorPt = 1;
	}
	else if (pNMUpDown->iDelta == 1) // �����ֵΪ1, ˵�������Spin�����ϵļ�ͷ  
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

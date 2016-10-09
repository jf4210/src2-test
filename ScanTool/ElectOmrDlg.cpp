// ElectOmrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ElectOmrDlg.h"
#include "afxdialogex.h"


// ElectOmrDlg 对话框

IMPLEMENT_DYNAMIC(ElectOmrDlg, CDialog)

ElectOmrDlg::ElectOmrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ElectOmrDlg::IDD, pParent)
	, m_nCurrentSel(-1), m_nAllCount(0), m_nRealItem(0)
{

}

ElectOmrDlg::~ElectOmrDlg()
{
	for (int i = 0; i < m_vecElectOmrInfo.size(); i++)
	{
		pELECTOMRGROUPINFO pElectOmr = m_vecElectOmrInfo[i];
		SAFE_RELEASE(pElectOmr);
	}
}

void ElectOmrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_Group, m_comboGroup);
	DDX_Text(pDX, IDC_EDIT_All, m_nAllCount);
	DDX_Text(pDX, IDC_EDIT_Real, m_nRealItem);
}


BEGIN_MESSAGE_MAP(ElectOmrDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_New, &ElectOmrDlg::OnBnClickedBtnNew)
	ON_BN_CLICKED(IDC_BTN_DEL, &ElectOmrDlg::OnBnClickedBtnDel)
	ON_BN_CLICKED(IDC_BTN_SAVE, &ElectOmrDlg::OnBnClickedBtnSave)
	ON_CBN_SELCHANGE(IDC_COMBO_Group, &ElectOmrDlg::OnCbnSelchangeComboGroup)
END_MESSAGE_MAP()


BOOL ElectOmrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitCtrlPosition();

	return TRUE;
}

void ElectOmrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int nGap = 2;	//控件的间隔
	int nTopGap = 2;	//距离上边边缘的间隔
	int nBottomGap = 2;	//距离下边边缘的间隔
	int nLeftGap = 2;	//距离左边边缘的间隔
	int nRightGap = 2;	//距离右边边缘的间隔

	int nStaticHeight = 20;		//校验点类型Static控件高度
	int nStaticWidth = (rcClient.Width() - nLeftGap - nRightGap - nGap) / 2;
	int nBtnW = (rcClient.Width() - nLeftGap - nRightGap - 2 * nGap) / 3;
	int nBtnH = 25;

	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;
	if (GetDlgItem(IDC_BTN_New)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_New)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nCurrentLeft + nBtnW + nGap;
	}
	if (GetDlgItem(IDC_BTN_DEL)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_DEL)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nCurrentLeft + nBtnW + nGap;
	}
	if (GetDlgItem(IDC_BTN_SAVE)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_SAVE)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW, nBtnH);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nBtnH + nGap;
	}

	if (GetDlgItem(IDC_STATIC_Group)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Group)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nCurrentLeft + nStaticWidth + nGap;
	}
	if (GetDlgItem(IDC_COMBO_Group)->GetSafeHwnd())
	{
		GetDlgItem(IDC_COMBO_Group)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nBtnH + nGap;
	}
	if (GetDlgItem(IDC_STATIC_All)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_All)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nCurrentLeft + nStaticWidth + nGap;
	}
	if (GetDlgItem(IDC_EDIT_All)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_All)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nBtnH + nGap;
	}
	if (GetDlgItem(IDC_STATIC_Real)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Real)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nCurrentLeft + nStaticWidth + nGap;
	}
	if (GetDlgItem(IDC_EDIT_Real)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_Real)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nBtnH + nGap * 3;
	}

	//tips
	int nTipsW = rcClient.Width() - nLeftGap - nRightGap;
	int nTipsH = rcClient.Height() - nCurrentTop - nBottomGap;
	if (GetDlgItem(IDC_STATIC_TIPS)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TIPS)->MoveWindow(nCurrentLeft, nCurrentTop, nTipsW, nTipsH);
	}
}

void ElectOmrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

BOOL ElectOmrDlg::PreTranslateMessage(MSG* pMsg)
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


void ElectOmrDlg::OnBnClickedBtnNew()
{
	int nCount = m_comboGroup.GetCount();
	char szVal[10] = { 0 };
	sprintf_s(szVal, "%d", nCount + 1);

	USES_CONVERSION;

	m_comboGroup.InsertString(nCount, A2T(szVal));

	pELECTOMRGROUPINFO pElectOmr = new ELECTOMRGROUPINFO;
	pElectOmr->nGroupID = nCount + 1;
	m_comboGroup.SetItemDataPtr(nCount, pElectOmr);

	m_comboGroup.SetCurSel(nCount);
	m_nCurrentSel = nCount;

	m_nAllCount = pElectOmr->nAllCount;
	m_nRealItem = pElectOmr->nRealCount;
	UpdateData(FALSE);

	m_vecElectOmrInfo.push_back(pElectOmr);
}


void ElectOmrDlg::OnBnClickedBtnDel()
{
	//********	注意：如果有已经属于此组的选做题，则不能删除，需要先将那些属于此组的选项删除，然后才能删除此组信息

	pELECTOMRGROUPINFO pElectOmr = (pELECTOMRGROUPINFO)m_comboGroup.GetItemDataPtr(m_comboGroup.GetCurSel());

	m_comboGroup.DeleteString(m_comboGroup.GetCurSel());

	std::vector<pELECTOMRGROUPINFO>::iterator itElectOmr = m_vecElectOmrInfo.begin();
	for (; itElectOmr != m_vecElectOmrInfo.end(); )
	{
		if (*itElectOmr == pElectOmr)
		{
			itElectOmr = m_vecElectOmrInfo.erase(itElectOmr);
			SAFE_RELEASE(pElectOmr);
		}
		else
			itElectOmr++;
	}
}


void ElectOmrDlg::OnBnClickedBtnSave()
{
	UpdateData(TRUE);
	if (m_nRealItem == 0 || m_nAllCount == 0)
	{
		AfxMessageBox(_T("选项数量设置不合法，不能报错！"));
		return;
	}
	int nItem = m_comboGroup.GetCurSel();
	pELECTOMRGROUPINFO pElectOmr = (pELECTOMRGROUPINFO)m_comboGroup.GetItemDataPtr(nItem);

	pElectOmr->nAllCount = m_nAllCount;
	pElectOmr->nRealCount = m_nRealItem;
}

void ElectOmrDlg::OnCbnSelchangeComboGroup()
{
	if (m_nCurrentSel == m_comboGroup.GetCurSel())
		return;

	m_nCurrentSel = m_comboGroup.GetCurSel();
	pELECTOMRGROUPINFO pElectOmr = (pELECTOMRGROUPINFO)m_comboGroup.GetItemDataPtr(m_nCurrentSel);
	m_nAllCount = pElectOmr->nAllCount;
	m_nRealItem = pElectOmr->nRealCount;
	UpdateData(FALSE);
}

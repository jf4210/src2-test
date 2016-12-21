// ElectOmrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ElectOmrDlg.h"
#include "afxdialogex.h"
#include "MakeModelDlg.h"

// ElectOmrDlg 对话框

IMPLEMENT_DYNAMIC(ElectOmrDlg, CDialog)

ElectOmrDlg::ElectOmrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ElectOmrDlg::IDD, pParent)
	, m_nCurrentSel(-1), m_nAllCount(0), m_nRealItem(0), m_pCurrentGroup(NULL), m_nCurrPaperId(0)
{

}

ElectOmrDlg::~ElectOmrDlg()
{
	for (int i = 0; i < m_vecElectOmrInfoAll.size(); i++)
	{
		pELECTOMRGROUPINFO pElectOmr = m_vecElectOmrInfoAll[i];
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
	int nGroupId = (m_nCurrPaperId + 1) * 100;
	for (int i = 0; i < m_vecElectOmrInfoAll.size(); i++)
		if (m_vecElectOmrInfoAll[i]->nGroupID > nGroupId)
			nGroupId = m_vecElectOmrInfoAll[i]->nGroupID;

	char szVal[10] = { 0 };
	sprintf_s(szVal, "%d", nGroupId + 1);

	USES_CONVERSION;
	int nCount = m_comboGroup.GetCount();
	m_comboGroup.InsertString(nCount, A2T(szVal));

	pELECTOMRGROUPINFO pElectOmr = new ELECTOMRGROUPINFO;
	pElectOmr->nGroupID = nGroupId + 1;
	m_comboGroup.SetItemDataPtr(nCount, pElectOmr);

	m_comboGroup.SetCurSel(nCount);
	m_nCurrentSel = nCount;
	m_pCurrentGroup = pElectOmr;

	m_nAllCount = pElectOmr->nAllCount;
	m_nRealItem = pElectOmr->nRealCount;
	UpdateData(FALSE);

	m_vecElectOmrInfoAll.push_back(pElectOmr);
}


void ElectOmrDlg::OnBnClickedBtnDel()
{
	//********	注意：如果有已经属于此组的选做题，则不能删除，需要先将那些属于此组的选项删除，然后才能删除此组信息
	CMakeModelDlg* pDlg = (CMakeModelDlg*)GetParent();

	int nItem = m_comboGroup.GetCurSel();
	if (nItem < 0)
	{
		AfxMessageBox(_T("请先新建选项组"));
		return;
	}
	pELECTOMRGROUPINFO pElectOmr = (pELECTOMRGROUPINFO)m_comboGroup.GetItemDataPtr(nItem);

	bool bFind = false;
	for (int i = 0; i < pDlg->m_vecPaperModelInfo[pDlg->m_nCurrTabSel]->vecElectOmr.size(); i++)
	{
		if (pElectOmr->nGroupID == pDlg->m_vecPaperModelInfo[pDlg->m_nCurrTabSel]->vecElectOmr[i].sElectOmrGroupInfo.nGroupID)
		{
			bFind = true;
			break;
		}
	}
	if (bFind)
	{
		AfxMessageBox(_T("请先清除属于此组选做题的选项"));
		return;
	}

	m_comboGroup.DeleteString(m_comboGroup.GetCurSel());

	std::vector<pELECTOMRGROUPINFO>::iterator itElectOmr = m_vecElectOmrInfoReal.begin();
	for (; itElectOmr != m_vecElectOmrInfoReal.end();)
	{
		if (*itElectOmr == pElectOmr)
		{
			itElectOmr = m_vecElectOmrInfoReal.erase(itElectOmr);
		}
		else
			itElectOmr++;
	}
	m_nCurrentSel = -1;
	m_pCurrentGroup = NULL;
	m_comboGroup.SetCurSel(-1);
}

void ElectOmrDlg::OnBnClickedBtnSave()
{
	UpdateData(TRUE);
	if (m_nRealItem == 0 || m_nAllCount == 0)
	{
		AfxMessageBox(_T("选项数量设置不合法，不能保存！"));
		return;
	}
	int nItem = m_comboGroup.GetCurSel();
	if (nItem < 0)
	{
		AfxMessageBox(_T("请先新建选项组"));
		return;
	}
	pELECTOMRGROUPINFO pElectOmr = (pELECTOMRGROUPINFO)m_comboGroup.GetItemDataPtr(nItem);

	pElectOmr->nAllCount = m_nAllCount;
	pElectOmr->nRealCount = m_nRealItem;

	bool bFind = false;
	for (int i = 0; i < m_vecElectOmrInfoReal.size(); i++)
	{
		if (m_vecElectOmrInfoReal[i] == pElectOmr)
		{
			bFind = true;
			break;
		}
	}
	if (!bFind)
		m_vecElectOmrInfoReal.push_back(pElectOmr);
}

void ElectOmrDlg::OnCbnSelchangeComboGroup()
{
	if (m_nCurrentSel == m_comboGroup.GetCurSel())
		return;

	m_nCurrentSel = m_comboGroup.GetCurSel();
	pELECTOMRGROUPINFO pElectOmr = (pELECTOMRGROUPINFO)m_comboGroup.GetItemDataPtr(m_nCurrentSel);
	m_nAllCount = pElectOmr->nAllCount;
	m_nRealItem = pElectOmr->nRealCount;
	m_pCurrentGroup = pElectOmr;
	UpdateData(FALSE);
}

void ElectOmrDlg::InitGroupInfo(std::vector<ELECTOMR_QUESTION>& vecElectOmr, int nPaperId)
{
	for (int i = 0; i < m_vecElectOmrInfoAll.size(); i++)
	{
		pELECTOMRGROUPINFO pElectOmr = m_vecElectOmrInfoAll[i];
		SAFE_RELEASE(pElectOmr);
	}
	m_vecElectOmrInfoAll.clear();
	m_vecElectOmrInfoReal.clear();
	m_comboGroup.ResetContent();

	m_nCurrPaperId = nPaperId;

	for (int i = 0; i < vecElectOmr.size(); i++)
	{
		pELECTOMRGROUPINFO pElectOmr = new ELECTOMRGROUPINFO;
		pElectOmr->nGroupID = vecElectOmr[i].sElectOmrGroupInfo.nGroupID;
		pElectOmr->nAllCount = vecElectOmr[i].sElectOmrGroupInfo.nAllCount;
		pElectOmr->nRealCount = vecElectOmr[i].sElectOmrGroupInfo.nRealCount;
		m_vecElectOmrInfoAll.push_back(pElectOmr);
		m_vecElectOmrInfoReal.push_back(pElectOmr);
	}
	InitUI();
}

void ElectOmrDlg::ReleaseData()
{
	for (int i = 0; i < m_vecElectOmrInfoAll.size(); i++)
	{
		pELECTOMRGROUPINFO pElectOmr = m_vecElectOmrInfoAll[i];
		SAFE_RELEASE(pElectOmr);
	}
	m_vecElectOmrInfoAll.clear();
	m_vecElectOmrInfoReal.clear();
	m_comboGroup.ResetContent();

	InitUI();
}

void ElectOmrDlg::InitUI()
{
	USES_CONVERSION;
	for (int i = 0; i < m_vecElectOmrInfoReal.size(); i++)
	{
		char szVal[10] = { 0 };
		sprintf_s(szVal, "%d", m_vecElectOmrInfoReal[i]->nGroupID);
		
		m_comboGroup.InsertString(i, A2T(szVal));
		m_comboGroup.SetItemDataPtr(i, m_vecElectOmrInfoReal[i]);
	}
	if (m_vecElectOmrInfoReal.size())
	{
		m_comboGroup.SetCurSel(0);
		m_nCurrentSel = 0;

		m_nAllCount = m_vecElectOmrInfoReal[0]->nAllCount;
		m_nRealItem = m_vecElectOmrInfoReal[0]->nRealCount;
		m_pCurrentGroup = m_vecElectOmrInfoReal[0];
	}
	else
	{
		m_comboGroup.SetCurSel(-1);
		m_nCurrentSel = -1;

		m_nAllCount = 0;
		m_nRealItem = 0;
	}	
	UpdateData(FALSE);
}

bool ElectOmrDlg::checkValid()
{
	for (int i = 0; i < m_vecElectOmrInfoReal.size(); i++)
	{
		if (m_vecElectOmrInfoReal[i] == m_pCurrentGroup)
			return true;
	}
	return false;
}

void ElectOmrDlg::showUI(int nGroup)
{
	int nCount = m_comboGroup.GetCount();
	bool bFind = false;
	for (int i = 0; i < nCount; i++)
	{
		pELECTOMRGROUPINFO pElectOmr = (pELECTOMRGROUPINFO)m_comboGroup.GetItemDataPtr(i);
		if (pElectOmr->nGroupID == nGroup)
		{
			bFind = true;
			m_comboGroup.SetCurSel(i);
			m_nAllCount = pElectOmr->nAllCount;
			m_nRealItem = pElectOmr->nRealCount;
			m_pCurrentGroup = pElectOmr;
			UpdateData(FALSE);
			break;
		}
	}
	if (!bFind)
		m_comboGroup.SetCurSel(-1);
}

// AdanceSetMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "AdanceSetMgrDlg.h"
#include "afxdialogex.h"


// CAdanceSetMgrDlg 对话框

IMPLEMENT_DYNAMIC(CAdanceSetMgrDlg, CDialog)

CAdanceSetMgrDlg::CAdanceSetMgrDlg(pMODEL	pModel, AdvanceParam stSensitiveParam, CWnd* pParent /*=NULL*/)
	: CTipBaseDlg(IDD_ADANCESETMGRDLG, pParent)
	, m_pModel(pModel), _stSensitiveParam(stSensitiveParam), m_nCurrTabSel(0)
{

}

CAdanceSetMgrDlg::~CAdanceSetMgrDlg()
{
}

void CAdanceSetMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_bmpBtnClose);
	DDX_Control(pDX, IDC_TAB_Param, m_tabParamMgr);
}


BEGIN_MESSAGE_MAP(CAdanceSetMgrDlg, CTipBaseDlg)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CAdanceSetMgrDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDOK, &CAdanceSetMgrDlg::OnBnClickedOk)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_Param, &CAdanceSetMgrDlg::OnTcnSelchangeTabParam)
END_MESSAGE_MAP()


// CAdanceSetMgrDlg 消息处理程序
BOOL CAdanceSetMgrDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	InitUI();

	return TRUE;
}

void CAdanceSetMgrDlg::InitUI()
{
	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);

	USES_CONVERSION;
	CRect rtTab;
	m_tabParamMgr.GetClientRect(&rtTab);

	char szTabHeadName[20] = { 0 };
	strcpy_s(szTabHeadName, "扫描参数");

	m_tabParamMgr.InsertItem(0, A2T(szTabHeadName));
	
	ZeroMemory(szTabHeadName, 20);
	strcpy_s(szTabHeadName, "识别参数");
	m_tabParamMgr.InsertItem(1, A2T(szTabHeadName));

	CScanParamSetDlg* pScanSetDlg = new CScanParamSetDlg(this);
	pScanSetDlg->Create(CScanParamSetDlg::IDD, &m_tabParamMgr);
	pScanSetDlg->ShowWindow(SW_HIDE);
	pScanSetDlg->MoveWindow(&rtTab);
	pScanSetDlg->InitData(_stSensitiveParam);
	m_vecTabDlg.push_back(pScanSetDlg);
	
	CRecogParamSetDlg* pRecogParamSetDlg = new CRecogParamSetDlg(this);
	pRecogParamSetDlg->Create(CRecogParamSetDlg::IDD, &m_tabParamMgr);
	pRecogParamSetDlg->ShowWindow(SW_HIDE);
	pRecogParamSetDlg->MoveWindow(&rtTab);
	pRecogParamSetDlg->InitData(_stSensitiveParam);
	m_vecTabDlg.push_back(pRecogParamSetDlg);

	m_tabParamMgr.SetCurSel(0);
	if (m_vecTabDlg.size())
	{
		m_vecTabDlg[0]->ShowWindow(SW_SHOW);
	}
	m_nCurrTabSel = 0;

	if (m_tabParamMgr.GetSafeHwnd())
	{
		CRect rtTab;
		m_tabParamMgr.GetClientRect(&rtTab);
		int nTabHead_H = 24;		//tab控件头的高度
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
		for (int i = 0; i < m_vecTabDlg.size(); i++)
			m_vecTabDlg[i]->MoveWindow(&rtPic);
	}
}


void CAdanceSetMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 2;	//上边的间隔
	const int nBottomGap = 30;	//下边的间隔
	const int nLeftGap = 2;		//左边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	int nGap = 5;

	if (m_tabParamMgr.GetSafeHwnd())
	{
		m_tabParamMgr.MoveWindow(nLeftGap, nTopGap, cx - nLeftGap - nRightGap, cy - nTopGap - nBottomGap);
	}
}

void CAdanceSetMgrDlg::OnBnClickedBtnClose()
{
	OnCancel();
}

void CAdanceSetMgrDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	for (int i = 0; i < m_vecTabDlg.size(); i++)
		if (!m_vecTabDlg[i]->SaveParamData(_stSensitiveParam))
			return;

	CTipBaseDlg::OnOK();
}

void CAdanceSetMgrDlg::OnDestroy()
{
	CTipBaseDlg::OnDestroy();

	for (int i = 0; i < m_vecTabDlg.size(); i++)
		SAFE_RELEASE(m_vecTabDlg[i]);
	m_vecTabDlg.clear();
}


void CAdanceSetMgrDlg::OnTcnSelchangeTabParam(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	int nIndex = m_tabParamMgr.GetCurSel();
	if (nIndex >= m_vecTabDlg.size())
		return;
	
	m_nCurrTabSel = nIndex;
	m_vecTabDlg[m_nCurrTabSel]->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecTabDlg.size(); i++)
	{
		if (i != nIndex)
			m_vecTabDlg[i]->ShowWindow(SW_HIDE);
	}
}

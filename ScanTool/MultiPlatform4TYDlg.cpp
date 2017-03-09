// MultiPlatform4TYDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "MultiPlatform4TYDlg.h"
#include "afxdialogex.h"


// CMultiPlatform4TYDlg 对话框

IMPLEMENT_DYNAMIC(CMultiPlatform4TYDlg, CDialog)

CMultiPlatform4TYDlg::CMultiPlatform4TYDlg(VEC_PLATFORM_TY& vecPlatform, CWnd* pParent /*=NULL*/)
: CDialog(CMultiPlatform4TYDlg::IDD, pParent)
, _vecPlatform(vecPlatform)
, _pPlatform(NULL)
, m_nStatusSize(24)
{

}

CMultiPlatform4TYDlg::~CMultiPlatform4TYDlg()
{
}

void CMultiPlatform4TYDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Platform, m_lcPlatform);
}


BOOL CMultiPlatform4TYDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_colorStatus = RGB(255, 100, 0);
	SetFontSize(m_nStatusSize);

	m_lcPlatform.SetExtendedStyle(m_lcPlatform.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lcPlatform.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 40);
	m_lcPlatform.InsertColumn(1, _T("平台名称"), LVCFMT_CENTER, 250);
	InitData();
	
	return TRUE;
}

BEGIN_MESSAGE_MAP(CMultiPlatform4TYDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Platform, &CMultiPlatform4TYDlg::OnNMDblclkListPlatform)
END_MESSAGE_MAP()


// CMultiPlatform4TYDlg 消息处理程序


void CMultiPlatform4TYDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_STATIC_MultiPlatform)->SetFont(&m_fontStatus);
}

void CMultiPlatform4TYDlg::InitData()
{
	USES_CONVERSION;
	for (auto p : _vecPlatform)
	{
		int nCount = m_lcPlatform.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lcPlatform.InsertItem(nCount, NULL);
		m_lcPlatform.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lcPlatform.SetItemText(nCount, 1, (LPCTSTR)A2T(p->strPlatformName.c_str()));
		m_lcPlatform.SetItemData(nCount, (DWORD_PTR)p);
	}
}

void CMultiPlatform4TYDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
}


HBRUSH CMultiPlatform4TYDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);


	UINT CurID = pWnd->GetDlgCtrlID();

	if (IDC_STATIC_MultiPlatform == CurID)
	{
		pDC->SetTextColor(m_colorStatus);

		return hbr;	// hbrsh;
	}
	return hbr;
}


void CMultiPlatform4TYDlg::OnNMDblclkListPlatform(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (pNMItemActivate->iItem < 0)
		return;

	pST_PLATFORMINFO pSt = (pST_PLATFORMINFO)(DWORD_PTR)m_lcPlatform.GetItemData(pNMItemActivate->iItem);
	_pPlatform = pSt;
	CDialog::OnOK();
}

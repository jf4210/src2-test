// SetScanNumDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "SetScanNumDlg.h"
#include "afxdialogex.h"


// CSetScanNumDlg 对话框

IMPLEMENT_DYNAMIC(CSetScanNumDlg, CDialog)

CSetScanNumDlg::CSetScanNumDlg(int nScanNum, CWnd* pParent /*=NULL*/)
	: CTipBaseDlg(IDD_SETSCANNUMDLG, pParent)
	,m_nScanNum(nScanNum)
{

}

CSetScanNumDlg::~CSetScanNumDlg()
{
}

void CSetScanNumDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE_SetScanNumDlg, m_bmpBtnClose);
	DDX_Text(pDX, IDC_EDIT_ScanNum, m_nScanNum);
}


BEGIN_MESSAGE_MAP(CSetScanNumDlg, CTipBaseDlg)
	ON_BN_CLICKED(IDC_BTN_CLOSE_SetScanNumDlg, &CSetScanNumDlg::OnBnClickedBtnCloseSetscannumdlg)
	ON_BN_CLICKED(IDC_BTN_OK_SetScanNumDlg, &CSetScanNumDlg::OnBnClickedBtnOkSetscannumdlg)
END_MESSAGE_MAP()


// CSetScanNumDlg 消息处理程序

BOOL CSetScanNumDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);

	UpdateData(FALSE);
	return TRUE;
}

void CSetScanNumDlg::SetFontSize()
{
	CFont fontStatus;
	fontStatus.CreateFont(25, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("宋体"));
	GetDlgItem(IDC_STATIC)->SetFont(&fontStatus);
}


void CSetScanNumDlg::OnBnClickedBtnCloseSetscannumdlg()
{
	OnCancel();
}


void CSetScanNumDlg::OnBnClickedBtnOkSetscannumdlg()
{
	UpdateData(TRUE);
	OnOK();
}

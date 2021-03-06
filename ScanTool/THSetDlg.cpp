// THSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "THSetDlg.h"
#include "afxdialogex.h"


// CTHSetDlg 对话框

IMPLEMENT_DYNAMIC(CTHSetDlg, CDialog)

CTHSetDlg::CTHSetDlg(int nStartTH/* = 0*/, CWnd* pParent /*=NULL*/)
	: CDialog(CTHSetDlg::IDD, pParent)
	, m_nStartTH(nStartTH + 1)
{

}

CTHSetDlg::~CTHSetDlg()
{
}

void CTHSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TH, m_nStartTH);
}


BEGIN_MESSAGE_MAP(CTHSetDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTHSetDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CTHSetDlg 消息处理程序

BOOL CTHSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	UpdateData(FALSE);

	return TRUE;
}

void CTHSetDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	CDialog::OnOK();
}

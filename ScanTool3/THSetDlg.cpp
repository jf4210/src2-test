// THSetDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "THSetDlg.h"
#include "afxdialogex.h"


// CTHSetDlg �Ի���

IMPLEMENT_DYNAMIC(CTHSetDlg, CDialog)

CTHSetDlg::CTHSetDlg(int nStartTH/* = 0*/, CWnd* pParent /*=NULL*/)
: CTipBaseDlg(CTHSetDlg::IDD, pParent)
	, m_nStartTH(nStartTH + 1)
{

}

CTHSetDlg::~CTHSetDlg()
{
}

void CTHSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TH, m_nStartTH);
}


BEGIN_MESSAGE_MAP(CTHSetDlg, CTipBaseDlg)
	ON_BN_CLICKED(IDOK, &CTHSetDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CTHSetDlg ��Ϣ�������

BOOL CTHSetDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();
	UpdateData(FALSE);

	return TRUE;
}

void CTHSetDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	CTipBaseDlg::OnOK();
}

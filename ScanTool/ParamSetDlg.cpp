// ParamSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ParamSetDlg.h"
#include "afxdialogex.h"


// CParamSetDlg 对话框

IMPLEMENT_DYNAMIC(CParamSetDlg, CDialog)

CParamSetDlg::CParamSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParamSetDlg::IDD, pParent)
{

}

CParamSetDlg::~CParamSetDlg()
{
}

void CParamSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS_File, m_cFileAddr);
	DDX_Control(pDX, IDC_IPADDRESS_CMD, m_cCmdAddr);
	DDX_Text(pDX, IDC_EDIT_FilePort, m_nFilePort);
	DDX_Text(pDX, IDC_EDIT_CmdPort, m_nCmdPort);
}


BEGIN_MESSAGE_MAP(CParamSetDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_OK, &CParamSetDlg::OnBnClickedBtnOk)
END_MESSAGE_MAP()

BOOL CParamSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	int nField0 = 0, nField1 = 0, nField2 = 0, nField3 = 0;
	_stscanf(m_strFileIP, _T("%d.%d.%d.%d"), &nField0, &nField1, &nField2, &nField3);
	m_cFileAddr.SetAddress(nField0, nField1, nField2, nField3);

	nField0 = 0, nField1 = 0, nField2 = 0, nField3 = 0;
	_stscanf(m_strCmdIP, _T("%d.%d.%d.%d"), &nField0, &nField1, &nField2, &nField3);
	m_cCmdAddr.SetAddress(nField0, nField1, nField2, nField3);


	UpdateData(FALSE);
	return TRUE;
}


// CParamSetDlg 消息处理程序


void CParamSetDlg::OnBnClickedBtnOk()
{
	UpdateData(TRUE);

	BYTE nField0 = 0, nField1 = 0, nField2 = 0, nField3 = 0;
	m_cFileAddr.GetAddress(nField0, nField1, nField2, nField3);
	
	m_strFileIP.Format(_T("%d.%d.%d.%d"), nField0, nField1, nField2, nField3);

	nField0 = 0, nField1 = 0, nField2 = 0, nField3 = 0;
	m_cCmdAddr.GetAddress(nField0, nField1, nField2, nField3);
	m_strCmdIP.Format(_T("%d.%d.%d.%d"), nField0, nField1, nField2, nField3);
	OnOK();
}

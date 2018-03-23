// ChkIdentityDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ChkIdentityDlg.h"
#include "afxdialogex.h"
#include "global.h"
#include "NewMessageBox.h"

// CChkIdentityDlg 对话框

IMPLEMENT_DYNAMIC(CChkIdentityDlg, CDialog)

CChkIdentityDlg::CChkIdentityDlg(CWnd* pParent /*=NULL*/)
	: CTipBaseDlg(IDD_CHKIDENTITYDLG, pParent)
{

}

CChkIdentityDlg::~CChkIdentityDlg()
{
}

void CChkIdentityDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_bmpBtnClose);
	DDX_Control(pDX, IDOK, m_bmpBtnOK);
	DDX_Text(pDX, IDC_EDIT_User, m_strUser);
	DDX_Text(pDX, IDC_EDIT_Pwd, m_strPwd);
}


BEGIN_MESSAGE_MAP(CChkIdentityDlg, CTipBaseDlg)
	ON_BN_CLICKED(IDOK, &CChkIdentityDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CChkIdentityDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CChkIdentityDlg 消息处理程序
BOOL CChkIdentityDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	InitUI();
	return TRUE;
}

void CChkIdentityDlg::InitUI()
{
	m_bmpBtnOK.SetStateBitmap(IDB_RecordDlg_Btn, 0, IDB_RecordDlg_Btn_Hover);
	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);
}



void CChkIdentityDlg::OnBnClickedOk()
{
	USES_CONVERSION;
	UpdateData(TRUE);
	if (g_strIdentityChkUser == std::string(T2A(m_strUser)) && g_strIdentityChkPwd == std::string(T2A(m_strPwd)))
	{
		OnOK();
	}
	else
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "身份验证失败");
		dlg.DoModal();

		OnCancel();
	}
}


void CChkIdentityDlg::OnBnClickedCancel()
{
	OnCancel();
}

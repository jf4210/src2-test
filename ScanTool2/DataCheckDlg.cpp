// DataCheckDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "DataCheckDlg.h"
#include "afxdialogex.h"


// CDataCheckDlg 对话框

IMPLEMENT_DYNAMIC(CDataCheckDlg, CDialog)

CDataCheckDlg::CDataCheckDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataCheckDlg::IDD, pParent)
{

}

CDataCheckDlg::~CDataCheckDlg()
{
}

void CDataCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BOOL CDataCheckDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitCtrlPosition();

	return TRUE;
}

BOOL CDataCheckDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CDataCheckDlg, CDialog)
END_MESSAGE_MAP()


// CDataCheckDlg 消息处理程序
void CDataCheckDlg::InitCtrlPosition()
{

}


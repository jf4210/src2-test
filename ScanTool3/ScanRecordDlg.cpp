// ScanRecordDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanRecordDlg.h"
#include "afxdialogex.h"


// CScanRecordDlg �Ի���

IMPLEMENT_DYNAMIC(CScanRecordDlg, CDialog)

CScanRecordDlg::CScanRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanRecordDlg::IDD, pParent)
{

}

CScanRecordDlg::~CScanRecordDlg()
{
}

void CScanRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BOOL CScanRecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitCtrlPosition();

	return TRUE;
}

BOOL CScanRecordDlg::PreTranslateMessage(MSG* pMsg)
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

BEGIN_MESSAGE_MAP(CScanRecordDlg, CDialog)
END_MESSAGE_MAP()


// CScanRecordDlg ��Ϣ�������
void CScanRecordDlg::InitCtrlPosition()
{

}


// ShowFileTransferDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ShowFileTransferDlg.h"
#include "afxdialogex.h"
#include "global.h"

#define TIMER_PROCESS	(WM_APP + 201)

// CShowFileTransferDlg 对话框

IMPLEMENT_DYNAMIC(CShowFileTransferDlg, CDialog)

CShowFileTransferDlg::CShowFileTransferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowFileTransferDlg::IDD, pParent)
{

}

CShowFileTransferDlg::~CShowFileTransferDlg()
{
}

void CShowFileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FileTransfer, m_lFileTranser);
}


BEGIN_MESSAGE_MAP(CShowFileTransferDlg, CDialog)
	ON_NOTIFY(NM_HOVER, IDC_LIST_FileTransfer, &CShowFileTransferDlg::OnNMHoverListFiletransfer)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CShowFileTransferDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	ShowFileTransferList();
	SetTimer(TIMER_PROCESS, 1000, NULL);

	return TRUE;
}

void CShowFileTransferDlg::InitUI()
{
	m_lFileTranser.SetExtendedStyle(m_lFileTranser.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lFileTranser.InsertColumn(0, _T("文件名"), LVCFMT_CENTER, 200);
	m_lFileTranser.InsertColumn(1, _T("上传进度"), LVCFMT_CENTER, 80);
	m_lFileTranser.InsertColumn(2, _T("上传状态"), LVCFMT_CENTER, 70);


// 	CRect rc;
// 	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
// 	int sx = rc.Width();
// 	int sy = rc.Height();
// 	if (sx > MAX_DLG_WIDTH)
// 		sx = MAX_DLG_WIDTH;
// 	if (sy > MAX_DLG_HEIGHT)
// 		sy = MAX_DLG_HEIGHT;
// 
// 	MoveWindow(0, 0, sx, sy);
// 	CenterWindow();

	InitCtrlPosition();
}

void CShowFileTransferDlg::InitCtrlPosition()
{

}

// CShowFileTransferDlg 消息处理程序
void CShowFileTransferDlg::OnNMHoverListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;
}

void CShowFileTransferDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_PROCESS)
	{
		ShowFileTransferList();
	}
	CDialog::OnTimer(nIDEvent);
}

void CShowFileTransferDlg::ShowFileTransferList()
{
	m_lFileTranser.DeleteAllItems();

	USES_CONVERSION;
	SENDTASKLIST::iterator it = g_lSendTask.begin();
	for (int i = 0; it != g_lSendTask.end(); it++, i++)
	{
		char szPercent[10] = { 0 };
		sprintf_s(szPercent, "%.1f", (*it)->fSendPercent);
		char szState[20] = { 0 };
		if ((*it)->nSendState == 0)
			strcpy_s(szState, "未上传");
		else if ((*it)->nSendState == 1)
			strcpy_s(szState, "正在上传");
		else if ((*it)->nSendState == 2)
			strcpy_s(szState, "上传成功");
		else if ((*it)->nSendState == 3)
			strcpy_s(szState, "上传失败");

		m_lFileTranser.InsertItem(i, NULL);
		m_lFileTranser.SetItemText(i, 0, (LPCTSTR)A2T((*it)->strFileName.c_str()));
		m_lFileTranser.SetItemText(i, 1, (LPCTSTR)A2T(szPercent));
		m_lFileTranser.SetItemText(i, 2, (LPCTSTR)A2T(szState));
	}
}

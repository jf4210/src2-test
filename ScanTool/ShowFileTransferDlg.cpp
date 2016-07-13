// ShowFileTransferDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ShowFileTransferDlg.h"
#include "afxdialogex.h"
#include "global.h"

#define TIMER_PROCESS	(WM_APP + 201)

// CShowFileTransferDlg �Ի���

IMPLEMENT_DYNAMIC(CShowFileTransferDlg, CDialog)

CShowFileTransferDlg::CShowFileTransferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowFileTransferDlg::IDD, pParent)
	, m_nCurListItem(-1)
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
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FileTransfer, &CShowFileTransferDlg::OnNMRClickListFiletransfer)
	ON_COMMAND(ID_ReSendFile, &CShowFileTransferDlg::ReSendFile)
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
	m_lFileTranser.InsertColumn(0, _T("�ļ���"), LVCFMT_CENTER, 200);
	m_lFileTranser.InsertColumn(1, _T("�ϴ�����"), LVCFMT_CENTER, 80);
	m_lFileTranser.InsertColumn(2, _T("�ϴ�״̬"), LVCFMT_CENTER, 70);


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

// CShowFileTransferDlg ��Ϣ�������
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
			strcpy_s(szState, "δ�ϴ�");
		else if ((*it)->nSendState == 1)
			strcpy_s(szState, "�����ϴ�");
		else if ((*it)->nSendState == 2)
			strcpy_s(szState, "�ϴ��ɹ�");
		else if ((*it)->nSendState == 3)
			strcpy_s(szState, "�ϴ�ʧ��");

		m_lFileTranser.InsertItem(i, NULL);
		m_lFileTranser.SetItemText(i, 0, (LPCTSTR)A2T((*it)->strFileName.c_str()));
		m_lFileTranser.SetItemText(i, 1, (LPCTSTR)A2T(szPercent));
		m_lFileTranser.SetItemText(i, 2, (LPCTSTR)A2T(szState));
	}
}


void CShowFileTransferDlg::OnNMRClickListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (m_lFileTranser.GetSelectedCount() <= 0)
		return;

	m_lFileTranser.SetItemState(m_nCurListItem, 0, LVIS_DROPHILITED);		// ȡ��������ʾ
	m_nCurListItem = pNMItemActivate->iItem;
	m_lFileTranser.SetItemState(m_nCurListItem, LVIS_DROPHILITED, LVIS_DROPHILITED);		//������ʾһ�У�ʧȥ�����Ҳһֱ��ʾ

	//�������δ���, ��������Ӧ��ListCtrl  
	CMenu menu, *pPopup;
	menu.LoadMenu(IDR_MENU_ReSendFile);
	pPopup = menu.GetSubMenu(0);
	CPoint myPoint;
	ClientToScreen(&myPoint);
	GetCursorPos(&myPoint); //���λ��  
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);//GetParent()
}

void CShowFileTransferDlg::ReSendFile()
{

}

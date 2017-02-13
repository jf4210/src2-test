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
	, m_nCurListItem(-1), m_nStatusSize(27)
	, m_strFileChannel(_T("")), m_strCmdChannel(_T(""))
{

}

CShowFileTransferDlg::~CShowFileTransferDlg()
{
}

void CShowFileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FileTransfer, m_lFileTranser);
	DDX_Text(pDX, IDC_STATIC_CMD, m_strCmdChannel);
	DDX_Text(pDX, IDC_STATIC_FILE, m_strFileChannel);
}


BEGIN_MESSAGE_MAP(CShowFileTransferDlg, CDialog)
	ON_NOTIFY(NM_HOVER, IDC_LIST_FileTransfer, &CShowFileTransferDlg::OnNMHoverListFiletransfer)
	ON_WM_TIMER()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FileTransfer, &CShowFileTransferDlg::OnNMRClickListFiletransfer)
	ON_COMMAND(ID_ReSendFile, &CShowFileTransferDlg::ReSendFile)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL CShowFileTransferDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	ShowFileTransferList();
	
	if (g_bCmdConnect)
	{
		m_colorCmdStatus = RGB(0, 0, 255);
		m_strCmdChannel = _T("正常");
	}
	else
	{
		m_colorCmdStatus = RGB(200, 0, 0);
		m_strCmdChannel = _T("断开");
	}
	if (g_bFileConnect)
	{
		m_colorFileStatus = RGB(0, 0, 255);
		m_strFileChannel = _T("正常");
	}
	else
	{
		m_colorFileStatus = RGB(200, 0, 0);
		m_strFileChannel = _T("断开");
	}
	UpdateData(FALSE);

	SetTimer(TIMER_PROCESS, 2000, NULL);

	return TRUE;
}

void CShowFileTransferDlg::InitUI()
{
	m_lFileTranser.SetExtendedStyle(m_lFileTranser.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lFileTranser.InsertColumn(0, _T("文件名"), LVCFMT_CENTER, 210);
	m_lFileTranser.InsertColumn(1, _T("上传进度"), LVCFMT_CENTER, 75);
	m_lFileTranser.InsertColumn(2, _T("上传状态"), LVCFMT_CENTER, 70);

	SetFontSize(m_nStatusSize);
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

		if (g_bCmdConnect)
		{
			m_colorCmdStatus = RGB(0, 0, 255);
			m_strCmdChannel = _T("正常");
		}
		else
		{
			m_colorCmdStatus = RGB(200, 0, 0);
			m_strCmdChannel = _T("断开");
		}
		if (g_bFileConnect)
		{
			m_colorFileStatus = RGB(0, 0, 255);
			m_strFileChannel = _T("正常");
		}
		else
		{
			m_colorFileStatus = RGB(200, 0, 0);
			m_strFileChannel = _T("断开");
		}
		UpdateData(FALSE);
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


void CShowFileTransferDlg::OnNMRClickListFiletransfer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (m_lFileTranser.GetSelectedCount() <= 0)
		return;

//	m_lFileTranser.SetItemState(m_nCurListItem, 0, LVIS_DROPHILITED);		// 取消高亮显示
	m_nCurListItem = pNMItemActivate->iItem;
//	m_lFileTranser.SetItemState(m_nCurListItem, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行，失去焦点后也一直显示

#if 0
	//下面的这段代码, 不单单适应于ListCtrl  
	CMenu menu, *pPopup;
	menu.LoadMenu(IDR_MENU_ReSendFile);
	pPopup = menu.GetSubMenu(0);
	CPoint myPoint;
	ClientToScreen(&myPoint);
	GetCursorPos(&myPoint); //鼠标位置  
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);//GetParent()
#endif
}

void CShowFileTransferDlg::ReSendFile()
{
// 	char szFileFullPath[300] = { 0 };
// 	sprintf_s(szFileFullPath, "%s.zip", szPapersSavePath);
// 	pSENDTASK pTask = new SENDTASK;
// 	pTask->strFileName = szZipName;
// 	pTask->strPath = szFileFullPath;
// 	g_fmSendLock.lock();
// 	g_lSendTask.push_back(pTask);
// 	g_fmSendLock.unlock();
}

void CShowFileTransferDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_STATIC_CMD)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_FILE)->SetFont(&m_fontStatus);
}


HBRUSH CShowFileTransferDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();

	if (IDC_STATIC_CMD == CurID)
	{
		pDC->SetTextColor(m_colorCmdStatus);

		return hbr;	// hbrsh;
	}
	if (IDC_STATIC_FILE == CurID)
	{
		pDC->SetTextColor(m_colorFileStatus);

		return hbr;	// hbrsh;
	}
	return hbr;
}

// ScanThread.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanThread.h"

//#include "ScanCtrlDlg.h"

// CScanThread

//IMPLEMENT_DYNAMIC(CScanThread, CWinThread)
IMPLEMENT_DYNCREATE(CScanThread, CWinThread)

CScanThread::CScanThread()
: m_bTwainInit(FALSE), m_pTwainWnd(NULL)
#ifdef TEST_DLG
, m_pScanCtrlDlg(NULL)
#endif
{

}

CScanThread::~CScanThread()
{
	SAFE_RELEASE(m_pTwainWnd);
}


BEGIN_MESSAGE_MAP(CScanThread, CWinThread)
	ON_THREAD_MESSAGE(MSG_START_SCAN, StartScan)
END_MESSAGE_MAP()

BOOL CScanThread::InitInstance()
{
//	m_bAutoDelete = TRUE;
// 	CFrameWnd* pWnd = new CFrameWnd;
// 	pWnd->Create(NULL, L"CWinThread Test");
// 	pWnd->ShowWindow(SW_HIDE);
// 	pWnd->UpdateWindow();
// 	m_pMainWnd = pWnd;

	m_pTwainWnd = new CFrameWnd;
	m_pTwainWnd->Create(NULL, L"CWinThread Test");
	m_pTwainWnd->ShowWindow(SW_HIDE);
	m_pTwainWnd->UpdateWindow();
	m_pMainWnd = m_pTwainWnd;


#ifdef TEST_DLG
	m_pScanCtrlDlg = new CScanCtrlDlg(m_scanSourceArry, m_pMainWnd);
// 	m_pScanCtrlDlg->Create(CScanCtrlDlg::IDD, m_pMainWnd);
//  	m_pScanCtrlDlg->ShowWindow(SW_HIDE);
//	m_pMainWnd = m_pScanCtrlDlg;
#endif

	return TRUE;
}

int CScanThread::ExitInstance()
{
#ifdef TEST_DLG
	SAFE_RELEASE(m_pScanCtrlDlg);
#endif
	return CWinThread::ExitInstance();
}

void CScanThread::setScanPath(CString& strPath)
{
	m_strScanSavePath = strPath;
}

BOOL CScanThread::PreTranslateMessage(MSG* pMsg)
{
// 	if (ProcessMessage(*pMsg))
// 		return TRUE;
	return CWinThread::PreTranslateMessage(pMsg);
}

void CScanThread::StartScan(WPARAM wParam, LPARAM lParam)
{
	// ����TWAIN ��ʼ��ɨ������
//	ReleaseTwain();
	if (!m_bTwainInit)
	{
// 		m_bTwainInit = InitTwain(m_pTwainWnd->GetSafeHwnd());	//m_pMainWnd->GetSafeHwnd()
// 		if (!IsValidDriver())
// 		{
// 			AfxMessageBox(_T("Unable to load Twain Driver."));
// 		}
		m_scanSourceArry.RemoveAll();
		ScanSrcInit();
	}

#if 1
#ifdef TEST_DLG
// 	if(m_pScanCtrlDlg)
// 		m_pScanCtrlDlg->ShowWindow(SW_SHOW);
	if (m_pScanCtrlDlg->DoModal() != IDOK)
		return;
#endif

//	m_Source = m_scanSourceArry.GetAt(2);
	int nDuplex = 0;		//��˫��,0-����,1-˫��

	bool bShowScanSrcUI = g_bShowScanSrcUI;

	if (1)
	{
		bShowScanSrcUI = true;
// 		if (!Acquire(TWCPP_ANYCOUNT, bShowScanSrcUI))
// 		{
// 			TRACE("ɨ��ʧ��\n");
// 		}
		//		GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(TRUE);
		return;
	}

	int nSize = 1;							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3
	int nPixel = 2;							//0-�ڰף�1-�Ҷȣ�2-��ɫ
	int nResolution = 200;					//dpi: 72, 150, 200, 300

	int nNum = 0;

	if (nDuplex == 1)
		nNum *= 2;

	if (nNum == 0)
		nNum = TWCPP_ANYCOUNT;
#else
	CScanCtrlDlg dlg(m_scanSourceArry, m_pMainWnd);
	if (dlg.DoModal() != IDOK)
	{
		dlg.m_nFlags &= ~WF_CONTINUEMODAL;
		dlg.EndDialog(IDCANCEL);
		return;
	}

//	GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(FALSE);

	m_Source = m_scanSourceArry.GetAt(dlg.m_nCurrScanSrc);
	int nDuplex = dlg.m_nCurrDuplex;		//��˫��,0-����,1-˫��

	bool bShowScanSrcUI = g_bShowScanSrcUI;

	if (dlg.m_bAdvancedSetting)
	{
		bShowScanSrcUI = true;
		if (!Acquire(TWCPP_ANYCOUNT, bShowScanSrcUI))
		{
			TRACE("ɨ��ʧ��\n");
		}
//		GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(TRUE);
		return;
	}

	int nSize = 1;							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3
	int nPixel = 2;							//0-�ڰף�1-�Ҷȣ�2-��ɫ
	int nResolution = 200;					//dpi: 72, 150, 200, 300

	int nNum = dlg.m_nStudentNum;

	if (nDuplex == 1)
		nNum *= 2;

	if (nNum == 0)
		nNum = TWCPP_ANYCOUNT;

	#if 1
//		dlg.EndModalLoop(1);
//		dlg.PostMessage(WM_QUIT);
//		dlg.CloseWindow();
	#else
//		dlg.m_nFlags &= ~WF_CONTINUEMODAL;
//		::EndDialog(dlg.m_hWnd, IDOK);
	#endif

#endif
	TRACE("��ʼɨ��\n");
// 	if (!Acquire(nNum, nDuplex, nSize, nPixel, nResolution, bShowScanSrcUI))
// 	{
// 		TRACE("ɨ��ʧ��\n");
// 	}

}

BOOL CScanThread::ScanSrcInit()
{
	USES_CONVERSION;
// 	if (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &m_Source))
// 	{
// 		TW_IDENTITY temp_Source = m_Source;
// 		m_scanSourceArry.Add(temp_Source);
// 		while (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &m_Source))
// 		{
// 			TW_IDENTITY temp_Source = m_Source;
// 			m_scanSourceArry.Add(temp_Source);
// 		}
// 		m_bSourceSelected = TRUE;
// 	}
// 	else
// 	{
// 		m_bSourceSelected = FALSE;
// 	}
// 	return m_bSourceSelected;
	return TRUE;
}



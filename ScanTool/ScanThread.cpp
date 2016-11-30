// ScanThread.cpp : 实现文件
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
	if (ProcessMessage(*pMsg))
		return TRUE;
	return CWinThread::PreTranslateMessage(pMsg);
}

void CScanThread::StartScan(WPARAM wParam, LPARAM lParam)
{
	// 调用TWAIN 初始化扫描设置
//	ReleaseTwain();
	if (!m_bTwainInit)
	{
		m_bTwainInit = InitTwain(m_pTwainWnd->GetSafeHwnd());	//m_pMainWnd->GetSafeHwnd()
		if (!IsValidDriver())
		{
			AfxMessageBox(_T("Unable to load Twain Driver."));
		}
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

	m_Source = m_scanSourceArry.GetAt(2);
	int nDuplex = 0;		//单双面,0-单面,1-双面

	bool bShowScanSrcUI = g_bShowScanSrcUI;

	if (1)
	{
		bShowScanSrcUI = true;
		if (!Acquire(TWCPP_ANYCOUNT, bShowScanSrcUI))
		{
			TRACE("扫描失败\n");
		}
		//		GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(TRUE);
		return;
	}

	int nSize = 1;							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3
	int nPixel = 2;							//0-黑白，1-灰度，2-彩色
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
	int nDuplex = dlg.m_nCurrDuplex;		//单双面,0-单面,1-双面

	bool bShowScanSrcUI = g_bShowScanSrcUI;

	if (dlg.m_bAdvancedSetting)
	{
		bShowScanSrcUI = true;
		if (!Acquire(TWCPP_ANYCOUNT, bShowScanSrcUI))
		{
			TRACE("扫描失败\n");
		}
//		GetDlgItem(IDC_BTN_ScanModel)->EnableWindow(TRUE);
		return;
	}

	int nSize = 1;							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3
	int nPixel = 2;							//0-黑白，1-灰度，2-彩色
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
	TRACE("开始扫描\n");
	if (!Acquire(nNum, nDuplex, nSize, nPixel, nResolution, bShowScanSrcUI))
	{
		TRACE("扫描失败\n");
	}

}

BOOL CScanThread::ScanSrcInit()
{
	USES_CONVERSION;
	if (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &m_Source))
	{
		TW_IDENTITY temp_Source = m_Source;
		m_scanSourceArry.Add(temp_Source);
		while (CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &m_Source))
		{
			TW_IDENTITY temp_Source = m_Source;
			m_scanSourceArry.Add(temp_Source);
		}
		m_bSourceSelected = TRUE;
	}
	else
	{
		m_bSourceSelected = FALSE;
	}
	return m_bSourceSelected;
}

void CScanThread::CopyImage(HANDLE hBitmap, TW_IMAGEINFO& info)
{
	SetImage(hBitmap, info.BitsPerPixel);
}

void CScanThread::SetImage(HANDLE hBitmap, int bits)
{
	CDIB dib;
	dib.CreateFromHandle(hBitmap, bits);

	BITMAPFILEHEADER bFile;
	::ZeroMemory(&bFile, sizeof(bFile));
	memcpy((void *)&bFile.bfType, "BM", 2);
	bFile.bfSize = dib.GetDIBSize() + sizeof(bFile);
	bFile.bfOffBits = sizeof(BITMAPINFOHEADER) + dib.GetPaletteSize()*sizeof(RGBQUAD) + sizeof(BITMAPFILEHEADER);
	unsigned char *pBits = NULL;
	try
	{
		pBits = (unsigned char *)malloc(bFile.bfSize);
	}
	catch (...)
	{
		char szLog[100] = { 0 };
		sprintf_s(szLog, "获取内存失败。");
		g_pLogger->information(szLog);
		return;
	}

	memcpy(pBits, &bFile, sizeof(BITMAPFILEHEADER));
	memcpy(pBits + sizeof(BITMAPFILEHEADER), dib.m_pVoid, dib.GetDIBSize());

	BYTE *p = pBits;
	BITMAPFILEHEADER fheader;
	memcpy(&fheader, p, sizeof(BITMAPFILEHEADER));
	BITMAPINFOHEADER bmphdr;
	p += sizeof(BITMAPFILEHEADER);
	memcpy(&bmphdr, p, sizeof(BITMAPINFOHEADER));
	int w = bmphdr.biWidth;
	int h = bmphdr.biHeight;
	p = pBits + fheader.bfOffBits;

	int nChannel = (bmphdr.biBitCount == 1) ? 1 : bmphdr.biBitCount / 8;
	int depth = (bmphdr.biBitCount == 1) ? IPL_DEPTH_1U : IPL_DEPTH_8U;
	IplImage *pIpl2 = cvCreateImage(cvSize(w, h), depth, nChannel);

	int height;
	bool isLowerLeft = bmphdr.biHeight > 0;
	height = (bmphdr.biHeight > 0) ? bmphdr.biHeight : -bmphdr.biHeight;
	CopyData(pIpl2->imageData, (char*)p, bmphdr.biSizeImage, isLowerLeft, height);
	free(pBits);
	pBits = NULL;

	// 	IplImage* pIpl = DIB2IplImage(dib);
	// 	cv::Mat matTest = cv::cvarrToMat(pIpl);
	USES_CONVERSION;


	static int nModelScan = 1;
	char szPicPath[MAX_PATH] = { 0 };
	sprintf_s(szPicPath, "%s\\model%d.jpg", T2A(m_strScanSavePath), nModelScan);
	nModelScan++;


	cv::Mat matTest2 = cv::cvarrToMat(pIpl2);
	//	cv::Mat matTest3 = matTest2.clone();


	std::string strPicName = szPicPath;
	imwrite(strPicName, matTest2);

	std::string strLog = "Get model pic: " + strPicName;
	g_pLogger->information(strLog);

	cvReleaseImage(&pIpl2);
}

void CScanThread::ScanDone(int nStatus)
{
	TRACE("扫描完成\n");
//	AfxMessageBox(_T("扫描完成"));
	CString strSelect = _T("/root,");
	strSelect.Append(m_strScanSavePath);
	ShellExecute(NULL, _T("open"), _T("explorer.exe"), strSelect, NULL, SW_SHOWNORMAL);
}



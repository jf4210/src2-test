#pragma once

#include "global.h"
#include "MyCodeConvert.h"
#include "TwainCpp.h"
#include "DIB.h"
#include "bmp2ipl.h"

#ifdef TEST_SCAN_THREAD
	#include "twain/CommonTWAIN.h"
	#include "twain/twainapp.h"
#endif

#include "ScanCtrlDlg.h"
// CScanThread

#define MSG_START_SCAN	(WM_APP + 201)

#define TEST_DLG

class CScanThread : public CWinThread/*, public TwainApp*//*, public CWnd*/
{
//	DECLARE_DYNAMIC(CScanThread)
	DECLARE_DYNCREATE(CScanThread)

public:
	CScanThread();
	virtual ~CScanThread();


//	enum { IDD = IDD_TEST};

	void setScanPath(CString& strPath);


	void StartScan(WPARAM wParam, LPARAM lParam);

	//…®√Ë
	BOOL m_bTwainInit;
	CString		m_strScanSavePath;
	CArray<TW_IDENTITY, TW_IDENTITY> m_scanSourceArry;
	BOOL ScanSrcInit();

private:
	CFrameWnd* m_pTwainWnd;

#ifdef TEST_DLG
	CScanCtrlDlg*	m_pScanCtrlDlg;
#endif

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	DECLARE_MESSAGE_MAP()
};



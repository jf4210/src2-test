#pragma once

#include "global.h"
#include "MyCodeConvert.h"
#include "TwainCpp.h"
#include "DIB.h"
#include "bmp2ipl.h"
// CScanThread

#define MSG_START_SCAN	(WM_APP + 201)


class CScanThread : public CWinThread, public CTwain/*, public CWnd*/
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

	void CopyImage(HANDLE hBitmap, TW_IMAGEINFO& info);
	void SetImage(HANDLE hBitmap, int bits);
	void ScanDone(int nStatus);

private:
	CFrameWnd* m_pWnd;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	DECLARE_MESSAGE_MAP()
};



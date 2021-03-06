#pragma once
#include "global.h"
#include "CommonTWAIN.h"
#include "TwainApp.h"

#define OPENCV_TEST		//获取到图像数据后使用opencv写文件

// CScanThread
#define MSG_START_SCAN	(WM_APP + 201)
#define MSG_SCAN_DONE	(WM_APP + 202)
#define MSG_SCAN_ERR	(WM_APP + 203)

typedef struct _tagScanCtrl_
{
	bool	bShowUI;		//是否显示原始Twain界面
	int		nScannerId;
	int		nScanPixelType;	//0-黑白，1-灰度，2-彩色
	int		nScanCount;
	int		nScanDuplexenable;	//单双面扫描
	int		nScanSize;		//纸张类型
	int		nScanResolution;	//扫描DPI
	int		nAutoCut;	//自动裁剪
	int		nRotate;	//自动纠偏

	int		nSaveIndex;	//文件保存时的起始索引，如从第1张开始，第5张开始
	_tagScanCtrl_()
	{
		nScannerId = 0;
		nScanPixelType = 2;
		nScanCount = 0;
		nScanDuplexenable = 1;
		nScanSize = TWSS_A4LETTER;
		nScanResolution = 200;
		nAutoCut = 1;
		nRotate = 1;
		bShowUI = false;
		nSaveIndex = 1;
	}
}ST_SCANCTRL, *pST_SCANCTRL;

typedef struct _tagScanResult_
{
	bool		bScanOK;
	int			nState;
	IplImage*	pIpl2;
	std::string strResult;
	_tagScanResult_()
	{
		bScanOK = false;
		nState = 0;
		pIpl2 = NULL;
	}
	~_tagScanResult_()
	{
		if (pIpl2)
		{
			cvReleaseImage(&pIpl2);
			pIpl2 = NULL;
		}
	}
}ST_SCAN_RESULT, *pST_SCAN_RESULT;

class CScanThread : public CWinThread, public TwainApp
{
	DECLARE_DYNCREATE(CScanThread)

public:
	CScanThread();           // 动态创建所使用的受保护的构造函数
	virtual ~CScanThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void	StartScan(WPARAM wParam, LPARAM lParam);

	int		Scan();
	int		GetImgMemory();
	int		GetImgNative();
	void	setStop();
	void	setNotifyDlg(void* pDlg);
	void	setModelInfo(int nModelPicNums, std::string& strSavePath);
	void	resetData();
private:	
	bool	m_bStop;
	void*	m_pDlg;		//消息通知窗口
	int		m_nStartSaveIndex;		//文件保存时的起始索引
	int		m_nScanCount;
	int		m_nModelPicNums;
	std::string		m_strCurrPicSavePath;		//gb2312

	void* SaveFile(IplImage *pIpl);
	std::string ErrCode2Str(int nErr);	//错误代码转字符串
protected:
	DECLARE_MESSAGE_MAP()

private:
};



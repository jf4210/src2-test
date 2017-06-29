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
	bool		bScanOK;	//本次扫描完成？
	int			nState;
	int			nPaperId;
	int			nPicId;
	pST_PaperInfo pPaper;
	IplImage*	pIpl2;
	cv::Mat		matShowPic;
	std::string strPicName;
	std::string strPicPath;
	std::string strResult;
	_tagScanResult_()
	{
		bScanOK = false;
		nState = 0;
		nPaperId = 0;
		nPicId = 0;
		pPaper = NULL;
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

	void	ResetGlobalVal();
	void	StartScan(WPARAM wParam, LPARAM lParam);

	int		Scan();
	int		GetImgMemory();
	int		GetImgNative();
	void	setStop();
	void	setNotifyDlg(void* pDlg);
	void	setModelInfo(int nModelPicNums, std::string& strSavePath);
	void	resetData();	//重置计数器
	void	setNotifyDlgType(int n = 1);

	void	TestMode();
private:	
	bool	m_bStop;
	int		m_nNotifyDlgType;	//通知消息类型， 1-主扫描，2-扫描模板窗口
	void*	m_pDlg;		//消息通知窗口
	int		m_nStartSaveIndex;		//文件保存时的起始索引
	int		m_nModelPicNums;
	pST_PaperInfo	m_pCurrPaper;	//当前试卷
	std::string		m_strCurrPicSavePath;		//gb2312

	int		m_nDoubleScan;	//是否双面扫描

	int		CheckOrientation4Fix(cv::Mat& matSrc, int n);	//定点模式下的方向
	int		CheckOrientation4Head(cv::Mat& matSrc, int n);	//同步头模式下的方向
	int		CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan);

	void* SaveFile(IplImage *pIpl);
	std::string ErrCode2Str(int nErr);	//错误代码转字符串
protected:
	DECLARE_MESSAGE_MAP()

private:
};



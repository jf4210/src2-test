#pragma once
#include "global.h"
#include "CommonTWAIN.h"
#include "TwainApp.h"

// CScanThread
#define MSG_START_SCAN	(WM_APP + 201)
#define MSG_SCAN_DONE	(WM_APP + 202)
#define MSG_SCAN_ERR	(WM_APP + 203)

typedef struct _tagScanCtrl_
{
	bool	bShowUI;		//�Ƿ���ʾԭʼTwain����
	int		nScannerId;
	int		nScanPixelType;	//0-�ڰף�1-�Ҷȣ�2-��ɫ
	int		nScanCount;
	int		nScanDuplexenable;	//��˫��ɨ��
	int		nScanSize;		//ֽ������
	int		nScanResolution;	//ɨ��DPI
	int		nAutoCut;	//�Զ��ü�
	int		nRotate;	//�Զ���ƫ
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
	CScanThread();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CScanThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void	StartScan(WPARAM wParam, LPARAM lParam);
	int		Scan();
	int		GetImgMemory();
	int		GetImgNative();
	void	setStop();
private:	
	bool	m_bStop;
	std::string ErrCode2Str(int nErr);	//�������ת�ַ���
protected:
	DECLARE_MESSAGE_MAP()

private:
};



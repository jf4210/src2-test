// ScanThread.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanThread.h"

#include "ScanMgrDlg.h"
#include "NewMakeModelDlg.h"

#include "DSMInterface.h"
#include "TwainString.h"

#include "CTiffWriter.h"

#include "MakeModelDlg.h"
#include "OmrRecog.h"

CScanThread* g_pTWAINApp = NULL;
extern bool gUSE_CALLBACKS;    // defined in TwainApp.cpp

/**
* Callback funtion for DS.  This is a callback function that will be called by
* the source when it is ready for the application to start a scan. This
* callback needs to be registered with the DSM before it can be called.
* It is important that the application returns right away after recieving this
* message.  Set a flag and return.  Do not process the callback in this function.
*/
TW_UINT16 FAR PASCAL DSMCallback(pTW_IDENTITY _pOrigin,
								 pTW_IDENTITY _pDest,
								 TW_UINT32    _DG,
								 TW_UINT16    _DAT,
								 TW_UINT16    _MSG,
								 TW_MEMREF    _pData)
{
	UNUSEDARG(_pDest);
	UNUSEDARG(_DG);
	UNUSEDARG(_DAT);
	UNUSEDARG(_pData);

	TW_UINT16 twrc = TWRC_FAILURE;
	// _pData stores the RefCon from the when the callback was registered
	// RefCon is a TW_INT32 and can not store a pointer for 64bit

	// we are only waiting for callbacks from our datasource, so validate
	// that the originator.
	if (0 != _pOrigin
		&& 0 != g_pTWAINApp
		&& g_pTWAINApp->getDataSource()
		&& _pOrigin->Id == g_pTWAINApp->getDataSource()->Id)
	{
		switch (_MSG)
		{
			case MSG_XFERREADY:
			case MSG_CLOSEDSREQ:
			case MSG_CLOSEDSOK:
			case MSG_NULL:
				g_pTWAINApp->m_DSMessage = _MSG;
				twrc = TWRC_SUCCESS;
				break;

			default:
				TRACE("Error - Unknown message in callback routine");
				g_pTWAINApp->m_DSMessage = MSG_NULL;
				twrc = TWRC_FAILURE;
				break;
		}
	}
	TRACE("============>>>>>>	DSMCallback \n");
	// Force a refresh, so that we process the message...
//	g_pTWAINApp->RedrawWindow();

	// All done...
	return twrc;
}


// CScanThread

IMPLEMENT_DYNCREATE(CScanThread, CWinThread)

CScanThread::CScanThread():
m_bStop(false), m_pDlg(NULL), m_nStartSaveIndex(0), m_pCurrPaper(NULL), m_nNotifyDlgType(1), m_nDoubleScan(1)
{
}

CScanThread::~CScanThread()
{
	exit();
	g_pTWAINApp = NULL;
	TRACE("~CScanThread(), �߳��ͷ�\n");
}

BOOL CScanThread::InitInstance()
{
	g_pTWAINApp = this;
	return TRUE;
}

int CScanThread::ExitInstance()
{
	exit();
	g_pTWAINApp = NULL;
	TRACE("ExitInstance(), �߳��˳�\n");
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CScanThread, CWinThread)
	ON_THREAD_MESSAGE(MSG_START_SCAN, StartScan)
END_MESSAGE_MAP()

void CScanThread::StartScan(WPARAM wParam, LPARAM lParam)
{
	int nId = (int)wParam;
	pST_SCANCTRL pScanCtrl = (pST_SCANCTRL)lParam;
	ST_SCANCTRL stScanCtrl = *pScanCtrl;
	delete pScanCtrl;
	pScanCtrl = NULL;
	
	_nScanStatus_ = 1;

#ifdef Test_Data
	TestMode();
	return;
#endif

	CDialog* pDlg;
	if (m_nNotifyDlgType == 2)
	{
	#ifndef TEST_NEW_MAKEMODEL
		pDlg = (CMakeModelDlg*)m_pDlg;
	#else
		pDlg = (CNewMakeModelDlg*)m_pDlg;
	#endif
	}
	else
		pDlg = (CScanMgrDlg*)m_pDlg;

	connectDSM();
	if (m_DSMState < 3)
	{
		_nScanStatus_ = -1;
		exit();
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->bScanOK = true;
		pResult->strResult = "����ɨ��Դʧ��";
		
		pDlg->PostMessage(MSG_SCAN_ERR, (WPARAM)pResult, NULL);
		return;
	}
	m_bStop = false;

	loadDS(nId);
	if (m_pDataSource == 0)
	{
		_nScanStatus_ = -2;
		exit();
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->bScanOK = true;
		pResult->strResult = "����ɨ��Դʧ��";
		
		pDlg->PostMessage(MSG_SCAN_ERR, (WPARAM)pResult, NULL);
		return;
	}

	TW_INT16  twrc = TWRC_FAILURE;
	
#if 1
	twrc = set_CapabilityOneValue(ICAP_XFERMECH, TWSX_NATIVE, TWTY_UINT16);
//	twrc = set_CapabilityOneValue(ICAP_XFERMECH, TWSX_MEMORY, TWTY_UINT16);
#else	//���JPG
	twrc = set_CapabilityOneValue(ICAP_XFERMECH, TWSX_FILE, TWTY_UINT16);
	twrc = set_CapabilityOneValue(ICAP_IMAGEFILEFORMAT, TWFF_JFIF, TWTY_UINT16);		//TWFF_JFIF
#endif
	//˫��ɨ��
	m_nDoubleScan = stScanCtrl.nScanDuplexenable;
	twrc = set_CapabilityOneValue(CAP_DUPLEXENABLED, stScanCtrl.nScanDuplexenable, TWTY_BOOL);
	twrc = set_CapabilityOneValue(ICAP_SUPPORTEDSIZES, stScanCtrl.nScanSize, TWTY_INT16);
	twrc = set_CapabilityOneValue(ICAP_PIXELTYPE, stScanCtrl.nScanPixelType, TWTY_INT16);
	int nResolution = stScanCtrl.nScanResolution;
	twrc = set_CapabilityOneValue(ICAP_XRESOLUTION, (pTW_FIX32)&nResolution);
	twrc = set_CapabilityOneValue(ICAP_YRESOLUTION, (pTW_FIX32)&nResolution);

	twrc = set_CapabilityOneValue(ICAP_AUTOMATICBORDERDETECTION, stScanCtrl.nAutoCut, TWTY_INT16);	//�Զ��ü�
	twrc = set_CapabilityOneValue(ICAP_AUTOMATICDESKEW, stScanCtrl.nRotate, TWTY_INT16);		//�Զ���ƫ

	twrc = set_CapabilityOneValue(CAP_XFERCOUNT, stScanCtrl.nScanCount, TWTY_INT16);
#if 0
	TW_UINT16 nBright = (135 - 128) / 0.125;
	twrc = set_CapabilityOneValue(ICAP_BRIGHTNESS, nBright, TWTY_INT16);		//���ȵ���

	TW_UINT16 nContrast = (135 - 128) / 0.125;
	twrc = set_CapabilityOneValue(ICAP_CONTRAST, nContrast, TWTY_INT16);		//�Աȶȵ���
#endif

	m_DSMessage = (TW_UINT16)-1;
	HWND hWnd = AfxGetMainWnd()->m_hWnd;
//	hWnd = GetDesktopWindow();
	if (!enableDS(hWnd, stScanCtrl.bShowUI))
	{
		_nScanStatus_ = -3;
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->bScanOK = true;
		pResult->strResult = "ɨ��ʧ��";

		pDlg->PostMessage(MSG_SCAN_ERR, (WPARAM)pResult, NULL);
		return;
	}

	m_nStartSaveIndex = stScanCtrl.nSaveIndex;
	int nScanResult = 0;

	// now we have to wait until we hear something back from the DS.
	while ((TW_UINT16)-1 == m_DSMessage)
	{
		// If we are using callbacks, there is nothing to do here except sleep
		// and wait for our callback from the DS.  If we are not using them, 
		// then we have to poll the DSM.
		MSG Msg;

		if (!GetMessage((LPMSG)&Msg, NULL, 0, 0))
		{
			break;//WM_QUIT
		}
		TW_EVENT twEvent = { 0 };
		twEvent.pEvent = (TW_MEMREF)&Msg;
		twEvent.TWMessage = MSG_NULL;
		TW_UINT16  twRC = TWRC_NOTDSEVENT;
		twRC = _DSM_Entry(getAppIdentity(),
						  m_pDataSource,
						  DG_CONTROL,
						  DAT_EVENT,
						  MSG_PROCESSEVENT,
						  (TW_MEMREF)&twEvent);

		if (!gUSE_CALLBACKS && twRC == TWRC_DSEVENT)
		{
			// check for message from Source
			switch (twEvent.TWMessage)
			{
				case MSG_XFERREADY:
				case MSG_CLOSEDSREQ:
				case MSG_NULL:
					m_DSMessage = twEvent.TWMessage;
					break;

				case MSG_CLOSEDSOK:
					TRACE("\nError - MSG_CLOSEDSOK in MSG_PROCESSEVENT loop for Scan\n");
					break;

				default:
					TRACE("\nError - Unknown message in MSG_PROCESSEVENT loop for Scan\n");
					break;
			}
		}
		if (twRC != TWRC_DSEVENT)
		{
			TranslateMessage((LPMSG)&Msg);
			DispatchMessage((LPMSG)&Msg);
		}
	}

	// At this point the source has sent us a callback saying that it is ready to
	// transfer the image.

	if (m_DSMessage == MSG_XFERREADY)
	{
		// move to state 6 as a result of the data source. We can start a scan now.
		m_DSMState = 6;
		updateIMAGEINFO();
		nScanResult = Scan();
		m_DSMessage = (TW_UINT16)-1;
	}
	
	// Scan is done, disable the ds, thus moving us back to state 4 where we
	// can negotiate caps again.
	disableDS();

	exit();

//	CScanMgrDlg* pDlg = (CScanMgrDlg*)AfxGetMainWnd();
//	CScanDlg* pDlg = (CScanDlg*)m_pDlg;

	if (m_bStop)
	{
		_nScanStatus_ = 3;
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->bScanOK = true;
		pResult->nState = nScanResult;
		pResult->strResult = "ɨ���Ǳ���ֹ";
		pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);
	}
	else
	{
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->nState = nScanResult;
		if (nScanResult != 0)
		{
			_nScanStatus_ = -3;
			pResult->bScanOK = true;
			pResult->strResult = ErrCode2Str(nScanResult);
			pDlg->PostMessage(MSG_SCAN_ERR, (WPARAM)pResult, NULL);
		}
		else
		{
			_nScanStatus_ = 2;
			pResult->bScanOK = true;
			pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);
		}
	}

	return;
}

int CScanThread::Scan()
{
	TW_UINT32       mech;
	TW_CAPABILITY   Cap;
	int nResult = 0;

	Cap.Cap = ICAP_XFERMECH;
	Cap.hContainer = 0;

	if (TWCC_SUCCESS != get_CAP(Cap, MSG_GETCURRENT)
		|| !getCurrent(&Cap, mech))
	{
		TRACE("Error: could not get the transfer mechanism");
		nResult = -1;
		return nResult;
	}

	if (Cap.hContainer)
	{
		_DSM_Free(Cap.hContainer);
	}

	switch ((TW_UINT16)mech)
	{
		case TWSX_NATIVE:
//			initiateTransfer_Native();
			nResult = GetImgNative();
			break;
		case TWSX_FILE:
		{
			TW_UINT32 fileformat = TWFF_TIFF;
			Cap.Cap = ICAP_IMAGEFILEFORMAT;
			Cap.hContainer = 0;

			if (TWCC_SUCCESS == get_CAP(Cap, MSG_GETCURRENT))
			{
				getCurrent(&Cap, fileformat);
			}

			if (Cap.hContainer)
			{
				_DSM_Free(Cap.hContainer);
			}

			initiateTransfer_File((TW_UINT16)fileformat);
		}
			break;
		case TWSX_MEMORY:
//			initiateTransfer_Memory();
			nResult = GetImgMemory();
			break;
	}

	return nResult;
}

int CScanThread::GetImgMemory()
{
	PrintCMDMessage("app: Starting a TWSX_MEMORY transfer...\n");

	// For memory transfers, the FreeImage library will not be used, instead a
	// tiff will be progressively written. This method was chosen because it
	// is possible that a 4GB image could be transferred and an image of that
	// size can not fit in most systems memory.

	CTiffWriter      *pTifImg = 0;
	TW_STR255         szOutFileName;
	TW_SETUPMEMXFER   SourcesBufferSizes;   /**< Used to set up the buffer size used by memory transfer method */
	bool              bPendingXfers = true;
	TW_UINT16         twrc = TWRC_SUCCESS;
	string            strPath = m_strSavePath;

	stringstream	  ss;
	int		nState = 0;		//ɨ��״̬	-101:��ȡͼ����Ϣʧ�ܣ�-102:�����ڴ�ʧ�ܣ�-103:ͼ�������-104:�жϽ���ͼ����ʧ��
	
	// start the transfer
	while (bPendingXfers)
	{
		m_nXferNum++;

		if (!updateIMAGEINFO())
		{
			break;
		}
		// The data returned by ImageInfo can be used to determine if this image is wanted.
		// If it is not then DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER can be 
		// used to skip to the next image.
		
		// get the buffer sizes that the source wants to use
		PrintCMDMessage("app: getting the buffer sizes...\n");
		memset(&SourcesBufferSizes, 0, sizeof(SourcesBufferSizes));

		twrc = DSM_Entry(DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, (TW_MEMREF)&(SourcesBufferSizes));

		if (TWRC_SUCCESS != twrc)
		{
			printError(m_pDataSource, "Error while trying to get the buffer sizes from the source!");
			nState = -101;
			break;
		}

		// -setup a buffer to hold the strip from the data source
		// -this buffer is a template that will be used to reset the real
		// buffer before each call to get a strip.
		TW_IMAGEMEMXFER memXferBufTemplate;
		memXferBufTemplate.Compression = TWON_DONTCARE16;
		memXferBufTemplate.BytesPerRow = TWON_DONTCARE32;
		memXferBufTemplate.Columns = TWON_DONTCARE32;
		memXferBufTemplate.Rows = TWON_DONTCARE32;
		memXferBufTemplate.XOffset = TWON_DONTCARE32;
		memXferBufTemplate.YOffset = TWON_DONTCARE32;
		memXferBufTemplate.BytesWritten = TWON_DONTCARE32;

		memXferBufTemplate.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
		memXferBufTemplate.Memory.Length = SourcesBufferSizes.Preferred;

		TW_HANDLE hMem = (TW_HANDLE)_DSM_Alloc(SourcesBufferSizes.Preferred);
		if (0 == hMem)
		{
			printError(0, "Error allocating memory");
			nState = -102;
			break;
		}

		memXferBufTemplate.Memory.TheMem = (TW_MEMREF)_DSM_LockMemory(hMem);

		// this is the real buffer that will be sent to the data source
		TW_IMAGEMEMXFER memXferBuf;

		// this is set to true once one row has been successfully acquired. We have
		// to track this because we can't transition to state 7 until a row has been
		// received.
		bool bScanStarted = false;

		int nBytePerRow = (((m_ImageInfo.ImageWidth * m_ImageInfo.BitsPerPixel) + 7) / 8);

		// now that the memory has been setup, get the data from the scanner
		PrintCMDMessage("app: starting the memory transfer...\n");
		for (;;)
		{
			// reset the xfer buffer
			memcpy(&memXferBuf, &memXferBufTemplate, sizeof(memXferBufTemplate));

			// clear the row data buffer
			memset(memXferBuf.Memory.TheMem, 0, memXferBuf.Memory.Length);

			// get the row data
			twrc = DSM_Entry(DG_IMAGE, DAT_IMAGEMEMXFER, MSG_GET, (TW_MEMREF)&(memXferBuf));
			if (TWRC_SUCCESS == twrc || TWRC_XFERDONE == twrc)
			{
				if (!bScanStarted)
				{
					// the state can be changed to state 7 now that we have successfully
					// received at least one strip
					m_DSMState = 7;
					bScanStarted = true;
				}

				char* pbuf = reinterpret_cast<char*>(memXferBuf.Memory.TheMem);
				
				// write the received image data to the image file
				for (unsigned int x = 0; x < memXferBuf.Rows; ++x)
				{
					ss.write(pbuf, memXferBuf.BytesPerRow);
					pbuf += memXferBuf.BytesPerRow;
				}

				if (TWRC_XFERDONE == twrc)
				{
					int w = m_ImageInfo.ImageWidth;
					int h = m_ImageInfo.ImageLength;

					int nChannel = m_ImageInfo.SamplesPerPixel;
					int depth = (m_ImageInfo.BitsPerPixel == 1) ? IPL_DEPTH_1U : IPL_DEPTH_8U;
					try
					{
						IplImage *pIpl2 = cvCreateImage(cvSize(w, h), depth, nChannel);

						int height;
						bool isLowerLeft = m_ImageInfo.ImageLength > 0;
						height = (m_ImageInfo.ImageLength > 0) ? m_ImageInfo.ImageLength : -m_ImageInfo.ImageLength;

						int n = ss.str().length();
						TRACE("ͼƬ��Ϣ: w=%d, h=%d, area = %d, ʵ�����ݳ���n=%d,���= %d\n", w, h, w * h, n, w*h - n);
						CopyData(pIpl2->imageData, (char*)ss.str().c_str(), ss.str().length(), false, height);

						SaveFile(pIpl2);
					}
					catch (cv::Exception& exc)
					{
					}
					ss.str("");
					
					updateEXTIMAGEINFO();
					break;
				}
			}
			else if (TWRC_CANCEL == twrc)
			{
				printError(m_pDataSource, "Canceled transfer while trying to get a strip of data from the source!");
				break;
			}
			else if (TWRC_FAILURE == twrc)
			{
				printError(m_pDataSource, "Error while trying to get a strip of data from the source!");
				break;
			}
		}

		// cleanup
		if (pTifImg)
		{
			delete pTifImg;
			pTifImg = 0;
		}
		// cleanup memory used to transfer image
		_DSM_UnlockMemory(hMem);
		_DSM_Free(hMem);

		if (TWRC_XFERDONE != twrc)
		{
			// We were not able to transfer an image don't try to transfer more
			nState = -103;
			break;
		}

		// The transfer is done. Tell the source
		PrintCMDMessage("app: Checking to see if there are more images to transfer...\n");
		TW_PENDINGXFERS pendxfers;
		memset(&pendxfers, 0, sizeof(pendxfers));

		twrc = DSM_Entry(DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, (TW_MEMREF)&pendxfers);

		if (TWRC_SUCCESS == twrc)
		{
			PrintCMDMessage("app: Remaining images to transfer: %u\n", pendxfers.Count);
			if (0 == pendxfers.Count)
			{
				// nothing left to transfer, finished.
				bPendingXfers = false;
			}
		}
		else
		{
			printError(m_pDataSource, "failed to properly end the transfer");
			bPendingXfers = false;
			nState = -104;
		}

		//����ֹͣɨ����
		if (m_bStop)
		{
			nState = -100;
			break;
		}
	}

	// Check to see if we left the scan loop before we were actualy done scanning
	// This will hapen if we had an error.  Need to let the DS know we are not going 
	// to transfer more images
	if (bPendingXfers == true)
	{
		twrc = DoAbortXfer();
	}

	// adjust our state now that the scanning session is done
	m_DSMState = 5;

	PrintCMDMessage("app: DONE!\n");

	return nState;
}

int CScanThread::GetImgNative()
{
	PrintCMDMessage("app: Starting a TWSX_NATIVE transfer...\n");

	TW_STR255   szOutFileName;
	bool        bPendingXfers = true;
	TW_UINT16   twrc = TWRC_SUCCESS;
	string      strPath = m_strSavePath;
	int			nState = 0;		//ɨ��״̬	-201:�ڴ�����ʧ��
	
	while (bPendingXfers)
	{
		m_nXferNum++;
		memset(szOutFileName, 0, sizeof(szOutFileName));

		if (!updateIMAGEINFO())
		{
			break;
		}
		// The data returned by ImageInfo can be used to determine if this image is wanted.
		// If it is not then DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER can be 
		// used to skip to the next image.

		TW_MEMREF hImg = 0;

		PrintCMDMessage("app: Starting the transfer...\n");
		twrc = DSM_Entry(DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, (TW_MEMREF)&hImg);

		if (TWRC_XFERDONE == twrc)
		{
			// -Here we get a handle to a DIB. Save it to disk as a bmp. 
			// -After saving it to disk, I could open it up again using FreeImage
			// if I wanted to do more transforms on it or save it as a different format.
			PBITMAPINFOHEADER pDIB = (PBITMAPINFOHEADER)_DSM_LockMemory(hImg);

			if (0 == pDIB)
			{
				printError(m_pDataSource, "App: Unable to lock memory, transfer failed");
				nState = -201;
				break;
			}

			// Save the image to disk
			DWORD dwPaletteSize = 0;

			switch (pDIB->biBitCount)
			{
				case 1:
					dwPaletteSize = 2;
					break;
				case 8:
					dwPaletteSize = 256;
					break;
				case 24:
					break;
				default:
					assert(0); //Not going to work!
					break;
			}

			// If the driver did not fill in the biSizeImage field, then compute it
			// Each scan line of the image is aligned on a DWORD (32bit) boundary
			if (pDIB->biSizeImage == 0)
			{
				pDIB->biSizeImage = ((((pDIB->biWidth * pDIB->biBitCount) + 31) & ~31) / 8) * pDIB->biHeight;

				// If a compression scheme is used the result may infact be larger
				// Increase the size to account for this.
				if (pDIB->biCompression != 0)
				{
					pDIB->biSizeImage = (pDIB->biSizeImage * 3) / 2;
				}
			}

			int nImageSize = pDIB->biSizeImage + (sizeof(RGBQUAD)*dwPaletteSize) + sizeof(BITMAPINFOHEADER);

			BITMAPFILEHEADER bmpFIH = { 0 };
			bmpFIH.bfType = ((WORD)('M' << 8) | 'B');
			bmpFIH.bfSize = nImageSize + sizeof(BITMAPFILEHEADER);
			bmpFIH.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+(sizeof(RGBQUAD)*dwPaletteSize);
			
		#ifdef OPENCV_TEST
			int w = m_ImageInfo.ImageWidth;
			int h = m_ImageInfo.ImageLength;

			int nChannel = m_ImageInfo.SamplesPerPixel;
			int depth = (m_ImageInfo.BitsPerPixel == 1) ? IPL_DEPTH_1U : IPL_DEPTH_8U;
			try
			{
				IplImage *pIpl2 = cvCreateImage(cvSize(w, h), depth, nChannel);

				int height;
				bool isLowerLeft = m_ImageInfo.ImageLength > 0;
				height = (m_ImageInfo.ImageLength > 0) ? m_ImageInfo.ImageLength : -m_ImageInfo.ImageLength;

				char* p = (char*)pDIB + bmpFIH.bfOffBits;
				TRACE("ͼƬ��Ϣ1: w=%d, h=%d, area = %d, ʵ�����ݳ���n=%d,���= %d, channel=%d, depth=%d\n", w, h, w * h, nImageSize, w*h - nImageSize, nChannel, depth);
				TRACE("ͼƬ��Ϣ2: w=%d, h=%d, area = %d, ʵ�����ݳ���n=%d,���= %d, channel=%d, depth=%d\n", w, h, w * h, pDIB->biSizeImage, w*h - pDIB->biSizeImage, nChannel, depth);
				CopyData(pIpl2->imageData, (char*)p, pDIB->biSizeImage, isLowerLeft, height);

				SaveFile(pIpl2);
			}
			catch (...)
			{
			}
		#endif

			_DSM_UnlockMemory(hImg);
			_DSM_Free(hImg);
			pDIB = 0;

			updateEXTIMAGEINFO();

			// see if there are any more transfers to do
			PrintCMDMessage("app: Checking to see if there are more images to transfer...\n");
			TW_PENDINGXFERS pendxfers;
			memset(&pendxfers, 0, sizeof(pendxfers));

			twrc = DSM_Entry(DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, (TW_MEMREF)&pendxfers);

			if (TWRC_SUCCESS == twrc)
			{
				PrintCMDMessage("app: Remaining images to transfer: %u\n", pendxfers.Count);

				if (0 == pendxfers.Count)
				{
					// nothing left to transfer, finished.
					bPendingXfers = false;
				}
			}
			else
			{
				printError(m_pDataSource, "failed to properly end the transfer");
				bPendingXfers = false;
			}
		}
		else if (TWRC_CANCEL == twrc)
		{
			printError(m_pDataSource, "Canceled transfer image");
			nState = -202;
			break;
		}
		else if (TWRC_FAILURE == twrc)
		{
			printError(m_pDataSource, "Failed to transfer image");
			nState = -203;
			break;
		}

		//����ֹͣɨ����
		if (m_bStop)
		{
			nState = -200;
			break;
		}
	}

	// Check to see if we left the scan loop before we were actualy done scanning
	// This will hapen if we had an error.  Need to let the DS know we are not going 
	// to transfer more images
	if (bPendingXfers == true)
	{
		twrc = DoAbortXfer();
	}

	// adjust our state now that the scanning session is done
	m_DSMState = 5;

	PrintCMDMessage("app: DONE!\n");

	return nState;
}

void CScanThread::setStop()
{
	m_bStop = true;
}

void CScanThread::setNotifyDlg(void* pDlg)
{
	m_pDlg = pDlg;
}

void CScanThread::setModelInfo(int nModelPicNums, std::string& strSavePath)
{
	m_nModelPicNums = nModelPicNums;
	m_strCurrPicSavePath = strSavePath;
}

void CScanThread::resetData()
{
	_nScanCount_ = 0;
}

void CScanThread::setNotifyDlgType(int n /*= 1*/)
{
	m_nNotifyDlgType = n;
}


void sharpenImage1(const cv::Mat &image, cv::Mat &result, int nKernel)
{
	//��������ʼ���˲�ģ��
	cv::Mat kernel(nKernel, nKernel, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = 5;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//��ͼ������˲�
	cv::filter2D(image, result, image.depth(), kernel);
}

int GetRects(cv::Mat& matSrc, cv::Rect rt, pMODEL pModel, int nPic, int nOrientation, int nHead)
{
	int nResult = 0;
	std::vector<cv::Rect>RectCompList;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matSrc.cols)
		{
			rt.width = matSrc.cols - rt.x;
		}
		if (rt.br().y > matSrc.rows)
		{
			rt.height = matSrc.rows - rt.y;
		}

		cv::Mat matCompRoi;
		matCompRoi = matSrc(rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
		sharpenImage1(matCompRoi, matCompRoi, 3);

#ifdef USES_GETTHRESHOLD_ZTFB
		const int channels[1] = { 0 };
		const int histSize[1] = { 150 };
		float hranges[2] = { 0, 150 };
		const float* ranges[1];
		ranges[0] = hranges;
		cv::MatND hist;
		calcHist(&matCompRoi, 1, channels, cv::Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		float fMean = (float)nSum / nCount;		//��ֵ

		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nDevSum += pow(h - fMean, 2)*binVal;
		}
		float fStdev = sqrt(nDevSum / nCount);	//��׼��
		int nThreshold = fMean + 2 * fStdev;
		if (fStdev > fMean)
			nThreshold = fMean + fStdev;

		if (nThreshold > 150) nThreshold = 150;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);

		// 		int blockSize = 25;		//25
		// 		int constValue = 10;
		// 		cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
#else
		threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
		//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
		cv::Mat element_Anticlutter = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
		dilate(matCompRoi, matCompRoi, element_Anticlutter);
		erode(matCompRoi, matCompRoi, element_Anticlutter);

		cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));	//Size(6, 6)	��ͨ�հ׿��ʶ��
		dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//��ȡ����  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
#if 1
		//ģ��ͼ���ˮƽͬ��ͷƽ������
		RECTLIST::iterator itBegin;
		if (nHead == 1)	//���ˮƽͬ��ͷ
			itBegin = pModel->vecPaperModel[nPic]->lH_Head.begin();
		else
			itBegin = pModel->vecPaperModel[nPic]->lV_Head.begin();
		RECTINFO rcFist = *itBegin;
		RECTINFO rcSecond = *(++itBegin);

		int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
		int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

		float fPer_W, fPer_H;	//ģ��ڶ��������һ����Ŀ��ߵı�����������Сֵ����
		cv::Rect rtFirst, rtSecond;
		if (nOrientation == 1 || nOrientation == 4)
		{
			rtSecond = rcSecond.rt;
			rtFirst = rcFist.rt;
			fPer_W = 0.5;
			fPer_H = 0.25;
		}
		else if (nOrientation == 2 || nOrientation == 3)
		{
			rtSecond.width = rcSecond.rt.height;
			rtSecond.height = rcSecond.rt.width;

			rtFirst.width = rcFist.rt.height;
			rtFirst.height = rcFist.rt.width;
			fPer_W = 0.25;
			fPer_H = 0.5;
		}

		if (pModel->nType == 1)
		{
			int nMid_modelW = rcSecond.rt.width;
			int nMid_modelH = rcSecond.rt.height;
			int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
			nMidInterW = 3;
			nMidInterH = 3;
			nHeadInterW = 4;
			nHeadInterH = 4;
			nMid_minW = nMid_modelW - nMidInterW;
			nMid_maxW = nMid_modelW + nMidInterW;
			nMid_minH = nMid_modelH - nMidInterH;
			nMid_maxH = nMid_modelH + nMidInterH;

			nHead_minW = rcFist.rt.width - nHeadInterW;
			nHead_maxW = rcFist.rt.width + nHeadInterW;
			nHead_minH = rcFist.rt.height - nHeadInterH;
			nHead_maxH = rcFist.rt.height + nHeadInterH;

			// 			float fOffset = 0.1;
			// 			int nMid_modelW = rtSecond.width + 2;		//��2����Ϊ�ƾ�ģ����û�о�����߿����㣬������߿����������������Σ���Ҫ�������¸�1����λ���߿�
			// 			int nMid_modelH = rtSecond.height + 2;
			// 			if (nMid_modelW < rtFirst.width * fPer_W + 0.5)	nMid_modelW = rtFirst.width * fPer_W + 0.5;
			// 			if (nMid_modelH < rtFirst.height * fPer_H + 0.5)	nMid_modelH = rtFirst.height * fPer_H + 0.5;
			// 			nMid_minW = nMid_modelW * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			// 			nMid_maxW = nMid_modelW * (1 + fOffset * 4) + 0.5;		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			// 			nMid_minH = nMid_modelH * (1 - fOffset);				//ͬ��
			// 			nMid_maxH = nMid_modelH * (1 + fOffset * 4) + 0.5;		//ͬ��
			// 
			// 			nHead_minW = rtFirst.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			// 			nHead_maxW = rtFirst.width * (1 + fOffset * 4) + 0.5;		//ͬ��
			// 			nHead_minH = rtFirst.height * (1 - fOffset);				//ͬ��
			// 			nHead_maxH = rtFirst.height * (1 + fOffset * 4) + 0.5;		//ͬ��
		}
		else
		{
			float fOffset = 0.2;
			nMid_minW = rtSecond.width * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_maxW = rtSecond.width * (1 + fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_minH = rtSecond.height * (1 - fOffset);		//ͬ��
			nMid_maxH = rtSecond.height * (1 + fOffset);		//ͬ��

			nHead_minW = rtFirst.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nHead_maxW = rtFirst.width * (1 + fOffset);		//ͬ��
			nHead_minH = rtFirst.height * (1 - fOffset);		//ͬ��
			nHead_maxH = rtFirst.height * (1 + fOffset);		//ͬ��
		}
		TRACE("w(%d, %d), h(%d, %d)������:w(%d, %d), h(%d, %d)\n", nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, \
			  nHead_minW, nHead_maxW, nHead_minH, nHead_maxH);

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;

			if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
			{
				if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//�ų���һ�������һ�����ͬ��ͷ
				{
					TRACE("����ͬ��ͷ(%d,%d,%d,%d), Ҫ��ΧW:[%d,%d], H[%d,%d], �ο���С(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
					continue;
				}
				else
				{
					TRACE("��βͬ��ͷ(����λ��ͬ��ͷ)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
				}
			}
			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
#else
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;
			if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50 || rm.area() < 70)	//********** ��ҪѰ��һ���µķ��������˾���	********
			{
				//				TRACE("���˾���:(%d,%d,%d,%d), ���: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
				continue;
			}
			RectCompList.push_back(rm);
		}
#endif
		nResult = RectCompList.size();
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "ʶ��ͬ��ͷ�쳣: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		nResult = -1;
	}
	return nResult;
}

cv::Rect GetRectByOrientation(cv::Rect& rtPic, cv::Rect rt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Rect rtResult;
	if (nOrientation == 1)	//matSrc����
	{
		rtResult = rt;
	}
	else if (nOrientation == 2)	//matSrc��ת90��
	{
		cv::Point pt1, pt2;
		pt1.x = nH - rt.tl().y;
		pt1.y = rt.tl().x;
		pt2.x = nH - rt.br().y;
		pt2.y = rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 3)	//matSrc��ת90��
	{
		cv::Point pt1, pt2;
		pt1.x = rt.tl().y;
		pt1.y = nW - rt.tl().x;
		pt2.x = rt.br().y;
		pt2.y = nW - rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 4)	//matSrc��ת180��
	{
		cv::Point pt1, pt2;
		pt1.x = nW - rt.tl().x;
		pt1.y = nH - rt.tl().y;
		pt2.x = nW - rt.br().x;
		pt2.y = nH - rt.br().y;
		rtResult = cv::Rect(pt1, pt2);
	}
	return rtResult;
}

float GetRtDensity(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod)
{
	cv::Mat matCompRoi;
	matCompRoi = matSrc(rt);
	cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);
	cv::GaussianBlur(matCompRoi, matCompRoi, cv::Size(rcMod.nGaussKernel, rcMod.nGaussKernel), 0, 0);
	sharpenImage1(matCompRoi, matCompRoi, rcMod.nSharpKernel);

	const int channels[1] = { 0 };
	const float* ranges[1];
	const int histSize[1] = { 1 };
	float hranges[2];
	hranges[0] = g_nRecogGrayMin;
	hranges[1] = static_cast<float>(rcMod.nThresholdValue);
	ranges[0] = hranges;

	cv::MatND src_hist;
	cv::calcHist(&matCompRoi, 1, channels, cv::Mat(), src_hist, 1, histSize, ranges, false);

	float fRealVal = src_hist.at<float>(0);
	float fRealArea = rt.area();
	float fRealDensity = fRealVal / fRealArea;

	return fRealVal;
}

bool bGetMaxRect(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod, cv::Rect& rtMax)
{
	clock_t start, end;
	start = clock();

	bool bResult = false;

	int nResult = 0;
	std::vector<cv::Rect>RectCompList;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matSrc.cols)
		{
			rt.width = matSrc.cols - rt.x;
		}
		if (rt.br().y > matSrc.rows)
		{
			rt.height = matSrc.rows - rt.y;
		}

		cv::Mat matCompRoi;
		matCompRoi = matSrc(rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(rcMod.nGaussKernel, rcMod.nGaussKernel), 0, 0);
		sharpenImage1(matCompRoi, matCompRoi, rcMod.nSharpKernel);

#ifdef USES_GETTHRESHOLD_ZTFB
		const int channels[1] = { 0 };
		const int histSize[1] = { 150 };
		float hranges[2] = { 0, 150 };
		const float* ranges[1];
		ranges[0] = hranges;
		cv::MatND hist;
		calcHist(&matCompRoi, 1, channels, cv::Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		int nThreshold = 150;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		if (nCount > 0)
		{
			float fMean = (float)nSum / nCount;		//��ֵ

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);	//��׼��
			nThreshold = fMean + 2 * fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;
		}
		if (nThreshold > 150) nThreshold = 150;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, cv::THRESH_BINARY);

		// 		int blockSize = 25;		//25
		// 		int constValue = 10;
		// 		cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
#else
		threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
		//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
		cv::Mat element_Anticlutter = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
		dilate(matCompRoi, matCompRoi, element_Anticlutter);
		erode(matCompRoi, matCompRoi, element_Anticlutter);

		cv::Canny(matCompRoi, matCompRoi, 0, rcMod.nCannyKernel, 5);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(6, 6));	//Size(6, 6)	��ͨ�հ׿��ʶ��
		dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//��ȡ����  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
#if 0
		//ģ��ͼ���ˮƽͬ��ͷƽ������

		RECTLIST::iterator itBegin;
		if (nHead == 1)	//���ˮƽͬ��ͷ
			itBegin = pModel->vecPaperModel[nPic]->lH_Head.begin();
		else if (nHead == 2)
			itBegin = pModel->vecPaperModel[nPic]->lV_Head.begin();
		RECTINFO rcFist = *itBegin;
		RECTINFO rcSecond = *(++itBegin);

		int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
		int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

		float fPer_W, fPer_H;	//ģ��ڶ��������һ����Ŀ��ߵı�����������Сֵ����
		cv::Rect rtFirst, rtSecond;
		if (nOrientation == 1 || nOrientation == 4)
		{
			rtSecond = rcSecond.rt;
			rtFirst = rcFist.rt;
			fPer_W = 0.5;
			fPer_H = 0.25;
		}
		else if (nOrientation == 2 || nOrientation == 3)
		{
			rtSecond.width = rcSecond.rt.height;
			rtSecond.height = rcSecond.rt.width;

			rtFirst.width = rcFist.rt.height;
			rtFirst.height = rcFist.rt.width;
			fPer_W = 0.25;
			fPer_H = 0.5;
		}

		if (pModel->nType == 1)
		{
			int nMid_modelW = rcSecond.rt.width;
			int nMid_modelH = rcSecond.rt.height;
			int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
			nMidInterW = 3;
			nMidInterH = 3;
			nHeadInterW = 4;
			nHeadInterH = 4;
			nMid_minW = nMid_modelW - nMidInterW;
			nMid_maxW = nMid_modelW + nMidInterW;
			nMid_minH = nMid_modelH - nMidInterH;
			nMid_maxH = nMid_modelH + nMidInterH;

			nHead_minW = rcFist.rt.width - nHeadInterW;
			nHead_maxW = rcFist.rt.width + nHeadInterW;
			nHead_minH = rcFist.rt.height - nHeadInterH;
			nHead_maxH = rcFist.rt.height + nHeadInterH;
		}
		else
		{
			float fOffset = 0.2;
			nMid_minW = rtSecond.width * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_maxW = rtSecond.width * (1 + fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_minH = rtSecond.height * (1 - fOffset);		//ͬ��
			nMid_maxH = rtSecond.height * (1 + fOffset);		//ͬ��

			nHead_minW = rtFirst.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nHead_maxW = rtFirst.width * (1 + fOffset);		//ͬ��
			nHead_minH = rtFirst.height * (1 - fOffset);		//ͬ��
			nHead_maxH = rtFirst.height * (1 + fOffset);		//ͬ��
		}

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;

			if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
			{
				if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//�ų���һ�������һ�����ͬ��ͷ
				{
					TRACE("����ͬ��ͷ(%d,%d,%d,%d), Ҫ��ΧW:[%d,%d], H[%d,%d], �ο���С(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
					continue;
				}
				else
				{
					TRACE("��βͬ��ͷ(����λ��ͬ��ͷ)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
				}
			}
			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
#else
		int nMaxArea = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			cv::Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;
			if (rm.area() > nMaxArea)
			{
				rtMax = rm;
				nMaxArea = rm.area();
			}
		}
#endif
		if (nMaxArea > 0)
			bResult = true;
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "ʶ��У�������쳣: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		nResult = -1;
	}
	end = clock();
	TRACE("�����������ʱ��: %d\n", end - start);

	return bResult;
}

int CScanThread::CheckOrientation4Head(cv::Mat& matSrc, int n)
{
	bool bFind = false;
	int nResult = 1;	//1:���򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180

	if (!_pModel_->nHasHead)
		return nResult;

	const float fMinPer = 0.5;		//ʶ�������/ģ������� ������Сֵ����Ϊ���ϸ�
	const float fMaxPer = 1.5;		//ʶ�������/ģ������� �������ֵ����Ϊ���ϸ�
	const float fMidPer = 0.8;

	cv::Rect rtModelPic;
	rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
	rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matSrc.cols;
	rtSrcPic.height = matSrc.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matSrc.cols / matSrc.rows;

	cv::Rect rt1 = _pModel_->vecPaperModel[n]->rtHTracker;
	cv::Rect rt2 = _pModel_->vecPaperModel[n]->rtVTracker;
	TRACE("ˮƽ��Ƥ��:(%d,%d,%d,%d), ��ֱ��Ƥ��(%d,%d,%d,%d)\n", rt1.x, rt1.y, rt1.width, rt1.height, rt2.x, rt2.y, rt2.width, rt2.height);

	float fFirst_H, fFirst_V, fSecond_H, fSecond_V;
	fFirst_H = fFirst_V = fSecond_H = fSecond_V = 0.0;
	if (nModelPicPersent == nSrcPicPercent)	//��ģ��ͼƬ����һ�£����ж������Ƿ���һ��
	{
		TRACE("��ģ��ͼƬ����һ��\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			TRACE("��ˮƽͬ��ͷ\n");
			cv::Rect rtH = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rtHTracker, i);
			int nHead_H = GetRects(matSrc, rtH, _pModel_, n, i, 1);		//��ˮƽͬ��ͷ����
			int nSum_H = _pModel_->vecPaperModel[n]->lH_Head.size();

			float fSimilarity_H = (float)nHead_H / nSum_H;
			if (fSimilarity_H < fMinPer || fSimilarity_H > fMaxPer)
				continue;

			if (i == 1)
				fFirst_H = fSimilarity_H;
			else
				fSecond_H = fSimilarity_H;

			TRACE("�鴹ֱͬ��ͷ\n");
			cv::Rect rtH2 = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rtVTracker, i);
			int nHead_V = GetRects(matSrc, rtH2, _pModel_, n, i, 2);		//�鴹ֱͬ��ͷ����
			int nSum_V = _pModel_->vecPaperModel[n]->lV_Head.size();

			float fSimilarity_V = (float)nHead_V / nSum_V;

			char szLog[300] = { 0 };
			sprintf_s(szLog, "rtH = (%d,%d,%d,%d), rtH2 = (%d,%d,%d,%d),\nnHead_H = %d, nHead_V = %d, nSum_H = %d, nSum_V = %d, H=%.2f, V=%.2f\n", rtH.tl().x, rtH.tl().y, rtH.width, rtH.height, rtH2.tl().x, rtH2.tl().y, rtH2.width, rtH2.height, \
					  nHead_H, nHead_V, nSum_H, nSum_V, fSimilarity_H, fSimilarity_V);
			g_pLogger->information(szLog);
			TRACE(szLog);

			if (fSimilarity_H > fMidPer)
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (fSimilarity_V > fMidPer)
				{
					bFind = true;
					nResult = i;
					break;
				}
				else    //fSimilarity_V in [0.5,0.8]	�п��ܣ��ٽ��н�һ���ж�
				{
					if (i == 1)
						fFirst_V = fSimilarity_V;
					else
						fSecond_V = fSimilarity_V;
				}
			}
			else	//fSimilarity_H in [0.5,0.8]	�п��ܣ��ٽ��н�һ���ж�
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (i == 1)
					fFirst_V = fSimilarity_V;
				else
					fSecond_V = fSimilarity_V;
			}
		}

		if (!bFind)
		{
			if (fFirst_H > fSecond_H && fFirst_V > fSecond_V)
			{
				nResult = 1;
			}
			else if (fFirst_H < fSecond_H && fFirst_V < fSecond_V)
			{
				nResult = 4;
			}
			else
			{
				TRACE("�޷��ж�ͼƬ����\n");
				g_pLogger->information("�޷��ж�ͼƬ����");
				nResult = 1;
			}
		}
	}
	else	//��ģ��ͼƬ����һ�£����ж�������ת90����������ת90
	{
		TRACE("��ģ��ͼƬ����һ��\n");
		for (int i = 2; i <= 3; i++)
		{
			TRACE("��ˮƽͬ��ͷ\n");
			cv::Rect rtH = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rtHTracker, i);
			int nHead_H = GetRects(matSrc, rtH, _pModel_, n, i, 1);		//��ˮƽͬ��ͷ����
			int nSum_H = _pModel_->vecPaperModel[n]->lH_Head.size();

			float fSimilarity_H = (float)nHead_H / nSum_H;
			if (fSimilarity_H < fMinPer || fSimilarity_H > fMaxPer)
				continue;

			if (i == 2)
				fFirst_H = fSimilarity_H;
			else
				fSecond_H = fSimilarity_H;

			TRACE("�鴹ֱͬ��ͷ\n");
			cv::Rect rtH2 = GetRectByOrientation(rtModelPic, _pModel_->vecPaperModel[n]->rtVTracker, i);
			int nHead_V = GetRects(matSrc, rtH2, _pModel_, n, i, 2);		//�鴹ֱͬ��ͷ����
			int nSum_V = _pModel_->vecPaperModel[n]->lV_Head.size();

			float fSimilarity_V = (float)nHead_V / nSum_V;

			char szLog[300] = { 0 };
			sprintf_s(szLog, "rtH = (%d,%d,%d,%d), rtH2 = (%d,%d,%d,%d),\nnHead_H = %d, nHead_V = %d, nSum_H = %d, nSum_V = %d, H=%.2f, V=%.2f\n", rtH.tl().x, rtH.tl().y, rtH.width, rtH.height, rtH2.tl().x, rtH2.tl().y, rtH2.width, rtH2.height, \
					  nHead_H, nHead_V, nSum_H, nSum_V, fSimilarity_H, fSimilarity_V);
			g_pLogger->information(szLog);
			TRACE(szLog);

			if (fSimilarity_H > fMidPer)
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (fSimilarity_V > fMidPer)
				{
					bFind = true;
					nResult = i;
					break;
				}
				else    //fSimilarity_V in [0.5,0.8]	�п��ܣ��ٽ��н�һ���ж�
				{
					if (i == 2)
						fFirst_V = fSimilarity_V;
					else
						fSecond_V = fSimilarity_V;
				}
			}
			else	//fSimilarity_H in [0.5,0.8]	�п��ܣ��ٽ��н�һ���ж�
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (i == 2)
					fFirst_V = fSimilarity_V;
				else
					fSecond_V = fSimilarity_V;
			}
		}

		if (!bFind)
		{
			if (fFirst_H > fSecond_H && fFirst_V > fSecond_V)
			{
				nResult = 2;
			}
			else if (fFirst_H < fSecond_H && fFirst_V < fSecond_V)
			{
				nResult = 3;
			}
			else
			{
				TRACE("�޷��ж�ͼƬ����\n");
				g_pLogger->information("�޷��ж�ͼƬ����");
				nResult = 1;
			}
		}
	}
	return nResult;
}

int CScanThread::CheckOrientation4Fix(cv::Mat& matSrc, int n)
{
	bool bFind = false;
	int nResult = 1;	//1:���򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180

	if (_pModel_->nHasHead)
		return nResult;

	std::string strLog;

	cv::Rect rtModelPic;
	rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
	rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matSrc.cols;
	rtSrcPic.height = matSrc.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matSrc.cols / matSrc.rows;

	if (_pModel_->nZkzhType == 2)			//ʹ�������ʱ����ͨ���������жϷ���
	{
		if (nModelPicPersent == nSrcPicPercent)
		{
			TRACE("��ģ��ͼƬ����һ��\n");
			for (int i = 1; i <= 4; i = i + 3)
			{
				COmrRecog omrRecogObj;
				bool bResult = omrRecogObj.RecogZkzh(n, matSrc, _pModel_, i);
				if (!bResult)
					continue;

				bFind = true;
				nResult = i;
				break;
			}
		}
		else
		{
			TRACE("��ģ��ͼƬ����һ��\n");
			for (int i = 2; i <= 3; i++)
			{
				COmrRecog omrRecogObj;
				bool bResult = omrRecogObj.RecogZkzh(n, matSrc, _pModel_, i);
				if (!bResult)
					continue;

				bFind = true;
				nResult = i;
				break;
			}
		}
		if (bFind)
			return nResult;

		strLog.append("ͨ����������ά���ж��Ծ���ת����ʧ�ܣ�����ͨ����λ���ж�\n");
	}

	int nCount = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
	if (nCount == 0)
		return nResult;

	if (nModelPicPersent == nSrcPicPercent)	//��ģ��ͼƬ����һ�£����ж������Ƿ���һ��
	{
		TRACE("��ģ��ͼƬ����һ��\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			//�Ȳ鶨��
			RECTLIST lFix;
			COmrRecog omrRecogObj;
			bool bResult = omrRecogObj.RecogFixCP(n, matSrc, lFix, _pModel_, i);
			// 			if (!bResult)
			// 				continue;
#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			PicTransfer(0, matSrc, lFix, _pModel_->vecPaperModel[n]->lFix, inverseMat);
#endif

			RECTLIST lModelTmp;
			if (lFix.size() < 3)
			{
				RECTLIST::iterator itFix = lFix.begin();
				for (auto itFix : lFix)
				{
					RECTLIST::iterator itModel = _pModel_->vecPaperModel[n]->lFix.begin();
					for (int j = 0; itModel != _pModel_->vecPaperModel[n]->lFix.end(); j++, itModel++)
					{
						if (j == itFix.nTH)
						{
							RECTINFO rcModel = *itModel;

							cv::Rect rtModelPic;
							rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
							rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
							rcModel.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcModel.rt, i);

							lModelTmp.push_back(rcModel);
							break;
						}
					}
				}
			}

			TRACE("��Ҷ�У���\n");
			bool bContinue = false;
			int nRtCount = 0;
			for (auto rcGray : _pModel_->vecPaperModel[n]->lGray)
			{
				RECTINFO rcItem = rcGray;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
					rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
					rcItem.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
				}
				else
					GetPosition(lFix, _pModel_->vecPaperModel[n]->lFix, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��

				if (omrRecogObj.RecogRtVal(rcItem, matSrc))
				{
					if (rcItem.fRealDensity / rcGray.fStandardDensity > rcGray.fStandardValuePercent && rcItem.fRealValue / rcGray.fStandardValue > rcGray.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("�жϻҶ�У�����ܶȰٷֱ�: %f, ����Ҫ���: %f\n", rcItem.fRealValuePercent, rcGray.fStandardValuePercent);
						// 						bContinue = true;
						// 						break;
					}
				}
				else
				{
					// 					bContinue = true;
					// 					break;
				}
			}
			if (bContinue)
				continue;

			TRACE("��ĿУ���\n");
			bContinue = false;
			for (auto rcSubject : _pModel_->vecPaperModel[n]->lCourse)
			{
				RECTINFO rcItem = rcSubject;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
					rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
					rcItem.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
				}
				else
					GetPosition(lFix, _pModel_->vecPaperModel[n]->lFix, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��

				if (omrRecogObj.RecogRtVal(rcItem, matSrc))
				{
					if (rcItem.fRealDensity / rcSubject.fStandardDensity > rcSubject.fStandardValuePercent && rcItem.fRealValue / rcSubject.fStandardValue > rcSubject.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("�жϿ�ĿУ�����ܶȰٷֱ�: %f, ����Ҫ���: %f\n", rcItem.fRealValuePercent, rcSubject.fStandardValuePercent);
						// 						bContinue = true;
						// 						break;
					}
				}
				else
				{
					// 					bContinue = true;
					// 					break;
				}
			}
			if (bContinue)
				continue;

			//�ж�����
			int nAllCount = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
			if (nAllCount <= 2)
			{
				if (nRtCount >= nAllCount)
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("��У�����=%d, ʵ��ʶ��У�����=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
			else
			{
				if (nRtCount >= (int)(nAllCount * 0.9))
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("��У�����=%d, ʵ��ʶ��У�����=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
		}

		if (!bFind)
		{
			TRACE("�޷��ж�ͼƬ����\n");
			strLog.append("�޷��ж�ͼƬ����\n");
			g_pLogger->information(strLog);
			nResult = 1;
		}
	}
	else	//��ģ��ͼƬ����һ�£����ж�������ת90����������ת90
	{
		TRACE("��ģ��ͼƬ����һ��\n");
		for (int i = 2; i <= 3; i++)
		{
			//�Ȳ鶨��
			RECTLIST lFix;
			COmrRecog omrRecogObj;
			bool bResult = omrRecogObj.RecogFixCP(n, matSrc, lFix, _pModel_, i);
			// 			if (!bResult)
			// 				continue;
#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			cv::Mat matDst;
			PicTransfer2(0, matSrc, matDst, lFix, _pModel_->vecPaperModel[n]->lFix, inverseMat);
#endif

			RECTLIST lModelTmp;
			if (lFix.size() < 3)
			{
				matDst = matSrc;

				RECTLIST::iterator itFix = lFix.begin();
				for (auto itFix : lFix)
				{
					RECTLIST::iterator itModel = _pModel_->vecPaperModel[n]->lFix.begin();
					for (int j = 0; itModel != _pModel_->vecPaperModel[n]->lFix.end(); j++, itModel++)
					{
						if (j == itFix.nTH)
						{
							RECTINFO rcModel = *itModel;

							cv::Rect rtModelPic;
							rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
							rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
							rcModel.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcModel.rt, i);

							lModelTmp.push_back(rcModel);
							break;
						}
					}
				}
			}

			TRACE("��Ҷ�У���\n");
			bool bContinue = false;
			int nRtCount = 0;
			for (auto rcGray : _pModel_->vecPaperModel[n]->lGray)
			{
				RECTINFO rcItem = rcGray;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
					rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
					rcItem.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
				}
				else
					GetPosition(lFix, _pModel_->vecPaperModel[n]->lFix, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��

				if (omrRecogObj.RecogRtVal(rcItem, matDst))
				{
					if (rcItem.fRealDensity / rcGray.fStandardDensity > rcGray.fStandardValuePercent && rcItem.fRealValue / rcGray.fStandardValue > rcGray.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("�жϻҶ�У�����ܶȰٷֱ�: %f, ����Ҫ���: %f\n", rcItem.fRealValuePercent, rcGray.fStandardValuePercent);
						// 						bContinue = true;
						// 						break;
					}
				}
				else
				{
					// 					bContinue = true;
					// 					break;
				}
			}
			if (bContinue)
				continue;

			TRACE("��ĿУ���\n");
			bContinue = false;
			for (auto rcSubject : _pModel_->vecPaperModel[n]->lCourse)
			{
				RECTINFO rcItem = rcSubject;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = _pModel_->vecPaperModel[n]->nPicW;
					rtModelPic.height = _pModel_->vecPaperModel[n]->nPicH;
					rcItem.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
				}
				else
					GetPosition(lFix, _pModel_->vecPaperModel[n]->lFix, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��

				if (omrRecogObj.RecogRtVal(rcItem, matDst))
				{
					if (rcItem.fRealDensity / rcSubject.fStandardDensity > rcSubject.fStandardValuePercent && rcItem.fRealValue / rcSubject.fStandardValue > rcSubject.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("�жϿ�ĿУ�����ܶȰٷֱ�: %f, ����Ҫ���: %f\n", rcItem.fRealValuePercent, rcSubject.fStandardValuePercent);
						// 						bContinue = true;
						// 						break;
					}
				}
				else
				{
					// 					bContinue = true;
					// 					break;
				}
			}
			if (bContinue)
				continue;

			//�ж�����
			int nAllCount = _pModel_->vecPaperModel[n]->lGray.size() + _pModel_->vecPaperModel[n]->lCourse.size();
			if (nAllCount <= 2)
			{
				if (nRtCount >= nAllCount)
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("��У�����=%d, ʵ��ʶ��У�����=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
			else
			{
				if (nRtCount >= (int)(nAllCount * 0.9))
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("��У�����=%d, ʵ��ʶ��У�����=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
		}

		if (!bFind)
		{
			TRACE("�޷��ж�ͼƬ���򣬲���Ĭ������90�ȵķ���\n");
			strLog.append("�޷��ж�ͼƬ���򣬲���Ĭ������90�ȵķ���\n");
			g_pLogger->information(strLog);
			nResult = 2;	//��������޷��ж�ͼ����ʱ��Ĭ��ģ����Ҫ����90�ȱ�ɴ�ͼ���򣬼�Ĭ�Ϸ��ط���Ϊ����90�ȣ���Ϊ����ֻ������90��������90������ѡ�񣬴˴�������Ĭ�ϵ�1������2
		}
	}

	return nResult;
}

int CScanThread::CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan)
{
	clock_t start, end;
	start = clock();

	//*********************************
	//*********	���Խ��� **************
	//ǰ�᣺˫��ɨ��
	//1�����治��Ҫ��ת ==> ����Ҳ����Ҫ��ת
	//2��������Ҫ��ת90�� ==> ������Ҫ��ת90��
	//3��������Ҫ��ת90�� ==> ������Ҫ��ת90��
	//4��������Ҫ��ת180�� ==> ����Ҳ��Ҫ��ת180��
	//*********************************
	int nResult = 1;	//1:���򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180
	static int nFristOrientation = 1;
	if (bDoubleScan && n % 2 != 0)	//˫��ɨ��, ������˫��ɨ��ĵڶ�������
	{
		if (nFristOrientation == 1) nResult = 1;
		else if (nFristOrientation == 2) nResult = 3;
		else if (nFristOrientation == 3) nResult = 2;
		else if (nFristOrientation == 4) nResult = 4;
		end = clock();
		TRACE("�ж���ת����ʱ��: %dms\n", end - start);

		std::string strDirection;
		switch (nResult)
		{
			case 1: strDirection = "���򣬲���Ҫ��ת"; break;
			case 2: strDirection = "����90"; break;
			case 3: strDirection = "����90"; break;
			case 4: strDirection = "����180"; break;
		}
		std::string strLog = "˫��ɨ��ڶ��棬���ݵ�һ�淽���жϽ����" + strDirection;
		g_pLogger->information(strLog);
		TRACE("%s\n", strLog.c_str());
		return nResult;
	}

	cv::Mat matCom = matSrc.clone();
	if (_pModel_->nHasHead)
		nResult = CheckOrientation4Head(matCom, n);
	else
		nResult = CheckOrientation4Fix(matCom, n);

	if (bDoubleScan && n % 2 == 0)		//˫��ɨ�裬������ɨ��ĵ�һ��
		nFristOrientation = nResult;

	end = clock();
	TRACE("�ж���ת����ʱ��: %dms\n", end - start);

	std::string strDirection;
	switch (nResult)
	{
		case 1: strDirection = "���򣬲���Ҫ��ת"; break;
		case 2: strDirection = "����90"; break;
		case 3: strDirection = "����90"; break;
		case 4: strDirection = "����180"; break;
	}
	std::string strLog = "�����жϽ����" + strDirection;
	g_pLogger->information(strLog);
	TRACE("%s\n", strLog.c_str());

	return nResult;
}

void* CScanThread::SaveFile(IplImage *pIpl)
{
	int nStudentId = _nScanCount_ / m_nModelPicNums + 1;
	int nOrder = _nScanCount_ % m_nModelPicNums + 1;

	char szPicName[50] = { 0 };
	char szPicPath[MAX_PATH] = { 0 };
	if (m_nNotifyDlgType != 2)
	{
		sprintf_s(szPicName, "S%d_%d.jpg", nStudentId, nOrder);
		sprintf_s(szPicPath, "%s\\S%d_%d.jpg", m_strCurrPicSavePath.c_str(), nStudentId, nOrder);
	}
	else
	{
		sprintf_s(szPicName, "model%d.jpg", nStudentId);
		sprintf_s(szPicPath, "%s\\model%d.jpg", m_strCurrPicSavePath.c_str(), nStudentId);
	}

	_nScanCount_++;

//	CScanMgrDlg* pDlg = (CScanMgrDlg*)m_pDlg;
	CDialog* pDlg;
	if (m_nNotifyDlgType == 2)
	{
	#ifndef TEST_NEW_MAKEMODEL
		pDlg = (CMakeModelDlg*)m_pDlg;
	#else
		pDlg = (CNewMakeModelDlg*)m_pDlg;
	#endif
	}
	else
		pDlg = (CScanMgrDlg*)m_pDlg;

	try
	{
		cv::Mat matSrc = cv::cvarrToMat(pIpl);

		//++ 2016.8.26 �ж�ɨ��ͼƬ���򣬲�������ת
		if (_pModel_ && m_nNotifyDlgType == 1/*&& m_pModel->nType*/)	//ֻ���ʹ���ƾ����Զ����ɵ�ģ��ʹ����ת��⹦�ܣ���Ϊ�ƾ��ߵ�ͼƬ����̶�
		{
			int nResult = CheckOrientation(matSrc, nOrder - 1, m_nDoubleScan == 0 ? false : true);
			switch (nResult)	//1:���ģ��ͼ����Ҫ���е���ת�����򣬲���Ҫ��ת��2����ת90(ģ��ͼ����ת), 3����ת90(ģ��ͼ����ת), 4����ת180(ģ��ͼ����ת)
			{
				case 1:	break;
				case 2:
				{
						  cv::Mat dst;
						  transpose(matSrc, dst);	//����90������ 
						  flip(dst, matSrc, 0);		//����90��ģ��ͼ����Ҫ����90��ԭͼ����Ҫ����90
				}
					break;
				case 3:
				{
						  cv::Mat dst;
						  transpose(matSrc, dst);	//����90������ 
						  flip(dst, matSrc, 1);		//����90��ģ��ͼ����Ҫ����90��ԭͼ����Ҫ����90
				}
					break;
				case 4:
				{
						  cv::Mat dst;
						  transpose(matSrc, dst);	//����90������ 
						  cv::Mat dst2;
						  flip(dst, dst2, 1);
						  cv::Mat dst5;
						  transpose(dst2, dst5);
						  flip(dst5, matSrc, 1);	//����180
				}
					break;
				default: break;
			}
		}
		//--

		cv::Mat matShow = matSrc.clone();

		std::string strPicName = szPicPath;
		imwrite(strPicName, matSrc);
		cvReleaseImage(&pIpl);
		
		if (m_nNotifyDlgType == 2)
		{
			pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
			pResult->bScanOK = false;
			pResult->nState = 1;			//��ʶ����ɨ��
			pResult->nPaperId = nStudentId;
			pResult->nPicId = nOrder;
			pResult->pPaper = NULL;
			pResult->strPicName = szPicName;
			pResult->strPicPath = szPicPath;
			pResult->matShowPic = matShow;
			pResult->strResult = "���ģ��ͼ��";
			pResult->strResult.append(szPicName);

			TRACE("%s\n", pResult->strResult.c_str());
			pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);
		}
		else
		{
			//++����Ծ�
			pST_PicInfo pPic = new ST_PicInfo;
			pPic->strPicName = szPicName;
			pPic->strPicPath = szPicPath;
			if (nOrder == 1)	//��һҳ��ʱ�򴴽��µ��Ծ���Ϣ
			{
				CScanMgrDlg* pDlg = (CScanMgrDlg*)m_pDlg;

				char szStudentName[30] = { 0 };
				sprintf_s(szStudentName, "S%d", nStudentId);
				m_pCurrPaper = new ST_PaperInfo;
				m_pCurrPaper->nIndex = nStudentId;
				m_pCurrPaper->strStudentInfo = szStudentName;
				m_pCurrPaper->pModel = _pModel_;
				m_pCurrPaper->pPapers = _pCurrPapersInfo_;
				m_pCurrPaper->pSrcDlg = pDlg->GetScanMainDlg();		//m_pDlg;
				m_pCurrPaper->lPic.push_back(pPic);

				_pCurrPapersInfo_->fmlPaper.lock();
				_pCurrPapersInfo_->lPaper.push_back(m_pCurrPaper);
				_pCurrPapersInfo_->fmlPaper.unlock();
			}
			else
			{
				m_pCurrPaper->lPic.push_back(pPic);
			}
			pPic->pPaper = m_pCurrPaper;
			//--

			pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
			pResult->bScanOK = false;
			pResult->nState = 1;			//��ʶ����ɨ��
			pResult->nPaperId = nStudentId;
			pResult->nPicId = nOrder;
			pResult->pPaper = m_pCurrPaper;
			pResult->matShowPic = matShow;
			pResult->strPicName = szPicName;
			pResult->strPicPath = szPicPath;
			pResult->strResult = "���ͼ��";
			pResult->strResult.append(szPicName);

			TRACE("%s\n", pResult->strResult.c_str());
			pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);

			//��ӵ�ʶ�������б�
			if (_pModel_ && _pCurrExam_->nModel == 0)	//����ģʽ�µ��Ծ�ż���ʶ�����
			{
				pRECOGTASK pTask = new RECOGTASK;
				pTask->pPaper = m_pCurrPaper;
				g_lRecogTask.push_back(pTask);
			}
		}		
	}
	catch (cv::Exception& exc)
	{
	}

	return NULL;
}

std::string CScanThread::ErrCode2Str(int nErr)
{
	std::string strResult;
	switch (nErr)
	{
		case -1:
			strResult = "ɨ���ʼ��--��ȡɨ������Ϣʧ��";
			break;
		case -100:
			strResult = "�ڴ洫��ģʽ--ɨ���Ǳ���ֹ";
			break;
		case -101:
			strResult = "�ڴ洫��ģʽ--��ȡͼ����Ϣʧ��";
			break;
		case -102:
			strResult = "�ڴ洫��ģʽ--�����ڴ�ʧ��";
			break;
		case -103:
			strResult = "�ڴ洫��ģʽ--ͼ�������";
			break;
		case -104:
			strResult = "�ڴ洫��ģʽ--�жϽ���ͼ����ʧ��";
			break;
		case -200:
			strResult = "����ģʽ--ɨ�豻��ֹ";
			break;
		case -201:
			strResult = "����ģʽ--�ڴ�����ʧ��";
			break;
		case -202:
			strResult = "����ģʽ--ȡ��ͼ����";
			break;
		case -203:
			strResult = "����ģʽ--����ͼ��ʧ��";
			break;
		default:
			strResult = "δ֪����";
			break;
	}
	_nScanStatus_ = nErr;
	return strResult;
}

void CScanThread::TestMode()
{
	CScanMgrDlg* pDlg = (CScanMgrDlg*)m_pDlg;
	USES_CONVERSION;
	for (int i = 0; i < 40; i++)
	{
		int nStudentId = i / m_nModelPicNums + 1;
		int nOrder = i % m_nModelPicNums + 1;
		char szPicName[50] = { 0 };
		char szPicPath[MAX_PATH] = { 0 };
		sprintf_s(szPicName, "S%d_%d.jpg", nStudentId, nOrder);
		sprintf_s(szPicPath, "%sPaper\\TestPic\\S%d_%d.jpg", T2A(g_strCurrentPath), nStudentId, nOrder);
		pST_PicInfo pPic = new ST_PicInfo;
		pPic->strPicName = szPicName;
		pPic->strPicPath = szPicPath;

		cv::Mat matSrc = cv::imread(pPic->strPicPath);

//		pDlg->ChildDlgShowPic(matSrc);

		_nScanCount_++;
		if (nOrder == 1)
		{
			char szStudentName[30] = { 0 };
			sprintf_s(szStudentName, "S%d", nStudentId);
			m_pCurrPaper = new ST_PaperInfo;
			m_pCurrPaper->nIndex = nStudentId;
			m_pCurrPaper->strStudentInfo = szStudentName;
			m_pCurrPaper->pModel = _pModel_;
			m_pCurrPaper->pPapers = _pCurrPapersInfo_;
			m_pCurrPaper->pSrcDlg = pDlg->GetScanMainDlg();
			m_pCurrPaper->lPic.push_back(pPic);

			_pCurrPapersInfo_->fmlPaper.lock();
			_pCurrPapersInfo_->lPaper.push_back(m_pCurrPaper);
			_pCurrPapersInfo_->fmlPaper.unlock();
		}
		else
			m_pCurrPaper->lPic.push_back(pPic);

		pPic->pPaper = m_pCurrPaper;


		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->bScanOK = false;
		pResult->nState = 1;			//��ʶ����ɨ��
		pResult->nPaperId = nStudentId;
		pResult->nPicId = nOrder;
		pResult->pPaper = m_pCurrPaper;
		pResult->matShowPic = matSrc;
		pResult->strResult = "���ͼ��";
		pResult->strResult.append(szPicName);

		TRACE("%s\n", pResult->strResult.c_str());
		CScanMgrDlg* pDlg = (CScanMgrDlg*)m_pDlg;
		pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);

		//��ӵ�ʶ�������б�
		if (_pModel_)
		{
			pRECOGTASK pTask = new RECOGTASK;
			pTask->pPaper = m_pCurrPaper;
			g_lRecogTask.push_back(pTask);
		}
	}
	pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
	pResult->nState = 0;
	_nScanStatus_ = 2;
	pResult->bScanOK = true;
	pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);
}



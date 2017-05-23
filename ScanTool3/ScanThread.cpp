// ScanThread.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanThread.h"

#include "ScanMgrDlg.h"

#include "DSMInterface.h"
#include "TwainString.h"

#include "CTiffWriter.h"



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
m_bStop(false), m_pDlg(NULL), m_nStartSaveIndex(0), m_pCurrPaper(NULL)
{
}

CScanThread::~CScanThread()
{
	exit();
	g_pTWAINApp = NULL;
	TRACE("~CScanThread(), 线程释放\n");
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
	TRACE("ExitInstance(), 线程退出\n");
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

	CScanMgrDlg* pDlg = (CScanMgrDlg*)m_pDlg;
//	pDlg->UpdateChildDlgInfo();

	connectDSM();
	if (m_DSMState < 3)
	{
		_nScanStatus_ = -1;
		exit();
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->bScanOK = false;
		pResult->strResult = "连接扫描源失败";
		
//		CScanMgrDlg* pDlg = (CScanMgrDlg*)AfxGetMainWnd();
		CScanDlg* pDlg = (CScanDlg*)m_pDlg;

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
		pResult->bScanOK = false;
		pResult->strResult = "加载扫描源失败";

//		CScanMgrDlg* pDlg = (CScanMgrDlg*)AfxGetMainWnd();
		CScanDlg* pDlg = (CScanDlg*)m_pDlg;

		pDlg->PostMessage(MSG_SCAN_ERR, (WPARAM)pResult, NULL);
		return;
	}

	TW_INT16  twrc = TWRC_FAILURE;
	
#if 1
	twrc = set_CapabilityOneValue(ICAP_XFERMECH, TWSX_NATIVE, TWTY_UINT16);
//	twrc = set_CapabilityOneValue(ICAP_XFERMECH, TWSX_MEMORY, TWTY_UINT16);
#else	//输出JPG
	twrc = set_CapabilityOneValue(ICAP_XFERMECH, TWSX_FILE, TWTY_UINT16);
	twrc = set_CapabilityOneValue(ICAP_IMAGEFILEFORMAT, TWFF_JFIF, TWTY_UINT16);		//TWFF_JFIF
#endif
	//双面扫描
	twrc = set_CapabilityOneValue(CAP_DUPLEXENABLED, stScanCtrl.nScanDuplexenable, TWTY_BOOL);
	twrc = set_CapabilityOneValue(ICAP_SUPPORTEDSIZES, stScanCtrl.nScanSize, TWTY_INT16);
	twrc = set_CapabilityOneValue(ICAP_PIXELTYPE, stScanCtrl.nScanPixelType, TWTY_INT16);
	int nResolution = stScanCtrl.nScanResolution;
	twrc = set_CapabilityOneValue(ICAP_XRESOLUTION, (pTW_FIX32)&nResolution);
	twrc = set_CapabilityOneValue(ICAP_YRESOLUTION, (pTW_FIX32)&nResolution);

	twrc = set_CapabilityOneValue(ICAP_AUTOMATICBORDERDETECTION, stScanCtrl.nAutoCut, TWTY_INT16);	//自动裁剪
	twrc = set_CapabilityOneValue(ICAP_AUTOMATICDESKEW, stScanCtrl.nRotate, TWTY_INT16);		//自动纠偏

	twrc = set_CapabilityOneValue(CAP_XFERCOUNT, stScanCtrl.nScanCount, TWTY_INT16);
#if 0
	TW_UINT16 nBright = (135 - 128) / 0.125;
	twrc = set_CapabilityOneValue(ICAP_BRIGHTNESS, nBright, TWTY_INT16);		//亮度调节

	TW_UINT16 nContrast = (135 - 128) / 0.125;
	twrc = set_CapabilityOneValue(ICAP_CONTRAST, nContrast, TWTY_INT16);		//对比度调节
#endif

	m_DSMessage = (TW_UINT16)-1;
	HWND hWnd = AfxGetMainWnd()->m_hWnd;
//	hWnd = GetDesktopWindow();
	if (!enableDS(hWnd, stScanCtrl.bShowUI))
	{
		_nScanStatus_ = -3;
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->bScanOK = false;
		pResult->strResult = "扫描失败";

//		CScanMgrDlg* pDlg = (CScanMgrDlg*)AfxGetMainWnd();
		CScanDlg* pDlg = (CScanDlg*)m_pDlg;

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


//	CScanMgrDlg* pDlg = (CScanMgrDlg*)AfxGetMainWnd();
//	CScanDlg* pDlg = (CScanDlg*)m_pDlg;

	if (m_bStop)
	{
		_nScanStatus_ = 3;
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->bScanOK = true;
		pResult->nState = nScanResult;
		pResult->strResult = "扫描仪被中止";
		pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);
	}
	else
	{
		pST_SCAN_RESULT pResult = new ST_SCAN_RESULT();
		pResult->nState = nScanResult;
		if (nScanResult != 0)
		{			
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
	int		nState = 0;		//扫描状态	-101:获取图像信息失败，-102:分配内存失败，-103:图像传输错误，-104:判断结束图像传输失败
	
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
						TRACE("图片信息: w=%d, h=%d, area = %d, 实际数据长度n=%d,相差= %d\n", w, h, w * h, n, w*h - n);
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

		//主动停止扫描仪
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
	int			nState = 0;		//扫描状态	-201:内存锁定失败
	
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
				TRACE("图片信息1: w=%d, h=%d, area = %d, 实际数据长度n=%d,相差= %d, channel=%d, depth=%d\n", w, h, w * h, nImageSize, w*h - nImageSize, nChannel, depth);
				TRACE("图片信息2: w=%d, h=%d, area = %d, 实际数据长度n=%d,相差= %d, channel=%d, depth=%d\n", w, h, w * h, pDIB->biSizeImage, w*h - pDIB->biSizeImage, nChannel, depth);
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

		//主动停止扫描仪
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

void* CScanThread::SaveFile(IplImage *pIpl)
{
	int nStudentId = _nScanCount_ / m_nModelPicNums + 1;
	int nOrder = _nScanCount_ % m_nModelPicNums + 1;

	char szPicName[50] = { 0 };
	char szPicPath[MAX_PATH] = { 0 };
	sprintf_s(szPicName, "S%d_%d.jpg", nStudentId, nOrder);
	sprintf_s(szPicPath, "%s\\S%d_%d.jpg", m_strCurrPicSavePath.c_str(), nStudentId, nOrder);

	_nScanCount_++;
	try
	{
		cv::Mat matTest = cv::cvarrToMat(pIpl);

		std::string strPicName = szPicPath;
		imwrite(strPicName, matTest);
		cvReleaseImage(&pIpl);

		//++添加试卷
		pST_PicInfo pPic = new ST_PicInfo;
		pPic->strPicName = szPicName;
		pPic->strPicPath = szPicPath;
		if (nOrder == 1)	//第一页的时候创建新的试卷信息
		{
			char szStudentName[30] = { 0 };
			sprintf_s(szStudentName, "S%d", nStudentId);
			m_pCurrPaper = new ST_PaperInfo;
			m_pCurrPaper->strStudentInfo = szStudentName;
			m_pCurrPaper->pModel = _pModel_;
			m_pCurrPaper->pPapers = _pCurrPapersInfo_;
			m_pCurrPaper->pSrcDlg = m_pDlg;
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
		pResult->nState = 1;			//标识正在扫描
		pResult->nPaperId = nStudentId;
		pResult->nPicId = nOrder;
		pResult->pPaper = m_pCurrPaper;
		pResult->strResult = "获得图像";
		pResult->strResult.append(szPicName);

		TRACE("%s\n", pResult->strResult.c_str());
		CScanMgrDlg* pDlg = (CScanMgrDlg*)m_pDlg;
		pDlg->PostMessage(MSG_SCAN_DONE, (WPARAM)pResult, NULL);
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
			strResult = "扫描初始化--获取扫描仪信息失败";
			break;
		case -100:
			strResult = "内存传输模式--扫描仪被中止";
			break;
		case -101:
			strResult = "内存传输模式--获取图像信息失败";
			break;
		case -102:
			strResult = "内存传输模式--分配内存失败";
			break;
		case -103:
			strResult = "内存传输模式--图像传输错误";
			break;
		case -104:
			strResult = "内存传输模式--判断结束图像传输失败";
			break;
		case -200:
			strResult = "本地模式--扫描被中止";
			break;
		case -201:
			strResult = "本地模式--内存锁定失败";
			break;
		case -202:
			strResult = "本地模式--取消图像传输";
			break;
		case -203:
			strResult = "本地模式--传输图像失败";
			break;
		default:
			strResult = "未知错误";
			break;
	}
	_nScanStatus_ = nErr;
	return strResult;
}



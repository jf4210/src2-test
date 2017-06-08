// ScanDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanDlg.h"
#include "afxdialogex.h"
#include "ScanTool3Dlg.h"
#include "ScanMgrDlg.h"


// CScanDlg 对话框

IMPLEMENT_DYNAMIC(CScanDlg, CDialog)

CScanDlg::CScanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanDlg::IDD, pParent)
	, m_nStatusSize(18), m_nCurrentScanCount(0), m_nModelPicNums(1), m_bAdvancedScan(false)
{

}

CScanDlg::~CScanDlg()
{
}

void CScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
// 	DDX_Text(pDX, IDC_STATIC_ExamName_Data, m_strExamName);
// 	DDX_Text(pDX, IDC_STATIC_Subject_Data, m_strSubjectName);
	DDX_Control(pDX, IDC_COMBO_Scanner, m_comboScanner);
	DDX_Control(pDX, IDC_COMBO_ScannerType, m_comboDuplex);
	DDX_Control(pDX, IDC_BTN_Scan, m_bmpBtnScan);
}


BOOL CScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetFontSize(m_nStatusSize);
	InitCtrlPosition();
	InitUI();
	m_comboScanner.AdjustDroppedWidth();
	m_comboDuplex.AdjustDroppedWidth();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CScanDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_Scan, &CScanDlg::OnBnClickedBtnScan)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_CHK_AdvanceScan, &CScanDlg::OnBnClickedChkAdvancescan)
END_MESSAGE_MAP()


// CScanDlg 消息处理程序

void CScanDlg::InitUI()
{
//	InitScanner(); 
	int nSrc = 0;
	int nDuplex = 1;
	char* ret;
	ret = new char[20];
	ret[0] = '\0';
	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanSrc", ret) == 0)
	{
		nSrc = atoi(ret);
	}
	memset(ret, 0, 20);

	if (ReadRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanDuplex", ret) == 0)
	{
		nDuplex = atoi(ret);
	}
	SAFE_RELEASE_ARRY(ret);
	if (nSrc >= m_comboScanner.GetCount())
		nSrc = 0;
	m_comboScanner.SetCurSel(nSrc);

	m_comboDuplex.ResetContent();
	m_comboDuplex.AddString(_T("单面扫描"));
	m_comboDuplex.AddString(_T("双面扫描"));
	m_comboDuplex.SetCurSel(nDuplex);

	((CButton*)GetDlgItem(IDC_CHK_AdvanceScan))->SetCheck(m_bAdvancedScan);

	//pic
	m_bmpBk.DeleteObject();
	m_bmpBk.LoadBitmap(IDB_Main_Bk);
	m_bmpBtnScan.SetStateBitmap(IDB_ScanMgr_StartScanBtn, 0, IDB_ScanMgr_StartScanBtn_Hover);
	
}

void CScanDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 20;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	int nBaseLeft = nLeftGap + (cx - nLeftGap - nRightGap) * 0.1;
	int nCurrLeft = nBaseLeft;
	int nCurrTop = nTopGap;
	int nStaticW = 60;
	int nStaticH = 30;
// 	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
// 	{
// 		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nStaticH);
// 		nCurrLeft += (nStaticW + nGap);
// 	}
// 
// 	//从右往左排列
// 	nCurrLeft = cx - nRightGap - (cx - nLeftGap - nRightGap) * 0.1 - nStaticW;
// 	nCurrTop = nTopGap;
// 	if (GetDlgItem(IDC_BTN_ChangeExam)->GetSafeHwnd())
// 	{
// 		GetDlgItem(IDC_BTN_ChangeExam)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nStaticH);
// 		int nTmpGap = (cx - nLeftGap - nRightGap) * 0.1;
// 		nCurrLeft -= (nStaticW * 2 + nGap + nTmpGap);
// 	}
// 	if (GetDlgItem(IDC_STATIC_Subject_Data)->GetSafeHwnd())
// 	{
// 		GetDlgItem(IDC_STATIC_Subject_Data)->MoveWindow(nCurrLeft, nCurrTop, nStaticW * 2, nStaticH);
// 		nCurrLeft -= (nStaticW + nGap);
// 	}
// 	if (GetDlgItem(IDC_STATIC_Subject)->GetSafeHwnd())
// 	{
// 		GetDlgItem(IDC_STATIC_Subject)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nStaticH);
// 		nCurrLeft -= (nStaticW + nGap);
// 	}
// 	int nStaticExamW = nCurrLeft - nLeftGap - nStaticW - nGap;	//考试名称控件的宽度
// 	nCurrLeft = nBaseLeft + nStaticW + nGap;
// 	if (GetDlgItem(IDC_STATIC_ExamName_Data)->GetSafeHwnd())
// 	{
// 		GetDlgItem(IDC_STATIC_ExamName_Data)->MoveWindow(nCurrLeft, nCurrTop, nStaticExamW, nStaticH);
// 	}
// 	nCurrTop += (nStaticH + nGap);

	//中间控件
	int nTmp = cy - nCurrTop - nBottomGap; 
	int nGapH = nTmp * 0.1;			//中心区域控件高度的间隔
	if (nGapH < 10) nGapH = 10;
	if (nGapH > 30) nGapH = 30;

	nCurrTop += nTmp * 0.3;
	nCurrLeft = (cx - nLeftGap - nRightGap) * 0.3;
	if (GetDlgItem(IDC_STATIC_Scaner)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Scaner)->MoveWindow(nCurrLeft, nCurrTop, 100, nStaticH);
		nCurrLeft += (100 + nGap * 3);
	}
	if (GetDlgItem(IDC_COMBO_Scanner)->GetSafeHwnd())
	{
		int nCommboW = (cx - nLeftGap - nRightGap) * 0.1;
		if (nCommboW < 250) nCommboW = 250;
		if (nCommboW > 300) nCommboW = 300;
		GetDlgItem(IDC_COMBO_Scanner)->MoveWindow(nCurrLeft, nCurrTop, nCommboW, nStaticH);
		nCurrLeft = (cx - nLeftGap - nRightGap) * 0.3;
		
		nCurrTop += (nStaticH + nGapH);
	}
	if (GetDlgItem(IDC_STATIC_ScanType)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ScanType)->MoveWindow(nCurrLeft, nCurrTop, 100, nStaticH);
		nCurrLeft += (100 + nGap * 3);
	}
	if (GetDlgItem(IDC_COMBO_ScannerType)->GetSafeHwnd())
	{
		int nCommboW = (cx - nLeftGap - nRightGap) * 0.1;
		if (nCommboW < 250) nCommboW = 250;
		if (nCommboW > 450) nCommboW = 450;
		GetDlgItem(IDC_COMBO_ScannerType)->MoveWindow(nCurrLeft, nCurrTop, nCommboW, nStaticH);
		nCurrTop += (nStaticH + nGapH);
	}

	//scan
	int nScanBtnW = 150;
	int nScanBtnH = nTmp * 0.2;
	nScanBtnW = (cx - nLeftGap - nRightGap) * 0.5;
	if (nScanBtnW < 220) nScanBtnW = 220;
	if (nScanBtnW > 230) nScanBtnW = 230;
	if (nScanBtnH < 40) nScanBtnH = 40;
	if (nScanBtnH > 50) nScanBtnH = 50;

	nCurrLeft = (cx - nLeftGap - nRightGap) * 0.5 - nScanBtnW / 2 - 50;
	nCurrTop += nGapH;
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nCurrLeft, nCurrTop, nScanBtnW, nScanBtnH);
		nCurrLeft += (nScanBtnW + nGap * 3);
	}
	if (GetDlgItem(IDC_CHK_AdvanceScan)->GetSafeHwnd())
	{
		int nW = 100;
		int nH = nStaticH;
		nCurrTop += (nScanBtnH / 2 - nH / 2);
		GetDlgItem(IDC_CHK_AdvanceScan)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}

	Invalidate();
}


void CScanDlg::SetFontSize(int nSize)
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
	m_fontScanBtn.DeleteObject();
	m_fontScanBtn.CreateFont(20, 0, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("幼圆"));
	GetDlgItem(IDC_STATIC_Scaner)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_ScanType)->SetFont(&m_fontStatus);
	m_bmpBtnScan.SetBtnFont(m_fontScanBtn);
	m_bmpBtnScan.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 0);
}

void CScanDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CScanDlg::SetScanSrcInfo(std::vector<CString>& vec)
{
	m_comboScanner.ResetContent();

	int nDefaultItem = -1;
	for (int i = 0; i < vec.size(); i++)
	{
		nDefaultItem = 0;
		m_comboScanner.AddString(vec[i]);
		m_comboScanner.SetItemData(i, i);
	}
	m_comboScanner.SetCurSel(nDefaultItem);

	InitUI();
}

void CScanDlg::UpdateInfo()
{
	USES_CONVERSION;

	InitUI();
	UpdateData(FALSE);
	Invalidate();
}

void CScanDlg::OnDestroy()
{
	CDialog::OnDestroy();

// 	if (_pTWAINApp)
// 	{
// 		_pTWAINApp->exit();
// 		SAFE_RELEASE(_pTWAINApp);
// 	}
}

void CScanDlg::OnBnClickedBtnScan()
{
	int           sel = m_comboScanner.GetCurSel();
	TW_INT16      index = (TW_INT16)m_comboScanner.GetItemData(sel);
	pTW_IDENTITY  pID = NULL;

	_nScanStatus_ = 0;
	if (!_bLogin_)
	{
		AfxMessageBox(_T("未登录, 无法扫描"));
		return;
	}
	if (!_pModel_)
	{
		AfxMessageBox(_T("当前考试无模板信息"));	//模板解析错误
		return;
	}

#ifdef Test_Data
	TestData();
	return;
#endif

	USES_CONVERSION;
	char szPicTmpPath[MAX_PATH] = { 0 };
	sprintf_s(szPicTmpPath, "%sPaper\\Tmp", T2A(g_strCurrentPath));

	std::string strUtfPath = CMyCodeConvert::Gb2312ToUtf8(szPicTmpPath);
	try
	{
		Poco::File tmpPath(strUtfPath);
		if (tmpPath.exists())
			tmpPath.remove(true);

		Poco::File tmpPath1(strUtfPath);
		tmpPath1.createDirectories();
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "删除临时文件夹失败(" + exc.message() + "): ";
		strLog.append(szPicTmpPath);
		g_pLogger->information(strLog);
	}

	m_strCurrPicSavePath = szPicTmpPath;

	//获取扫描参数
	int nScanSize = 1;				//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3, TWSS_NONE-自定义
	int nScanType = 2;				//0-黑白，1-灰度，2-彩色
	int nScanDpi = 200;				//dpi: 72, 150, 200, 300
	int nAutoCut = 1;
	nScanSize = _pModel_->nScanSize;
	nScanType = _pModel_->nScanType;
	nScanDpi = _pModel_->nScanDpi;
	nAutoCut = _pModel_->nAutoCut;

	m_nModelPicNums = _pModel_->nPicNum;

	bool bShowScanSrcUI = g_bShowScanSrcUI ? true : m_bAdvancedScan;			//显示高级扫描界面

	int nDuplex = m_comboDuplex.GetCurSel();		//单双面,0-单面,1-双面
	int nSize = TWSS_NONE;							//1-A4		//TWSS_A4LETTER-a4, TWSS_A3-a3, TWSS_NONE-自定义
	if (nScanSize == 1)
		nSize = TWSS_A4LETTER;
	else if (nScanSize == 2)
		nSize = TWSS_A3;
	else
		nSize = TWSS_NONE;
	
	int nNum = 0;
	if (nDuplex == 0)
	{
		nNum = m_nCurrentScanCount * m_nModelPicNums;
	}
	else
	{
		int nModelPics = m_nModelPicNums;
		if (nModelPics % 2)
			nModelPics++;

		nNum = m_nCurrentScanCount * nModelPics;
	}
	if (nNum == 0)
		nNum = -1;
	
	CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
	if (NULL != (pID = pDlg->GetScanSrc(index)))
	{
		SAFE_RELEASE(_pCurrPapersInfo_);
		_pCurrPapersInfo_ = new PAPERSINFO();


		_nScanStatus_ = 1;
		pST_SCANCTRL pScanCtrl = new ST_SCANCTRL();
		pScanCtrl->nScannerId = pID->Id;
		pScanCtrl->nScanCount = 2;			//nNum
		pScanCtrl->nScanDuplexenable = nDuplex;
		pScanCtrl->nScanPixelType = nScanType;
		pScanCtrl->nScanResolution = nScanDpi;
		pScanCtrl->nScanSize = nSize;
		pScanCtrl->bShowUI = bShowScanSrcUI;	//bShowScanSrcUI;

		pDlg->m_scanThread.setNotifyDlg(pDlg);
		pDlg->m_scanThread.setModelInfo(m_nModelPicNums, m_strCurrPicSavePath);
		pDlg->m_scanThread.resetData();
		pDlg->ResetChildDlg();
		pDlg->m_scanThread.PostThreadMessage(MSG_START_SCAN, pID->Id, (LPARAM)pScanCtrl);

		pDlg->ShowChildDlg(3);
	}
	else
	{
		AfxMessageBox(_T("获取扫描源失败"));
	}

	char szRet[20] = { 0 };
	sprintf_s(szRet, "%d", sel);
	WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanSrc", szRet);
	memset(szRet, 0, 20);
	sprintf_s(szRet, "%d", nDuplex);
	WriteRegKey(HKEY_CURRENT_USER, "Software\\EasyTNT\\AppKey", REG_SZ, "scanDuplex", szRet);
}

BOOL CScanDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

HBRUSH CScanDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_Scaner || CurID == IDC_STATIC_ScanType)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetTextColor(RGB(116, 116, 116));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_CHK_AdvanceScan)
	{
		HBRUSH hMYbr = ::CreateSolidBrush(RGB(255, 255, 255));	//62, 147, 254

		//		pDC->SetBkColor(RGB(0,0,255));
		pDC->SetBkMode(TRANSPARENT);

		//		return (HBRUSH)GetStockObject(NULL_BRUSH);
		return hMYbr;
	}
	return hbr;
}


BOOL CScanDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	//	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	CDialog::OnEraseBkgnd(pDC);

	int iX, iY;
	CDC memDC;
	BITMAP bmp;

	iX = iY = 0;
	GetClientRect(&rcClient);

	if (memDC.CreateCompatibleDC(pDC))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_bmpBk);
		m_bmpBk.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(iX, iY, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		memDC.SelectObject(pOldBmp);
	}
	memDC.DeleteDC();

	ReleaseDC(pDC);

	return TRUE;
}


void CScanDlg::OnBnClickedChkAdvancescan()
{
	m_bAdvancedScan = ((CButton*)GetDlgItem(IDC_CHK_AdvanceScan))->GetCheck();
}

void CScanDlg::TestData()
{
	SAFE_RELEASE(_pCurrPapersInfo_);
	_pCurrPapersInfo_ = new PAPERSINFO();

	m_nModelPicNums = _pModel_->nPicNum;

	CScanMgrDlg* pDlg = (CScanMgrDlg*)GetParent();
	pDlg->m_scanThread.setNotifyDlg(pDlg);
	pDlg->m_scanThread.setModelInfo(m_nModelPicNums, m_strCurrPicSavePath);
	pDlg->m_scanThread.resetData();
	pDlg->ResetChildDlg();
	pDlg->m_scanThread.PostThreadMessage(MSG_START_SCAN, 0, (LPARAM)NULL);

	pDlg->ShowChildDlg(3);
}

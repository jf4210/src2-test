// ScanDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ScanDlg.h"
#include "afxdialogex.h"
#include "ScanTool2Dlg.h"


// CScanDlg 对话框

IMPLEMENT_DYNAMIC(CScanDlg, CDialog)

CScanDlg::CScanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanDlg::IDD, pParent)
	, m_nStatusSize(20), _pTWAINApp(NULL)
{

}

CScanDlg::~CScanDlg()
{
}

void CScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_ExamName_Data, m_strExamName);
	DDX_Text(pDX, IDC_STATIC_Subject_Data, m_strSubjectName);
	DDX_Control(pDX, IDC_COMBO_Scanner, m_comboScanner);
	DDX_Control(pDX, IDC_COMBO_ScannerType, m_comboDuplex);
}


BOOL CScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetFontSize(m_nStatusSize);
	InitCtrlPosition();
	InitUI();

	return TRUE;
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

BEGIN_MESSAGE_MAP(CScanDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_ChangeExam, &CScanDlg::OnBnClickedBtnChangeexam)
END_MESSAGE_MAP()


// CScanDlg 消息处理程序

void CScanDlg::InitUI()
{
	InitScanner();
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
	m_comboScanner.SetCurSel(nSrc);

	m_comboDuplex.AddString(_T("单面扫描"));
	m_comboDuplex.AddString(_T("双面扫描"));
	m_comboDuplex.SetCurSel(nDuplex);

	m_comboScanner.AdjustDroppedWidth();
	m_comboDuplex.AdjustDroppedWidth();
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
	int nStaticW = 70;
	int nStaticH = 30;
	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nStaticH);
		nCurrLeft += (nStaticW + nGap);
	}

	//从右往左排列
	nCurrLeft = cx - nRightGap - (cx - nLeftGap - nRightGap) * 0.1 - nStaticW;
	nCurrTop = nTopGap;
	if (GetDlgItem(IDC_BTN_ChangeExam)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ChangeExam)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nStaticH);
		int nTmpGap = (cx - nLeftGap - nRightGap) * 0.1;
		nCurrLeft -= (nStaticW * 2 + nGap + nTmpGap);
	}
	if (GetDlgItem(IDC_STATIC_Subject_Data)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Subject_Data)->MoveWindow(nCurrLeft, nCurrTop, nStaticW * 2, nStaticH);
		nCurrLeft -= (nStaticW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_Subject)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Subject)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nStaticH);
		nCurrLeft -= (nStaticW + nGap);
	}
	int nStaticExamW = nCurrLeft - nLeftGap - nStaticW - nGap;	//考试名称控件的宽度
	nCurrLeft = nBaseLeft + nStaticW + nGap;
	if (GetDlgItem(IDC_STATIC_ExamName_Data)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ExamName_Data)->MoveWindow(nCurrLeft, nCurrTop, nStaticExamW, nStaticH);
	}
	nCurrTop += (nStaticH + nGap);

	//中间控件
	int nTmp = cy - nCurrTop - nBottomGap; 
	int nGapH = nTmp * 0.1;			//中心区域控件高度的间隔
	if (nGapH < 10) nGapH = 10;
	if (nGapH > 30) nGapH = 30;

	nCurrTop += nTmp * 0.3;
	nCurrLeft = (cx - nLeftGap - nRightGap) * 0.4;
	if (GetDlgItem(IDC_STATIC_Scaner)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Scaner)->MoveWindow(nCurrLeft, nCurrTop, 80, nStaticH);
		nCurrLeft += (80 + nGap * 3);
	}
	if (GetDlgItem(IDC_COMBO_Scanner)->GetSafeHwnd())
	{
		int nCommboW = (cx - nLeftGap - nRightGap) * 0.1;
		if (nCommboW < 250) nCommboW = 250;
		if (nCommboW > 450) nCommboW = 450;
		GetDlgItem(IDC_COMBO_Scanner)->MoveWindow(nCurrLeft, nCurrTop, nCommboW, nStaticH);
		nCurrLeft = (cx - nLeftGap - nRightGap) * 0.4;
		
		nCurrTop += (nStaticH + nGapH);
	}
	if (GetDlgItem(IDC_STATIC_ScanType)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ScanType)->MoveWindow(nCurrLeft, nCurrTop, 80, nStaticH);
		nCurrLeft += (80 + nGap * 3);
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
	nScanBtnW = (cx - nLeftGap - nRightGap) * 0.2;
	if (nScanBtnW < 100) nScanBtnW = 100;
	if (nScanBtnW > 150) nScanBtnW = 150;
	if (nScanBtnH < 40) nScanBtnH = 40;
	if (nScanBtnH > 70) nScanBtnH = 70;

	nCurrLeft = (cx - nLeftGap - nRightGap) * 0.5 - nScanBtnW / 2;
	nCurrTop += nGapH;
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nCurrLeft, nCurrTop, nScanBtnW, nScanBtnH);
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
	GetDlgItem(IDC_STATIC_ExamName_Data)->SetFont(&m_fontStatus);
}

void CScanDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}


void CScanDlg::OnBnClickedBtnChangeexam()
{
	CScanTool2Dlg* pDlg = (CScanTool2Dlg*)AfxGetMainWnd();
	pDlg->SwitchDlg(0);
}

void CScanDlg::InitScanner()
{
	_pTWAINApp = new TwainApp(m_hWnd);

	TW_IDENTITY *pAppID = _pTWAINApp->getAppIdentity();

	pAppID->Version.MajorNum = 2;
	pAppID->Version.MinorNum = 1;
	pAppID->Version.Language = TWLG_ENGLISH_CANADIAN;
	pAppID->Version.Country = TWCY_CANADA;
	SSTRCPY(pAppID->Version.Info, sizeof(pAppID->Version.Info), "2.1.1");
	pAppID->ProtocolMajor = TWON_PROTOCOLMAJOR;
	pAppID->ProtocolMinor = TWON_PROTOCOLMINOR;
	pAppID->SupportedGroups = DF_APP2 | DG_IMAGE | DG_CONTROL;
	SSTRCPY(pAppID->Manufacturer, sizeof(pAppID->Manufacturer), "TWAIN Working Group");
	SSTRCPY(pAppID->ProductFamily, sizeof(pAppID->ProductFamily), "Sample");
	SSTRCPY(pAppID->ProductName, sizeof(pAppID->ProductName), "MFC Supported Caps");

	_pTWAINApp->connectDSM();
	if (_pTWAINApp->m_DSMState >= 3)
	{
		pTW_IDENTITY pID = NULL;
		int   i = 0;
		int   index = 0;
		int   nDefault = -1;

		// Emply the list the refill
		m_comboScanner.ResetContent();

		if (NULL != (pID = _pTWAINApp->getDefaultDataSource())) // Get Default
		{
			nDefault = pID->Id;
		}
		USES_CONVERSION;
		while (NULL != (pID = _pTWAINApp->getDataSource((TW_INT16)i)))
		{
			index = m_comboScanner.AddString(A2T(pID->ProductName));
			if (LB_ERR == index)
			{
				break;
			}

			m_comboScanner.SetItemData(index, i);

			if (nDefault == (int)pID->Id)
			{
				m_comboScanner.SetCurSel(index);
			}

			i++;
		}
		_pTWAINApp->disconnectDSM();
	}
	m_comboScanner.SetCurSel(0);
}

void CScanDlg::UpdateInfo()
{
	USES_CONVERSION;
	if (_pCurrExam_ && _pCurrSub_)
	{
		m_strExamName		= A2T(_pCurrExam_->strExamName.c_str());
		m_strSubjectName	= A2T((_pCurrExam_->strGradeName + _pCurrSub_->strSubjName).c_str());
	}
	UpdateData(FALSE);
	Invalidate();
}

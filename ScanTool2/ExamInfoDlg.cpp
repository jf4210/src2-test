// ExamInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ExamInfoDlg.h"
#include "afxdialogex.h"


// CExamInfoDlg 对话框

IMPLEMENT_DYNAMIC(CExamInfoDlg, CDialog)

CExamInfoDlg::CExamInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExamInfoDlg::IDD, pParent)
	, _pExamInfo(NULL), _pSubjectInfo(NULL), _strExamName(_T("")), _strSubject(_T("")), _strMakeModel(_T("制作模板")), _strScan(_T("扫描提卡")), _strUpLoadProcess(_T("上传进度:"))
	, m_nStatusSize(20)
{

}

CExamInfoDlg::~CExamInfoDlg()
{
}

void CExamInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_ExamName, _strExamName);
	DDX_Text(pDX, IDC_STATIC_ExamSubject, _strSubject);
	DDX_Text(pDX, IDC_STATIC_MakeModel, _strMakeModel);
	DDX_Text(pDX, IDC_STATIC_Scan, _strScan);
	DDX_Text(pDX, IDC_STATIC_UploadProcess, _strUpLoadProcess);
}


BEGIN_MESSAGE_MAP(CExamInfoDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CExamInfoDlg 消息处理程序
BOOL CExamInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitData();
	SetFontSize(m_nStatusSize);
	InitCtrlPosition();

	return TRUE;
}

BOOL CExamInfoDlg::PreTranslateMessage(MSG* pMsg)
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

void CExamInfoDlg::InitData()
{
	_strExamName = _T("");
	_strSubject = _T("");
	_strMakeModel = _T("制作模板");
	_strScan = _T("扫描提卡");
	_strUpLoadProcess = _T("上传进度: 已上传0份");

	if (!_pExamInfo || !_pSubjectInfo) return;

	USES_CONVERSION;
	_strExamName = A2T(_pExamInfo->strExamName.c_str());
	_strSubject = A2T((_pExamInfo->strGradeName + _pSubjectInfo->strSubjName).c_str());
	UpdateData(FALSE);
}

void CExamInfoDlg::SetExamInfo(pEXAMINFO pExamInfo, pEXAM_SUBJECT pSubjectInfo)
{
	_pExamInfo		= pExamInfo;
	_pSubjectInfo	= pSubjectInfo;
	InitData();
}

void CExamInfoDlg::SetUploadPapers(int nCount)
{
	_strUpLoadProcess.Format(_T("上传进度: 已上传%d份"), nCount);
	UpdateData(FALSE);
}

void CExamInfoDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 2;	//上边的间隔
	const int nBottomGap = 2;	//下边的间隔
	const int nLeftGap = 2;		//左边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	int nStaticH = cy - nTopGap - nBottomGap;	//控件高度
	int nGap = 10;
	int nRealW = cx - nLeftGap - nRightGap;
	
	int nCurrTop = nTopGap;
	int nCurrLeft = nLeftGap;
// 	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
// 	{
// 		int nW = nRealW * 0.4;
// 		if (nW > 300) nW = 300;
// 		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
// 		nCurrLeft += (nW + nGap);
// 	}
// 	if (GetDlgItem(IDC_STATIC_ExamSubject)->GetSafeHwnd())
// 	{
// 		int nW = nRealW * 0.1;
// 		GetDlgItem(IDC_STATIC_ExamSubject)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
// 		nCurrLeft += (nW + nGap);
// 	}
	//从右往左算相对位置
	int nW1 = nRealW * 0.2;
	nCurrLeft = cx - nRightGap - nW1;
	if (GetDlgItem(IDC_STATIC_UploadProcess)->GetSafeHwnd())
	{
		int nW = nRealW * 0.1;
		GetDlgItem(IDC_STATIC_UploadProcess)->MoveWindow(nCurrLeft, nCurrTop, nW1, nStaticH);
		nCurrLeft -= (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_Scan)->GetSafeHwnd())
	{
		int nW = nRealW * 0.1;
		GetDlgItem(IDC_STATIC_Scan)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft -= (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_MakeModel)->GetSafeHwnd())
	{
		int nW = nRealW * 0.1;
		GetDlgItem(IDC_STATIC_MakeModel)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);

		int nMidGap = 100;

		int nTmpW = nRealW * 0.1;
		int nW2 = nCurrLeft - nMidGap - nTmpW - nGap - nLeftGap;
		if (nW2 > 650)
			nMidGap = nCurrLeft - nLeftGap - 650 - nGap - nTmpW;

		nCurrLeft -= (nW + nGap + nMidGap);
	}

	if (GetDlgItem(IDC_STATIC_ExamSubject)->GetSafeHwnd())
	{
		int nW = nRealW * 0.1;
		GetDlgItem(IDC_STATIC_ExamSubject)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft -= (nGap);
	}

	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		int nW = nCurrLeft - nLeftGap;
		if (nW > 650) nW = 650;
		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nLeftGap, nCurrTop, nW, nStaticH);
	}
}

void CExamInfoDlg::DrawBorder(CDC *pDC)
{
	CPen *pOldPen = NULL;
	CPen pPen;
	CRect rcClient(0, 0, 0, 0);
	GetClientRect(&rcClient);
	pPen.CreatePen(PS_SOLID, 2, RGB(166, 218, 239));

	pDC->SelectStockObject(NULL_BRUSH);
	pOldPen = pDC->SelectObject(&pPen);
	pDC->Rectangle(&rcClient);
	pDC->SelectObject(pOldPen);
	pPen.Detach();
	ReleaseDC(pDC);
}

void CExamInfoDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_STATIC_ExamName)->SetFont(&m_fontStatus);
}

BOOL CExamInfoDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(225, 242, 250)));
	DrawBorder(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CExamInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	InitCtrlPosition();
}


HBRUSH CExamInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_ExamName || CurID == IDC_STATIC_ExamSubject || CurID == IDC_STATIC_MakeModel || CurID == IDC_STATIC_Scan || CurID == IDC_STATIC_UploadProcess)
	{
//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

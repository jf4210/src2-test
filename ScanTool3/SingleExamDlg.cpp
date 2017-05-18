// SingleExamDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "SingleExamDlg.h"
#include "afxdialogex.h"
#include "ScanTool3Dlg.h"


// CSingleExamDlg 对话框

IMPLEMENT_DYNAMIC(CSingleExamDlg, CDialog)

CSingleExamDlg::CSingleExamDlg(CWnd* pParent /*=NULL*/)
: CDialog(CSingleExamDlg::IDD, pParent)
, _pExamInfo(NULL), _strExamName(_T("")), _strExamTime(_T("")), _strExamType(_T("")), _strExamGrade(_T(""))
, m_nStatusSize(25), m_nSubjectBtnH(30), m_nMaxSubsRow(2), _bMouseInDlg(false)
{

}

CSingleExamDlg::~CSingleExamDlg()
{
	ReleaseData();
}

void CSingleExamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_ExamName, _strExamName);
	DDX_Text(pDX, IDC_STATIC_ExamType, _strExamType);
	DDX_Text(pDX, IDC_STATIC_ExamGrade, _strExamGrade);
	DDX_Text(pDX, IDC_STATIC_ExamTime, _strExamTime);
}


BEGIN_MESSAGE_MAP(CSingleExamDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSingleExamDlg 消息处理程序

BOOL CSingleExamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitData();
	SetFontSize(m_nStatusSize);
	InitCtrlPosition();

	return TRUE;
}

BOOL CSingleExamDlg::PreTranslateMessage(MSG* pMsg)
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

LRESULT CSingleExamDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_COMMAND == message)
	{
		USES_CONVERSION;
		WORD wID = LOWORD(wParam);
		for (auto pSub : _pExamInfo->lSubjects)
		{
			if (wID == pSub->nSubjID)
			{
// 				CString strMsg = _T("");
// 				strMsg.Format(_T("%s"), A2T(pSub->strModelName.c_str()));
// 				AfxMessageBox(strMsg);
				_pCurrExam_ = _pExamInfo;
				_pCurrSub_	= pSub;

				CScanTool3Dlg* pDlg = (CScanTool3Dlg*)AfxGetMainWnd();
				pDlg->SwitchDlg(1);
				break;
			}
		}
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CSingleExamDlg::InitData()
{
	_strExamName = _T("");
	_strExamTime = _T("无考试时间");
	_strExamType = _T("");
	_strExamGrade = _T("");

	if (!_pExamInfo) return;

	USES_CONVERSION;
	_strExamName = A2T(_pExamInfo->strExamName.c_str());
	_strExamType = A2T(_pExamInfo->strExamTypeName.c_str());
	_strExamGrade = A2T(_pExamInfo->strGradeName.c_str());
	UpdateData(FALSE);
}

void CSingleExamDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 2;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 10;		//左边的空白间隔
	const int nRightGap = 10;	//右边的空白间隔
	int nGap = 2;
	int nStaticH = (cy - nTopGap - nBottomGap) / 4;	//静态控件高度
	if (nStaticH < 20) nStaticH = 20;
	if (nStaticH > 30) nStaticH = 30;

	int nBtnW = (cx - nLeftGap - nRightGap) * 0.1;	//一行最多放2个按钮
	if (nBtnW < 80) nBtnW = 80;
	if (nBtnW > 100) nBtnW = 100;

	int nCurrTop = nTopGap;
	int nCurrLeft = nLeftGap;

	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		int nH = cy - nTopGap - nBottomGap - m_nSubjectBtnH - nGap - nStaticH - nGap;// nStaticH * 2 - nGap;
		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrTop += nH + nGap;
	}
	if (GetDlgItem(IDC_STATIC_ExamType)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		GetDlgItem(IDC_STATIC_ExamType)->MoveWindow(nCurrLeft, nCurrTop, nW * 0.15, nStaticH);
		nCurrLeft += (nW * 0.15 + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ExamGrade)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		GetDlgItem(IDC_STATIC_ExamGrade)->MoveWindow(nCurrLeft, nCurrTop, nW * 0.1, nStaticH);
		nCurrLeft += (nW * 0.1 + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ExamTime)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		nW = nW * 0.5 - nGap * 2;
		GetDlgItem(IDC_STATIC_ExamTime)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrTop += (nStaticH + nGap);
	}
	if (GetDlgItem(IDC_BTN_ScanProcesses)->GetSafeHwnd())
	{
		nCurrLeft = nLeftGap;
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		nW = nW * 0.2;
		if (nW < 90) nW = 90;
		if (nW > 110) nW = 110;
		int nH = m_nSubjectBtnH;
		GetDlgItem(IDC_BTN_ScanProcesses)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_BTN_MakeScanModel)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		nW = nW * 0.2;
		if (nW < 90) nW = 90;
		if (nW > 110) nW = 110;
		int nH = m_nSubjectBtnH;
		GetDlgItem(IDC_BTN_MakeScanModel)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrLeft += (nW + nGap);
	}

	int nMaxBtnRow = m_nMaxSubsRow;			//一行最多显示2个科目按钮
	int nBtnH = m_nSubjectBtnH;
	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		nCurrLeft = cx - nRightGap - (nMaxBtnRow - i % nMaxBtnRow) * nBtnW - (nMaxBtnRow - i % nMaxBtnRow - 1) * nGap;
		nCurrTop = nTopGap + (i / nMaxBtnRow) * (nBtnH + nGap);
		if (m_vecBtn[i]->GetSafeHwnd())
		{
			m_vecBtn[i]->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		}
	}
//	TRACE("按钮数量: %d\n", m_vecBtn.size());

	Invalidate();
}

void CSingleExamDlg::SetFontSize(int nSize)
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

void CSingleExamDlg::ReleaseData()
{
//	TRACE("CSingleExamDlg::ReleaseData, 原始向量大小: %d\n", m_vecBtn.size());
	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		CButton* pBtn = m_vecBtn[i];
		SAFE_RELEASE(pBtn);
		m_vecBtn[i] = NULL;
	}
	m_vecBtn.clear();
}

void CSingleExamDlg::DrawBorder(CDC *pDC)
{
	CPen *pOldPen = NULL;
	CPen pPen;
	CRect rcClient(0, 0, 0, 0);
	GetClientRect(&rcClient);
	if (!_bMouseInDlg)
		pPen.CreatePen(PS_SOLID, 2, RGB(166, 218, 239));
	else
		pPen.CreatePen(PS_SOLID, 2, RGB(106, 218, 239));

	pDC->SelectStockObject(NULL_BRUSH);
	pOldPen = pDC->SelectObject(&pPen);
	pDC->Rectangle(&rcClient);
	pDC->SelectObject(pOldPen);
	pPen.Detach();
	ReleaseDC(pDC);
}

void CSingleExamDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CSingleExamDlg::SetExamInfo(pEXAMINFO pExamInfo)
{
	_pExamInfo = pExamInfo;

	ReleaseData();
	InitData();
	USES_CONVERSION;
	for (auto pSub : _pExamInfo->lSubjects)
	{
		pEXAM_SUBJECT pSubject = pSub;
		CButton* pNewButton = new CButton();// 也可以定义为类的成员变量。
		CRect rcButton(10, 10, 50, 30); // 按钮在对话框中的位置。
		pNewButton->Create(A2T(pSub->strSubjName.c_str()), 0, rcButton, this, pSubject->nSubjID);
		pNewButton->ShowWindow(SW_SHOW);
		m_vecBtn.push_back(pNewButton);
	}
	InitCtrlPosition();
}


BOOL CSingleExamDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 242, 250
	DrawBorder(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}


HBRUSH CSingleExamDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_ExamName || CurID == IDC_STATIC_ExamType || CurID == IDC_STATIC_ExamGrade || CurID == IDC_STATIC_ExamTime)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

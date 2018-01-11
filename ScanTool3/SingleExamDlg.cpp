// SingleExamDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "SingleExamDlg.h"
#include "afxdialogex.h"
#include "ScanTool3Dlg.h"
#include "NewMessageBox.h"
#include "Net_Cmd_Protocol.h"
#include "NewMakeModelDlg.h"
#include "MakeModelDlg.h"

// CSingleExamDlg 对话框

IMPLEMENT_DYNAMIC(CSingleExamDlg, CDialog)

CSingleExamDlg::CSingleExamDlg(CWnd* pParent /*=NULL*/)
: CDialog(CSingleExamDlg::IDD, pParent)
, _pExamInfo(NULL), _strExamName(_T("")), _strExamTime(_T("")), _strExamType(_T("")), _strExamGrade(_T("")), _strNetHandType(_T(""))
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
	DDX_Text(pDX, IDC_STATIC_NetHand_Mode, _strNetHandType);
	DDX_Text(pDX, IDC_STATIC_PaperType, _strShowPaperType);
	DDX_Control(pDX, IDC_BTN_ScanProcesses, m_bmpBtnScanProcess);
	DDX_Control(pDX, IDC_BTN_MakeScanModel, m_bmpBtnMakeModel);
}


BEGIN_MESSAGE_MAP(CSingleExamDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_ScanProcesses, &CSingleExamDlg::OnBnClickedBtnScanprocesses)
	ON_BN_CLICKED(IDC_BTN_MakeScanModel, &CSingleExamDlg::OnBnClickedBtnMakescanmodel)
END_MESSAGE_MAP()


// CSingleExamDlg 消息处理程序

BOOL CSingleExamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
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
			if (wID == (pSub->nSubjID * 10 + pSub->nSubjID + _pExamInfo->nExamID))
			{
				_pCurrExam_ = _pExamInfo;
				_pCurrSub_	= pSub;

				CScanTool3Dlg* pDlg = (CScanTool3Dlg*)AfxGetMainWnd();
				if (_pCurrExam_->nModel == 1)	//手阅时，不下载报名库和模板，直接扫描
					pDlg->SwitchDlg(1, 2);
				else
					pDlg->SwitchDlg(1);
				break;
			}
		}
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CSingleExamDlg::InitUI()
{
	m_bmpBtnScanProcess.SetStateBitmap(IDB_Exam_ScanProcessBtn_Hover, 0, IDB_Exam_ScanProcessBtn);
	m_bmpBtnMakeModel.SetStateBitmap(IDB_Exam_MakeModelBtn_Hover, 0, IDB_Exam_MakeModelBtn);
// 	m_bmpBtnScanProcess.SetStateBitmap(IDB_Exam_ScanProcessBtn, 0, IDB_Exam_ScanProcessBtn_Hover);
// 	m_bmpBtnMakeModel.SetStateBitmap(IDB_Exam_MakeModelBtn, 0, IDB_Exam_MakeModelBtn_Hover);
	m_bmpBtnMakeModel.SetWindowText(_T("  制作扫描模板"));
	m_bmpBtnMakeModel.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 0);
	m_bmpBtnScanProcess.SetWindowText(_T("  扫描进度查询"));
	m_bmpBtnScanProcess.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 0);

	if (_pExamInfo)
	{
		if (_pExamInfo->nModel != 0)
		{
			m_bmpExamType.LoadBitmap(IDB_Exam_Hand);
			m_bmpExamTypeLeft.LoadBitmap(IDB_Exam_Hand1);
		}
		else
		{
			m_bmpExamType.LoadBitmap(IDB_Exam_Net);
			m_bmpExamTypeLeft.LoadBitmap(IDB_Exam_Net1);
		}
	}	
}

void CSingleExamDlg::InitData()
{
	_strExamName = _T("");
	_strExamTime = _T("考试时间:");
	_strExamType = _T("");
	_strExamGrade = _T("");
	_strShowPaperType = _T("题卡类型:");

	if (!_pExamInfo) return;
	if (_pExamInfo->nModel != 0)
	{
		m_bmpExamType.LoadBitmap(IDB_Exam_Hand);
		m_bmpExamTypeLeft.LoadBitmap(IDB_Exam_Hand1);
		_strNetHandType = _T("手");
		m_bmpBtnMakeModel.ShowWindow(SW_HIDE);
	}
	else
	{
		m_bmpExamType.LoadBitmap(IDB_Exam_Net);
		m_bmpExamTypeLeft.LoadBitmap(IDB_Exam_Net1);
		_strNetHandType = _T("网");
		m_bmpBtnMakeModel.ShowWindow(SW_SHOW);

#ifndef _DEBUG
//		m_bmpBtnMakeModel.ShowWindow(SW_HIDE);
#endif
	}

	USES_CONVERSION;
	_strExamName = A2T(_pExamInfo->strExamName.c_str());
	if (_pExamInfo->nModel == 0)	//网阅模式才显示，手阅模式即天喻版本显示空
		_strExamType = A2T(_pExamInfo->strExamTypeName.c_str());
	_strExamGrade = A2T(_pExamInfo->strGradeName.c_str());
//	_strExamTime = A2T(_pExamInfo->strExamTime.c_str());
	if (!_pExamInfo->strExamTime.empty())
		_strExamTime.Format(_T("考试时间: %s"), A2T(_pExamInfo->strExamTime.c_str()));
	if (_pExamInfo->nModel == 0)
		_strShowPaperType = _T("题卡类型: 网阅");
	else
		_strShowPaperType = _T("题卡类型: 手阅");
	UpdateData(FALSE);
}

void CSingleExamDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 20;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 10;		//左边的空白间隔
	const int nRightGap = 50;	//右边的空白间隔
	int nGap = 5;
	int nStaticH = (cy - nTopGap - nBottomGap) / 4;	//静态控件高度
	if (nStaticH < 10) nStaticH = 10;
	if (nStaticH > 20) nStaticH = 20;

	int nBtnW = (cx - nLeftGap - nRightGap) * 0.1;	//一行最多放2个按钮
	if (nBtnW < 80) nBtnW = 80;
	if (nBtnW > 100) nBtnW = 100;

	int nCurrTop = nTopGap;
	int nCurrLeft = nLeftGap;

	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		int nH = cy - nTopGap - nBottomGap - (m_nSubjectBtnH + 10 + nGap * 2) - nStaticH - nGap;// nStaticH * 2 - nGap;
		if (nH > 30) nH = 30;
		if (nH < 20) nH = 20;
		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrTop += nH + nGap;
	}
// 	if (GetDlgItem(IDC_STATIC_ExamType)->GetSafeHwnd())
// 	{
// 		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
// 		GetDlgItem(IDC_STATIC_ExamType)->MoveWindow(nCurrLeft, nCurrTop, nW * 0.15, nStaticH);
// 		nCurrLeft += (nW * 0.15 + nGap);
	// 	}
	if (GetDlgItem(IDC_STATIC_ExamTime)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		nW = nW * 0.2 - nGap * 2;
		GetDlgItem(IDC_STATIC_ExamTime)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_STATIC_ExamGrade)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		GetDlgItem(IDC_STATIC_ExamGrade)->MoveWindow(nCurrLeft, nCurrTop, nW * 0.1, nStaticH);
		nCurrLeft += (nW * 0.1 + nGap);
	}
// 	if (GetDlgItem(IDC_STATIC_ExamTime)->GetSafeHwnd())
// 	{
// 		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
// 		nW = nW * 0.2 - nGap * 2;
// 		GetDlgItem(IDC_STATIC_ExamTime)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
// 		nCurrLeft += (nW + nGap);
// //		nCurrTop += (nStaticH + nGap * 2);
// 	}
	if (GetDlgItem(IDC_STATIC_PaperType)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		nW = nW * 0.15;
		GetDlgItem(IDC_STATIC_PaperType)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
		nCurrLeft += (nW + nGap);
//		nCurrTop += (nStaticH + nGap * 2);
	}
	if (GetDlgItem(IDC_STATIC_ExamType)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		GetDlgItem(IDC_STATIC_ExamType)->MoveWindow(nCurrLeft, nCurrTop, nW * 0.15, nStaticH);
//		nCurrLeft += (nW * 0.15 + nGap);
		nCurrTop += (nStaticH + nGap * 2);
	}

	if (GetDlgItem(IDC_BTN_ScanProcesses)->GetSafeHwnd())
	{
		nCurrLeft = nLeftGap;
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		nW = nW * 0.2;
		if (nW < 160) nW = 160;
		if (nW > 170) nW = 170;
		int nH = m_nSubjectBtnH + 10;
		GetDlgItem(IDC_BTN_ScanProcesses)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrLeft += (nW + nGap);
	}
	if (GetDlgItem(IDC_BTN_MakeScanModel)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		nW = nW * 0.2;
		if (nW < 160) nW = 160;
		if (nW > 170) nW = 170;
		int nH = m_nSubjectBtnH + 10;
		GetDlgItem(IDC_BTN_MakeScanModel)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrLeft += (nW + nGap);
	}

	//网阅or手阅
	if (GetDlgItem(IDC_STATIC_NetHand_Mode)->GetSafeHwnd())
	{
		int nW = 22;
		int nH = nStaticH;
		nCurrLeft = cx - nW - nGap*2;
		nCurrTop = 0;
		GetDlgItem(IDC_STATIC_NetHand_Mode)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
	}

	int nMaxBtnRow = m_nMaxSubsRow;			//一行最多显示的科目按钮
	int nBtnH = m_nSubjectBtnH;
	int nMaxRow = ceil((double)m_vecBtn.size() / nMaxBtnRow);
	int nTmpTop = nTopGap + (cy - nTopGap - nBottomGap) / 2 - (nBtnH * nMaxRow + (nMaxRow - 1) * nGap) / 2;
	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		nCurrLeft = cx - nRightGap - (nMaxBtnRow - i % nMaxBtnRow) * nBtnW - (nMaxBtnRow - i % nMaxBtnRow - 1) * nGap;
		nCurrTop = nTmpTop + (i / nMaxBtnRow) * (nBtnH + nGap);
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
	m_fontNetHandType.DeleteObject();
	m_fontNetHandType.CreateFont(14, 0, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("Arial"));
	m_fontBtn.DeleteObject();
	m_fontBtn.CreateFont(14, 0, 0, 0,
								 FW_BOLD, FALSE, FALSE, 0,
								 DEFAULT_CHARSET,
								 OUT_DEFAULT_PRECIS,
								 CLIP_DEFAULT_PRECIS,
								 DEFAULT_QUALITY,
								 DEFAULT_PITCH | FF_SWISS,
								 _T("幼圆"));
	GetDlgItem(IDC_STATIC_ExamName)->SetFont(&m_fontStatus);
	GetDlgItem(IDC_STATIC_NetHand_Mode)->SetFont(&m_fontNetHandType);
	m_bmpBtnMakeModel.SetBtnFont(m_fontBtn); 
	m_bmpBtnScanProcess.SetBtnFont(m_fontBtn);
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
		pPen.CreatePen(PS_SOLID, 1, RGB(166, 218, 239));
	else
		pPen.CreatePen(PS_SOLID, 1, RGB(106, 218, 239));

	pDC->SelectStockObject(NULL_BRUSH);
	pOldPen = pDC->SelectObject(&pPen);
//	pDC->Rectangle(&rcClient);
	pDC->RoundRect(rcClient, CPoint(10, 10));
	pDC->SelectObject(pOldPen);
	pPen.Detach();
//	ReleaseDC(pDC);
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
		CBmpButton* pNewButton = new CBmpButton();// 也可以定义为类的成员变量。
		pNewButton->SetStateBitmap(IDB_Exam_SubjectBtn, 0, IDB_Exam_SubjectBtn_Hover);
		CRect rcButton(10, 10, 50, 30); // 按钮在对话框中的位置。
		pNewButton->Create(A2T(pSub->strSubjName.c_str()), 0, rcButton, this, pSubject->nSubjID * 10 + pSubject->nSubjID + _pExamInfo->nExamID);
		pNewButton->ShowWindow(SW_SHOW);
		m_vecBtn.push_back(pNewButton);
	}
	InitCtrlPosition();
}

BOOL CSingleExamDlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 242, 250

	//画虚线
	if (GetDlgItem(IDC_STATIC_ExamTime)->GetSafeHwnd())
	{
		CPen *pOldPen = NULL;
		CPen pPen;
		pPen.CreatePen(PS_DASH, 1, RGB(106, 218, 239));
		pOldPen = pDC->SelectObject(&pPen);

		CRect rtTmp;
		GetDlgItem(IDC_STATIC_ExamTime)->GetWindowRect(rtTmp);
		ScreenToClient(&rtTmp);

		CPoint pt1, pt2;
		pt1.x = rtTmp.left;
		pt1.y = rtTmp.bottom + 1;
		pt2.x = pt1.x + 350;
		pt2.y = pt1.y;
		pDC->MoveTo(pt1);
		pDC->LineTo(pt2);
		pDC->SelectObject(pOldPen);
		pPen.Detach();
	}
	DrawBorder(pDC);

	int iX, iY;
	CDC memDC;
	BITMAP bmp;
	if (memDC.CreateCompatibleDC(pDC))
	{
		CBitmap *pOldBmp = memDC.SelectObject(&m_bmpExamType);
		m_bmpExamType.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(rcClient.right - bmp.bmWidth - 10, 1, bmp.bmWidth, bmp.bmHeight, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);


		pOldBmp = memDC.SelectObject(&m_bmpExamTypeLeft);
		m_bmpExamTypeLeft.GetBitmap(&bmp);
		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(rcClient.left, rcClient.top, bmp.bmWidth, rcClient.bottom, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

		memDC.SelectObject(pOldBmp);
	}
	memDC.DeleteDC();

	return TRUE;
}

HBRUSH CSingleExamDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_ExamName)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_ExamType || CurID == IDC_STATIC_ExamGrade || CurID == IDC_STATIC_ExamTime || CurID == IDC_STATIC_PaperType)
	{
		pDC->SetTextColor(RGB(146, 146, 146));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_NetHand_Mode)
	{
		pDC->SetTextColor(RGB(230, 230, 255));	//173, 209, 255
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

void CSingleExamDlg::OnBnClickedBtnScanprocesses()
{
	_pCurrExam_ = _pExamInfo;
	CScanTool3Dlg* pDlg = (CScanTool3Dlg*)AfxGetMainWnd();

	//检查是否需要下载报名库
	if (_pCurrExam_->nModel == 0)
	{
		EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
		if (itFindExam == g_mapBmkMgr.end())		//如果已经下载了当前考试的报名库，就提取报名库，如何直接下载模板
		{
			ST_GET_BMK_INFO stGetBmkInfo;
			ZeroMemory(&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
			stGetBmkInfo.nExamID = _pCurrExam_->nExamID;
			stGetBmkInfo.nSubjectID = 0;
			strcpy(stGetBmkInfo.szEzs, _strEzs_.c_str());

			g_eGetBmk.reset();

			pTCP_TASK pTcpTask = new TCP_TASK;
			pTcpTask->usCmd = USER_GET_EXAM_BMK;
			pTcpTask->nPkgLen = sizeof(ST_GET_BMK_INFO);
			memcpy(pTcpTask->szSendBuf, (char*)&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
			g_fmTcpTaskLock.lock();
			g_lTcpTask.push_back(pTcpTask);
			g_fmTcpTaskLock.unlock();
		}
	}	

	pDlg->SwitchDlg(1, 4);		//显示第1个窗口，并一开始就显示这个窗口中的第4个窗口
}


void CSingleExamDlg::OnBnClickedBtnMakescanmodel()
{
	_pCurrExam_ = _pExamInfo;
	_pCurrSub_	= NULL;

#ifdef TEST_TIP_SHOW
	//存在考号为空的考生，若不修改，将影响参加后面的评卷，忽略？
	//当前试卷袋有20份试卷未保存，是否删除?
	//正在保存f91f021afcbc479ba4a77cb4d3f70a52_402-590_20170531110518_56.pkg
	//保存f91f021afcbc479ba4a77cb4d3f70a52_402-590_20170531110518_56.pkg成功
	CNewMessageBox	dlg1;
	dlg1.setShowInfo(2, 2, "当前试卷袋有20份试卷未保存，是否删除?");
	dlg1.DoModal();
	return;
#endif

	CNewMakeModelDlg dlg;
	dlg.DoModal();
}

// ScanMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanMgrDlg.h"
#include "afxdialogex.h"

#include "Net_Cmd_Protocol.h"
#include "ZipObj.h"
#include "ScanTool3Dlg.h"
// CScanMgrDlg 对话框

IMPLEMENT_DYNAMIC(CScanMgrDlg, CDialog)

CScanMgrDlg::CScanMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanMgrDlg::IDD, pParent)
	, m_pScanDlg(NULL), m_pWaitDownloadDlg(NULL), m_pScanProcessDlg(NULL), m_pScanRecordMgrDlg(NULL)
	, m_strExamName(_T("")), m_nStatusSize(30)
{

}

CScanMgrDlg::~CScanMgrDlg()
{
}

void CScanMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_ExamName, m_strExamName);
	DDX_Control(pDX, IDC_COMBO_Subject, m_comboSubject);
}


BOOL CScanMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitExamData();
	InitChildDlg();
	InitCtrlPosition();
	m_scanThread.CreateThread();

	SetFontSize(m_nStatusSize);
	m_comboSubject.AdjustDroppedWidth();

	return TRUE;
}

BOOL CScanMgrDlg::PreTranslateMessage(MSG* pMsg)
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

BEGIN_MESSAGE_MAP(CScanMgrDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BTN_ChangeExam, &CScanMgrDlg::OnBnClickedBtnChangeexam)
	ON_MESSAGE(MSG_SCAN_DONE, &CScanMgrDlg::ScanDone)
	ON_MESSAGE(MSG_SCAN_ERR, &CScanMgrDlg::ScanErr)
END_MESSAGE_MAP()


// CScanMgrDlg 消息处理程序

void CScanMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 100;	//上边的间隔
	const int nBottomGap = 10;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	m_rtChildDlg.left = nLeftGap;
	m_rtChildDlg.top = nTopGap;
	m_rtChildDlg.right = cx - nRightGap;
	m_rtChildDlg.bottom = cy - nBottomGap;

	int nCurrLeft = nLeftGap;
	int nCurrTop = 20;
	int nStaticW = (cx - nLeftGap - nRightGap) * 0.1;
	if (nStaticW < 60) nStaticW = 60;
	if (nStaticW > 70) nStaticW = 70;
	int nH = nTopGap - nCurrTop;
	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		int nRightTmpGap = 100;	//最右边临时空出一块

		int nCommboW = nStaticW * 1.5;
		if (nCommboW < 100) nCommboW = 100;
		if (nCommboW > 110) nCommboW = 110;

		int nW = cx - nLeftGap - nRightGap - nStaticW - nGap - nRightTmpGap - nCommboW - nGap - nStaticW - nGap * 3;
		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrLeft += (nW + nGap * 3);
	}
	if (GetDlgItem(IDC_STATIC_CurrSubject)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_CurrSubject)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nH);
		nCurrLeft += (nStaticW + nGap);
	}
	if (GetDlgItem(IDC_COMBO_Subject)->GetSafeHwnd())
	{
		int nCommboH = 10;
		nCurrTop += (nH / 2 - nCommboH);
		int nCommboW = nStaticW * 1.5;
		if (nCommboW < 100) nCommboW = 100;
		if (nCommboW > 110) nCommboW = 110;
		GetDlgItem(IDC_COMBO_Subject)->MoveWindow(nCurrLeft, nCurrTop, nCommboW, nH);
		nCurrLeft += (nCommboW + nGap);
	}
	if (GetDlgItem(IDC_BTN_ChangeExam)->GetSafeHwnd())
	{
		int nBtnH = 25;
		GetDlgItem(IDC_BTN_ChangeExam)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nBtnH);
		nCurrLeft += (nStaticW + nGap);
	}

	//child dlg
	if (m_pWaitDownloadDlg && m_pWaitDownloadDlg->GetSafeHwnd())
		m_pWaitDownloadDlg->MoveWindow(m_rtChildDlg);
	if (m_pScanDlg && m_pScanDlg->GetSafeHwnd())
		m_pScanDlg->MoveWindow(m_rtChildDlg);
	if (m_pScanProcessDlg && m_pScanProcessDlg->GetSafeHwnd())
		m_pScanProcessDlg->MoveWindow(m_rtChildDlg);
	if (m_pScanRecordMgrDlg && m_pScanRecordMgrDlg->GetSafeHwnd())
		m_pScanRecordMgrDlg->MoveWindow(m_rtChildDlg);
	Invalidate();
}


void CScanMgrDlg::InitChildDlg()
{
	m_pWaitDownloadDlg = new CWaitDownloadDlg(this);
	m_pWaitDownloadDlg->Create(CWaitDownloadDlg::IDD, this);
	m_pWaitDownloadDlg->ShowWindow(SW_SHOW);

	m_pScanDlg = new CScanDlg(this);
	m_pScanDlg->Create(CScanDlg::IDD, this);
	m_pScanDlg->ShowWindow(SW_HIDE);

	m_pScanProcessDlg = new CScanProcessDlg(this);
	m_pScanProcessDlg->Create(CScanProcessDlg::IDD, this);
	m_pScanProcessDlg->ShowWindow(SW_HIDE);

	m_pScanRecordMgrDlg = new CScanRecordMgrDlg(this);
	m_pScanRecordMgrDlg->Create(CScanRecordMgrDlg::IDD, this);
	m_pScanRecordMgrDlg->ShowWindow(SW_HIDE);
}

void CScanMgrDlg::ReleaseDlg()
{
	if (m_pWaitDownloadDlg)
	{
		m_pWaitDownloadDlg->DestroyWindow();
		SAFE_RELEASE(m_pWaitDownloadDlg);
	}
	if (m_pScanDlg)
	{
		m_pScanDlg->DestroyWindow();
		SAFE_RELEASE(m_pScanDlg);
	}
	if (m_pScanProcessDlg)
	{
		m_pScanProcessDlg->DestroyWindow();
		SAFE_RELEASE(m_pScanProcessDlg);
	}
	if (m_pScanRecordMgrDlg)
	{
		m_pScanRecordMgrDlg->DestroyWindow();
		SAFE_RELEASE(m_pScanRecordMgrDlg);
	}
}

void CScanMgrDlg::SetFontSize(int nSize)
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

	CFont fontStatus;
	fontStatus.CreateFont(nSize - 5, 0, 0, 0,
							FW_BOLD, FALSE, FALSE, 0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_SWISS,
							_T("Arial"));
	GetDlgItem(IDC_STATIC_CurrSubject)->SetFont(&fontStatus);
}

void CScanMgrDlg::DrawBorder(CDC *pDC)
{
	CPen *pOldPen = NULL;
	CPen pPen;
	CRect rcClient(0, 0, 0, 0);
	GetClientRect(&rcClient);
	pPen.CreatePen(PS_SOLID, 2, RGB(0, 0, 255));

	pDC->SelectStockObject(NULL_BRUSH);
	pOldPen = pDC->SelectObject(&pPen);
	pDC->Rectangle(&rcClient);
	pDC->SelectObject(pOldPen);
	pPen.Detach();
	ReleaseDC(pDC);
}

void CScanMgrDlg::ShowChildDlg(int n)
{
//	InitData();
	if (n == 1)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_SHOW);
		m_pScanDlg->ShowWindow(SW_HIDE);
		m_pScanProcessDlg->ShowWindow(SW_HIDE);
		m_pScanRecordMgrDlg->ShowWindow(SW_HIDE);
		if (!DownLoadModel())
		{
			AfxMessageBox(_T("考试信息为空"));
			//跳到考试管理页面
			CScanTool3Dlg* pDlg = (CScanTool3Dlg*)GetParent();
			pDlg->SwitchDlg(0);
		}
	}
	else if (n == 2)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_HIDE);
		m_pScanDlg->ShowWindow(SW_SHOW);
		m_pScanProcessDlg->ShowWindow(SW_HIDE);
		m_pScanRecordMgrDlg->ShowWindow(SW_HIDE);
	}
	else if (n == 3)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_HIDE);
		m_pScanDlg->ShowWindow(SW_HIDE);
		m_pScanProcessDlg->ShowWindow(SW_SHOW);
		m_pScanRecordMgrDlg->ShowWindow(SW_HIDE);

		m_pScanProcessDlg->InitShow();
	}
	else if (n == 4)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_HIDE);
		m_pScanDlg->ShowWindow(SW_HIDE);
		m_pScanProcessDlg->ShowWindow(SW_HIDE);
		m_pScanRecordMgrDlg->ShowWindow(SW_SHOW);
	}
}

void CScanMgrDlg::ResetChildDlg()
{
	if (m_pScanProcessDlg)
		m_pScanProcessDlg->ResetPicList();
}

void CScanMgrDlg::UpdateChildDlgInfo()
{
	if (m_pScanProcessDlg)
		m_pScanProcessDlg->InitShow();
}

void CScanMgrDlg::OnDestroy()
{
	CDialog::OnDestroy();

	ReleaseDlg();
}

void CScanMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CScanMgrDlg::InitExamData()
{
	USES_CONVERSION;
	m_comboSubject.ResetContent();
	if (_pCurrExam_)
	{
		m_strExamName = A2T(_pCurrExam_->strExamName.c_str());

		if (_pCurrSub_)
		{
			int i = 0;
			int nShowSubject = -1;
			for (auto pSubject : _pCurrExam_->lSubjects)
			{
				m_comboSubject.AddString(A2T(pSubject->strSubjName.c_str()));
				if (_pCurrSub_ == pSubject)
					nShowSubject = i;
				i++;
			}
			m_comboSubject.SetCurSel(nShowSubject);
		}
	}

	UpdateChildDlgInfo();

	UpdateData(FALSE);
}

bool CScanMgrDlg::SearchModel()
{
	if (!_pCurrExam_ || !_pCurrSub_) return false;

	if (_pCurrSub_->strModelName.empty())
		return false;

	if (_pModel_)
		SAFE_RELEASE(_pModel_);

	USES_CONVERSION;
	std::string strModelName = _pCurrSub_->strModelName;	//gb2312
	std::string strModelFullPath = T2A(g_strCurrentPath + _T("Model\\"));
	strModelFullPath += strModelName;
	bool bExist = false;
	try
	{
		Poco::File fModel(CMyCodeConvert::Gb2312ToUtf8(strModelFullPath));		//考试对应的模板文件存在，则直接解压此模板，否则就通过examID_subID来搜索模板
		if (fModel.exists())
			bExist = true;
	}
	catch (Poco::Exception& e)
	{
	}

	//这部分是加载含有_examID_subID.的模板，即如果没有和科目信息中记录的模板名称一样的模板存在的话，就加载考试ID和科目ID一样的模板
	//暂且不用
#if 0		
	if (!bExist)
	{
		std::string strExamSub = Poco::format("_%d_%d.", _pCurrExam_->nExamID, _pCurrSub_->nSubjID);
		std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
		try
		{
			Poco::DirectoryIterator it(strModelPath);
			Poco::DirectoryIterator end;
			while (it != end)
			{
				Poco::Path p(it->path());
				if (it->isFile() && p.getExtension() == "mod" && p.getFileName().find(strExamSub) != std::string::npos)
				{
					strModelFullPath = CMyCodeConvert::Utf8ToGb2312(p.toString());
					bExist = true;
					break;
				}
				++it;
			}
		}
		catch (Poco::FileException& exc)
		{
			std::cerr << exc.displayText() << std::endl;
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << exc.displayText() << std::endl;
		}
	}
#endif
	
	bool bResult = false;
	if (bExist)
	{
		CZipObj zipObj;
		zipObj.setLogger(g_pLogger);
		zipObj.UnZipFile(A2T(strModelFullPath.c_str()));

		CString strModelFilePath = g_strCurrentPath + _T("Model\\") + A2T(strModelName.c_str());
		strModelFilePath = strModelFilePath.Left(strModelFilePath.ReverseFind('.'));
		_pModel_ = LoadModelFile(strModelFilePath);
		std::string strLog;
		if (_pModel_)
		{
			strLog = Poco::format("模板%s加载成功", strModelName);
			bResult = true;
		}
		else
			strLog = Poco::format("模板%s加载失败", strModelName);
		g_pLogger->information(strLog);
	}
	return bResult;
}

bool CScanMgrDlg::DownLoadModel()
{
	if (!_pCurrExam_ || !_pCurrSub_) return false;

	g_nDownLoadModelStatus = 0;

	//先查本地列表，如果没有则请求，如果有，计算crc，和服务器不同则下载
	USES_CONVERSION;
	CString modelPath = g_strCurrentPath + _T("Model");
	modelPath = modelPath + _T("\\") + A2T(_pCurrSub_->strModelName.c_str());
	std::string strModelPath = T2A(modelPath);

	ST_DOWN_MODEL stModelInfo;
	ZeroMemory(&stModelInfo, sizeof(ST_DOWN_MODEL));
	stModelInfo.nExamID = _pCurrExam_->nExamID;
	stModelInfo.nSubjectID = _pCurrSub_->nSubjID;
	sprintf_s(stModelInfo.szUserNo, "%s", _strUserName_.c_str());
	sprintf_s(stModelInfo.szModelName, "%s", _pCurrSub_->strModelName.c_str());

	Poco::File fileModel(CMyCodeConvert::Gb2312ToUtf8(strModelPath));
	if (fileModel.exists())
	{
		std::string strMd5 = calcFileMd5(strModelPath);
		strncpy(stModelInfo.szMD5, strMd5.c_str(), strMd5.length());
	}

	std::string strLog = "请求下载模板: ";
	strLog.append(stModelInfo.szModelName);
	g_pLogger->information(strLog);

	g_eDownLoadModel.reset();

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_NEED_DOWN_MODEL;
	pTcpTask->nPkgLen = sizeof(ST_DOWN_MODEL);
	memcpy(pTcpTask->szSendBuf, (char*)&stModelInfo, sizeof(ST_DOWN_MODEL));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();

	return true;
}

HBRUSH CScanMgrDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_ExamName || CurID == IDC_STATIC_CurrSubject)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
// 	else if (CurID == IDC_COMBO_Subject)
// 	{
// 		pDC->SetBkMode(TRANSPARENT);
// 		return (HBRUSH)GetStockObject(WHITE_BRUSH);
// 	}
	return hbr;
}

BOOL CScanMgrDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	DrawBorder(pDC);

//	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CScanMgrDlg::OnBnClickedBtnChangeexam()
{
	CScanTool3Dlg* pDlg = (CScanTool3Dlg*)GetParent();
	pDlg->SwitchDlg(0);
}

LRESULT CScanMgrDlg::ScanDone(WPARAM wParam, LPARAM lParam)
{
	pST_SCAN_RESULT pResult = (pST_SCAN_RESULT)wParam;
	if (pResult)
	{
		TRACE("扫描完成消息。%s\n", pResult->strResult.c_str());
		g_pLogger->information(pResult->strResult);

		if (pResult->nState == 1)
		{
			//试卷列表显示扫描试卷
			if (pResult->nPicId == 1)	//第一页的时候创建新的试卷信息
			{
				m_pScanProcessDlg->AddPaper(pResult->nPaperId, pResult->pPaper);
			}
		}

		m_pScanProcessDlg->UpdateChildInfo(pResult->bScanOK);
		if (pResult->bScanOK)	//扫描完成
		{
			if (m_pScanProcessDlg)
			{
				m_pScanProcessDlg->InitShow();
			}			
		}

		delete pResult;
		pResult = NULL;
	}
	return 1;
}

LRESULT CScanMgrDlg::ScanErr(WPARAM wParam, LPARAM lParam)
{
	pST_SCAN_RESULT pResult = (pST_SCAN_RESULT)wParam;
	if (pResult)
	{
		TRACE("扫描错误。%s\n", pResult->strResult.c_str());
		UpdateChildDlgInfo();
		delete pResult;
		pResult = NULL;
	}
	return 1;
}

// ScanMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanMgrDlg.h"
#include "afxdialogex.h"

#include "Net_Cmd_Protocol.h"
#include "ZipObj.h"
#include "ScanTool3Dlg.h"
#include "NewMessageBox.h"
// CScanMgrDlg 对话框

IMPLEMENT_DYNAMIC(CScanMgrDlg, CDialog)

CScanMgrDlg::CScanMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanMgrDlg::IDD, pParent)
	, m_pScanDlg(NULL), m_pWaitDownloadDlg(NULL), m_pScanProcessDlg(NULL), m_pScanRecordMgrDlg(NULL)
	, m_strExamName(_T("")), m_nStatusSize(30), _pTWAINApp(NULL)
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
	DDX_Control(pDX, IDC_BTN_ChangeExam, m_bmpBtnChangeExam);
}


BOOL CScanMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	InitScanner();
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
	ON_MESSAGE(MSG_Compress_Done, &CScanMgrDlg::MsgCompressResult)
//	ON_WM_TIMER()
ON_CBN_SELCHANGE(IDC_COMBO_Subject, &CScanMgrDlg::OnCbnSelchangeComboSubject)
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
	int nBtnW = nStaticW + 10;
	int nH = nTopGap - nCurrTop;
	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		int nRightTmpGap = 100;	//最右边临时空出一块

		int nCommboW = nStaticW * 1.5;
		if (nCommboW < 100) nCommboW = 100;
		if (nCommboW > 110) nCommboW = 110;

		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nRightTmpGap - nCommboW - nGap - nStaticW - nGap * 3;
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
		GetDlgItem(IDC_BTN_ChangeExam)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
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
//	ReleaseDC(pDC);
}

bool CScanMgrDlg::chkChangeExamLegal()
{
	if (_eCurrDlgType_ == Dlg_ScanRecordMgr)
	{
//		AfxMessageBox(_T("当前窗口不允许切换"));
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "当前窗口不允许切换");
		dlg.DoModal();
		return false;
	}
	if (_nScanStatus_ == 1)
	{
//		AfxMessageBox(_T("正在扫描, 请稍后。。。"));
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 1, "正在扫描, 请稍后。。。");
		dlg.DoModal();
		return false;
	}
	if (!_pCurrPapersInfo_)
		return true;

	if (_pCurrExam_ && _pCurrExam_->nModel == 0)	//网阅模式
	{
		bool bRecogComplete = true;
		for (auto p : _pCurrPapersInfo_->lPaper)
		{
			if (!p->bRecogComplete)
			{
				bRecogComplete = false;
				break;
			}
		}
		if (!bRecogComplete)
		{
//			AfxMessageBox(_T("请稍后，图像正在识别！"));
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "请稍后，图像正在识别！");
			dlg.DoModal();
			return false;
		}
	}
	int nCount = _pCurrPapersInfo_->lPaper.size() + _pCurrPapersInfo_->lIssue.size();
	if (nCount > 0)
	{
		CNewMessageBox	dlg;
		dlg.setShowInfo(2, 2, "当前试卷袋信息未保存，是否切换？");
		dlg.DoModal();
		if (dlg.m_nResult != IDYES)
			return false;
// 		if (MessageBox(_T("当前试卷袋信息未保存，是否切换？"), _T("警告"), MB_YESNO) != IDYES)
// 			return false;
	}

	return true;
}

bool CScanMgrDlg::getCurrSubjectBmk()
{
	bool bResult = false;
	g_lBmkStudent.clear();
	EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
	if (itFindExam != g_mapBmkMgr.end())
	{
		for (auto student : itFindExam->second)
		{
			for (auto subject : student.lSubjectScanStatus)
			{
				if (subject.nSubjectID == _pCurrSub_->nSubjID)
				{
					ST_STUDENT _currSubjectStudent;
					_currSubjectStudent.strZkzh = student.strZkzh;
					_currSubjectStudent.strName = student.strName;
					_currSubjectStudent.strClassroom = student.strClassroom;
					_currSubjectStudent.strSchool = student.strSchool;
					_currSubjectStudent.nScaned = subject.nScaned;
					g_lBmkStudent.push_back(_currSubjectStudent);
				}
			}
		}
	}
	if (g_lBmkStudent.size() > 0)
	{
		_bGetBmk_ = true;
		bResult = true;
	}
	else
		_bGetBmk_ = false;
	return bResult;
}

void CScanMgrDlg::ShowChildDlg(int n, int nOprater /*= 0*/)
{
//	InitData();
	if (n == 1)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_SHOW);
		m_pScanDlg->ShowWindow(SW_HIDE);
		m_pScanProcessDlg->ShowWindow(SW_HIDE);
		m_pScanRecordMgrDlg->ShowWindow(SW_HIDE);

		_eCurrDlgType_ = DLG_DownloadModle;
		int nResult = GetBmkInfo();
		if (nResult == 0)
		{
//			AfxMessageBox(_T("考试信息为空"));
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "考试信息为空");
			dlg.DoModal();
			//跳到考试管理页面
			CScanTool3Dlg* pDlg = (CScanTool3Dlg*)GetParent();
			pDlg->SwitchDlg(0);
		}
		if (nResult == -2)	// && (MessageBox(_T("获取考生报名库失败, 是否继续?"), _T("提示"), MB_YESNO) != IDYES)
		{
			CNewMessageBox	dlg;
			dlg.setShowInfo(1, 2, "获取考生报名库失败, 是否继续？");
			dlg.DoModal();
			if (dlg.m_nResult != IDYES)
			{
				CScanTool3Dlg* pDlg = (CScanTool3Dlg*)GetParent();
				pDlg->SwitchDlg(0);
				return;
			}
		}
		if ((nResult == 2 || nResult == -2) && !DownLoadModel())	//如果已经下载了当前考试的报名库，就提取报名库，如何直接下载模板
		{
			CNewMessageBox	dlg;
			if (!_pCurrExam_ || !_pCurrSub_)
				dlg.setShowInfo(2, 1, "考试或科目信息为空");
			else if (_pCurrSub_->strModelName.empty())
				dlg.setShowInfo(2, 1, "未设置扫描模板");
			dlg.DoModal();
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

		m_pScanDlg->SetScanSrcInfo(m_vecScanSrc);
		m_pScanProcessDlg->InitTmpSubjectBmk();

		_eCurrDlgType_ = DLG_ScanStart;
	}
	else if (n == 3)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_HIDE);
		m_pScanDlg->ShowWindow(SW_HIDE);
		m_pScanProcessDlg->ShowWindow(SW_SHOW);
		m_pScanRecordMgrDlg->ShowWindow(SW_HIDE);

		_eCurrDlgType_ = Dlg_ScanProcess;
		if (/*_pCurrExam_->nModel == 1 && */nOprater == 1)
			return;

		m_pScanProcessDlg->UpdateChildInfo();
		m_pScanProcessDlg->InitShow();
	}
	else if (n == 4)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_HIDE);
		m_pScanDlg->ShowWindow(SW_HIDE);
		m_pScanProcessDlg->ShowWindow(SW_HIDE);
		m_pScanRecordMgrDlg->ShowWindow(SW_SHOW);

		_eCurrDlgType_ = Dlg_ScanRecordMgr;
		m_pScanRecordMgrDlg->UpdateChildDlg();
	}
}

void CScanMgrDlg::ResetChildDlg()
{
	if (m_pScanProcessDlg)
		m_pScanProcessDlg->ResetPicList();
}

void CScanMgrDlg::UpdateChildDlgInfo(int nType /*= 0*/)
{
	if (_eCurrDlgType_ == Dlg_ScanRecordMgr)
	{
		m_pScanRecordMgrDlg->UpdateChildDlg();
	}
	else if (_eCurrDlgType_ == Dlg_ScanProcess)
	{
		if (m_pScanProcessDlg)
		{
			if (nType == 0)
				m_pScanProcessDlg->InitShow();
			else if (nType == 1)
				m_pScanProcessDlg->ReShowCurrPapers();
		}
	}
}

pTW_IDENTITY CScanMgrDlg::GetScanSrc(int nIndex)
{
	return _pTWAINApp->getDataSource(nIndex);
}

void* CScanMgrDlg::GetScanMainDlg()
{
	return m_pScanProcessDlg;
}

void CScanMgrDlg::SetReturnDlg(int nFlag /*= 2*/)
{
	if (nFlag == 1)
	{
		GetDlgItem(IDC_STATIC_CurrSubject)->ShowWindow(SW_HIDE);
		m_comboSubject.ShowWindow(SW_HIDE);
		m_bmpBtnChangeExam.ShowWindow(SW_HIDE);
	}
	else if (nFlag == 2)
	{
		GetDlgItem(IDC_STATIC_CurrSubject)->EnableWindow(FALSE);
		m_comboSubject.EnableWindow(FALSE);
		m_bmpBtnChangeExam.ShowWindow(SW_HIDE);
	}
	m_pScanRecordMgrDlg->SetReBackDlg(nFlag);
}

LRESULT CScanMgrDlg::MsgCompressResult(WPARAM wParam, LPARAM lParam)
{
	CString str = (LPCTSTR)lParam;
	bool bWarn = (bool)wParam;
	m_pScanProcessDlg->SetStatusShow(2, str, bWarn);
	return TRUE;
}

void CScanMgrDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (_pTWAINApp)
	{
		_pTWAINApp->exit();
		SAFE_RELEASE(_pTWAINApp);
	}

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
				int nCount = m_comboSubject.GetCount();
				m_comboSubject.SetItemDataPtr(nCount - 1, pSubject);
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

	if (_pCurrSub_->strModelName.empty())	return false;

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

int CScanMgrDlg::GetBmkInfo()
{
	if (!_pCurrExam_ || !_pCurrSub_) return 0;

	if (_pCurrExam_->nModel != 0) return -3;		//只针对网阅考试获取报名库

#if 1		//NewBmkTest
	TRACE("请求报名库(%d)...\n", _pCurrExam_->nExamID);
	EXAMBMK_MAP::iterator itFindExam = g_mapBmkMgr.find(_pCurrExam_->nExamID);
	if (itFindExam != g_mapBmkMgr.end())		//如果已经下载了当前考试的报名库，就提取报名库，如何直接下载模板
	{
		int nResult = 0;
		if (getCurrSubjectBmk())
			nResult = 2;
		else
			nResult = -2;	//获取科目学生列表失败
		return nResult;
	}

	ST_GET_BMK_INFO stGetBmkInfo;
	ZeroMemory(&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
	stGetBmkInfo.nExamID = _pCurrExam_->nExamID;
	stGetBmkInfo.nSubjectID = _pCurrSub_->nSubjID;
	strcpy(stGetBmkInfo.szEzs, _strEzs_.c_str());

	g_eGetBmk.reset();

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_GET_EXAM_BMK;
	pTcpTask->nPkgLen = sizeof(ST_GET_BMK_INFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
#else
	TRACE("请求报名库(%d:%d)...\n", _pCurrExam_->nExamID, _pCurrSub_->nSubjID);

	ST_GET_BMK_INFO stGetBmkInfo;
	ZeroMemory(&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
	stGetBmkInfo.nExamID = _pCurrExam_->nExamID;
	stGetBmkInfo.nSubjectID = _pCurrSub_->nSubjID;
	strcpy(stGetBmkInfo.szEzs, _strEzs_.c_str());

	g_eGetBmk.reset();

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_GET_BMK;
	pTcpTask->nPkgLen = sizeof(ST_GET_BMK_INFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
#endif
	return 1;
}

HBRUSH CScanMgrDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_ExamName /*|| CurID == IDC_STATIC_CurrSubject*/)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else if (CurID == IDC_STATIC_CurrSubject)
	{
		pDC->SetTextColor(RGB(81, 195, 201));
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
//	DrawBorder(pDC);

	ReleaseDC(pDC);

	return CDialog::OnEraseBkgnd(pDC);
}

void CScanMgrDlg::OnBnClickedBtnChangeexam()
{
	if (!chkChangeExamLegal())
		return;

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
			
			if (_pCurrExam_->nModel == 1)
			{
				ChildDlgShowPic(pResult->matShowPic);
			}
		}

		m_pScanProcessDlg->UpdateChildInfo(pResult->bScanOK);
		if (pResult->bScanOK)	//扫描完成
		{
			if (m_pScanProcessDlg)
			{
				m_pScanProcessDlg->ScanCompleted();
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
		m_pScanProcessDlg->UpdateChildInfo(pResult->bScanOK);
		UpdateChildDlgInfo();
		delete pResult;
		pResult = NULL;
	}
	return 1;
}

void CScanMgrDlg::InitUI()
{
	m_bmpBtnChangeExam.SetStateBitmap(IDB_ScanMgr_Btn_ChangeExam, 0, IDB_ScanMgr_Btn_ChangeExam_Hover);
	m_bmpBtnChangeExam.SetBtnTextColor(RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 0);
}

void CScanMgrDlg::InitScanner()
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
		m_vecScanSrc.clear();

		if (NULL != (pID = _pTWAINApp->getDefaultDataSource())) // Get Default
		{
			nDefault = pID->Id;
		}
		USES_CONVERSION;
		while (NULL != (pID = _pTWAINApp->getDataSource((TW_INT16)i)))
		{
//			index = m_comboScanner.AddString(A2T(pID->ProductName));
			m_vecScanSrc.push_back(A2T(pID->ProductName));
			if (LB_ERR == index)
			{
				break;
			}

// 			m_comboScanner.SetItemData(index, i);
// 
// 			if (nDefault == (int)pID->Id)
// 			{
// 				m_comboScanner.SetCurSel(index);
// 			}

			i++;
		}
		_pTWAINApp->disconnectDSM();
	}
}

void CScanMgrDlg::OnCbnSelchangeComboSubject()
{
	if (!chkChangeExamLegal())
		return;

	int nItem = m_comboSubject.GetCurSel();
	_pCurrSub_ = (pEXAM_SUBJECT)m_comboSubject.GetItemDataPtr(nItem);
	CScanTool3Dlg* pDlg = (CScanTool3Dlg*)GetParent();
	pDlg->SwitchDlg(1);
}

void CScanMgrDlg::ChildDlgShowPic(cv::Mat& matPic)
{
	m_pScanProcessDlg->ShowSinglePic(matPic);
}

void CScanMgrDlg::ResetSubjectUI()
{
	GetDlgItem(IDC_STATIC_CurrSubject)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CurrSubject)->EnableWindow(TRUE);
	m_comboSubject.ShowWindow(SW_SHOW);
	m_comboSubject.EnableWindow(TRUE);
	m_bmpBtnChangeExam.ShowWindow(SW_SHOW);
}

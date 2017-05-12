// ScanMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ScanMgrDlg.h"
#include "afxdialogex.h"

#include "Net_Cmd_Protocol.h"
// CScanMgrDlg 对话框

IMPLEMENT_DYNAMIC(CScanMgrDlg, CDialog)

CScanMgrDlg::CScanMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanMgrDlg::IDD, pParent)
	, m_pScanDlg(NULL), m_pDataCheckDlg(NULL), m_pScanRecordDlg(NULL)
{

}

CScanMgrDlg::~CScanMgrDlg()
{
}

void CScanMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BOOL CScanMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitChildDlg();
	InitCtrlPosition();

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
	ON_BN_CLICKED(IDC_BTN_Scan, &CScanMgrDlg::OnBnClickedBtnScan)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_DataCheck, &CScanMgrDlg::OnBnClickedBtnDatacheck)
	ON_BN_CLICKED(IDC_BTN_ScanRecord, &CScanMgrDlg::OnBnClickedBtnScanrecord)
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
	int nBtnW = cx * 0.1;
	int nBtnH = 50;
	if (nBtnW < 100) nBtnW = 100;
	if (nBtnW > 200) nBtnW = 200;
	if (GetDlgItem(IDC_BTN_Scan)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Scan)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (GetDlgItem(IDC_BTN_DataCheck)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_DataCheck)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (GetDlgItem(IDC_BTN_ScanRecord)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_ScanRecord)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}

	//child dlg
	if (m_pScanDlg && m_pScanDlg->GetSafeHwnd())
		m_pScanDlg->MoveWindow(m_rtChildDlg);
	if (m_pDataCheckDlg && m_pDataCheckDlg->GetSafeHwnd())
		m_pDataCheckDlg->MoveWindow(m_rtChildDlg);
	if (m_pScanRecordDlg && m_pScanRecordDlg->GetSafeHwnd())
		m_pScanRecordDlg->MoveWindow(m_rtChildDlg);
	Invalidate();
}


void CScanMgrDlg::InitChildDlg()
{
	m_pScanDlg = new CScanDlg(this);
	m_pScanDlg->Create(CScanDlg::IDD, this);
	m_pScanDlg->ShowWindow(SW_SHOW);

	m_pDataCheckDlg = new CDataCheckDlg(this);
	m_pDataCheckDlg->Create(CDataCheckDlg::IDD, this);
	m_pDataCheckDlg->ShowWindow(SW_HIDE);

	m_pScanRecordDlg = new CScanRecordDlg(this);
	m_pScanRecordDlg->Create(CScanRecordDlg::IDD, this);
	m_pScanRecordDlg->ShowWindow(SW_HIDE);
}

void CScanMgrDlg::ReleaseDlg()
{
	if (m_pScanDlg)
	{
		m_pScanDlg->DestroyWindow();
		SAFE_RELEASE(m_pScanDlg);
	}
	if (m_pDataCheckDlg)
	{
		m_pDataCheckDlg->DestroyWindow();
		SAFE_RELEASE(m_pDataCheckDlg);
	}
	if (m_pScanRecordDlg)
	{
		m_pScanRecordDlg->DestroyWindow();
		SAFE_RELEASE(m_pScanRecordDlg);
	}
}

void CScanMgrDlg::OnBnClickedBtnScan()
{
	m_pScanDlg->ShowWindow(SW_SHOW);
	m_pDataCheckDlg->ShowWindow(SW_HIDE);
	m_pScanRecordDlg->ShowWindow(SW_HIDE);
}

void CScanMgrDlg::OnBnClickedBtnDatacheck()
{
	m_pScanDlg->ShowWindow(SW_HIDE);
	m_pDataCheckDlg->ShowWindow(SW_SHOW);
	m_pScanRecordDlg->ShowWindow(SW_HIDE);
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
void CScanMgrDlg::UpdateInfo()
{
	if (m_pScanDlg)	m_pScanDlg->UpdateInfo();
}


void CScanMgrDlg::SearchModel()
{
	if (!_pCurrExam_ || !_pCurrSub_) return;

	if (_pModel_)
		SAFE_RELEASE(_pModel_);

	USES_CONVERSION;
	std::string strModelName = _pCurrSub_->strModelName;	//gb2312
	std::string strModelFullPath = T2A(g_strCurrentPath + _T("Model\\"));
	strModelFullPath += strModelName;
	bool bExist = false;
	try
	{
		Poco::File fModel(strModelFullPath);		//考试对应的模板文件存在，则直接解压此模板，否则就通过examID_subID来搜索模板
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
	
	if (bExist)
	{
		_pModel_ = LoadModelFile(A2T(strModelFullPath.c_str()));
	}
}

void CScanMgrDlg::DownLoadModel()
{
	if (!_pCurrExam_ || !_pCurrSub_) return;

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

	Poco::File fileModel(strModelPath);
	if (fileModel.exists())
	{
		std::string strMd5 = calcFileMd5(strModelPath);
		strncpy(stModelInfo.szMD5, strMd5.c_str(), strMd5.length());
	}

	std::string strLog = "请求下载模板: ";
	strLog.append(stModelInfo.szModelName);
	g_pLogger->information(strLog);

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_NEED_DOWN_MODEL;
	pTcpTask->nPkgLen = sizeof(ST_DOWN_MODEL);
	memcpy(pTcpTask->szSendBuf, (char*)&stModelInfo, sizeof(ST_DOWN_MODEL));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();


	g_eDownLoadModel.wait();
	if (g_nDownLoadModelStatus == 2 || g_nDownLoadModelStatus == 3)
	{
		//模板下载完成
	}
}

void CScanMgrDlg::OnBnClickedBtnScanrecord()
{
	m_pScanDlg->ShowWindow(SW_HIDE);
	m_pDataCheckDlg->ShowWindow(SW_HIDE);
	m_pScanRecordDlg->ShowWindow(SW_SHOW);
}

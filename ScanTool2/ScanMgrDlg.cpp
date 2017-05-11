// ScanMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ScanMgrDlg.h"
#include "afxdialogex.h"


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
	if (_pModel_)
		SAFE_RELEASE(_pModel_);

	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
	try
	{
		Poco::DirectoryIterator it(strModelPath);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			if (it->isFile() && p.getExtension() == "mod")
			{
				std::string strName = p.getFileName();
				std::string strPath = p.toString();

				std::string strModelName = strName;
				std::string strExamID;
				std::string strSubjectID;
				int nPos = 0;
				int nOldPos = 0;
				nPos = strModelName.find("_N_");
				if (nPos != std::string::npos)	//新模板名称
				{
					int nPos2 = strModelName.find("_", nPos + 3);

					strExamID = strModelName.substr(nPos + 3, nPos2 - nPos - 3);
					nOldPos = nPos2;
					nPos2 = strModelName.find(".", nPos2 + 1);
					strSubjectID = strModelName.substr(nOldPos + 1, nPos2 - nOldPos - 1);
				}
				else
				{
					nPos = strModelName.find("_");
					strExamID = strModelName.substr(0, nPos);
					nOldPos = nPos;
					nPos = strModelName.find(".", nPos + 1);
					strSubjectID = strModelName.substr(nOldPos + 1, nPos - nOldPos - 1);
				}

				char szIndex[50] = { 0 };
				sprintf(szIndex, "%s_%s", strExamID.c_str(), strSubjectID.c_str());

// 				pMODELINFO pModelInfo = new MODELINFO;
// 				pModelInfo->nExamID = atoi(strExamID.c_str());
// 				pModelInfo->nSubjectID = atoi(strSubjectID.c_str());
// 				pModelInfo->strName = CMyCodeConvert::Utf8ToGb2312(strName);
// 				pModelInfo->strPath = CMyCodeConvert::Utf8ToGb2312(strPath);
// 				pModelInfo->strMd5 = calcFileMd5(strPath);
// 
// 				_mapModelLock_.lock();
// 				_mapModel_.insert(MAP_MODEL::value_type(szIndex, pModelInfo));
// 				_mapModelLock_.unlock();
			}
			++it;
		}
	}
	catch (Poco::FileException& exc)
	{
		std::cerr << exc.displayText() << std::endl;
		return;
	}
	catch (Poco::Exception& exc)
	{
		std::cerr << exc.displayText() << std::endl;
		return;
	}
}

void CScanMgrDlg::OnBnClickedBtnScanrecord()
{
	m_pScanDlg->ShowWindow(SW_HIDE);
	m_pDataCheckDlg->ShowWindow(SW_HIDE);
	m_pScanRecordDlg->ShowWindow(SW_SHOW);
}

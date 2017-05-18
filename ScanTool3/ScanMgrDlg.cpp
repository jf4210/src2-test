// ScanMgrDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ScanMgrDlg.h"
#include "afxdialogex.h"

#include "Net_Cmd_Protocol.h"
#include "ZipObj.h"
// CScanMgrDlg �Ի���

IMPLEMENT_DYNAMIC(CScanMgrDlg, CDialog)

CScanMgrDlg::CScanMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanMgrDlg::IDD, pParent)
	, m_pScanDlg(NULL), m_pWaitDownloadDlg(NULL)
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

	InitData();
	InitChildDlg();
	InitCtrlPosition();
	m_scanThread.CreateThread();

	SetFontSize(m_nStatusSize);

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
END_MESSAGE_MAP()


// CScanMgrDlg ��Ϣ�������

void CScanMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 100;	//�ϱߵļ��
	const int nBottomGap = 10;	//�±ߵļ��
	const int nLeftGap = 20;		//��ߵĿհ׼��
	const int nRightGap = 20;	//�ұߵĿհ׼��
	int nGap = 5;

	m_rtChildDlg.left = nLeftGap;
	m_rtChildDlg.top = nTopGap;
	m_rtChildDlg.right = cx - nRightGap;
	m_rtChildDlg.bottom = cy - nBottomGap;

	int nCurrLeft = nLeftGap;
	int nCurrTop = 20;
	int nStaticW = (cx - nLeftGap - nRightGap) * 0.1;
	if (nStaticW < 50) nStaticW = 50;
	if (nStaticW > 70) nStaticW = 70;
	int nH = nTopGap - nCurrTop;
	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		int nRightTmpGap = 100;	//���ұ���ʱ�ճ�һ��
		int nW = cx - nLeftGap - nRightGap - nRightTmpGap - nStaticW - nGap - nStaticW - nGap * 3;
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
		GetDlgItem(IDC_COMBO_Subject)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nH);
		nCurrLeft += (nStaticW + nGap);
	}

	//child dlg
	if (m_pScanDlg && m_pScanDlg->GetSafeHwnd())
		m_pScanDlg->MoveWindow(m_rtChildDlg);
	if (m_pWaitDownloadDlg && m_pWaitDownloadDlg->GetSafeHwnd())
		m_pWaitDownloadDlg->MoveWindow(m_rtChildDlg);
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
	if (n == 1)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_SHOW);
		m_pScanDlg->ShowWindow(SW_HIDE);
	}
	else if (n == 2)
	{
		m_pWaitDownloadDlg->ShowWindow(SW_HIDE);
		m_pScanDlg->ShowWindow(SW_SHOW);
	}
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

void CScanMgrDlg::InitData()
{
	USES_CONVERSION;
	m_comboSubject.ResetContent();
	if (_pCurrExam_ && _pCurrSub_)
	{
		m_strExamName = A2T(_pCurrExam_->strExamName.c_str());

		int nShowSubject = -1;
		for (auto pSubject : _pCurrExam_->lSubjects)
		{
			int i = 0;
			m_comboSubject.AddString(A2T(pSubject->strSubjName.c_str()));
			if (_pCurrSub_ == pSubject)
				nShowSubject = i;
			i++;
		}
		m_comboSubject.SetCurSel(nShowSubject);
	}
	UpdateData(FALSE);
}


void CScanMgrDlg::ShowDlg()
{
	ShowChildDlg(1);
	SearchModel();		//����ģ��
	if (DownLoadModel())			//����ģ��ŵ��ȴ������д���,������Ͼ�����
	{
		ShowChildDlg(2);
		UpdateInfo();
	}
}

void CScanMgrDlg::UpdateInfo()
{
	InitData();
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
		Poco::File fModel(CMyCodeConvert::Gb2312ToUtf8(strModelFullPath));		//���Զ�Ӧ��ģ���ļ����ڣ���ֱ�ӽ�ѹ��ģ�壬�����ͨ��examID_subID������ģ��
		if (fModel.exists())
			bExist = true;
	}
	catch (Poco::Exception& e)
	{
	}

	//�ⲿ���Ǽ��غ���_examID_subID.��ģ�壬�����û�кͿ�Ŀ��Ϣ�м�¼��ģ������һ����ģ����ڵĻ����ͼ��ؿ���ID�Ϳ�ĿIDһ����ģ��
	//���Ҳ���
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
		CZipObj zipObj;
		zipObj.setLogger(g_pLogger);
		zipObj.UnZipFile(A2T(strModelFullPath.c_str()));

		CString strModelFilePath = g_strCurrentPath + _T("Model\\") + A2T(strModelName.c_str());
		strModelFilePath = strModelFilePath.Left(strModelFilePath.ReverseFind('.'));
		_pModel_ = LoadModelFile(strModelFilePath);
		std::string strLog;
		if (_pModel_)
			strLog = Poco::format("ģ��%s���سɹ�", strModelName);
		else
			strLog = Poco::format("ģ��%s����ʧ��", strModelName);
		g_pLogger->information(strLog);
	}
}

bool CScanMgrDlg::DownLoadModel()
{
	if (!_pCurrExam_ || !_pCurrSub_) return false;

	g_nDownLoadModelStatus = 0;

	//�Ȳ鱾���б����û������������У�����crc���ͷ�������ͬ������
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

	std::string strLog = "��������ģ��: ";
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
	bool bResult = false;
	if (g_nDownLoadModelStatus == 2)
	{
		//ģ���������
		strLog.append(" ==>��ģ�����سɹ�.");
		bResult = true;
	}
	else if (g_nDownLoadModelStatus == 3)
	{
		strLog.append(" ==>��ģ�屾���Լ����������޸ģ�����Ҫ��������.");
		bResult = true;
	}
	else if (g_nDownLoadModelStatus == -1)
	{
		strLog.append(" ==>�������˿�Ŀģ�岻����.");
	}
	else if (g_nDownLoadModelStatus == -2)
	{
		strLog.append(" ==>��������ȡ�ļ�ʧ��.");
	}
	g_pLogger->information(strLog);
	return bResult;
}


HBRUSH CScanMgrDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_ExamName || CurID == IDC_STATIC_CurrSubject || CurID == IDC_COMBO_Subject)
	{
		//		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
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

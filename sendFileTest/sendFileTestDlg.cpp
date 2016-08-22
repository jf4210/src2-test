
// sendFileTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "sendFileTest.h"
#include "sendFileTestDlg.h"
#include "afxdialogex.h"
#include "MyCodeConvert.h"
#include "Net_Cmd_Protocol.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int					g_nExitFlag = 0;				//�˳���ʾ

Poco::Logger*		g_pLogger;
Poco::Event*		g_peStartMulticast = new Poco::Event(false);

std::string			g_strCurPath;

Poco::FastMutex		g_fmSendLock;
SENDTASKLIST		g_lSendTask;	

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CsendFileTestDlg �Ի���



CsendFileTestDlg::CsendFileTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CsendFileTestDlg::IDD, pParent)
	, m_nThreads(0), m_nFileTasks(0), m_nChildProcess(0), m_nChildThreads(0), m_pMulticastServer(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CsendFileTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_SendFile, m_ctrlSendFile);
	DDX_Text(pDX, IDC_EDIT_Threads, m_nThreads);
	DDX_Text(pDX, IDC_EDIT_FileTasks, m_nFileTasks);
	DDX_Text(pDX, IDC_EDIT_Childs, m_nChildProcess);
	DDX_Text(pDX, IDC_EDIT_ChildThreads, m_nChildThreads);
}

BEGIN_MESSAGE_MAP(CsendFileTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SendFile, &CsendFileTestDlg::OnBnClickedBtnSendfile)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_SendTest, &CsendFileTestDlg::OnBnClickedBtnSendtest)
	ON_BN_CLICKED(IDC_BTN_Multicast, &CsendFileTestDlg::OnBnClickedBtnMulticast)
	ON_BN_CLICKED(IDC_BTN_StartChild, &CsendFileTestDlg::OnBnClickedBtnStartchild)
	ON_BN_CLICKED(IDC_BTN_StartChildThread, &CsendFileTestDlg::OnBnClickedBtnStartchildthread)
	ON_BN_CLICKED(IDC_BTN_Test, &CsendFileTestDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_Test2, &CsendFileTestDlg::OnBnClickedBtnTest2)
END_MESSAGE_MAP()


// CsendFileTestDlg ��Ϣ�������

BOOL CsendFileTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	InitConfig();
	m_pMulticastServer = new MulticastServer(m_strMulticastIP, m_nMulticastPort);

	UpdateData(FALSE);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CsendFileTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CsendFileTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CsendFileTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CsendFileTestDlg::OnBnClickedBtnSendfile()
{
	UpdateData(TRUE);
	if (m_nThreads <= 0)
	{
		AfxMessageBox(_T("�����߳�����"));
		return;
	}

	m_pRecogThread = new Poco::Thread[m_nThreads];
	for (int i = 0; i < m_nThreads; i++)
	{
		CSendFileThread* pObj = new CSendFileThread(m_strServerIP, m_nServerPort);
		m_pRecogThread[i].start(*pObj);

		m_vecRecogThreadObj.push_back(pObj);
	}
}


void CsendFileTestDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	g_nExitFlag = 1;
	if (g_peStartMulticast)
	{
		delete g_peStartMulticast;
		g_peStartMulticast = NULL;
	}
	Sleep(1000);
	for (int i = 0; i < m_vecRecogThreadObj.size(); i++)
	{
		m_pRecogThread[i].join();
	}
	Sleep(500);
	std::vector<CSendFileThread*>::iterator itRecogObj = m_vecRecogThreadObj.begin();
	for (; itRecogObj != m_vecRecogThreadObj.end();)
	{
		CSendFileThread* pObj = *itRecogObj;
		if (pObj)
		{
			delete pObj;
			pObj = NULL;
		}
		itRecogObj = m_vecRecogThreadObj.erase(itRecogObj);
	}
}

void CsendFileTestDlg::OnBnClickedBtnSendtest()
{
	CString strFilePath;
	m_ctrlSendFile.GetWindowTextW(strFilePath);

	if (strFilePath.IsEmpty())
	{
		AfxMessageBox(_T("δ�����ļ�·��"));
		return;
	}

	UpdateData(TRUE);
	if (m_nFileTasks <= 0)
	{
		AfxMessageBox(_T("�����ļ���������"));
		return;
	}

	std::string strPath;
	std::string strExt;
	std::string strBaseName;

	int nCurrentThreadID = Poco::Thread::currentTid();

	USES_CONVERSION;
	std::string strSrcPath = T2A(strFilePath);
	try
	{
		Poco::Path filePath(CMyCodeConvert::Gb2312ToUtf8(strSrcPath));
		strPath = filePath.toString();
		strExt = filePath.getExtension();
		strBaseName = filePath.getBaseName();

		int nPos = strPath.rfind('.');
		strPath = strPath.substr(0, nPos);

		for (int i = 1; i <= m_nFileTasks; i++)
		{
			std::string strNewFilePath = Poco::format("%s_%d_%d.%s", strPath, i, nCurrentThreadID, strExt);

			Poco::File fileTask(CMyCodeConvert::Gb2312ToUtf8(strSrcPath));
			fileTask.copyTo(strNewFilePath);			
		}
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "�ļ������쳣: " + CMyCodeConvert::Utf8ToGb2312(exc.displayText()) + "\n";
		TRACE(strLog.c_str());
	}

	for (int i = 1; i <= m_nFileTasks; i++)
	{
		std::string strNewFilePath = Poco::format("%s_%d_%d.%s", strPath, i, nCurrentThreadID, strExt);
		std::string strNewName = Poco::format("%s_%d_%d.%s", strBaseName, i, nCurrentThreadID, strExt);

		pSENDTASK pTask = new SENDTASK;
		char szFilePath[200] = { 0 };
		pTask->strPath = strNewFilePath;
		pTask->strName = strNewName;

		g_lSendTask.push_back(pTask);
	}
}

void CsendFileTestDlg::InitConfig()
{
	USES_CONVERSION;
	wchar_t szwPath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, szwPath, MAX_PATH);
	char szPath[MAX_PATH] = { 0 };
	int nLen = WideCharToMultiByte(CP_ACP, 0, szwPath, -1, NULL, 0, NULL, NULL);
	char* pszDst = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szwPath, -1, szPath, nLen, NULL, NULL);
	szPath[nLen - 1] = 0;
	delete[] pszDst;

	CString strFile(szPath);
	strFile = strFile.Left(strFile.ReverseFind('\\') + 1);
	g_strCurPath = T2A(strFile);
	
	std::string strName = Poco::format("sendFile_%d.log", (int)Poco::Thread::currentTid());

	std::string strLogPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strFile));	//_T("sendFile.log")
	strLogPath.append(strName);
	Poco::AutoPtr<Poco::PatternFormatter> pFormatter(new Poco::PatternFormatter("%L%Y-%m-%d %H:%M:%S.%F %q:%t"));
	Poco::AutoPtr<Poco::FormattingChannel> pFCFile(new Poco::FormattingChannel(pFormatter));
	Poco::AutoPtr<Poco::FileChannel> pFileChannel(new Poco::FileChannel(strLogPath));
	pFCFile->setChannel(pFileChannel);
	pFCFile->open();
	pFCFile->setProperty("rotation", "1 M");
	pFCFile->setProperty("archive", "timestamp");
	pFCFile->setProperty("compress", "true");
	pFCFile->setProperty("purgeCount", "5");

//	std::string strName = Poco::format("sendFile_%d", Poco::Thread::currentTid());

	Poco::Logger& appLogger = Poco::Logger::create(strName, pFCFile, Poco::Message::PRIO_INFORMATION);
	g_pLogger = &appLogger;

	CString strConfigPath = strFile;
	strConfigPath.Append(_T("config_sendFileTest.ini"));
	std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(T2A(strConfigPath));
	Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf(new Poco::Util::IniFileConfiguration(strUtf8Path));
	m_strServerIP = pConf->getString("SERVER.IP");
	m_nServerPort = pConf->getInt("SERVER.Port", 19980);

	m_strMulticastIP = pConf->getString("Multicast.IP");
	m_nMulticastPort = pConf->getInt("Multicast.Port", 19980);
}


void CsendFileTestDlg::OnBnClickedBtnMulticast()
{
	ST_CMD_HEADER stHead;
	stHead.usCmd = MULTICAST_START;
	stHead.uPackSize = 0;
	char szSendBuf[1024] = { 0 };
	memcpy(szSendBuf, &stHead, HEAD_SIZE);

	Poco::Net::MulticastSocket ms;
	int n = ms.sendTo(szSendBuf, HEAD_SIZE, m_pMulticastServer->group());
}


void CsendFileTestDlg::OnBnClickedBtnStartchild()
{
	UpdateData(TRUE);
	if (m_nChildProcess <= 0)
	{
		AfxMessageBox(_T("�����������ӽ�������"));
		return;
	}

	ST_CMD_HEADER stHead;
	stHead.usCmd = MULTICAST_INIT_PROCESS;
	stHead.uPackSize = 0;
	stHead.usResult = m_nChildProcess;
	char szSendBuf[1024] = { 0 };
	memcpy(szSendBuf, &stHead, HEAD_SIZE);

	Poco::Net::MulticastSocket ms;
	int n = ms.sendTo(szSendBuf, HEAD_SIZE, m_pMulticastServer->group());
}


void CsendFileTestDlg::OnBnClickedBtnStartchildthread()
{
	UpdateData(TRUE);
	if (m_nChildThreads <= 0)
	{
		AfxMessageBox(_T("�������������߳�����"));
		return;
	}

	ST_CMD_HEADER stHead;
	stHead.usCmd = MULTICAST_INIT_THREAD;
	stHead.uPackSize = 0;
	stHead.usResult = m_nChildThreads;
	char szSendBuf[1024] = { 0 };
	memcpy(szSendBuf, &stHead, HEAD_SIZE);

	Poco::Net::MulticastSocket ms;
	int n = ms.sendTo(szSendBuf, HEAD_SIZE, m_pMulticastServer->group());
}

bool SortByIndex(RECTPOS& rc1, RECTPOS& rc2)
{
	return rc1.nIndex < rc2.nIndex;
}

void CsendFileTestDlg::OnBnClickedBtnTest()
{
	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("All Files (*.*)|*.*;)||"),
					NULL);
	if (dlg.DoModal() != IDOK)
		return;

	USES_CONVERSION;
	std::string strJsnModel = T2A(dlg.GetPathName());

	std::string strJsnData;
	std::ifstream in(strJsnModel);
	if (!in)
		return ;

	std::string strJsnLine;
	while (!in.eof())
	{
		getline(in, strJsnLine);					//�����˿ո�
		strJsnData.append(strJsnLine);
	}
	in.close();

	pMODEL pModel = new MODEL;

	Poco::JSON::Parser parser;
	Poco::Dynamic::Var result;
	try
	{
		result = parser.parse(strJsnData);		//strJsnData
		Poco::JSON::Array::Ptr arryData = result.extract<Poco::JSON::Array::Ptr>();	
		for (int i = 0; i < arryData->size(); i++)
		{
			Poco::JSON::Object::Ptr objTK = arryData->getObject(i);
			Poco::JSON::Object::Ptr objHeader = objTK->getObject("syncHeader");
			Poco::JSON::Object::Ptr objAnchorPoint = objTK->getObject("anchorPoint");
			Poco::JSON::Object::Ptr objSubject = objTK->getObject("subject");
			Poco::JSON::Object::Ptr objPageNum = objTK->getObject("pageNum");
//			Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
			Poco::JSON::Object::Ptr objElement = objTK->getObject("elements");

			Poco::JSON::Object::Ptr objCurSubject = objSubject->getObject("curSubject");
			pModel->strModelName = A2T(CMyCodeConvert::Gb2312ToUtf8(objCurSubject->get("name").convert<std::string>()).c_str());

			pPAPERMODEL pPaperModel = new PAPERMODEL;

			pPaperModel->nPaper = objPageNum->get("curPageNum").convert<int>();
			pPaperModel->strModelPicName = _T("");	//ͼƬ���ƣ�Ŀǰ��֪��

			//ͬ��ͷ
			std::vector<RECTPOS> vecHeader_H;
			std::vector<RECTPOS> vecHeader_V;
			//��ȡ�����е�ͬ��ͷ��Ϣ
			Poco::JSON::Array::Ptr arryItems = objAnchorPoint->getArray("items");
			for (int j = 0; j < arryItems->size(); j++)
			{
				Poco::JSON::Object::Ptr objItem = arryItems->getObject(j);
				int nHor = objItem->get("horIndex").convert<int>();
				int nVer = objItem->get("verIndex").convert<int>();
				RECTPOS rc;
				rc.rt.x			= objItem->get("x").convert<int>();
				rc.rt.y			= objItem->get("y").convert<int>();
				rc.rt.width		= objItem->get("width").convert<int>();
				rc.rt.height	= objItem->get("height").convert<int>();
				if (nHor == 0 && nVer == 0)
				{
					rc.nIndex = 0;
					vecHeader_H.push_back(rc);
					vecHeader_V.push_back(rc);
				}
				else if (nHor == 0 && nVer != 0)
				{
					rc.nIndex = nVer;
					vecHeader_V.push_back(rc);
				}
				else if (nHor != 0 && nVer == 0)
				{
					rc.nIndex = nHor;
					vecHeader_H.push_back(rc);
				}
			}
			Poco::JSON::Array::Ptr arryHorHeaders = objHeader->getArray("horHeaders");
			for (int k = 0; k < arryHorHeaders->size(); k++)
			{
				Poco::JSON::Object::Ptr objItem = arryHorHeaders->getObject(k);
				RECTPOS rc;
				rc.rt.x			= objItem->get("x").convert<int>();
				rc.rt.y			= objItem->get("y").convert<int>();
				rc.rt.width		= objItem->get("width").convert<int>();
				rc.rt.height	= objItem->get("height").convert<int>();
				rc.nIndex		= objItem->get("index").convert<int>();
				vecHeader_H.push_back(rc);
			}
			Poco::JSON::Array::Ptr arryVerHeaders = objHeader->getArray("verHeaders");
			for (int k = 0; k < arryVerHeaders->size(); k++)
			{
				Poco::JSON::Object::Ptr objItem = arryVerHeaders->getObject(k);
				RECTPOS rc;
				rc.rt.x			= objItem->get("x").convert<int>();
				rc.rt.y			= objItem->get("y").convert<int>();
				rc.rt.width		= objItem->get("width").convert<int>();
				rc.rt.height	= objItem->get("height").convert<int>();
				rc.nIndex		= objItem->get("index").convert<int>();
				vecHeader_V.push_back(rc);
			}
			std::sort(vecHeader_H.begin(), vecHeader_H.end(), SortByIndex);
			std::sort(vecHeader_H.begin(), vecHeader_H.end(), SortByIndex);

			//����ͬ��ͷ��Ƥ���С
			cv::Rect rcTracker_H, rcTracker_V;
			cv::Point pt1 = vecHeader_H[0].rt.tl() - cv::Point(50,50);
			cv::Point pt2 = vecHeader_H[vecHeader_H.size() - 1].rt.br() + cv::Point(50,50);
			cv::Point pt3 = vecHeader_V[0].rt.tl() - cv::Point(50, 50);
			cv::Point pt4 = vecHeader_V[vecHeader_V.size() - 1].rt.br() + cv::Point(50, 50);
			pPaperModel->rtHTracker = cv::Rect(pt1, pt2);
			pPaperModel->rtVTracker = cv::Rect(pt3, pt4);

			//����ͬ��ͷ
			for (int m = 0; m < vecHeader_H.size(); m++)
			{
				RECTINFO rc;
				rc.eCPType = H_HEAD;
				rc.nHItem = vecHeader_H[m].nIndex;
				rc.nVItem = 0;
				rc.rt = vecHeader_H[m].rt;
				pPaperModel->lSelHTracker.push_back(rc);
			}
			for (int m = 0; m < vecHeader_V.size(); m++)
			{
				RECTINFO rc;
				rc.eCPType = V_HEAD;
				rc.nHItem = 0;
				rc.nVItem = vecHeader_V[m].nIndex;
				rc.rt = vecHeader_V[m].rt;
				pPaperModel->lSelVTracker.push_back(rc);
			}

			//׼��֤��
			cv::Point ptZkzh1, ptZkzh2;
			if (!objTK->isNull("zkzh"))
			{
				Poco::JSON::Object::Ptr objBaseInfo = objTK->getObject("baseInfo");
				Poco::JSON::Object::Ptr objZKZH = objBaseInfo->getObject("zkzh");
				Poco::JSON::Array::Ptr arryZkzhItems = objZKZH->getArray("items");
				for (int n = 0; n < arryZkzhItems->size(); n++)
				{
					Poco::JSON::Object::Ptr objItem = arryZkzhItems->getObject(n);
					int nItem = objItem->get("index").convert<int>();
					if (nItem == 0)
					{
						Poco::JSON::Object::Ptr objPanel = objItem->getObject("panel");
						ptZkzh1.x = objPanel->get("x").convert<int>();
						ptZkzh1.y = objPanel->get("y").convert<int>();
					}
					if (nItem == arryZkzhItems->size() - 1)
					{
						Poco::JSON::Object::Ptr objPanel = objItem->getObject("panel");
						ptZkzh2.x = objPanel->get("x").convert<int>();
						ptZkzh2.y = objPanel->get("y").convert<int>();
					}

					//��ȡÿ�е�׼��֤��Ϣ
					pSN_ITEM pSnItem = new SN_ITEM;
					pSnItem->nItem = nItem;

					Poco::JSON::Array::Ptr arryZkzhGrids = objItem->getArray("grids");
					for (int k = 0; k < arryZkzhGrids->size(); k++)
					{
						Poco::JSON::Object::Ptr objGrids = arryZkzhGrids->getObject(k);
						RECTINFO rc;
						rc.eCPType = SN;
						rc.rt.x = objGrids->get("x").convert<int>();
						rc.rt.y = objGrids->get("y").convert<int>();
						rc.rt.width = objGrids->get("width").convert<int>();
						rc.rt.height = objGrids->get("height").convert<int>();
						rc.nSnVal = objGrids->get("index").convert<int>();
						rc.nHItem = objGrids->get("horIndex").convert<int>();
						rc.nVItem = objGrids->get("verIndex").convert<int>();
						pSnItem->lSN.push_back(rc);
					}
					pPaperModel->lSNInfo.push_back(pSnItem);
				}
				ptZkzh1 += cv::Point(2, 2);			//׼��֤��Ƥ�����ţ���ֹѡ��̫��
				ptZkzh2 -= cv::Point(2, 2);			//׼��֤��Ƥ�����ţ���ֹѡ��̫��
				pPaperModel->rtSNTracker = cv::Rect(ptZkzh1, ptZkzh2);
			}			

			//OMR����
			Poco::JSON::Array::Ptr arryQuestions = objElement->getArray("questions");
			for (int n = 0; n < arryQuestions->size(); n++)
			{
				Poco::JSON::Object::Ptr objQuestion = arryQuestions->getObject(n);
				OMR_QUESTION omrItem;
				omrItem.nTH = objQuestion->get("num").convert<int>();
				omrItem.nSingle = objQuestion->get("choiceType").convert<int>() - 1;

				Poco::JSON::Array::Ptr arryOptions = objQuestion->getArray("options");
				for (int k = 0; k < arryOptions->size(); k++)
				{
					Poco::JSON::Object::Ptr objOptions = arryOptions->getObject(k);
					RECTINFO rc;
					rc.eCPType = OMR;
					rc.rt.x = objOptions->get("x").convert<int>();
					rc.rt.y = objOptions->get("y").convert<int>();
					rc.rt.width = objOptions->get("width").convert<int>();
					rc.rt.height = objOptions->get("height").convert<int>();
					rc.nHItem = objOptions->get("horIndex").convert<int>();
					rc.nVItem = objOptions->get("verIndex").convert<int>();
					rc.nAnswer = (int)objOptions->get("label").convert<char>() - 65;
					rc.nTH = omrItem.nTH;
					rc.nSingle = omrItem.nSingle;
					omrItem.lSelAnswer.push_back(rc);
				}
				pPaperModel->lOMR2.push_back(omrItem);
			}

			//��ӿ�Ŀ��

			//���ȱ����

			//��ӻҶȵ�

			//��Ӱ�У���

			//����Ծ�ģ�嵽��ģ��
			pModel->vecPaperModel.push_back(pPaperModel);
		}

	}
	catch (Poco::JSON::JSONException& jsone)
	{
		std::string strErrInfo;
		strErrInfo.append("����ģ���ļ�����jsonʧ��: ");
		strErrInfo.append(jsone.message());
		g_pLogger->information(strErrInfo);
	}
	catch (Poco::Exception& exc)
	{
		std::string strErrInfo;
		strErrInfo.append("����ģ���ļ�����jsonʧ��2: ");
		strErrInfo.append(exc.message());
		g_pLogger->information(strErrInfo);
	}
}


void CsendFileTestDlg::OnBnClickedBtnTest2()
{
	std::string strPath = "E:\\myWorkspace\\yklx\\bin\\debug\\fileRecv\\";

	static int i = 1;

	USES_CONVERSION;
	std::string strPaperPath = CMyCodeConvert::Gb2312ToUtf8(g_strCurPath);
	Poco::DirectoryIterator it(strPaperPath);
	Poco::DirectoryIterator end;
	while (it != end)
	{
		Poco::Path p(it->path());
		if (it->isFile())
		{
			Poco::Path pSubFile(it->path());
			Poco::File file(it->path());
			std::string strOldFileName = pSubFile.getFileName();

			if (strOldFileName.find("_Child") != std::string::npos)
			{
				file.remove();
			}
		}
		it++;
	}
}

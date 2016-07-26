// ScanModleMgrDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanModleMgrDlg.h"
#include "afxdialogex.h"
#include "GetModelDlg.h"
#include "ScanToolDlg.h"
#include "MakeModelDlg.h"

// CScanModleMgrDlg �Ի���

IMPLEMENT_DYNAMIC(CScanModleMgrDlg, CDialog)

CScanModleMgrDlg::CScanModleMgrDlg(pMODEL pModel, CWnd* pParent /*=NULL*/)
	: CDialog(CScanModleMgrDlg::IDD, pParent)
	, m_pShowModelInfoDlg(NULL), m_pModel(NULL), m_nCurModelItem(-1)
	, m_pOldModel(pModel), m_strCurModelName(_T(""))
{

}

CScanModleMgrDlg::~CScanModleMgrDlg()
{
	SAFE_RELEASE(m_pShowModelInfoDlg);

	for (int i = 0; i < m_vecModel.size(); i++)
	{
		if (m_pModel != m_vecModel[i])
		{
			pMODEL pModel = m_vecModel[i];
			SAFE_RELEASE(pModel);
		}
	}
	m_vecModel.clear();
}

void CScanModleMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Model, m_ModelListCtrl);
	DDX_Text(pDX, IDC_EDIT_CurModel, m_strCurModelName);
}


BEGIN_MESSAGE_MAP(CScanModleMgrDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_Refresh, &CScanModleMgrDlg::OnBnClickedBtnRefresh)
	ON_BN_CLICKED(IDC_BTN_DLModel, &CScanModleMgrDlg::OnBnClickedBtnDlmodel)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Model, &CScanModleMgrDlg::OnNMDblclkListModel)
	ON_BN_CLICKED(IDC_BTN_DelModel, &CScanModleMgrDlg::OnBnClickedBtnDelmodel)
	ON_BN_CLICKED(IDC_BTN_AddModel, &CScanModleMgrDlg::OnBnClickedBtnAddmodel)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Model, &CScanModleMgrDlg::OnNMHoverListModel)
	ON_BN_CLICKED(IDOK, &CScanModleMgrDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_MakeModel, &CScanModleMgrDlg::OnBnClickedBtnMakemodel)
	ON_BN_CLICKED(IDCANCEL, &CScanModleMgrDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_uploadModel, &CScanModleMgrDlg::OnBnClickedBtnuploadmodel)
END_MESSAGE_MAP()

BOOL CScanModleMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_pOldModel)
		m_strCurModelName = m_pOldModel->strModelName;
	
	InitUI();

	OnBnClickedBtnRefresh();
	UpdateData(FALSE);

	return TRUE;
}

void CScanModleMgrDlg::InitUI()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	m_ModelListCtrl.SetExtendedStyle(m_ModelListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_ModelListCtrl.InsertColumn(0, _T("���"), LVCFMT_CENTER, 36);
	m_ModelListCtrl.InsertColumn(1, _T("ģ������"), LVCFMT_CENTER, 170);

	m_pShowModelInfoDlg = new CShowModelInfoDlg(this);
	m_pShowModelInfoDlg->Create(CShowModelInfoDlg::IDD, this);
	m_pShowModelInfoDlg->ShowWindow(SW_SHOW);

	InitCtrlPosition();
}

void CScanModleMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nGap = 2;
	int nLeftGag = 2;

	if (m_pShowModelInfoDlg && m_pShowModelInfoDlg->GetSafeHwnd())
	{
		m_pShowModelInfoDlg->MoveWindow(250, 12, 250, 235);
	}
}

// CScanModleMgrDlg ��Ϣ�������
void CScanModleMgrDlg::OnBnClickedBtnRefresh()
{
	m_pShowModelInfoDlg->ShowModelInfo(NULL);
	m_ModelListCtrl.DeleteAllItems();
	for (int i = 0; i < m_vecModel.size(); i++)
	{
		pMODEL pModel = m_vecModel[i];
		SAFE_RELEASE(pModel);
	}
	m_vecModel.clear();
	m_pModel = NULL;

	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));

	std::string strLog;
	int nCount = 0;
	try
	{
		std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strModelPath);
		Poco::DirectoryIterator it(strUtf8Path);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			if (it->isFile() && p.getExtension() == "mod")
			{
				//				std::string strModelName = p.getBaseName();
				std::string strModelName = CMyCodeConvert::Utf8ToGb2312(p.getBaseName());
				char szCount[10] = { 0 };
				sprintf_s(szCount, "%d", nCount + 1);
				m_ModelListCtrl.InsertItem(nCount, NULL);
				m_ModelListCtrl.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
				m_ModelListCtrl.SetItemText(nCount, 1, (LPCTSTR)A2T(strModelName.c_str()));
				m_ModelListCtrl.SetItemData(nCount, NULL);
				
				if (strModelName == T2A(m_strCurModelName))
				{
					m_ModelListCtrl.SetItemState(nCount, LVIS_DROPHILITED, LVIS_DROPHILITED);		//������ʾһ�У�ʧȥ�����Ҳһֱ��ʾ

					CString strModelFilePath = g_strCurrentPath + _T("Model\\") + m_strCurModelName;

					m_pModel = LoadModelFile(strModelFilePath);
					m_vecModel.push_back(m_pModel);
					m_pShowModelInfoDlg->ShowModelInfo(m_pModel, 1);
				}
				nCount++;
// 				CString strModelFilePath = g_strCurrentPath + _T("Model\\") + A2T(strModelName.c_str());
// 				pMODEL pModel;
// 				pModel = LoadModelFile(strModelFilePath);
// 				m_vecModel.push_back(pModel);
			}
			it++;
		}
		strLog = "����ģ�����";
	}
	catch (Poco::FileException& exc)
	{
		strLog = "����ģ��ʧ��: " + exc.displayText();
	}
	catch (Poco::Exception& exc)
	{
		strLog = "����ģ��ʧ��2: " + exc.displayText();
	}
	g_pLogger->information(strLog);
}


void CScanModleMgrDlg::OnBnClickedBtnDlmodel()
{
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();

	if (!pDlg->m_bLogin)
	{
		AfxMessageBox(_T("���ȵ�¼"));
		return;
	}

	USES_CONVERSION;
	CGetModelDlg dlg(A2T(pDlg->m_strCmdServerIP.c_str()), pDlg->m_nCmdPort);
	if (dlg.DoModal() != IDOK)
		return;

	OnBnClickedBtnRefresh();
}


void CScanModleMgrDlg::OnNMDblclkListModel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	m_ModelListCtrl.SetItemState(m_nCurModelItem, 0, LVIS_DROPHILITED);

	m_nCurModelItem = pNMItemActivate->iItem;
	if (m_nCurModelItem < 0)
		return;

	m_ModelListCtrl.SetItemState(m_nCurModelItem, LVIS_DROPHILITED, LVIS_DROPHILITED);

	pMODEL pModel = NULL;
	pModel = (pMODEL)m_ModelListCtrl.GetItemData(m_nCurModelItem);
	if (NULL == pModel)
	{
		CString strModelName = m_ModelListCtrl.GetItemText(m_nCurModelItem, 1);
		CString strModelFilePath = g_strCurrentPath + _T("Model\\") + strModelName;

		CString strModelFullPath = strModelFilePath + _T(".mod");
		UnZipFile(strModelFullPath);

		pModel = LoadModelFile(strModelFilePath);
		m_vecModel.push_back(pModel);

		m_ModelListCtrl.SetItemData(m_nCurModelItem, (DWORD_PTR)pModel);
	}
	m_pModel = pModel;
	if (m_pModel)
		m_strCurModelName = m_pModel->strModelName;

	m_pShowModelInfoDlg->ShowModelInfo(m_pModel, 1);
	UpdateData(FALSE);
}


void CScanModleMgrDlg::OnBnClickedBtnDelmodel()
{
	USES_CONVERSION;
	POSITION pos = m_ModelListCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		TRACE0("No items were selected!\n");
	else
	{
		while (pos)
		{
			int nItem = m_ModelListCtrl.GetNextSelectedItem(pos);
			TRACE1("Item %d was selected!\n", nItem);

			pMODEL pModel = NULL;
			pModel = (pMODEL)m_ModelListCtrl.GetItemData(nItem);
			if (pModel&& pModel == m_pModel)
			{
//				SAFE_RELEASE(m_pModel);
				m_pModel = NULL;
				m_strCurModelName = _T("");
			}

			CString strModelName = m_ModelListCtrl.GetItemText(nItem, 1);
			CString strModelFilePath = g_strCurrentPath + _T("Model\\") + strModelName + _T(".mod");
			CString strModelDirPath = g_strCurrentPath + _T("Model\\") + strModelName;

			std::string strUtfFilePath = CMyCodeConvert::Gb2312ToUtf8(T2A(strModelFilePath));
			std::string strUtfDirPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strModelDirPath));

			std::string strLog = "ɾ��ģ���ļ�: ";
			strLog.append(T2A(strModelFilePath));
			g_pLogger->information(strLog);

			try
			{
				Poco::File modelFile(strUtfFilePath);
				modelFile.remove(true);
				Poco::File modelDir(strUtfDirPath);	
				modelDir.remove(true);
			}
			catch (Poco::Exception &exc)
			{
				std::string strLog;
				strLog.append("model file remove error: " + exc.displayText());
				std::string strGBLog = CMyCodeConvert::Utf8ToGb2312(strLog);
				g_pLogger->information(strGBLog);
				TRACE(strGBLog.c_str());
			}
		}
		UpdateData(FALSE);
		OnBnClickedBtnRefresh();
	}
}


void CScanModleMgrDlg::OnBnClickedBtnAddmodel()
{
	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("mod Files (*.mod)|*.mod;)||"),
					NULL);
	if (dlg.DoModal() != IDOK)
		return;

	USES_CONVERSION;
	CString strModelPath = dlg.GetPathName();
	CString strModelName = dlg.GetFileName();
	CString strModelNewPath = g_strCurrentPath + _T("Model\\") + strModelName;
	try
	{
		std::string strUtf8OldPath	= CMyCodeConvert::Gb2312ToUtf8(T2A(strModelPath));
		std::string strUtf8ModelPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strModelNewPath));
		CString strModelNewDirPath	= strModelNewPath.Left(strModelNewPath.GetLength() - 4);
		std::string strUtf8ModelDir = CMyCodeConvert::Gb2312ToUtf8(T2A(strModelNewDirPath));

		Poco::File modelPath0(strUtf8ModelPath);
		if (modelPath0.exists())
		{
			if (MessageBox(_T("�Ѿ����ڴ�ģ�壬�Ƿ񸲸�"), _T(""), MB_YESNO) != IDYES)
			{
				return;
			}
			modelPath0.remove(true);

			Poco::File modelDirPath(strUtf8ModelDir);
			modelDirPath.remove(true);

			std::string strLog = "ɾ��ģ��: ";
			strLog.append(T2A(strModelName));
			g_pLogger->information(strLog);
		}
		std::string strLog = "����ģ��: ";
		strLog.append(T2A(strModelPath));
		g_pLogger->information(strLog);

		Poco::File fileModelPath(g_strModelSavePath);
		fileModelPath.createDirectories();

		Poco::File modelPath(strUtf8OldPath);
		modelPath.copyTo(strUtf8ModelPath);

		OnBnClickedBtnRefresh();
	}
	catch (Poco::Exception &exc)
	{
		std::string strLog;
		strLog.append("file cope error: " + exc.displayText());
		std::string strGBLog = CMyCodeConvert::Utf8ToGb2312(strLog);
		g_pLogger->information(strGBLog);
		TRACE(strGBLog.c_str());
	}
}


void CScanModleMgrDlg::OnNMHoverListModel(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;		//**********	�����������Ӧ��ͬʱ���ؽ��ֵ��Ϊ1�Ļ���	****************
						//**********	�ͻ��������TRACK SELECT��Ҳ���������ͣ	****************
						//**********	һ��ʱ����������Զ���ѡ��
}


void CScanModleMgrDlg::OnBnClickedOk()
{
	for (int i = 0; i < m_vecModel.size(); i++)
	{
		if (m_pModel != m_vecModel[i])
		{
			pMODEL pModel = m_vecModel[i];
			SAFE_RELEASE(pModel);
		}
	}
	m_vecModel.clear();
	CDialog::OnOK();
}


void CScanModleMgrDlg::OnBnClickedBtnMakemodel()
{
#ifndef SHOW_GUIDEDLG
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();

	pDlg->ReleaseTwain();
	pDlg->m_bTwainInit = FALSE;
#endif

	CMakeModelDlg dlg(m_pModel);
	dlg.DoModal();

	if (!m_pModel)	//���ģ�岻Ϊ�գ�˵��֮ǰ�Ѿ���ģ���ˣ�����Ҫʹ����ģ��
		m_pModel = dlg.m_pModel;

	if (m_pModel != dlg.m_pModel)
	{
		SAFE_RELEASE(dlg.m_pModel);
	}
}


void CScanModleMgrDlg::OnBnClickedCancel()
{
	for (int i = 0; i < m_vecModel.size(); i++)
	{
		if (m_pModel != m_vecModel[i])
		{
			pMODEL pModel = m_vecModel[i];
			SAFE_RELEASE(pModel);
		}
	}
	m_vecModel.clear();
	CDialog::OnCancel();
}


void CScanModleMgrDlg::OnBnClickedBtnuploadmodel()
{
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();

	if (!pDlg->m_bLogin)
	{
		AfxMessageBox(_T("���ȵ�¼"));
		return;
	}

	if (!m_pModel)
	{
		AfxMessageBox(_T("����ѡ��ģ��"));
		return;
	}

	if (m_pModel->nSaveMode == 1)
	{
		AfxMessageBox(_T("��ģ��Ϊ����ģʽ�������ϴ���"));
		return;
	}

	USES_CONVERSION;
	std::string strModelName = T2A(m_pModel->strModelName);
	strModelName.append(".mod");

	int nPos = 0;
	int nOldPos = 0;
	nPos = strModelName.find("_");
	std::string strExamID = strModelName.substr(0, nPos);
	nOldPos = nPos;
	nPos = strModelName.find(".", nPos + 1);
	std::string strSubjectID = strModelName.substr(nOldPos + 1, nPos - nOldPos - 1);

	CString modelPath = g_strCurrentPath + _T("Model");
	modelPath = modelPath + _T("\\") + m_pModel->strModelName + _T(".mod");
	setUploadModelInfo(m_pModel->strModelName, modelPath, atoi(strExamID.c_str()), atoi(strSubjectID.c_str()));

	AfxMessageBox(_T("����ϴ�������ɣ���̨�����С�����"));
}

void CScanModleMgrDlg::setUploadModelInfo(CString& strName, CString& strModelPath, int nExamId, int nSubjectId)
{
	USES_CONVERSION;
	std::string strPath = T2A(strModelPath);
	std::string strMd5;

	strMd5 = calcFileMd5(strPath);

	CScanToolDlg* pDlg = (CScanToolDlg*)AfxGetMainWnd();	//GetParent();

	ST_MODELINFO stModelInfo;
	ZeroMemory(&stModelInfo, sizeof(ST_MODELINFO));
	stModelInfo.nExamID = nExamId;
	stModelInfo.nSubjectID = nSubjectId;
	sprintf_s(stModelInfo.szUserNo, "%s", T2A(pDlg->m_strUserName));
	sprintf_s(stModelInfo.szModelName, "%s.mod", T2A(strName));
	sprintf_s(stModelInfo.szEzs, "%s", T2A(pDlg->m_strEzs));
	strncpy(stModelInfo.szMD5, strMd5.c_str(), strMd5.length());

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_SETMODELINFO;
	pTcpTask->nPkgLen = sizeof(ST_MODELINFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stModelInfo, sizeof(ST_MODELINFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
}

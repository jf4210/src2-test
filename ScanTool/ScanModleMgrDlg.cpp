// ScanModleMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanModleMgrDlg.h"
#include "afxdialogex.h"
#include "GetModelDlg.h"
#include "ScanToolDlg.h"
#include "GuideDlg.h"
#include "MakeModelDlg.h"


// CScanModleMgrDlg 对话框

IMPLEMENT_DYNAMIC(CScanModleMgrDlg, CDialog)

CScanModleMgrDlg::CScanModleMgrDlg(pMODEL pModel, CWnd* pParent /*=NULL*/)
	: CDialog(CScanModleMgrDlg::IDD, pParent)
	, m_pShowModelInfoDlg(NULL), m_pModel(NULL), m_nCurModelItem(-1)
	, m_pOldModel(pModel), m_strCurModelName(_T(""))
	, m_pMakeModelDlg(NULL)
{
	EnableToolTips(TRUE);
	m_ListTip.Create(this);
	m_ListTip.SetMaxTipWidth(500);
	m_ListTip.SetTipTextColor(RGB(255, 0, 0));//设置提示字体颜色
	m_ListTip.SetTipBkColor(RGB(255, 255, 255));//设置提示背景颜色
	m_ListTip.Activate(TRUE);
}

CScanModleMgrDlg::~CScanModleMgrDlg()
{
	SAFE_RELEASE(m_pShowModelInfoDlg);
	SAFE_RELEASE(m_pMakeModelDlg);

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
	ON_NOTIFY(LVN_HOTTRACK, IDC_LIST_Model, &CScanModleMgrDlg::OnLvnHotTrackListModel)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CScanModleMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	USES_CONVERSION;

	if (m_pOldModel)
		m_strCurModelName = A2T(m_pOldModel->strModelName.c_str());
	
	InitUI();

	OnBnClickedBtnRefresh();
	UpdateData(FALSE);

	return TRUE;
}

int CScanModleMgrDlg::GetBmkInfo()
{
	if (!m_pModel)
		return 0;

	CString strUser = _T("");
#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();

	strUser = pDlg->m_strUserName;
#else
	CScanToolDlg* pDlg = (CScanToolDlg*)AfxGetMainWnd();
	strUser = pDlg->m_strUserName;
#endif

	if (!pDlg->m_bLogin)
		return 0;

	USES_CONVERSION;
	ST_GET_BMK_INFO stGetBmkInfo;
	ZeroMemory(&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
	stGetBmkInfo.nExamID = m_pModel->nExamID;
	stGetBmkInfo.nSubjectID = m_pModel->nSubjectID;
	strcpy(stGetBmkInfo.szEzs, T2A(pDlg->m_strEzs));

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_GET_BMK;
	pTcpTask->nPkgLen = sizeof(ST_GET_BMK_INFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stGetBmkInfo, sizeof(ST_GET_BMK_INFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
}

void CScanModleMgrDlg::InitUI()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	m_ModelListCtrl.SetExtendedStyle(m_ModelListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_ModelListCtrl.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 36);
	m_ModelListCtrl.InsertColumn(1, _T("模板名称"), LVCFMT_CENTER, 400);
	m_ModelListCtrl.InsertColumn(2, _T("修改时间"), LVCFMT_CENTER, 100);

	m_pShowModelInfoDlg = new CShowModelInfoDlg(this);
	m_pShowModelInfoDlg->Create(CShowModelInfoDlg::IDD, this);
	m_pShowModelInfoDlg->ShowWindow(SW_SHOW);

	int sx = 1000;
	int sy = 600;
	MoveWindow(0, 0, sx, sy);
	CenterWindow();

	InitCtrlPosition();
}

void CScanModleMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nGap = 5;	//控件的间隔
	int nTopGap = 10;	//距离上边边缘的间隔
	int nBottomGap = 5;	//距离下边边缘的间隔
	int nLeftGap = 5;	//距离左边边缘的间隔
	int nRightGap = 5;	//距离右边边缘的间隔
	int nBtnHeigh = 30;
	int	nBtnWidth = (cx / 2 - nLeftGap - nGap * 2) / 3;

	int nStaticWidth = 60;
	int nStaticHeight = 20;		//校验点类型Static控件高度
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	if (GetDlgItem(IDC_STATIC_CurModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_CurModel)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth, nStaticHeight);
		nCurrentLeft = nCurrentLeft + nStaticWidth + nGap;
	}
	if (GetDlgItem(IDC_EDIT_CurModel)->GetSafeHwnd())
	{
		int nEditW = cx / 2 - nLeftGap - nStaticWidth - nGap;
		GetDlgItem(IDC_EDIT_CurModel)->MoveWindow(nCurrentLeft, nCurrentTop, nEditW, nStaticHeight);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}

	if (GetDlgItem(IDC_STATIC_List)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_List)->MoveWindow(nCurrentLeft, nCurrentTop, nStaticWidth + 20, nStaticHeight);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}

	if (GetDlgItem(IDC_LIST_Model)->GetSafeHwnd())
	{
		int nListW = cx / 2 - nLeftGap;
		int nListH = cy - nTopGap - nBottomGap - nStaticHeight - nGap - nStaticHeight - nGap - (nBtnHeigh + nGap + nBtnHeigh) - nGap;
		GetDlgItem(IDC_LIST_Model)->MoveWindow(nCurrentLeft, nCurrentTop, nListW, nListH);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nListH + nGap;
	}

	if (GetDlgItem(IDC_BTN_AddModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_AddModel)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentLeft = nCurrentLeft + nBtnWidth + nGap;
	}
	if (GetDlgItem(IDC_BTN_DLModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_DLModel)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentLeft = nCurrentLeft + nBtnWidth + nGap;
	}
	if (GetDlgItem(IDC_BTN_MakeModel)->GetSafeHwnd())
	{
		int nBtnH = nBtnHeigh * 2 + nGap;
		GetDlgItem(IDC_BTN_MakeModel)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnWidth, nBtnH);
		nCurrentLeft = nLeftGap;
		nCurrentTop = nCurrentTop + nBtnHeigh + nGap;
	}
	if (GetDlgItem(IDC_BTN_DelModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_DelModel)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentLeft = nCurrentLeft + nBtnWidth + nGap;
	}
	if (GetDlgItem(IDC_BTN_uploadModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_uploadModel)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnWidth, nBtnHeigh);
		nCurrentLeft = nCurrentLeft + nBtnWidth + nGap;
	}

	//右半边
	nCurrentLeft = cx / 2 + nGap;
	nCurrentTop = nTopGap;
	if (m_pShowModelInfoDlg && m_pShowModelInfoDlg->GetSafeHwnd())
	{
		int nW = cx / 2 - nGap - nRightGap;
		int nH = cy - nTopGap - nBottomGap - nBtnHeigh * 2 - nGap - nGap;
		m_pShowModelInfoDlg->MoveWindow(nCurrentLeft, nCurrentTop, nW, nH);
		nCurrentTop = nCurrentTop + nH + nGap;
	}

	nCurrentLeft = cx / 2 + nLeftGap + 10;
	nCurrentTop = cy - nBottomGap - nBtnHeigh;
	int nBtnW2 = (cx - nCurrentLeft - nGap * 2 - nRightGap - 10) / 3;
	if (GetDlgItem(IDC_BTN_Refresh)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Refresh)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW2, nBtnHeigh);
		nCurrentLeft = nCurrentLeft + nBtnW2 + nGap;
	}
	if (GetDlgItem(IDOK)->GetSafeHwnd())
	{
		GetDlgItem(IDOK)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW2, nBtnHeigh);
		nCurrentLeft = nCurrentLeft + nBtnW2 + nGap;
	}
	if (GetDlgItem(IDCANCEL)->GetSafeHwnd())
	{
		GetDlgItem(IDCANCEL)->MoveWindow(nCurrentLeft, nCurrentTop, nBtnW2, nBtnHeigh);
		nCurrentLeft = nCurrentLeft + nBtnW2 + nGap;
	}
}

// CScanModleMgrDlg 消息处理程序
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
				Poco::DateTime dt(it->getLastModified());
				std::string strLastModifyTime = Poco::format("%04d-%02d-%02d", dt.year(), dt.month(), dt.day());
				m_ModelListCtrl.SetItemText(nCount, 2, (LPCTSTR)A2T(strLastModifyTime.c_str()));
				m_ModelListCtrl.SetItemData(nCount, NULL);
				
				if (strModelName == T2A(m_strCurModelName))
				{
					m_ModelListCtrl.SetItemState(nCount, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行，失去焦点后也一直显示

					CString strModelFilePath = g_strCurrentPath + _T("Model\\") + m_strCurModelName;

					m_pModel = LoadModelFile(strModelFilePath);
					m_vecModel.push_back(m_pModel);
					m_pShowModelInfoDlg->ShowModelInfo(m_pModel, 1);
				}
				nCount++;
			}
			it++;
		}
		strLog = "搜索模板完成";
	}
	catch (Poco::FileException& exc)
	{
		strLog = "搜索模板失败: " + exc.displayText();
	}
	catch (Poco::Exception& exc)
	{
		strLog = "搜索模板失败2: " + exc.displayText();
	}
	g_pLogger->information(strLog);
}


void CScanModleMgrDlg::OnBnClickedBtnDlmodel()
{
#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();

	if (!pDlg->m_bLogin)
	{
		AfxMessageBox(_T("请先登录"));
		return;
	}
#else
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();
	if (!pDlg->m_bLogin)
	{
		AfxMessageBox(_T("请先登录"));
		return;
	}
#endif


	USES_CONVERSION;
	CGetModelDlg dlg(A2T(g_strCmdIP.c_str()), g_nCmdPort);
	if (dlg.DoModal() != IDOK)
		return;

	OnBnClickedBtnRefresh();
}


void CScanModleMgrDlg::OnNMDblclkListModel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	USES_CONVERSION;
	if (pNMItemActivate->iItem < 0)
		return;

	m_ModelListCtrl.SetItemState(m_nCurModelItem, 0, LVIS_DROPHILITED);		//使上一次的高亮显示行取消高亮
	m_nCurModelItem = pNMItemActivate->iItem;
	m_ModelListCtrl.SetItemState(m_nCurModelItem, LVIS_DROPHILITED, LVIS_DROPHILITED);		//高亮显示一行

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
		m_strCurModelName = A2T(m_pModel->strModelName.c_str());

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
		if (MessageBox(_T("确定删除所选模板？"), _T("警告！"), MB_YESNO) == IDYES)
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

				std::string strLog = "删除模板文件: ";
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
			if (MessageBox(_T("已经存在此模板，是否覆盖"), _T(""), MB_YESNO) != IDYES)
			{
				return;
			}
			modelPath0.remove(true);

			Poco::File modelDirPath(strUtf8ModelDir);
			modelDirPath.remove(true);

			std::string strLog = "删除模板: ";
			strLog.append(T2A(strModelName));
			g_pLogger->information(strLog);
		}
		std::string strLog = "导入模板: ";
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
	*pResult = 1;		//**********	这里如果不响应，同时返回结果值不为1的话，	****************
						//**********	就会产生产生TRACK SELECT，也就是鼠标悬停	****************
						//**********	一段时间后，所在行自动被选中
}


void CScanModleMgrDlg::OnBnClickedOk()
{
	if (!m_pModel)
	{
		AfxMessageBox(_T("当前模板为空，请选择扫描模板!"));
		return;
	}

	USES_CONVERSION;
	CString strShow = _T("");
	strShow.Format(_T("是否选择\"%s\"为扫描模板?"), A2T(m_pModel->strModelName.c_str()));
	if (MessageBox(strShow, _T("扫描模板确认"), MB_OKCANCEL) != IDOK)
		return;

	GetBmkInfo();

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

#if 0
	SAFE_RELEASE(m_pMakeModelDlg);

	m_pMakeModelDlg = new CMakeModelDlg(m_pModel);
	m_pMakeModelDlg->Create(CMakeModelDlg::IDD, this);
	m_pMakeModelDlg->ShowWindow(SW_SHOW);
//	m_pMakeModelDlg->DoModal();

	if (!m_pModel)	//如果模板不为空，说明之前已经有模板了，不需要使用新模板
		m_pModel = m_pMakeModelDlg->m_pModel;

	if (m_pModel != m_pMakeModelDlg->m_pModel)
	{
		if (m_pMakeModelDlg->m_pModel)
			TRACE("\n\n这是新建的模板\n\n");
		SAFE_RELEASE(m_pMakeModelDlg->m_pModel);
	}

	SAFE_RELEASE(m_pMakeModelDlg);
#else
	CMakeModelDlg dlg(m_pModel);
	dlg.DoModal();

	if (!m_pModel)	//如果模板不为空，说明之前已经有模板了，不需要使用新模板
		m_pModel = dlg.m_pModel;

	if (m_pModel != dlg.m_pModel)
	{
		if (dlg.m_pModel)
			TRACE("\n\n这是新建的模板\n\n");
		SAFE_RELEASE(dlg.m_pModel);
	}
#endif
	OnBnClickedBtnRefresh();
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
	int nTeacherId = -1;
	int nUserId = -1;
	CString strEzs = _T("");

#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();

	if (!pDlg->m_bLogin)
	{
		AfxMessageBox(_T("请先登录"));
		return;
	}
#else
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();
	if (!pDlg->m_bLogin)
	{
		AfxMessageBox(_T("请先登录"));
		return;
	}
#endif
	strEzs = pDlg->m_strEzs;
	nTeacherId = pDlg->m_nTeacherId;
	nUserId = pDlg->m_nUserId;

	if (!m_pModel)
	{
		AfxMessageBox(_T("请先选择模板"));
		return;
	}

	if (m_pModel->nSaveMode == 1)
	{
		AfxMessageBox(_T("此模板为本地模式，不可上传！"));
		return;
	}

	//++选做题的模板信息随模板信息上传
	std::string strElectOmrInfo;
	if (m_pModel->nHasElectOmr)
	{
		Poco::JSON::Object jsnDataObj;
		Poco::JSON::Array jsnElectOmrArry;
		for (int i = 0; i < m_pModel->vecPaperModel.size(); i++)
		{
			Poco::JSON::Object jsnPaperElectOmrObj;
			Poco::JSON::Array jsnPaperElectOmrArry;		//单页试卷上的选做题信息
			ELECTOMR_LIST::iterator itElectOmr = m_pModel->vecPaperModel[i]->lElectOmr.begin();
			for (; itElectOmr != m_pModel->vecPaperModel[i]->lElectOmr.end(); itElectOmr++)
			{
				Poco::JSON::Object jsnElectOmr;
				jsnElectOmr.set("paperId", i + 1);
				jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
				jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
				jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
				Poco::JSON::Array jsnPositionArry;
				RECTLIST::iterator itRect = itElectOmr->lItemInfo.begin();
				for (; itRect != itElectOmr->lItemInfo.end(); itRect++)
				{
					Poco::JSON::Object jsnItem;
					char szVal[5] = { 0 };
					sprintf_s(szVal, "%c", itRect->nAnswer + 65);
					jsnItem.set("val", szVal);
					jsnItem.set("x", itRect->rt.x);
					jsnItem.set("y", itRect->rt.y);
					jsnItem.set("w", itRect->rt.width);
					jsnItem.set("h", itRect->rt.height);
					jsnPositionArry.add(jsnItem);
				}
				jsnElectOmr.set("position", jsnPositionArry);
				jsnElectOmrArry.add(jsnElectOmr);
			}
		}

		std::stringstream jsnOmrString;
		jsnElectOmrArry.stringify(jsnOmrString, 0);
		strElectOmrInfo = jsnOmrString.str();
		TRACE("%s\n", strElectOmrInfo.c_str());
	}
	//--

	USES_CONVERSION;
	std::string strModelName = m_pModel->strModelName;
	strModelName.append(".mod");

	int nPos = 0;
	int nOldPos = 0;
	std::string strExamID;
	std::string strSubjectID;
#ifdef TEST_MODEL_NAME

	nPos = strModelName.find("_N_");

	if (nPos != std::string::npos)
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
	
#else
	nPos = strModelName.find("_");
	strExamID = strModelName.substr(0, nPos);
	nOldPos = nPos;
	nPos = strModelName.find(".", nPos + 1);
	strSubjectID = strModelName.substr(nOldPos + 1, nPos - nOldPos - 1);
#endif

	CString modelPath = g_strCurrentPath + _T("Model");
	modelPath = modelPath + _T("\\") + A2T(m_pModel->strModelName.c_str()) + _T(".mod");
	setUploadModelInfo(m_pModel->strModelName, modelPath, atoi(strExamID.c_str()), atoi(strSubjectID.c_str()), strElectOmrInfo);
	
	AfxMessageBox(_T("添加上传任务完成，后台操作中。。。"));
}

void CScanModleMgrDlg::setUploadModelInfo(std::string& strName, CString& strModelPath, int nExamId, int nSubjectId, std::string& strElectOmrInfo)
{
	USES_CONVERSION;
	std::string strPath = T2A(strModelPath);
	std::string strMd5;

	strMd5 = calcFileMd5(strPath);

	ST_MODELINFO stModelInfo;
	ZeroMemory(&stModelInfo, sizeof(ST_MODELINFO));
	stModelInfo.nExamID = nExamId;
	stModelInfo.nSubjectID = nSubjectId;

	sprintf_s(stModelInfo.szModelName, "%s.mod", strName.c_str());
	strncpy(stModelInfo.szMD5, strMd5.c_str(), strMd5.length());
	strncpy(stModelInfo.szElectOmr, strElectOmrInfo.c_str(), strElectOmrInfo.length());

#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();

	sprintf_s(stModelInfo.szUserNo, "%s", T2A(pDlg->m_strUserName));
	sprintf_s(stModelInfo.szEzs, "%s", T2A(pDlg->m_strEzs));
#else
	CScanToolDlg* pDlg = (CScanToolDlg*)AfxGetMainWnd();	//GetParent();
	
	sprintf_s(stModelInfo.szUserNo, "%s", T2A(pDlg->m_strUserName));
	sprintf_s(stModelInfo.szEzs, "%s", T2A(pDlg->m_strEzs));
#endif

	pTCP_TASK pTcpTask = new TCP_TASK;
	pTcpTask->usCmd = USER_SETMODELINFO;
	pTcpTask->nPkgLen = sizeof(ST_MODELINFO);
	memcpy(pTcpTask->szSendBuf, (char*)&stModelInfo, sizeof(ST_MODELINFO));
	g_fmTcpTaskLock.lock();
	g_lTcpTask.push_back(pTcpTask);
	g_fmTcpTaskLock.unlock();
}

BOOL CScanModleMgrDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		case WM_MOUSEMOVE:
			m_ListTip.RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}




void CScanModleMgrDlg::OnLvnHotTrackListModel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
// 	CRect   rect;
// 	m_ModelListCtrl.GetWindowRect(rect);
// 	CPoint   point;
// 	GetCursorPos(&point);
// 	point.x = point.x - rect.left;
// 	point.y = point.y - rect.top;
// 
// 	LVHITTESTINFO   LVHTestInfo;
// 	LVHTestInfo.pt = point;
// 	m_ModelListCtrl.HitTest(&LVHTestInfo);
// 	int	m_nRow = LVHTestInfo.iItem;   //行数
// 	m_ModelListCtrl.SubItemHitTest(&LVHTestInfo);
// 	int m_nCol = LVHTestInfo.iSubItem;  //列数

	CString stritem;
//	stritem = m_ModelListCtrl.GetItemText(m_nRow, m_nCol);
	stritem = m_ModelListCtrl.GetItemText(pNMLV->iItem, pNMLV->iSubItem);
	m_ListTip.AddTool(GetDlgItem(IDC_LIST_Model), stritem);
	// 显示提示框   
	m_ListTip.Pop();

	*pResult = 0;
}


void CScanModleMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

// ScanModleMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanModleMgrDlg.h"
#include "afxdialogex.h"
#include "GetModelDlg.h"
#include "ScanToolDlg.h"

// CScanModleMgrDlg 对话框

IMPLEMENT_DYNAMIC(CScanModleMgrDlg, CDialog)

CScanModleMgrDlg::CScanModleMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanModleMgrDlg::IDD, pParent)
	, m_pShowModelInfoDlg(NULL), m_pModel(NULL), m_nCurModelItem(-1)
{

}

CScanModleMgrDlg::~CScanModleMgrDlg()
{
	SAFE_RELEASE(m_pShowModelInfoDlg);

	for (int i = 0; i < m_vecModel.size(); i++)
	{
		pMODEL pModel = m_vecModel[i];
		SAFE_RELEASE(pModel);
	}
	m_vecModel.clear();
}

void CScanModleMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Model, m_ModelListCtrl);
}


BEGIN_MESSAGE_MAP(CScanModleMgrDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_Refresh, &CScanModleMgrDlg::OnBnClickedBtnRefresh)
	ON_BN_CLICKED(IDC_BTN_DLModel, &CScanModleMgrDlg::OnBnClickedBtnDlmodel)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Model, &CScanModleMgrDlg::OnNMDblclkListModel)
	ON_BN_CLICKED(IDC_BTN_DelModel, &CScanModleMgrDlg::OnBnClickedBtnDelmodel)
	ON_BN_CLICKED(IDC_BTN_AddModel, &CScanModleMgrDlg::OnBnClickedBtnAddmodel)
	ON_NOTIFY(NM_HOVER, IDC_LIST_Model, &CScanModleMgrDlg::OnNMHoverListModel)
END_MESSAGE_MAP()

BOOL CScanModleMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();

	OnBnClickedBtnRefresh();

	return TRUE;
}

void CScanModleMgrDlg::InitUI()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	m_ModelListCtrl.SetExtendedStyle(m_ModelListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_ModelListCtrl.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 36);
	m_ModelListCtrl.InsertColumn(1, _T("模板名称"), LVCFMT_CENTER, 170);

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
		m_pShowModelInfoDlg->MoveWindow(250, 12, 200, 200);
	}
}


// CScanModleMgrDlg 消息处理程序


void CScanModleMgrDlg::OnBnClickedBtnRefresh()
{
	m_ModelListCtrl.DeleteAllItems();
	for (int i = 0; i < m_vecModel.size(); i++)
	{
		pMODEL pModel = m_vecModel[i];
		SAFE_RELEASE(pModel);
	}
	m_vecModel.clear();

	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
//	g_strModelSavePath = CMyCodeConvert::Gb2312ToUtf8(strModelPath);

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
				nCount++;

// 				CString strModelFilePath = g_strCurrentPath + _T("Model\\") + A2T(strModelName.c_str());
// 				pMODEL pModel;
// 				pModel = LoadModelFile(strModelFilePath);
// 				m_vecModel.push_back(pModel);
			}
			it++;
		}
		strLog = "搜索模板完成";
		//		m_comboModel.SetCurSel(0);
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
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();

	if (!pDlg->m_bLogin)
	{
		AfxMessageBox(_T("请先登录"));
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

	m_nCurModelItem = pNMItemActivate->iItem;
	if (m_nCurModelItem < 0)
		return;

	pMODEL pModel = NULL;
	pModel = (pMODEL)m_ModelListCtrl.GetItemData(m_nCurModelItem);
	if (NULL == pModel)
	{
		CString strModelName = m_ModelListCtrl.GetItemText(m_nCurModelItem, 1);
		CString strModelFilePath = g_strCurrentPath + _T("Model\\") + strModelName;

		pModel = LoadModelFile(strModelFilePath);
		m_vecModel.push_back(pModel);

		m_ModelListCtrl.SetItemData(m_nCurModelItem, (DWORD_PTR)pModel);
	}
	m_pModel = pModel;
	m_pShowModelInfoDlg->ShowModelInfo(m_pModel);
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
		std::string strUtf8OldPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strModelPath));
		std::string strUtf8ModelPath = CMyCodeConvert::Gb2312ToUtf8(T2A(strModelNewPath));

		Poco::File modelPath0(strUtf8ModelPath);
		if (modelPath0.exists())
		{
			if (MessageBox(_T("已经存在此模板，是否覆盖"), _T(""), MB_YESNO) != IDYES)
			{
				return;
			}
		}
		std::string strLog = "导入模板: ";
		strLog.append(T2A(strModelPath));
		g_pLogger->information(strLog);

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

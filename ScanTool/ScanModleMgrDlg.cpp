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
	, m_pShowModelInfoDlg(NULL)
{

}

CScanModleMgrDlg::~CScanModleMgrDlg()
{
	SAFE_RELEASE(m_pShowModelInfoDlg);
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
END_MESSAGE_MAP()

BOOL CScanModleMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();

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
	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
	g_strModelSavePath = CMyCodeConvert::Gb2312ToUtf8(strModelPath);

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
				nCount++;
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
}


void CScanModleMgrDlg::OnNMDblclkListModel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	*pResult = 0;
}

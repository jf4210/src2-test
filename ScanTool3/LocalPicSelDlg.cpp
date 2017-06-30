// LocalPicSelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "LocalPicSelDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"

// CLocalPicSelDlg 对话框

IMPLEMENT_DYNAMIC(CLocalPicSelDlg, CDialog)

CLocalPicSelDlg::CLocalPicSelDlg(CWnd* pParent /*=NULL*/)
: CTipBaseDlg(CLocalPicSelDlg::IDD, pParent)
, m_nCurrentItem(-1)
{

}

CLocalPicSelDlg::~CLocalPicSelDlg()
{
}

void CLocalPicSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PIC, m_listPath);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_bmpBtnClose);
}


BEGIN_MESSAGE_MAP(CLocalPicSelDlg, CTipBaseDlg)
	ON_NOTIFY(NM_CLICK, IDC_LIST_PIC, &CLocalPicSelDlg::OnNMClickListPic)
	ON_BN_CLICKED(IDC_BTN_ADD, &CLocalPicSelDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CLocalPicSelDlg::OnBnClickedBtnDel)
	ON_BN_CLICKED(IDC_BTN_OK, &CLocalPicSelDlg::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CLocalPicSelDlg::OnBnClickedBtnClose)
END_MESSAGE_MAP()


// CLocalPicSelDlg 消息处理程序
BOOL CLocalPicSelDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();


	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);
	m_listPath.SetExtendedStyle(m_listPath.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_listPath.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 36);
	m_listPath.InsertColumn(1, _T("路径"), LVCFMT_CENTER, 330);

	m_vecPath.clear();

	return TRUE;
}

void CLocalPicSelDlg::OnNMClickListPic(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (pNMItemActivate->iItem < 0)
		return;

	m_nCurrentItem = pNMItemActivate->iItem;
}


void CLocalPicSelDlg::OnBnClickedBtnAdd()
{
	UpdateData(TRUE);

	// 	CFileDialog dlg(true, NULL, NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
	// 		_T("image file(*.bmp;*.png;*.tif;*.tiff;*.jpg)|*.bmp;*.png;*.tif;*.tiff;*.jpg|All Files(*.*)|*.*;)||"), NULL);
	//	dlg.m_ofn.lpstrTitle = _T("选择模板图像");

	// 	static char szFilter[] = "All File(*.*)|*.*||";
	// 	CFileDialog dlg(TRUE, _T("*.*"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);

	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("All Files (*.*)|*.*;)||"),
					NULL);
	if (dlg.DoModal() != IDOK)
		return;

	MODELPICPATH st;
	st.strName = dlg.GetFileName();
	st.strPath = dlg.GetPathName();
	m_vecPath.push_back(st);

	USES_CONVERSION;
	int nItem = m_vecPath.size() - 1;
	char szItem[5] = { 0 };
	sprintf_s(szItem, "%d", nItem + 1);
	m_listPath.InsertItem(nItem, NULL);
	m_listPath.SetItemText(nItem, 0, (LPCTSTR)A2T(szItem));
	m_listPath.SetItemText(nItem, 1, (LPCTSTR)st.strPath);
}


void CLocalPicSelDlg::OnBnClickedBtnDel()
{
	if (m_nCurrentItem < 0 || m_nCurrentItem > m_vecPath.size() - 1)
		return;

	std::vector<MODELPICPATH>::iterator it = m_vecPath.begin() + m_nCurrentItem;
	if (it != m_vecPath.end())
		m_vecPath.erase(it);

	USES_CONVERSION;
	m_listPath.DeleteAllItems();
	for (int i = 0; i < m_vecPath.size(); i++)
	{
		//		int nItem = m_vecPath.size() - 1;
		char szItem[5] = { 0 };
		sprintf_s(szItem, "%d", i + 1);
		m_listPath.InsertItem(i, NULL);
		m_listPath.SetItemText(i, 0, (LPCTSTR)A2T(szItem));
		m_listPath.SetItemText(i, 1, (LPCTSTR)m_vecPath[i].strPath);
	}
}


void CLocalPicSelDlg::OnBnClickedBtnOk()
{
	if (m_vecPath.size() <= 0)
	{
		CNewMessageBox dlg;
		dlg.setShowInfo(2, 2, "未选择模板图像信息");
		if (dlg.DoModal() != IDOK)
			return;

// 		if(MessageBox(_T("未选择模板图像信息"), _T(""), MB_OKCANCEL) != IDOK)
// 			return;
	}
	OnOK();
}


void CLocalPicSelDlg::OnBnClickedBtnClose()
{
	OnCancel();
}

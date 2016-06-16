// ModelInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ModelInfoDlg.h"
#include "afxdialogex.h"


// CModelInfoDlg 对话框

IMPLEMENT_DYNAMIC(CModelInfoDlg, CDialog)

CModelInfoDlg::CModelInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModelInfoDlg::IDD, pParent)
	, m_nPaperNum(2), m_bABPaperModel(FALSE), m_bHasHead(TRUE), m_nCurrentItem(-1)
{

}

CModelInfoDlg::~CModelInfoDlg()
{
}

void CModelInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Papers, m_nPaperNum);
	DDX_Control(pDX, IDC_LIST_PIC, m_listPath);
}


BEGIN_MESSAGE_MAP(CModelInfoDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_OK, &CModelInfoDlg::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_RADIO_NotABPaper, &CModelInfoDlg::OnBnClickedRadioNotabpaper)
	ON_BN_CLICKED(IDC_RADIO_ABPaper, &CModelInfoDlg::OnBnClickedRadioAbpaper)
	ON_BN_CLICKED(IDC_RADIO_NoHead, &CModelInfoDlg::OnBnClickedRadioNohead)
	ON_BN_CLICKED(IDC_RADIO_HasHead, &CModelInfoDlg::OnBnClickedRadioHashead)
	ON_BN_CLICKED(IDC_BTN_ADD, &CModelInfoDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CModelInfoDlg::OnBnClickedBtnDel)
	ON_NOTIFY(NM_CLICK, IDC_LIST_PIC, &CModelInfoDlg::OnNMClickListPic)
END_MESSAGE_MAP()

BOOL CModelInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);

	((CButton*)GetDlgItem(IDC_RADIO_NotABPaper))->SetCheck(!m_bABPaperModel);
	((CButton*)GetDlgItem(IDC_RADIO_ABPaper))->SetCheck(m_bABPaperModel);

	((CButton*)GetDlgItem(IDC_RADIO_HasHead))->SetCheck(m_bHasHead);
	((CButton*)GetDlgItem(IDC_RADIO_NoHead))->SetCheck(!m_bHasHead);

	m_listPath.SetExtendedStyle(m_listPath.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_listPath.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 36);
	m_listPath.InsertColumn(1, _T("路径"), LVCFMT_CENTER, 220);

	return TRUE;
}


// CModelInfoDlg 消息处理程序


void CModelInfoDlg::OnBnClickedBtnOk()
{
	UpdateData(TRUE);
	if (m_nPaperNum <= 0)
	{
		AfxMessageBox(_T("试卷数量设置错误"));
		return;
	}
	if (m_vecPath.size() != m_nPaperNum)
	{
		AfxMessageBox(_T("请设置每张模板图片的路径"));
		return;
	}

	m_bABPaperModel = static_cast<bool>(((CButton*)GetDlgItem(IDC_RADIO_ABPaper))->GetCheck());
	m_bHasHead = static_cast<bool>(((CButton*)GetDlgItem(IDC_RADIO_HasHead))->GetCheck());
	OnOK();
}


void CModelInfoDlg::OnBnClickedRadioNotabpaper()
{
	if (BST_UNCHECKED == ((CButton*)GetDlgItem(IDC_RADIO_NotABPaper))->GetCheck())
	{
		((CButton*)GetDlgItem(IDC_RADIO_NotABPaper))->SetCheck(BST_CHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_ABPaper))->SetCheck(BST_UNCHECKED);
	}
}


void CModelInfoDlg::OnBnClickedRadioAbpaper()
{
	if (BST_UNCHECKED == ((CButton*)GetDlgItem(IDC_RADIO_ABPaper))->GetCheck())
	{
		((CButton*)GetDlgItem(IDC_RADIO_NotABPaper))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_ABPaper))->SetCheck(BST_CHECKED);
	}
}


void CModelInfoDlg::OnBnClickedRadioNohead()
{
	if (BST_UNCHECKED == ((CButton*)GetDlgItem(IDC_RADIO_NoHead))->GetCheck())
	{
		((CButton*)GetDlgItem(IDC_RADIO_HasHead))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_NoHead))->SetCheck(BST_CHECKED);
	}
}


void CModelInfoDlg::OnBnClickedRadioHashead()
{
	if (BST_UNCHECKED == ((CButton*)GetDlgItem(IDC_RADIO_HasHead))->GetCheck())
	{
		((CButton*)GetDlgItem(IDC_RADIO_HasHead))->SetCheck(BST_CHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_NoHead))->SetCheck(BST_UNCHECKED);
	}
}


void CModelInfoDlg::OnBnClickedBtnAdd()
{
	UpdateData(TRUE);
	if (m_nPaperNum <= 0)
	{
		AfxMessageBox(_T("试卷数量设置错误"));
		return;
	}

	if (m_vecPath.size() >= m_nPaperNum)
	{
		AfxMessageBox(_T("模板图片路径添加达到最大值"));
		return;
	}
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

	TRACE("*************1\n\n");

	MODELPATH st;
	st.strName = dlg.GetFileName();
	st.strPath = dlg.GetPathName();
	m_vecPath.push_back(st);

	TRACE("*************2\n\n");

	USES_CONVERSION;
	int nItem = m_vecPath.size() - 1;
	char szItem[5] = { 0 };
	sprintf_s(szItem, "%d", nItem + 1);
	m_listPath.InsertItem(nItem, NULL);
	m_listPath.SetItemText(nItem, 0, (LPCTSTR)A2T(szItem));
	m_listPath.SetItemText(nItem, 1, (LPCTSTR)st.strPath);
}


void CModelInfoDlg::OnBnClickedBtnDel()
{
	if (m_nCurrentItem < 0 || m_nCurrentItem > m_vecPath.size() - 1)
		return;

	std::vector<MODELPATH>::iterator it = m_vecPath.begin() + m_nCurrentItem;
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


void CModelInfoDlg::OnNMClickListPic(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	m_nCurrentItem = pNMItemActivate->iItem;
}

// CreateModelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "CreateModelDlg.h"
#include "afxdialogex.h"
#include "ScanModelPaperDlg.h"
#include "NewMakeModelDlg.h"
#include "LocalPicSelDlg.h"
// CCreateModelDlg 对话框

IMPLEMENT_DYNAMIC(CCreateModelDlg, CDialog)

CCreateModelDlg::CCreateModelDlg(CWnd* pParent /*=NULL*/)
: CTipBaseDlg(CCreateModelDlg::IDD, pParent)
, m_nSearchType(1)
{

}

CCreateModelDlg::~CCreateModelDlg()
{
}

void CCreateModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCreateModelDlg, CTipBaseDlg)
	ON_BN_CLICKED(IDC_BTN_Start, &CCreateModelDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_RADIO_FromScanner, &CCreateModelDlg::OnBnClickedRadioFromscanner)
	ON_BN_CLICKED(IDC_RADIO_FromLocalFile, &CCreateModelDlg::OnBnClickedRadioFromlocalfile)
	ON_BN_CLICKED(IDC_RADIO_FromServer, &CCreateModelDlg::OnBnClickedRadioFromserver)
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_FromServer, &CCreateModelDlg::OnStnClickedStaticFromserver)
	ON_STN_CLICKED(IDC_STATIC_FromScanner, &CCreateModelDlg::OnStnClickedStaticFromscanner)
	ON_STN_CLICKED(IDC_STATIC_FromLocalFile, &CCreateModelDlg::OnStnClickedStaticFromlocalfile)
END_MESSAGE_MAP()


// CCreateModelDlg 消息处理程序
BOOL CCreateModelDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	SetFontSize();
	switch (m_nSearchType)
	{
		case 1:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(0);
			break;
		case 2:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(0);
			break;
		case 3:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(1);
			break;
	}

	return TRUE;
}

void CCreateModelDlg::OnBnClickedBtnStart()
{
	if (m_nSearchType == 1)
	{
		//扫描获取图像
		CNewMakeModelDlg* pDlg = (CNewMakeModelDlg*)GetParent();

		CScanModelPaperDlg dlg(pDlg);
		dlg.SetScanSrc(pDlg->m_vecScanSrc);
		if (dlg.DoModal() != IDOK)
			return;
		m_strScanSavePath = dlg.m_strSavePath;
	}
	else if (m_nSearchType == 2)
	{
		//从本地选择
// 		CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
// 						| OFN_ALLOWMULTISELECT, _T("image file(*.jpg)|*.jpg|All Files(*.*)|*.*||"), NULL/*AfxGetMainWnd()*/);  //构造文件打开对话框  
// 		CString strPath = _T("");                 //声明变量  
// 		if (dlg.DoModal() == IDOK)                       //判断是否按下"打开"按钮  
// 		{
// 			POSITION m_Position = dlg.GetStartPosition();
// 			while (m_Position != NULL)
// 			{
// 				strPath = dlg.GetNextPathName(m_Position);
// 				MODELPATH st;
// 				st.strName = dlg.GetFileName();
// 				st.strPath = strPath;
// 				m_vecPath.push_back(st);
// 			}
// 		}

	#if 1
		CLocalPicSelDlg dlg;
		dlg.DoModal();

		if(dlg.m_vecPath.size() <= 0)
		{
			OnCancel();
			return;
		}
		m_vecPath = dlg.m_vecPath;

	#else
		OPENFILENAME ofn;
		TCHAR szOpenFileNames[5 * MAX_PATH];
		TCHAR szPath[MAX_PATH];
		TCHAR szFileName[5 * MAX_PATH];

		TCHAR* p;
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szOpenFileNames;
		ofn.nMaxFile = sizeof(szOpenFileNames);
		ofn.lpstrFile[0] = NULL;
		ofn.lpstrFilter = _T("image file(*.jpg)|*.jpg|All Files(*.*)|*.*||");

		if (GetOpenFileName(&ofn))
		{
			// 获取文件夹路径szPath  
			lstrcpyn(szPath, szOpenFileNames, ofn.nFileOffset);
			lstrcat(szPath, _T("\\"));   // 末尾加上反斜杠  

			p = szOpenFileNames + ofn.nFileOffset; //把指针移到第一个文件  

			while (*p)
			{
				ZeroMemory(szFileName, sizeof(szFileName));
				lstrcat(szFileName, szPath);  //给文件名加上路径    
				lstrcat(szFileName, p);       //加上文件名       

				// 插入到List Box中  
				MODELPICPATH st;
				st.strName = p;
				st.strPath = szFileName;
				m_vecPath.push_back(st);

				p += lstrlen(p) + 1;           //移至下一个文件  
			}
		}
	#endif
	}
	else
	{
		//从服务器下载
	}

	OnOK();
}


void CCreateModelDlg::OnBnClickedRadioFromscanner()
{
	m_nSearchType = 1;
}


void CCreateModelDlg::OnBnClickedRadioFromlocalfile()
{
	m_nSearchType = 2;
}


void CCreateModelDlg::OnBnClickedRadioFromserver()
{
	m_nSearchType = 3;
}

void CCreateModelDlg::SetFontSize()
{
	CFont fontStatus;
	fontStatus.CreateFont(25, 0, 0, 0,
						  FW_BOLD, FALSE, FALSE, 0,
						  DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,
						  DEFAULT_PITCH | FF_SWISS,
						  _T("宋体"));
	GetDlgItem(IDC_STATIC_PIC_SRC)->SetFont(&fontStatus);
}

HBRUSH CCreateModelDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CTipBaseDlg::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();
	if (CurID == IDC_STATIC_PIC_SRC)
	{
		pDC->SetTextColor(RGB(100, 100, 100));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	return hbr;
}


void CCreateModelDlg::OnStnClickedStaticFromserver()
{
	m_nSearchType = 3;
	switch (m_nSearchType)
	{
		case 1:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(0);
			break;
		case 2:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(0);
			break;
		case 3:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(1);
			break;
	}
}


void CCreateModelDlg::OnStnClickedStaticFromscanner()
{
	m_nSearchType = 1;
	switch (m_nSearchType)
	{
		case 1:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(0);
			break;
		case 2:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(0);
			break;
		case 3:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(1);
			break;
	}
}


void CCreateModelDlg::OnStnClickedStaticFromlocalfile()
{
	m_nSearchType = 2;
	switch (m_nSearchType)
	{
		case 1:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(0);
			break;
		case 2:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(0);
			break;
		case 3:
			((CButton*)GetDlgItem(IDC_RADIO_FromScanner))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromLocalFile))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_FromServer))->SetCheck(1);
			break;
	}
}

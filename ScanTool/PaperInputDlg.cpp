// PaperInputDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanToolDlg.h"
#include "PaperInputDlg.h"
#include "afxdialogex.h"
#include <string.h>
#include <algorithm>

using namespace cv;
using namespace std;
// CPaperInputDlg �Ի���

IMPLEMENT_DYNAMIC(CPaperInputDlg, CDialog)

CPaperInputDlg::CPaperInputDlg(pMODEL pModel, CWnd* pParent /*=NULL*/)
	: CDialog(CPaperInputDlg::IDD, pParent)
	, m_strPapersPath(_T("")), m_nModelPicNums(1), m_nCurrTabSel(0), m_pCurrentPicShow(NULL), m_pModel(pModel), m_pOldModel(pModel)
	, m_strModelName(_T("")), m_strPapersName(_T("")), m_strPapersDesc(_T("")), m_nCurrItemPapers(0), m_nCurrItemPaper(-1)
	, m_colorStatus(RGB(0, 0, 255)), m_nStatusSize(35), m_pCurrentShowPaper(NULL)
{

}

CPaperInputDlg::~CPaperInputDlg()
{
	std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
	for (; itPic != m_vecPicShow.end();)
	{
		CPicShow* pModelPicShow = *itPic;
		if (pModelPicShow)
		{
			delete pModelPicShow;
			pModelPicShow = NULL;
		}
		itPic = m_vecPicShow.erase(itPic);
	}
	if (m_pModel != m_pOldModel)
		SAFE_RELEASE(m_pModel);
}

void CPaperInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Papers, m_lPapersCtrl);
	DDX_Control(pDX, IDC_LIST_Paper, m_lPaperCtrl);
	DDX_Control(pDX, IDC_LIST_IssuePaper, m_lIssuePaperCtrl);
	DDX_Control(pDX, IDC_COMBO_ModelList, m_comboModel);
	DDX_Control(pDX, IDC_TAB_PicShow, m_tabPicShow);
	DDX_Control(pDX, IDC_BTN_Broswer, m_btnBroswer);
	DDX_Text(pDX, IDC_EDIT_PapersPath, m_strPapersPath);
	DDX_Text(pDX, IDC_EDIT_ModelInfo, m_strModelName);
	DDX_Text(pDX, IDC_EDIT_PapersName, m_strPapersName);
	DDX_Text(pDX, IDC_EDIT_PapersDesc, m_strPapersDesc);
}


BEGIN_MESSAGE_MAP(CPaperInputDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_Broswer, &CPaperInputDlg::OnBnClickedBtnBroswer)
	ON_BN_CLICKED(IDC_BTN_Start, &CPaperInputDlg::OnBnClickedBtnStart)
	ON_CBN_SELCHANGE(IDC_COMBO_ModelList, &CPaperInputDlg::OnCbnSelchangeComboModellist)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Papers, &CPaperInputDlg::OnNMDblclkListPapers)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Paper, &CPaperInputDlg::OnNMDblclkListPaper)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PicShow, &CPaperInputDlg::OnTcnSelchangeTabPicshow)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_IssuePaper, &CPaperInputDlg::OnNMDblclkListIssuepaper)
	ON_MESSAGE(WM_CV_LBTNDOWN, &CPaperInputDlg::RoiLBtnDown)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CPaperInputDlg::OnBnClickedBtnSave)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CPaperInputDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitUI();
	SeachModel();
	if (NULL != m_pModel)
	{
		for (int i = 0; i < m_comboModel.GetCount(); i++)
		{
			CString strItemName;
			m_comboModel.GetLBText(i, strItemName);
			if (strItemName == m_pModel->strModelName)
			{
				m_comboModel.SetCurSel(i);
				m_ncomboCurrentSel = i;
				break;
			}
		}
	}
	else
		m_comboModel.SetCurSel(0);
	
	return TRUE;
}

void CPaperInputDlg::InitUI()
{
	if (m_pModel)
		m_nModelPicNums = m_pModel->nPicNum;

	USES_CONVERSION;
	CRect rtTab;
	m_tabPicShow.GetClientRect(&rtTab);
	for (int i = 0; i < m_nModelPicNums; i++)
	{
		char szTabHeadName[20] = { 0 };
		sprintf_s(szTabHeadName, "��%dҳ", i + 1);

		m_tabPicShow.InsertItem(i, A2T(szTabHeadName));

		CPicShow* pPicShow = new CPicShow(this);
		pPicShow->Create(CPicShow::IDD, &m_tabPicShow);
		pPicShow->ShowWindow(SW_HIDE);
		pPicShow->MoveWindow(&rtTab);
		m_vecPicShow.push_back(pPicShow);
	}
	m_tabPicShow.SetCurSel(0);
	m_vecPicShow[0]->ShowWindow(SW_SHOW);
	m_pCurrentPicShow = m_vecPicShow[0];

	m_lPapersCtrl.SetExtendedStyle(m_lPapersCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
//	m_lPapersCtrl.InsertColumn(0, _T("���"), LVCFMT_CENTER, 38);
	m_lPapersCtrl.InsertColumn(0, _T("�Ծ����"), LVCFMT_CENTER, 80);
	m_lPapersCtrl.InsertColumn(1, _T("����"), LVCFMT_CENTER, 36);
	m_lPapersCtrl.InsertColumn(2, _T("״̬"), LVCFMT_CENTER, 40);

	m_lPaperCtrl.SetExtendedStyle(m_lPaperCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lPaperCtrl.InsertColumn(0, _T("���"), LVCFMT_CENTER, 36);
	m_lPaperCtrl.InsertColumn(1, _T("�Ծ���"), LVCFMT_CENTER, 80);

	m_lIssuePaperCtrl.SetExtendedStyle(m_lIssuePaperCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lIssuePaperCtrl.InsertColumn(0, _T("���"), LVCFMT_CENTER, 36);
	m_lIssuePaperCtrl.InsertColumn(1, _T("�Ծ���"), LVCFMT_CENTER, 80);

	SetFontSize(m_nStatusSize);

	int sx = MAX_DLG_WIDTH;
	int sy = MAX_DLG_HEIGHT;
	MoveWindow(0, 0, sx, sy);
	CenterWindow();

	InitCtrlPosition();	
}

// CPaperInputDlg ��Ϣ�������

void CPaperInputDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CPaperInputDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nGap = 5;	//�ؼ��ļ��
	int nTopGap = 2;	//�����ϱ߱�Ե�ļ��
	int nBottomGap = 2;	//�����±߱�Ե�ļ��
	int nLeftGap = 2;	//������߱�Ե�ļ��
	int nRightGap = 2;	//�����ұ߱�Ե�ļ��
	int nStaticHeight = 20;						//��̬��ʾ�ؼ��߶�
	int nLeftCtrlWidth = rcClient.Width() / 3;		//��߿ؼ��������
	if (nLeftCtrlWidth > 300)
		nLeftCtrlWidth = 300;
	int nListWidth = (nLeftCtrlWidth - nGap) / 2;	//List�б�ؼ��Ŀ��
	int nCurrentTop = nTopGap;
	if (GetDlgItem(IDC_STATIC_PathTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PathTips)->MoveWindow(nLeftGap, nCurrentTop, nLeftCtrlWidth, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (GetDlgItem(IDC_EDIT_PapersPath)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_PapersPath)->MoveWindow(nLeftGap, nCurrentTop, nLeftCtrlWidth - nGap - 20, nStaticHeight);
//		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (m_btnBroswer.GetSafeHwnd())
	{
		m_btnBroswer.MoveWindow(nLeftGap + nLeftCtrlWidth - 20, nCurrentTop, 20, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	int nModelStaticWidth = 65;		//ģ���б���ʾ�ؼ��Ŀ��
	if (GetDlgItem(IDC_STATIC_ModelTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ModelTips)->MoveWindow(nLeftGap, nCurrentTop, nModelStaticWidth, nStaticHeight);
	}
	if (m_comboModel.GetSafeHwnd())
	{
		m_comboModel.MoveWindow(nLeftGap + nModelStaticWidth + nGap, nCurrentTop, nLeftCtrlWidth - nModelStaticWidth - nGap, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (GetDlgItem(IDC_STATIC_PapersTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PapersTips)->MoveWindow(nLeftGap, nCurrentTop, nLeftCtrlWidth, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	int nPapersListWidth = static_cast<int>(nLeftCtrlWidth * 0.55);
	int nIssueListWdith = nLeftCtrlWidth - nPapersListWidth - nGap;
	if (m_lPapersCtrl.GetSafeHwnd())
	{
		m_lPapersCtrl.MoveWindow(nLeftGap, nCurrentTop, nPapersListWidth, rcClient.Height() - nCurrentTop);
	}
	if (GetDlgItem(IDC_STATIC_PaperTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PaperTips)->MoveWindow(nLeftGap + nPapersListWidth + nGap, nCurrentTop - nStaticHeight - nGap, nIssueListWdith, nStaticHeight);		//���Ծ���б���ʾ��ƽ
//		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	int nPaperListHeight = (rcClient.Height() - nCurrentTop - nGap - nStaticHeight - nGap) / 2;		//�Ծ��б�ؼ��ĸ߶�
	if (m_lPaperCtrl.GetSafeHwnd())
	{
		m_lPaperCtrl.MoveWindow(nLeftGap + nPapersListWidth + nGap, nCurrentTop, nIssueListWdith, nPaperListHeight);
		nCurrentTop = nCurrentTop + nPaperListHeight + nGap;
	}
	if (GetDlgItem(IDC_STATIC_IssueTips)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_IssueTips)->MoveWindow(nLeftGap + nPapersListWidth + nGap, nCurrentTop, nIssueListWdith, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (m_lIssuePaperCtrl.GetSafeHwnd())
	{
		m_lIssuePaperCtrl.MoveWindow(nLeftGap + nPapersListWidth + nGap, nCurrentTop, nIssueListWdith, nPaperListHeight);
	}

	//Tab�ؼ���λ��
	int nTabLeftPos = nLeftGap + nLeftCtrlWidth + nGap;
	int nTabCtrlHeight = rcClient.Height() * 2 / 3;
	int nTabCtrlWidth = rcClient.Width() - nLeftGap - nLeftCtrlWidth - nGap - nRightGap;
	if (m_tabPicShow.GetSafeHwnd())
	{
		m_tabPicShow.MoveWindow(nTabLeftPos, nTopGap, nTabCtrlWidth, nTabCtrlHeight);
		nCurrentTop = nTopGap + nTabCtrlHeight + nGap;

		CRect rtTab;
		m_tabPicShow.GetClientRect(&rtTab);
		int nTabHead_H = 25;		//tab�ؼ�ͷ�ĸ߶�
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		//		if (m_pModelPicShow) m_pModelPicShow->MoveWindow(&rtPic);
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
	int nInfoStaticWidth = 100;			//�Ծ��������Ϣstatic�ؼ��Ŀ��
	if (GetDlgItem(IDC_STATIC_PapersModel)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PapersModel)->MoveWindow(nTabLeftPos, nCurrentTop, nInfoStaticWidth, nStaticHeight);
//		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (GetDlgItem(IDC_EDIT_ModelInfo)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_ModelInfo)->MoveWindow(nTabLeftPos + nInfoStaticWidth + nGap, nCurrentTop, nInfoStaticWidth * 1.5, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (GetDlgItem(IDC_STATIC_PapersName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PapersName)->MoveWindow(nTabLeftPos, nCurrentTop, nInfoStaticWidth, nStaticHeight);
//		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (GetDlgItem(IDC_EDIT_PapersName)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_PapersName)->MoveWindow(nTabLeftPos + nInfoStaticWidth + nGap, nCurrentTop, nInfoStaticWidth * 1.5, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}
	if (GetDlgItem(IDC_STATIC_PapersDesc)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_PapersDesc)->MoveWindow(nTabLeftPos, nCurrentTop, nInfoStaticWidth, nStaticHeight);
		nCurrentTop = nCurrentTop + nStaticHeight + nGap;
	}

	int nBtnWidth = 70;
	int nBtnHeight = 40;
	int	nTipsHeight = 30;

	int nPaperDescHeight = rcClient.Height() - nCurrentTop - nGap - nBtnHeight - nGap - nTipsHeight - nBottomGap;		//�Ծ�������Ϣ�ؼ��ĸ߶�
	if (GetDlgItem(IDC_EDIT_PapersDesc)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_PapersDesc)->MoveWindow(nTabLeftPos, nCurrentTop, nTabCtrlWidth, nPaperDescHeight);
		nCurrentTop = nCurrentTop + nPaperDescHeight + nGap;
	}
	
	if (GetDlgItem(IDC_BTN_Start)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Start)->MoveWindow(nTabLeftPos, nCurrentTop, nBtnWidth, nBtnHeight);
	}
	if (GetDlgItem(IDC_BTN_SAVE))
	{
		GetDlgItem(IDC_BTN_SAVE)->MoveWindow(nTabLeftPos + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeight);
		nCurrentTop = nCurrentTop + nBtnHeight + nGap;
	}
	if (GetDlgItem(IDC_STATIC_TIPS)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TIPS)->MoveWindow(nTabLeftPos, nCurrentTop, nTabCtrlWidth, nTipsHeight);
	}
}

void CPaperInputDlg::InitTab()
{
	if (m_pModel)
	{
		std::vector<CPicShow*>::iterator itPic = m_vecPicShow.begin();
		for (; itPic != m_vecPicShow.end();)
		{
			CPicShow* pModelPicShow = *itPic;
			if (pModelPicShow)
			{
				delete pModelPicShow;
				pModelPicShow = NULL;
			}
			itPic = m_vecPicShow.erase(itPic);
		}
		m_tabPicShow.DeleteAllItems();
	}

	USES_CONVERSION;
	CRect rtTab;
	m_tabPicShow.GetClientRect(&rtTab);
	for (int i = 0; i < m_nModelPicNums; i++)
	{
		char szTabHeadName[20] = { 0 };
		sprintf_s(szTabHeadName, "��%dҳ", i + 1);

		m_tabPicShow.InsertItem(i, A2T(szTabHeadName));

		CPicShow* pPicShow = new CPicShow(this);
		pPicShow->Create(CPicShow::IDD, &m_tabPicShow);
		pPicShow->ShowWindow(SW_HIDE);
		pPicShow->MoveWindow(&rtTab);
		m_vecPicShow.push_back(pPicShow);
	}
	m_tabPicShow.SetCurSel(0);
	if (m_vecPicShow.size())
	{
		m_vecPicShow[0]->ShowWindow(SW_SHOW);
		m_pCurrentPicShow = m_vecPicShow[0];
	}	

	if (m_tabPicShow.GetSafeHwnd())
	{
		CRect rtTab;
		m_tabPicShow.GetClientRect(&rtTab);
		int nTabHead_H = 25;		//tab�ؼ�ͷ�ĸ߶�
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		for (int i = 0; i < m_vecPicShow.size(); i++)
			m_vecPicShow[i]->MoveWindow(&rtPic);
	}
}

void CPaperInputDlg::OnBnClickedBtnBroswer()
{
	USES_CONVERSION;

	LPITEMIDLIST pidlRoot = NULL;
	SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlRoot);
	BROWSEINFO	bi;
	TCHAR		szPath[MAX_PATH];
	TCHAR		szFolderPath[MAX_PATH];
	ZeroMemory(&bi, sizeof(LPBROWSEINFO));
	bi.pidlRoot = pidlRoot;
	bi.lpszTitle = _T("ѡ���Ծ��ļ���·��");
	bi.lParam = NULL;
	bi.pszDisplayName = szPath;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	LPITEMIDLIST lpIDList = SHBrowseForFolder(&bi);
	if (!lpIDList)
	{
		return;
	}
	SHGetPathFromIDList(lpIDList, szFolderPath);

	m_strPapersPath = szFolderPath;
	UpdateData(FALSE);
}

bool SortbyNumASC(const std::string& x, const std::string& y)
{
//	USES_CONVERSION;
	char szX[MAX_PATH] = { 0 };
	char szY[MAX_PATH] = { 0 };
	sprintf_s(szX, "%s", x.c_str()/*T2A(x)*/);
	sprintf_s(szY, "%s", y.c_str()/*T2A(y)*/);
	int nLenX = x.length();	// x.GetLength();
	int nLenY = y.length();	// y.GetLength();

//	TRACE(_T("compare: %s, %s\n"), x, y);

	int nFlag = 0;
	while (nLenX && nLenY)
	{
		char szXPart[MAX_PATH] = { 0 };
		char szYPart[MAX_PATH] = { 0 };
		sscanf(szX, "%[A-Za-z]", szXPart);
		sscanf(szY, "%[A-Za-z]", szYPart);
		if (strlen(szXPart) && strlen(szYPart))
		{
			int nResult = stricmp(szXPart, szYPart);
			if (nResult == 0)
			{
				int nX = strlen(szXPart);
				int nY = strlen(szYPart);
				int nXAll = strlen(szX);
				int nYAll = strlen(szY);
				memmove(szX, szX + strlen(szXPart), nXAll - nX);
				memmove(szY, szY + strlen(szYPart), nYAll - nY);
				szX[nXAll - nX] = '\0';
				szY[nYAll - nY] = '\0';
				nLenX = strlen(szX);
				nLenY = strlen(szY);
			}
			else
			{
				return nResult < 0 ? true : false;
			}
		}
		else if (strlen(szXPart))
			return false;
		else if (strlen(szYPart))
			return true;
		else
		{
			sscanf(szX, "%[0-9]", szXPart);
			sscanf(szY, "%[0-9]", szYPart);
			if (strlen(szXPart) && strlen(szYPart))
			{
				int x = atoi(szXPart);
				int y = atoi(szYPart);
				if (x == y)
				{
					if (strlen(szXPart) == strlen(szYPart))
					{
						int nX = strlen(szXPart);
						int nY = strlen(szYPart);
						int nXAll = strlen(szX);
						int nYAll = strlen(szY);
						memmove(szX, szX + strlen(szXPart), nXAll - nX);
						memmove(szY, szY + strlen(szYPart), nYAll - nY);
						szX[nXAll - nX] = '\0';
						szY[nYAll - nY] = '\0';
						nLenX = strlen(szX);
						nLenY = strlen(szY);
					}
					else
					{
						return strlen(szXPart) > strlen(szYPart);		//��С��ͬ������Խ��Խ��ǰ
					}
				}
				else
					return x < y;
			}
			else if (strlen(szXPart))
				return false;
			else if (strlen(szYPart))
				return true;
			else
			{
				sscanf(szX, "%[^0-9A-Za-z]", szXPart);
				sscanf(szY, "%[^0-9A-Za-z]", szYPart);
				int nResult = stricmp(szXPart, szYPart);
				if (nResult == 0)
				{
					int nX = strlen(szXPart);
					int nY = strlen(szYPart);
					int nXAll = strlen(szX);
					int nYAll = strlen(szY);
					memmove(szX, szX + strlen(szXPart), nXAll - nX);
					memmove(szY, szY + strlen(szYPart), nYAll - nY);
					szX[nXAll - nX] = '\0';
					szY[nYAll - nY] = '\0';
					nLenX = strlen(szX);
					nLenY = strlen(szY);
				}
				else
				{
					char* p1 = szXPart;
					char* p2 = szYPart;
					while (*p1 != '\0' && *p2 != '\0')
					{
						if (*p1 == '-'&& *p2 != '-')
							return false;
						else if (*p1 != '-' && *p2 == '-')
							return true;
						else if (*p1 == '=' && *p2 != '=')
							return false;
						else if (*p1 != '=' && *p2 == '=')
							return true;
						else if (*p1 == '+' && *p2 != '+')
							return false;
						else if (*p1 != '+' && *p2 == '+')
							return true;
						else if (*p1 > *p2)
							return false;
						else if (*p1 < *p2)
							return true;
						else
						{
							p1++;
							p2++;
						}
					}
					if (*p1 == '\0' && *p2 != '\0')
					{
						if (*p2 == ' ')
							return false;
						else
							return true;
					}
					else if (*p1 != '\0' && *p2 == '\0')
					{
						if (*p1 == ' ')
							return true;
						else
							return false;
					}
					//return nResult < 0?true:false;
				}
			}
		}
	}

	return x.length() < y.length();
}
void CPaperInputDlg::OnBnClickedBtnStart()
{
	UpdateData(TRUE);
	if (m_strPapersPath == _T(""))
	{
		AfxMessageBox(_T("�������Ծ��·����"));
		return;
	}
	if (!m_pModel)
	{
		AfxMessageBox(_T("δ���ü���ģ����Ϣ��"));
		return;
	}

	USES_CONVERSION;
	std::string strPaperPath = T2A(m_strPapersPath);
	Poco::DirectoryIterator it(strPaperPath);
	Poco::DirectoryIterator end;
	while (it != end)
	{
		Poco::Path p(it->path());
		if (it->isDirectory())
		{
			std::wstring strWInput;
			Poco::UnicodeConverter::toUTF16(p.getFileName(), strWInput);
			char szDirName[90] = { 0 };
			sprintf_s(szDirName, "%s", T2A(strWInput.c_str()));
			TRACE("%s\n", szDirName);
			
			std::vector<std::string> lFileName;
			std::string strSubDirPath = p.toString();
			Poco::DirectoryIterator itSub(strSubDirPath);
			Poco::DirectoryIterator endSub;
			while (itSub != endSub)
			{
				Poco::Path pSubFile(itSub->path());
				if (itSub->isFile())
				{
					std::string strOldFileName = pSubFile.getFileName();
					
#if 1
					if (strOldFileName.find("papersInfo.dat") == std::string::npos)
					{
						lFileName.push_back(strOldFileName);
					}
#else
					char szNewName[100] = { 0 };
					sprintf_s(szNewName, "S%d_", i + 1);
					std::string strNewName = szNewName;
					std::string strNewFilePath = strSubPaperPath + "\\" + strNewName + strOldFileName;
					itSub->copyTo(strNewFilePath);

					pRECOGTASK pTask = new RECOGTASK;
					pTask->nPic = 0;
					pTask->strPath = strNewFilePath;
					pTask->pModel = m_pModel;
					g_lRecogTask.push_back(pTask);
#endif
				}
				itSub++;
			}

			if (lFileName.size() % m_pModel->nPicNum != 0)
			{
				char szErrorInfo[MAX_PATH] = { 0 };
				sprintf_s(szErrorInfo, "ɨ�赽�ļ���%s�Ծ�����%d,ģ��Ҫ��ÿ����%d���Ծ�, �����Ƿ��п����Ծ�ȱʧ", szDirName, lFileName.size(), m_pModel->nPicNum);
				AfxMessageBox(A2T(szErrorInfo));
				it++;
				continue;
			}

			//�����Ծ���б�
			char szPapersName[100] = { 0 };
			sprintf_s(szPapersName, "%s", szDirName);
			int nPapersCount = m_lPapersCtrl.GetItemCount();
			m_lPapersCtrl.InsertItem(nPapersCount, NULL);
			m_lPapersCtrl.SetItemText(nPapersCount, 0, (LPCTSTR)A2T(szPapersName));

			//�ļ����Ʊ�����ʱĿ¼
			std::string strSubPaperPath = g_strPaperSavePath + p.getFileName();
			char szSubPaperPath[MAX_PATH] = { 0 };
			sprintf_s(szSubPaperPath, "%s%s", g_strPaperSavePath.c_str(), szDirName);
			Poco::File tmpPath(strSubPaperPath);
			tmpPath.createDirectories();

			pPAPERSINFO pPapers = new PAPERSINFO;
			g_fmPapers.lock();
			g_lPapers.push_back(pPapers);
			g_fmPapers.unlock();

			pPapers->nPaperCount = lFileName.size() / m_pModel->nPicNum;
			pPapers->strPapersName = szDirName;

			int i = 0;
			pST_PaperInfo pPaper = NULL;
			std::sort(lFileName.begin(), lFileName.end(), SortbyNumASC);
			std::vector<std::string>::iterator itName = lFileName.begin();
			for (; itName != lFileName.end(); itName++)
			{
				TRACE("%s\n", (*itName).c_str());	//(*itName).c_str()

				char szNewName[100] = { 0 };
				sprintf_s(szNewName, "S%d_%s", i / m_pModel->nPicNum + 1, (*itName).c_str());

				std::string strNewName = szNewName;
				std::string strNewFilePath = strSubPaperPath + "\\" + strNewName;

				std::string strFileOldPath = strSubDirPath + "\\" + *itName;
				Poco::File oldFile(strFileOldPath);
				oldFile.copyTo(strNewFilePath);

				if (i % m_pModel->nPicNum == 0)
				{
					pPaper = new ST_PaperInfo;
					pPapers->lPaper.push_back(pPaper);
					char szStudentInfo[20] = { 0 };
					sprintf_s(szStudentInfo, "S%d", i / m_pModel->nPicNum + 1);
					pPaper->strStudentInfo = szStudentInfo;
					pPaper->pModel = m_pModel;
					pPaper->pPapers = pPapers;
					pPaper->pSrcDlg = this;
				}
				pST_PicInfo pPic = new ST_PicInfo;
				pPaper->lPic.push_back(pPic);

				char szNewFullPath[MAX_PATH] = { 0 };
				sprintf_s(szNewFullPath, "%s\\%s", szSubPaperPath, strNewName.c_str());
				pPic->strPicName = strNewName;
				pPic->strPicPath = CMyCodeConvert::Utf8ToGb2312(strNewFilePath);	// strNewFilePath;
				pPic->pPaper = pPaper;
				i++;
			}

			//�����Ծ������
			char szPapersCount[20] = { 0 };
			sprintf_s(szPapersCount, "%d", pPapers->lPaper.size());
			m_lPapersCtrl.SetItemText(nPapersCount, 1, (LPCTSTR)A2T(szPapersCount));
			m_lPapersCtrl.SetItemData(nPapersCount, (DWORD_PTR)pPapers);

			//��ӵ�ʶ�������б�
			PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
			for (; itPaper != pPapers->lPaper.end(); itPaper++)
			{
				pRECOGTASK pTask = new RECOGTASK;
				pTask->pPaper = *itPaper;
				g_lRecogTask.push_back(pTask);
			}			
		}
		it++;
	}
}

void CPaperInputDlg::SeachModel()
{
	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));
	g_strModelSavePath = CMyCodeConvert::Gb2312ToUtf8(strModelPath);

	std::string strLog;
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
				m_comboModel.AddString(A2T(strModelName.c_str()));
			}
			it++;
		}
		//		m_comboModel.SetCurSel(0);
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

void CPaperInputDlg::OnCbnSelchangeComboModellist()
{
	if (m_ncomboCurrentSel == m_comboModel.GetCurSel())
		return;

	CString strModelName;
	m_comboModel.GetLBText(m_comboModel.GetCurSel(), strModelName);
	CString strModelPath = g_strCurrentPath + _T("Model\\") + strModelName;
	CString strModelFullPath = strModelPath + _T(".mod");
	UnZipFile(strModelFullPath);		//UnZipModel(strModelFullPath);
	if (m_pModel && m_pModel != m_pOldModel)
	{
		delete m_pModel;
		m_pModel = NULL;
	}
	m_pModel = LoadModelFile(strModelPath);
	m_ncomboCurrentSel = m_comboModel.GetCurSel();

	m_nModelPicNums = m_pModel->nPicNum;
	InitTab();
}

void CPaperInputDlg::OnNMDblclkListPapers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	m_lPaperCtrl.DeleteAllItems();
	m_lIssuePaperCtrl.DeleteAllItems();

	USES_CONVERSION;
	pPAPERSINFO pPapers = (pPAPERSINFO)m_lPapersCtrl.GetItemData(pNMItemActivate->iItem);

	m_nCurrItemPapers = pNMItemActivate->iItem;
	m_strPapersName = A2T(pPapers->strPapersName.c_str());
	m_strPapersDesc = A2T(pPapers->strPapersDesc.c_str());

	PAPER_LIST::iterator itPaper = pPapers->lPaper.begin();
	for (int i = 0; itPaper != pPapers->lPaper.end(); i++, itPaper++)
	{
		if (i == 0)
		{
			m_strModelName = (*itPaper)->pModel->strModelName;
		}
		int nCount = m_lPaperCtrl.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lPaperCtrl.InsertItem(nCount, NULL);
		m_lPaperCtrl.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lPaperCtrl.SetItemText(nCount, 1, (LPCTSTR)A2T((*itPaper)->strStudentInfo.c_str()));
		m_lPaperCtrl.SetItemData(nCount, (DWORD_PTR)(*itPaper));
	}
	//issue paper list
	PAPER_LIST::iterator itIssuePaper = pPapers->lIssue.begin();
	for (int i = 0; itIssuePaper != pPapers->lIssue.end(); i++, itIssuePaper++)
	{
		int nCount = m_lIssuePaperCtrl.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lIssuePaperCtrl.InsertItem(nCount, NULL);
		m_lIssuePaperCtrl.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lIssuePaperCtrl.SetItemText(nCount, 1, (LPCTSTR)A2T((*itIssuePaper)->strStudentInfo.c_str()));
		m_lIssuePaperCtrl.SetItemData(nCount, (DWORD_PTR)(*itIssuePaper));
	}
	UpdateData(FALSE);
}


void CPaperInputDlg::OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lPaperCtrl.GetItemData(pNMItemActivate->iItem);
	m_nCurrItemPaper = pNMItemActivate->iItem;
	m_pCurrentShowPaper = pPaper;

	PaintRecognisedRect(pPaper);

	m_nCurrTabSel = 0;

	m_tabPicShow.SetCurSel(0);
	m_pCurrentPicShow = m_vecPicShow[0];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != 0)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}

void CPaperInputDlg::OnTcnSelchangeTabPicshow(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	int nIndex = m_tabPicShow.GetCurSel();
	m_nCurrTabSel = nIndex;
	m_pCurrentPicShow = m_vecPicShow[nIndex];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != nIndex)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}

void CPaperInputDlg::PaintRecognisedRect(pST_PaperInfo pPaper)
{
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		Mat matSrc = imread((*itPic)->strPicPath);
#ifdef PIC_RECTIFY_TEST
		Mat dst;
		Mat rotMat;
		PicRectify(matSrc, dst, rotMat);
		Mat matImg;
		if (dst.channels() == 1)
			cvtColor(dst, matImg, CV_GRAY2BGR);
		else
			matImg = dst;
#ifdef WarpAffine_TEST
		PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix);
#endif
#else
		Mat matImg = matSrc;
#endif
		Mat tmp = matImg.clone();
		Mat tmp2 = matImg.clone();
		RECTLIST::iterator itNormalRect = (*itPic)->lNormalRect.begin();
		for (int j = 0; itNormalRect != (*itPic)->lNormalRect.end(); itNormalRect++, j++)
		{
			cv::Rect rt = (*itNormalRect).rt;
			GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
//			char szCP[20] = { 0 };
// 			sprintf_s(szCP, "CP%d", j);
// 			putText(tmp, szCP, Point((*itNormalRect).rt.x, (*itNormalRect).rt.y + (*itNormalRect).rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}

		//��ӡOMR��SNλ��
#ifdef PaintOmrSnRect
		SNLIST::iterator itSN = pPaper->pModel->vecPaperModel[i]->lSNInfo.begin();
		for (; itSN != pPaper->pModel->vecPaperModel[i]->lSNInfo.end(); itSN++)
		{
			pSN_ITEM pSnItem = *itSN;
			RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
			for (; itSnItem != pSnItem->lSN.end(); itSnItem++)
			{
				cv::Rect rt = (*itSnItem).rt;
				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}

		OMRLIST::iterator itOmr = pPaper->pModel->vecPaperModel[i]->lOMR2.begin();
		for (; itOmr != pPaper->pModel->vecPaperModel[i]->lOMR2.end(); itOmr++)
		{
			pOMR_QUESTION pOmrQuestion = &(*itOmr);
			RECTLIST::iterator itOmrItem = pOmrQuestion->lSelAnswer.begin();
			for (; itOmrItem != pOmrQuestion->lSelAnswer.end(); itOmrItem++)
			{
				cv::Rect rt = (*itOmrItem).rt;
				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}
#endif
		addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
		m_vecPicShow[i]->ShowPic(tmp);
	}
}

void CPaperInputDlg::PaintIssueRect(pST_PaperInfo pPaper)
{
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		
		if ((*itPic)->bFindIssue)
		{
			Point pt(0, 0);
			Mat matSrc = imread((*itPic)->strPicPath);
#ifdef PIC_RECTIFY_TEST
			Mat dst;
			Mat rotMat;
			PicRectify(matSrc, dst, rotMat);
			Mat matImg;
			if (dst.channels() == 1)
				cvtColor(dst, matImg, CV_GRAY2BGR);
			else
				matImg = dst;
#ifdef WarpAffine_TEST
			PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix);
#endif
#else
			Mat matImg = matSrc;
#endif
			Mat tmp = matImg.clone();
			Mat tmp2 = matImg.clone();

			//-----------------------
			RECTLIST::iterator itSelRoi = pPaper->pModel->vecPaperModel[i]->lSelFixRoi.begin();													//��ʾʶ�𶨵��ѡ����
			for (int j = 0; itSelRoi != pPaper->pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++, j++)
			{
				cv::Rect rt = (*itSelRoi).rt;
//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				char szCP[20] = { 0 };
				rectangle(tmp, rt, CV_RGB(0, 0, 255), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}

			RECTLIST::iterator itPicFix = (*itPic)->lFix.begin();													//��ʾʶ������Ķ���
			for (int j = 0; itPicFix != (*itPic)->lFix.end(); itPicFix++, j++)
			{
				cv::Rect rt = (*itPicFix).rt;
//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				char szCP[20] = { 0 };
				sprintf_s(szCP, "F%d", j);
				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, rt, CV_RGB(0, 255, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			RECTLIST::iterator itFixRect = pPaper->pModel->vecPaperModel[i]->lFix.begin();							//��ʾģ���ϵĶ����Ӧ�����Ծ��ϵ��¶���
			for (int j = 0; itFixRect != pPaper->pModel->vecPaperModel[i]->lFix.end(); itFixRect++, j++)
			{
				cv::Rect rt = (*itFixRect).rt;
//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				TRACE("ģ�嶨�������: (%d, %d, %d, %d)\n", (*itFixRect).rt.x, (*itFixRect).rt.y, (*itFixRect).rt.width, (*itFixRect).rt.height);

				char szCP[20] = { 0 };
				sprintf_s(szCP, "M_F%d", j);
				putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}

			RECTLIST::iterator itNormal = (*itPic)->lNormalRect.begin();													//��ʾ����ʶ�������ĵ�
			for (int j = 0; itNormal != (*itPic)->lNormalRect.end(); itNormal++, j++)
			{
				cv::Rect rt = (*itNormal).rt;
				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				char szCP[20] = { 0 };
				rectangle(tmp, rt, CV_RGB(0, 255, 255), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			//----------------------------

			RECTLIST::iterator itIssueRect = (*itPic)->lIssueRect.begin();
			for (int j = 0; itIssueRect != (*itPic)->lIssueRect.end(); itIssueRect++, j++)
			{
				cv::Rect rt = (*itIssueRect).rt;
				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
				if (j == 0)
				{
					pt = rt.tl();
					pt.x = pt.x - 100;
					pt.y = pt.y - 100;
				}

				char szCP[20] = { 0 };
				sprintf_s(szCP, "Err%d", j);
				putText(tmp, szCP, Point((*itIssueRect).rt.x, (*itIssueRect).rt.y + (*itIssueRect).rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, (*itIssueRect).rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, (*itIssueRect).rt, CV_RGB(255, 200, 100), -1);
			}
			addWeighted(tmp, 0.5, tmp2, 0.5, 0, tmp);
			m_vecPicShow[i]->ShowPic(tmp, pt);
		}
	}
}

void CPaperInputDlg::OnNMDblclkListIssuepaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lIssuePaperCtrl.GetItemData(pNMItemActivate->iItem);
	m_pCurrentShowPaper = pPaper;

	int nIssuePaper = 0;
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		if ((*itPic)->bFindIssue)
		{
			nIssuePaper = i;
			m_nCurrTabSel = i;
			break;
		}
	}
	PaintIssueRect(pPaper);

	m_tabPicShow.SetCurSel(nIssuePaper);
	m_pCurrentPicShow = m_vecPicShow[nIssuePaper];
	m_pCurrentPicShow->ShowWindow(SW_SHOW);
	for (int i = 0; i < m_vecPicShow.size(); i++)
	{
		if (i != nIssuePaper)
			m_vecPicShow[i]->ShowWindow(SW_HIDE);
	}
}

void CPaperInputDlg::OnBnClickedBtnSave()
{
	UpdateData(TRUE);
	if (m_nCurrItemPapers < 0)
	{
		AfxMessageBox(_T("����ѡ���Ծ��"));
		return;
	}

	USES_CONVERSION;
	pPAPERSINFO pPapers = (pPAPERSINFO)m_lPapersCtrl.GetItemData(m_nCurrItemPapers);
	if (!pPapers)
	{
		AfxMessageBox(_T("û���Ծ����Ϣ"));
		return;
	}

	pPapers->strPapersDesc = T2A(m_strPapersDesc);
	pPapers->strPapersName = T2A(m_strPapersName);

	m_lPapersCtrl.SetItemText(m_nCurrItemPapers, 0, (LPCTSTR)A2T(pPapers->strPapersName.c_str()));

	clock_t start, end;
	start = clock();

	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();

	CPapersInfoSaveDlg dlg(pPapers);
	if (dlg.DoModal() != IDOK)
		return;

	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itNomarlPaper = pPapers->lPaper.begin();
	for (; itNomarlPaper != pPapers->lPaper.end(); itNomarlPaper++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itNomarlPaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itNomarlPaper)->strSN);

		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itNomarlPaper)->lSnResult.begin();
		for (; itSn != (*itNomarlPaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				if ((*itSn)->nRecogVal == itRect->nSnVal)
				{
					jsnSnPosition.set("x", itRect->rt.x);
					jsnSnPosition.set("y", itRect->rt.y);
					jsnSnPosition.set("w", itRect->rt.width);
					jsnSnPosition.set("h", itRect->rt.height);
					break;
				}
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaper.set("snDetail", jsnSnDetailArry);

		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itNomarlPaper)->lOmrResult.begin();
		for (; itOmr != (*itNomarlPaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("doubt", itOmr->nDoubt);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				if (itOmr->strRecogVal.find((char)(itRect->nAnswer + 65)) != std::string::npos)
				{
					Poco::JSON::Object jsnItem;
					jsnItem.set("x", itRect->rt.x);
					jsnItem.set("y", itRect->rt.y);
					jsnItem.set("w", itRect->rt.width);
					jsnItem.set("h", itRect->rt.height);
					jsnPositionArry.add(jsnItem);
				}
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);
		jsnPaperArry.add(jsnPaper);
	}
	PAPER_LIST::iterator itIssuePaper = pPapers->lIssue.begin();
	for (; itIssuePaper != pPapers->lIssue.end(); itIssuePaper++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itIssuePaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itIssuePaper)->strSN);

		Poco::JSON::Array jsnSnDetailArry;
		SNLIST::iterator itSn = (*itIssuePaper)->lSnResult.begin();
		for (; itSn != (*itIssuePaper)->lSnResult.end(); itSn++)
		{
			Poco::JSON::Object jsnSnItem;
			jsnSnItem.set("sn", (*itSn)->nItem);
			jsnSnItem.set("val", (*itSn)->nRecogVal);

			Poco::JSON::Object jsnSnPosition;
			RECTLIST::iterator itRect = (*itSn)->lSN.begin();
			for (; itRect != (*itSn)->lSN.end(); itRect++)
			{
				if ((*itSn)->nRecogVal == itRect->nSnVal)
				{
					jsnSnPosition.set("x", itRect->rt.x);
					jsnSnPosition.set("y", itRect->rt.y);
					jsnSnPosition.set("w", itRect->rt.width);
					jsnSnPosition.set("h", itRect->rt.height);
					break;
				}
			}
			jsnSnItem.set("position", jsnSnPosition);
			jsnSnDetailArry.add(jsnSnItem);
		}
		jsnPaper.set("snDetail", jsnSnDetailArry);

		Poco::JSON::Array jsnOmrArry;
		OMRRESULTLIST::iterator itOmr = (*itIssuePaper)->lOmrResult.begin();
		for (; itOmr != (*itIssuePaper)->lOmrResult.end(); itOmr++)
		{
			Poco::JSON::Object jsnOmr;
			jsnOmr.set("th", itOmr->nTH);
			jsnOmr.set("type", itOmr->nSingle);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("doubt", itOmr->nDoubt);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
			{
				if (itOmr->strRecogVal.find((char)(itRect->nAnswer + 65)) != std::string::npos)
				{
					Poco::JSON::Object jsnItem;
					jsnItem.set("x", itRect->rt.x);
					jsnItem.set("y", itRect->rt.y);
					jsnItem.set("w", itRect->rt.width);
					jsnItem.set("h", itRect->rt.height);
					jsnPositionArry.add(jsnItem);
				}
			}
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);
		jsnPaperArry.add(jsnPaper);
	}
	//д�Ծ����Ϣ���ļ�
	std::string strUploader = CMyCodeConvert::Gb2312ToUtf8(T2A(pDlg->m_strUserName));
	std::string strEzs = T2A(pDlg->m_strEzs);
	Poco::JSON::Object jsnFileData;
	jsnFileData.set("examId", dlg.m_nExamID);
	jsnFileData.set("subjectId", dlg.m_SubjectID);
	jsnFileData.set("uploader", strUploader);
	jsnFileData.set("ezs", strEzs);
	jsnFileData.set("nTeacherId", pDlg->m_nTeacherId);
	jsnFileData.set("nUserId", pDlg->m_nUserId);
	jsnFileData.set("scanNum", pPapers->nPaperCount);		//ɨ���ѧ������
	jsnFileData.set("detail", jsnPaperArry);
	std::stringstream jsnString;
	jsnFileData.stringify(jsnString, 0);

	char szExamInfoPath[MAX_PATH] = { 0 };
	sprintf_s(szExamInfoPath, "%s\\%s\\papersInfo.dat", T2A(m_strPapersPath), pPapers->strPapersName.c_str());
	ofstream out(szExamInfoPath);
	out << jsnString.str().c_str();
	out.close();
	//

	//�Ծ��ѹ��
	char szPapersSrcPath[MAX_PATH] = { 0 };
	char szPapersSavePath[MAX_PATH] = { 0 };
	char szZipName[50] = { 0 };
	if (pDlg->m_bLogin)
	{
		Poco::LocalDateTime now;
		char szTime[50] = { 0 };
		sprintf_s(szTime, "%d%02d%02d%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

		sprintf_s(szPapersSrcPath, "%s\\%s", T2A(m_strPapersPath), pPapers->strPapersName.c_str());
		sprintf_s(szPapersSavePath, "%sPaper\\%s_%s", T2A(g_strCurrentPath), T2A(pDlg->m_strUserName), szTime);
		sprintf_s(szZipName, "%s_%s.zip", T2A(pDlg->m_strUserName), szTime);
	}
	else
	{
		sprintf_s(szPapersSrcPath, "%s\\%s", T2A(m_strPapersPath), pPapers->strPapersName.c_str());
		sprintf_s(szPapersSavePath, "%sPaper\\%s", T2A(g_strCurrentPath), pPapers->strPapersName.c_str());
		sprintf_s(szZipName, "%s.zip", pPapers->strPapersName.c_str());
	}
	CString strInfo;
	bool bWarn = false;
	strInfo.Format(_T("���ڱ���%s..."), A2T(szZipName));
//	SetStatusShowInfo(strInfo, bWarn);
	if (!ZipFile(A2T(szPapersSrcPath), A2T(szPapersSavePath)))
	{
		bWarn = true;
		strInfo.Format(_T("����%sʧ��"), A2T(szZipName));
	}
	else
	{
		end = clock();
		strInfo.Format(_T("����%s�ɹ�,�Ծ��ѹ��ʱ��: %dms"), A2T(szZipName), end - start);
//		SAFE_RELEASE(m_pPapersInfo);
	}
	AfxMessageBox(strInfo);

	//����ϴ��б�	******************		��Ҫ���м�Ȩ����	***************
	char szFileFullPath[300] = { 0 };
	sprintf_s(szFileFullPath, "%s.zip", szPapersSavePath);
	pSENDTASK pTask = new SENDTASK;
	pTask->strFileName = szZipName;
	pTask->strPath = szFileFullPath;
	g_fmSendLock.lock();
	g_lSendTask.push_back(pTask);
	g_fmSendLock.unlock();
}

LRESULT CPaperInputDlg::RoiLBtnDown(WPARAM wParam, LPARAM lParam)
{
	cv::Point pt = *(cv::Point*)(wParam);
	ShowRectByPoint(pt, m_pCurrentShowPaper);
	return TRUE;
}

void CPaperInputDlg::ShowRectByPoint(cv::Point pt, pST_PaperInfo pPaper)
{
	if (!pPaper || !pPaper->pModel || pPaper->pModel->vecPaperModel.size() < m_nCurrTabSel)
		return;

	// 	if (!pPaper->bIssuePaper)		//��ǰ����û������㣬��������ʾ
	// 		return;

	int nFind = -1;
	RECTINFO* pRc = NULL;
	PIC_LIST::iterator itPic = pPaper->lPic.begin();
	for (int i = 0; itPic != pPaper->lPic.end(); itPic++, i++)
	{
		if (i == m_nCurrTabSel)
			break;
	}

	cv::Rect rt;
	rt.x = pt.x;
	rt.y = pt.y;
	//	GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[m_nCurrTabSel].lFix, rt);
	// 	GetPosition(pPaper->pModel->vecPaperModel[m_nCurrTabSel].lFix, (*itPic)->lFix, rt);
	// 	pt.x = rt.x;
	// 	pt.y = rt.y;
	nFind = GetRectInfoByPoint(pt, *itPic, pRc);
	if (nFind < 0)
		return;

	if (!pRc)
		return;

	CString strInfo;
	strInfo.Format(_T("��ֵ: %d, Ҫ�����: %f, ʵ��: %f"), pRc->nThresholdValue, pRc->fStandardValuePercent, pRc->fRealValuePercent);
	if (pPaper->bIssuePaper)
		SetStatusShowInfo(strInfo, TRUE);
	else
		SetStatusShowInfo(strInfo);
}

int CPaperInputDlg::GetRectInfoByPoint(cv::Point pt, pST_PicInfo pPic, RECTINFO*& pRc)
{
	int  nFind = -1;
	RECTLIST::iterator itRectInfo = pPic->lNormalRect.begin();
	for (int i = 0; itRectInfo != pPic->lNormalRect.end(); itRectInfo++, i++)
	{
		if (itRectInfo->rt.contains(pt))
		{
			nFind = i;
			pRc = &(*itRectInfo);
			break;
		}
	}
	if (nFind < 0)
	{
		RECTLIST::iterator itIssueRectInfo = pPic->lIssueRect.begin();
		for (int i = 0; itIssueRectInfo != pPic->lIssueRect.end(); itIssueRectInfo++, i++)
		{
			if (itIssueRectInfo->rt.contains(pt))
			{
				nFind = i;
				pRc = &(*itIssueRectInfo);
				break;
			}
		}
	}
	return nFind;
}

void CPaperInputDlg::SetStatusShowInfo(CString strMsg, BOOL bWarn /*= FALSE*/)
{
	if (bWarn)
		m_colorStatus = RGB(255, 0, 0);
	else
		m_colorStatus = RGB(0, 0, 255);
	GetDlgItem(IDC_STATIC_TIPS)->SetWindowText(strMsg);
	TRACE("\n----------------\n");
	TRACE(strMsg);
	TRACE("\n----------------\n");
}

void CPaperInputDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_STATIC_TIPS)->SetFont(&m_fontStatus);
}

HBRUSH CPaperInputDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT CurID = pWnd->GetDlgCtrlID();

	if (IDC_STATIC_TIPS == CurID)
	{
		pDC->SetTextColor(m_colorStatus);

		return hbr;	// hbrsh;
	}
	return hbr;
}

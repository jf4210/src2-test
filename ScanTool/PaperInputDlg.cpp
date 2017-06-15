// PaperInputDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool.h"
#include "ScanToolDlg.h"
#include "GuideDlg.h"
#include "PaperInputDlg.h"
#include "afxdialogex.h"
#include <string.h>
#include <algorithm>
#include "OmrRecog.h"
#include "ModifyZkzhDlg.h"

using namespace cv;
using namespace std;
// CPaperInputDlg �Ի���

IMPLEMENT_DYNAMIC(CPaperInputDlg, CDialog)

CPaperInputDlg::CPaperInputDlg(pMODEL pModel, CWnd* pParent /*=NULL*/)
	: CDialog(CPaperInputDlg::IDD, pParent)
	, m_strPapersPath(_T("")), m_nModelPicNums(1), m_nCurrTabSel(0), m_pCurrentPicShow(NULL), m_pModel(pModel), m_pOldModel(pModel)
	, m_strModelName(_T("")), m_strPapersName(_T("")), m_strPapersDesc(_T("")), m_nCurrItemPapers(-1), m_nCurrItemPaper(-1)
	, m_colorStatus(RGB(0, 0, 255)), m_nStatusSize(35), m_pCurrentShowPaper(NULL), m_nCurrItemPaperList(-1), m_pCurrentPapers(NULL), m_pStudentMgr(NULL)
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
	ON_MESSAGE(MSG_ZKZH_RECOG, &CPaperInputDlg::MsgZkzhRecog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_Test, &CPaperInputDlg::OnBnClickedBtnTest)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_Paper, &CPaperInputDlg::OnLvnKeydownListPaper)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CPaperInputDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;
	InitUI();
	SeachModel();
	if (NULL != m_pModel)
	{
		for (int i = 0; i < m_comboModel.GetCount(); i++)
		{
			CString strItemName;
			m_comboModel.GetLBText(i, strItemName);
			if (strItemName == A2T(m_pModel->strModelName.c_str()))
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

	m_lPaperCtrl.SetExtendedStyle(m_lPaperCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lPaperCtrl.InsertColumn(0, _T("���"), LVCFMT_CENTER, 36); 
	m_lPaperCtrl.InsertColumn(1, _T("����"), LVCFMT_CENTER, 36);
	m_lPaperCtrl.InsertColumn(2, _T("�Ծ���"), LVCFMT_CENTER, 70);
	m_lPaperCtrl.InsertColumn(3, _T("*"), LVCFMT_CENTER, 20);

	m_lIssuePaperCtrl.SetExtendedStyle(m_lIssuePaperCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS);
	m_lIssuePaperCtrl.InsertColumn(0, _T("���"), LVCFMT_CENTER, 36);
	m_lIssuePaperCtrl.InsertColumn(1, _T("�Ծ���"), LVCFMT_CENTER, 80);
	m_lIssuePaperCtrl.InsertColumn(2, _T("ԭ��"), LVCFMT_LEFT, 100);
	m_lIssuePaperCtrl.EnableToolTips(TRUE);

	m_comboModel.AdjustDroppedWidth();

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
	int nPapersListWidth = static_cast<int>(nLeftCtrlWidth * 0.4);
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
		int nTabHead_H = 24;		//tab�ؼ�ͷ�ĸ߶�
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
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
	if (GetDlgItem(IDC_BTN_SAVE)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_SAVE)->MoveWindow(nTabLeftPos + nBtnWidth + nGap, nCurrentTop, nBtnWidth, nBtnHeight);
//		nCurrentTop = nCurrentTop + nBtnHeight + nGap;
	}
	if (GetDlgItem(IDC_BTN_Test)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Test)->MoveWindow(nTabLeftPos + (nBtnWidth + nGap) * 2, nCurrentTop, nBtnWidth, nBtnHeight);
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
		int nTabHead_H = 24;		//tab�ؼ�ͷ�ĸ߶�
		CRect rtPic = rtTab;
		rtPic.top = rtPic.top + nTabHead_H;
		rtPic.left += 2;
		rtPic.right -= 4;
		rtPic.bottom -= 4;
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
	std::string strPaperPath = CMyCodeConvert::Gb2312ToUtf8(T2A(m_strPapersPath));
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
	
					if (strOldFileName.find("papersInfo.dat") == std::string::npos)
					{
						lFileName.push_back(strOldFileName);
					}
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

				//2016.8.29 for test
				static int j = 0;
				if ((i - 1) % m_pModel->nPicNum == 0)
					j = 0;
				Mat mtPic = imread(CMyCodeConvert::Utf8ToGb2312(strNewFilePath));
				bool bDoubleScan = m_pModel->vecPaperModel.size() % 2 == 0 ? true : false;
				int nResult = CheckOrientation(mtPic, j, bDoubleScan);
				switch (nResult)	//1:���ģ��ͼ����Ҫ���е���ת�����򣬲���Ҫ��ת��2����ת90(ģ��ͼ����ת), 3����ת90(ģ��ͼ����ת), 4����ת180(ģ��ͼ����ת)
				{
					case 1:	break;
					case 2:
					{
							  Mat dst;
							  transpose(mtPic, dst);	//����90������ 
							  flip(dst, mtPic, 0);		//����90��ģ��ͼ����Ҫ����90��ԭͼ����Ҫ����90
					}
						break;
					case 3:
					{
							  Mat dst;
							  transpose(mtPic, dst);	//����90������ 
							  flip(dst, mtPic, 1);		//����90��ģ��ͼ����Ҫ����90��ԭͼ����Ҫ����90
					}
						break;
					case 4:
					{
							  Mat dst;
							  transpose(mtPic, dst);	//����90������ 
							  Mat dst2;
							  flip(dst, dst2, 1);
							  Mat dst5;
							  transpose(dst2, dst5);
							  flip(dst5, mtPic, 1);	//����180
					}
						break;
					default: break;
				}
				imwrite(pPic->strPicPath, mtPic);
				j++;
				//--
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

bool sortModelFile2(ST_MODELFILE& st1, ST_MODELFILE& st2)
{
	if (st1.strModifyTime > st2.strModifyTime)	return true;
	else if (st1.strModifyTime == st2.strModifyTime)	return (st1.strModelName >= st2.strModelName ? true : false);
	else return false;
}

void CPaperInputDlg::SeachModel()
{
	USES_CONVERSION;
	std::string strModelPath = T2A(g_strCurrentPath + _T("Model"));

	std::string strLog;
	try
	{
		std::list<ST_MODELFILE> lModelFile;
		std::string strUtf8Path = CMyCodeConvert::Gb2312ToUtf8(strModelPath);
		Poco::DirectoryIterator it(strUtf8Path);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			if (it->isFile() && p.getExtension() == "mod")
			{
				std::string strModelName = CMyCodeConvert::Utf8ToGb2312(p.getBaseName());

				Poco::DateTime dt(it->getLastModified());
				std::string strLastModifyTime = Poco::format("%04d-%02d-%02d", dt.year(), dt.month(), dt.day());

				ST_MODELFILE stModelFile;
				stModelFile.strModelName = strModelName;
				stModelFile.strModifyTime = strLastModifyTime;
				lModelFile.push_back(stModelFile);
			}
			it++;
		}
		lModelFile.sort(sortModelFile2);

		for(auto modelFile : lModelFile)
		{
			std::string strModelName = modelFile.strModelName;
			m_comboModel.AddString(A2T(strModelName.c_str()));
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

void CPaperInputDlg::OnCbnSelchangeComboModellist()
{
	if (m_ncomboCurrentSel == m_comboModel.GetCurSel())
		return;

	CString strModelName;
	m_comboModel.GetLBText(m_comboModel.GetCurSel(), strModelName);
	CString strModelPath = g_strCurrentPath + _T("Model\\") + strModelName;
	CString strModelFullPath = strModelPath + _T(".mod");
//	UnZipFile(strModelFullPath);		//UnZipModel(strModelFullPath);
	CZipObj zipObj;
	zipObj.setLogger(g_pLogger);
	zipObj.UnZipFile(strModelFullPath);
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
	if (pNMItemActivate->iItem < 0) return;

	m_lPaperCtrl.DeleteAllItems();
	m_lIssuePaperCtrl.DeleteAllItems();

	USES_CONVERSION;
	pPAPERSINFO pPapers = (pPAPERSINFO)m_lPapersCtrl.GetItemData(pNMItemActivate->iItem);
	m_pCurrentPapers = pPapers;

	m_nCurrItemPapers = pNMItemActivate->iItem;
	m_strPapersName = A2T(pPapers->strPapersName.c_str());
	m_strPapersDesc = A2T(pPapers->strPapersDesc.c_str());

	ShowPapers(pPapers);

	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();
	if (g_nOperatingMode == 1 || pDlg->m_bModifySN)
	{
		KillTimer(TIMER_CheckRecogComplete);
		int nCount = m_lPaperCtrl.GetItemCount();
		for (int i = 0; i < nCount; i++)
		{
			pST_PaperInfo pItemPaper = (pST_PaperInfo)(DWORD_PTR)m_lPaperCtrl.GetItemData(i);
			if (pItemPaper)
			{
				m_lPaperCtrl.SetItemText(i, 2, (LPCTSTR)A2T(pItemPaper->strSN.c_str()));
			}
		}
		SetTimer(TIMER_CheckRecogComplete, 100, NULL);
	}

	UpdateData(FALSE);
}


void CPaperInputDlg::OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (pNMItemActivate->iItem < 0)
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lPaperCtrl.GetItemData(pNMItemActivate->iItem);
	m_nCurrItemPaper = pNMItemActivate->iItem;
	m_pCurrentShowPaper = pPaper;
	m_nCurrItemPaperList = m_nCurrItemPaper;

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

	//˫��Ϊ�յ�׼��֤��ʱ��ʾ׼��֤���޸Ĵ���
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();
	if ((g_nOperatingMode == 1 || pDlg->m_bModifySN) && m_pModel && pPaper && (pPaper->strSN.empty() || pPaper->bModifyZKZH))
	{
		if (!m_pStudentMgr)
		{
			USES_CONVERSION;
			m_pStudentMgr = new CStudentMgr();
			std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
			bool bResult = m_pStudentMgr->InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
		}
		CModifyZkzhDlg zkzhDlg(m_pModel, m_pCurrentPapers, m_pStudentMgr, pPaper);
		zkzhDlg.DoModal();

		ShowPapers(m_pCurrentPapers);
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
// #ifdef WarpAffine_TEST
// 			cv::Mat	inverseMat(2, 3, CV_32FC1);
// 			PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
// #endif
#else
		Mat matImg = matSrc;
#endif

#ifdef WarpAffine_TEST
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
#endif

#ifdef Test_ShowOriPosition
		cv::Mat	inverseMat(2, 3, CV_32FC1);
		GetInverseMat((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
#endif

		Mat tmp = matImg.clone();
		Mat tmp2 = matImg.clone();

		//++��ӡ������Ϣ
		//-----------------------
		RECTLIST::iterator itSelRoi = pPaper->pModel->vecPaperModel[i]->lSelFixRoi.begin();													//��ʾʶ�𶨵��ѡ����
		for (int j = 0; itSelRoi != pPaper->pModel->vecPaperModel[i]->lSelFixRoi.end(); itSelRoi++, j++)
		{
			cv::Rect rt = (*itSelRoi).rt;

			char szCP[20] = { 0 };
			rectangle(tmp, rt, CV_RGB(0, 0, 255), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}

		RECTLIST::iterator itPicFix = (*itPic)->lFix.begin();													//��ʾʶ������Ķ���
		for (int j = 0; itPicFix != (*itPic)->lFix.end(); itPicFix++, j++)
		{
			cv::Rect rt = (*itPicFix).rt;

			char szCP[20] = { 0 };
			sprintf_s(szCP, "R_F%d", j);
			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp, rt, CV_RGB(0, 255, 0), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
		RECTLIST::iterator itFixRect = pPaper->pModel->vecPaperModel[i]->lFix.begin();							//��ʾģ���ϵĶ����Ӧ�����Ծ��ϵ��¶���
		for (int j = 0; itFixRect != pPaper->pModel->vecPaperModel[i]->lFix.end(); itFixRect++, j++)
		{
			cv::Rect rt = (*itFixRect).rt;

			TRACE("ģ�嶨�������: (%d, %d, %d, %d)\n", (*itFixRect).rt.x, (*itFixRect).rt.y, (*itFixRect).rt.width, (*itFixRect).rt.height);

			char szCP[20] = { 0 };
			sprintf_s(szCP, "M_F%d", j);
			putText(tmp, szCP, Point(rt.x, rt.y + rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
			rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
			rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
		}
		//--

		RECTLIST::iterator itNormalRect = (*itPic)->lNormalRect.begin();
		for (int j = 0; itNormalRect != (*itPic)->lNormalRect.end(); itNormalRect++, j++)
		{
			cv::Rect rt = (*itNormalRect).rt;
			if (itNormalRect->eCPType == SN || itNormalRect->eCPType == OMR)
			{
				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			else
			{
				rectangle(tmp, rt, CV_RGB(250, 150, 20), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
		}

		//��ӡOMR��SNλ��
#ifdef PaintOmrSnRect
// 		SNLIST::iterator itSN = pPaper->pModel->vecPaperModel[i]->lSNInfo.begin();
// 		for (; itSN != pPaper->pModel->vecPaperModel[i]->lSNInfo.end(); itSN++)
// 		{
// 			pSN_ITEM pSnItem = *itSN;
// 			RECTLIST::iterator itSnItem = pSnItem->lSN.begin();
// 			for (; itSnItem != pSnItem->lSN.end(); itSnItem++)
// 			{
// 				cv::Rect rt = (*itSnItem).rt;
// #ifdef Test_ShowOriPosition
// 				GetPosition2(inverseMat, rt, rt);
// #else
// 				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
// #endif
// 
// 				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
// 				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
// 			}
// 		}
// 
// 		OMRLIST::iterator itOmr = pPaper->pModel->vecPaperModel[i]->lOMR2.begin();
// 		for (; itOmr != pPaper->pModel->vecPaperModel[i]->lOMR2.end(); itOmr++)
// 		{
// 			pOMR_QUESTION pOmrQuestion = &(*itOmr);
// 			RECTLIST::iterator itOmrItem = pOmrQuestion->lSelAnswer.begin();
// 			for (; itOmrItem != pOmrQuestion->lSelAnswer.end(); itOmrItem++)
// 			{
// 				cv::Rect rt = (*itOmrItem).rt;
// #ifdef Test_ShowOriPosition
// 				GetPosition2(inverseMat, rt, rt);
// #else
// 				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
// #endif
// 
// 				rectangle(tmp, rt, CV_RGB(255, 0, 0), 2);
// 				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
// 			}
// 		}
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
// #ifdef WarpAffine_TEST
// 			cv::Mat	inverseMat(2, 3, CV_32FC1);
// 			PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
// #endif
#else
			Mat matImg = matSrc;
#endif

#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			PicTransfer(i, matImg, (*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, inverseMat);
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
				sprintf_s(szCP, "R_F%d", j);
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
//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);

				char szCP[20] = { 0 };
				rectangle(tmp, rt, CV_RGB(0, 255, 255), 2);
				rectangle(tmp2, rt, CV_RGB(255, 233, 10), -1);
			}
			//----------------------------

			RECTLIST::iterator itIssueRect = (*itPic)->lIssueRect.begin();
			for (int j = 0; itIssueRect != (*itPic)->lIssueRect.end(); itIssueRect++, j++)
			{
				cv::Rect rt = (*itIssueRect).rt;
//				GetPosition((*itPic)->lFix, pPaper->pModel->vecPaperModel[i]->lFix, rt);
				if (j == 0)
				{
					pt = rt.tl();
					pt.x = pt.x - 100;
					pt.y = pt.y - 100;
				}

				char szCP[20] = { 0 };
				sprintf_s(szCP, "Err%d", j);
				putText(tmp, szCP, Point((*itIssueRect).rt.x, (*itIssueRect).rt.y + (*itIssueRect).rt.height / 2), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0));	//CV_FONT_HERSHEY_COMPLEX
				rectangle(tmp, (*itIssueRect).rt, CV_RGB(255, 20, 30), 2);
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
	
	if (pNMItemActivate->iItem < 0)
		return;

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

	//˫��Ϊ�յ�׼��֤��ʱ��ʾ׼��֤���޸Ĵ���
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();
	if ((g_nOperatingMode == 1 || pDlg->m_bModifySN) && m_pModel && pPaper)
	{
		if (!m_pStudentMgr)
		{
			USES_CONVERSION;
			m_pStudentMgr = new CStudentMgr();
			std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
			bool bResult = m_pStudentMgr->InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
		}
		CModifyZkzhDlg zkzhDlg(m_pModel, m_pCurrentPapers, m_pStudentMgr, pPaper);
		zkzhDlg.DoModal();

		ShowPapers(m_pCurrentPapers);
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


	if (pPapers->lIssue.size() > 0)
	{
		if (g_nOperatingMode == 2)
		{
			AfxMessageBox(_T("����ʶ���쳣�Ծ������ϴ������ȴ����쳣�Ծ�"));
			return;
		}
		else
		{
			CString strMsg = _T("");
			strMsg.Format(_T("����%d�������Ծ���Щ�Ծ���Ҫ�����ҳ�ɨ�裬�˴ν����ϴ���%d���Ծ��Ƿ�ȷ���ϴ�?"), pPapers->lIssue.size(), pPapers->lIssue.size());
			if (MessageBox(strMsg, _T("����"), MB_YESNO) != IDYES)
				return;
			pPapers->nPaperCount = pPapers->lPaper.size();		//�޸�ɨ���������������Ծ�ɾ�������㵽ɨ���Ծ��С�
		}
	}

	pPapers->strPapersDesc = T2A(m_strPapersDesc);
	pPapers->strPapersName = T2A(m_strPapersName);

	m_lPapersCtrl.SetItemText(m_nCurrItemPapers, 0, (LPCTSTR)A2T(pPapers->strPapersName.c_str()));

	clock_t start, end;
	start = clock();

	BOOL	bLogin = FALSE;
	CString strUser = _T("");
	CString strEzs = _T("");
	int nTeacherId = -1;
	int nUserId = -1;
#ifdef SHOW_GUIDEDLG
	CGuideDlg* pDlg = (CGuideDlg*)AfxGetMainWnd();

	strEzs = pDlg->m_strEzs;
	strUser = pDlg->m_strUserName;
	nTeacherId = pDlg->m_nTeacherId;
	nUserId = pDlg->m_nUserId;
	bLogin = pDlg->m_bLogin;
#else
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();	//AfxGetMainWnd()
	strEzs = pDlg->m_strEzs;
	strUser = pDlg->m_strUserName;
	nTeacherId = pDlg->m_nTeacherId;
	nUserId = pDlg->m_nUserId;
	bLogin = pDlg->m_bLogin;
#endif
	

	CPapersInfoSaveDlg dlg(pPapers, m_pModel);
	if (dlg.DoModal() != IDOK)
		return;

	Poco::JSON::Array jsnPaperArry;
	PAPER_LIST::iterator itNomarlPaper = pPapers->lPaper.begin();
	for (int i = 0; itNomarlPaper != pPapers->lPaper.end(); itNomarlPaper++, i++)
	{
		Poco::JSON::Object jsnPaper;
		jsnPaper.set("name", (*itNomarlPaper)->strStudentInfo);
		jsnPaper.set("zkzh", (*itNomarlPaper)->strSN);
		jsnPaper.set("qk", (*itNomarlPaper)->nQKFlag);

		int nIssueFlag = 0;			//0 - �����Ծ���ȫ����ʶ�������ģ����˹���Ԥ��1 - �����Ծ�ɨ��Ա�ֶ��޸Ĺ���2-׼��֤��Ϊ�գ�ɨ��Աû���޸ģ�3-ɨ��Ա��ʶ����Ҫ��ɨ���Ծ�
		if ((*itNomarlPaper)->strSN.empty() && !(*itNomarlPaper)->bModifyZKZH)
			nIssueFlag = 2;
		if ((*itNomarlPaper)->bModifyZKZH)
			nIssueFlag = 1;
		jsnPaper.set("issueFlag", nIssueFlag);
		//++���ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
		jsnPaper.set("modify", (*itNomarlPaper)->bModifyZKZH);	//׼��֤���޸ı�ʶ
		jsnPaper.set("reScan", (*itNomarlPaper)->bReScan);		//��ɨ��ʶ
		jsnPaper.set("IssueList", 0);		//��ʶ�˿������������б����ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
		//--

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
					jsnSnPosition.set("x", itRect->rt.x);
					jsnSnPosition.set("y", itRect->rt.y);
					jsnSnPosition.set("w", itRect->rt.width);
					jsnSnPosition.set("h", itRect->rt.height);
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
			jsnOmr.set("type", itOmr->nSingle + 1);
			jsnOmr.set("value", itOmr->strRecogVal);
			jsnOmr.set("value1", itOmr->strRecogVal1);
			jsnOmr.set("value2", itOmr->strRecogVal2);
			jsnOmr.set("value3", itOmr->strRecogVal3);
			jsnOmr.set("doubt", itOmr->nDoubt);
			Poco::JSON::Array jsnPositionArry;
			RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
			for (; itRect != itOmr->lSelAnswer.end(); itRect++)
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
			jsnOmr.set("position", jsnPositionArry);
			jsnOmrArry.add(jsnOmr);
		}
		jsnPaper.set("omr", jsnOmrArry);

		Poco::JSON::Array jsnElectOmrArry;
		ELECTOMR_LIST::iterator itElectOmr = (*itNomarlPaper)->lElectOmrResult.begin();
		for (; itElectOmr != (*itNomarlPaper)->lElectOmrResult.end(); itElectOmr++)
		{
			Poco::JSON::Object jsnElectOmr;
			jsnElectOmr.set("paperId", i + 1);
			jsnElectOmr.set("doubt", itElectOmr->nDoubt);
			jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
			jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
			jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
			jsnElectOmr.set("value", itElectOmr->strRecogResult);
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
#if 1
		if (jsnElectOmrArry.size() > 0)
			jsnPaper.set("electOmr", jsnElectOmrArry);		//ѡ������
#else
		jsnPaper.set("electOmr", jsnElectOmrArry);		//ѡ������
#endif
		jsnPaperArry.add(jsnPaper);
	}

	if (g_nOperatingMode == 1)		//��ģʽʱ���쳣�Ծ�Ҳһ���ϴ����������ʶ
	{
		PAPER_LIST::iterator itIssuePaper = pPapers->lIssue.begin();
		for (int j = 0; itIssuePaper != pPapers->lIssue.end(); itIssuePaper++, j++)
		{
			Poco::JSON::Object jsnPaper;
			jsnPaper.set("name", (*itIssuePaper)->strStudentInfo);
			jsnPaper.set("zkzh", (*itIssuePaper)->strSN);
			jsnPaper.set("qk", (*itIssuePaper)->nQKFlag);

			int nIssueFlag = 0;			//0 - �����Ծ���ȫ����ʶ�������ģ����˹���Ԥ��1 - �����Ծ�ɨ��Ա�ֶ��޸Ĺ���2-׼��֤��Ϊ�գ�ɨ��Աû���޸ģ�3-ɨ��Ա��ʶ����Ҫ��ɨ���Ծ�
			if ((*itIssuePaper)->strSN.empty())
				nIssueFlag = 2;
			if ((*itIssuePaper)->bReScan)		//������ɨȨ�޸��󣬷ź�������
				nIssueFlag = 3;
			jsnPaper.set("issueFlag", nIssueFlag);
			//++���ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
			jsnPaper.set("modify", (*itIssuePaper)->bModifyZKZH);	//׼��֤���޸ı�ʶ
			jsnPaper.set("reScan", (*itIssuePaper)->bReScan);		//��ɨ��ʶ
			jsnPaper.set("IssueList", 1);		//��ʶ�˿������������б����ϴ�������ʱ���ã�ֻ�ڴ�Pkg�ָ�Papersʱ����
			//--

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
					jsnSnPosition.set("x", itRect->rt.x);
					jsnSnPosition.set("y", itRect->rt.y);
					jsnSnPosition.set("w", itRect->rt.width);
					jsnSnPosition.set("h", itRect->rt.height);
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
				jsnOmr.set("type", itOmr->nSingle + 1);
				jsnOmr.set("value", itOmr->strRecogVal);
				jsnOmr.set("value1", itOmr->strRecogVal1);
				jsnOmr.set("value2", itOmr->strRecogVal2);
				jsnOmr.set("value3", itOmr->strRecogVal3);
				jsnOmr.set("doubt", itOmr->nDoubt);
				Poco::JSON::Array jsnPositionArry;
				RECTLIST::iterator itRect = itOmr->lSelAnswer.begin();
				for (; itRect != itOmr->lSelAnswer.end(); itRect++)
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
				jsnOmr.set("position", jsnPositionArry);
				jsnOmrArry.add(jsnOmr);
			}
			jsnPaper.set("omr", jsnOmrArry);

			Poco::JSON::Array jsnElectOmrArry;
			ELECTOMR_LIST::iterator itElectOmr = (*itIssuePaper)->lElectOmrResult.begin();
			for (; itElectOmr != (*itIssuePaper)->lElectOmrResult.end(); itElectOmr++)
			{
				Poco::JSON::Object jsnElectOmr;
				jsnElectOmr.set("paperId", j + 1);
				jsnElectOmr.set("doubt", itElectOmr->nDoubt);
				jsnElectOmr.set("th", itElectOmr->sElectOmrGroupInfo.nGroupID);
				jsnElectOmr.set("allItems", itElectOmr->sElectOmrGroupInfo.nAllCount);
				jsnElectOmr.set("realItem", itElectOmr->sElectOmrGroupInfo.nRealCount);
				jsnElectOmr.set("value", itElectOmr->strRecogResult);
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
			jsnPaper.set("electOmr", jsnElectOmrArry);		//ѡ������
			jsnPaperArry.add(jsnPaper);
		}
	}

	//д�Ծ����Ϣ���ļ�
	std::string strUploader = CMyCodeConvert::Gb2312ToUtf8(T2A(strUser));
	std::string sEzs = T2A(strEzs);
	Poco::JSON::Object jsnFileData;
	jsnFileData.set("examId", dlg.m_nExamID);
	jsnFileData.set("subjectId", dlg.m_SubjectID);
	jsnFileData.set("uploader", strUploader);
	jsnFileData.set("ezs", sEzs);
	jsnFileData.set("nTeacherId", nTeacherId);
	jsnFileData.set("nUserId", nUserId);
	jsnFileData.set("scanNum", pPapers->nPaperCount);		//ɨ���ѧ������
	jsnFileData.set("detail", jsnPaperArry);
	jsnFileData.set("desc", CMyCodeConvert::Gb2312ToUtf8(pPapers->strPapersDesc));

	jsnFileData.set("nOmrDoubt", pPapers->nOmrDoubt);
	jsnFileData.set("nOmrNull", pPapers->nOmrNull);
	jsnFileData.set("nSnNull", pPapers->nSnNull);
	jsnFileData.set("RecogMode", g_nOperatingMode);			//ʶ��ģʽ��1-��ģʽ(��������У��㲻ֹͣʶ��)��2-�ϸ�ģʽ
	std::stringstream jsnString;
	jsnFileData.stringify(jsnString, 0);


	std::string strFileData;
#ifdef USES_FILE_ENC
	if(!encString(jsnString.str(), strFileData))
		strFileData = jsnString.str();
#else
	strFileData = jsnString.str();
#endif

	char szExamInfoPath[MAX_PATH] = { 0 }; 
//	sprintf_s(szExamInfoPath, "%s\\%s\\papersInfo.dat", T2A(m_strPapersPath), pPapers->strPapersName.c_str());
	sprintf_s(szExamInfoPath, "%sPaper\\%s\\papersInfo.dat", T2A(g_strCurrentPath), T2A(m_strPapersName));
	ofstream out(szExamInfoPath);
	out << strFileData.c_str();
	out.close();
	//

	//�Ծ��ѹ��
	char szPapersSrcPath[MAX_PATH] = { 0 };
	char szPapersSavePath[MAX_PATH] = { 0 };
	char szZipName[100] = { 0 };
	char szZipBaseName[90] = { 0 };
	if (bLogin)
	{
		Poco::LocalDateTime now;
		char szTime[50] = { 0 };
		sprintf_s(szTime, "%d%02d%02d%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

//		sprintf_s(szPapersSrcPath, "%s\\%s", T2A(m_strPapersPath), pPapers->strPapersName.c_str());
		sprintf_s(szPapersSrcPath, "%sPaper\\%s", T2A(g_strCurrentPath), T2A(m_strPapersName));

		sprintf_s(szPapersSavePath, "%sPaper\\%s_%d-%d_%s_%d", T2A(g_strCurrentPath), T2A(strUser), dlg.m_nExamID, dlg.m_SubjectID, szTime, pPapers->nPaperCount);
		sprintf_s(szZipBaseName, "%s_%d-%d_%s_%d", T2A(strUser), dlg.m_nExamID, dlg.m_SubjectID, szTime, pPapers->nPaperCount);
		sprintf_s(szZipName, "%s_%d-%d_%s_%d%s", T2A(strUser), dlg.m_nExamID, dlg.m_SubjectID, szTime, pPapers->nPaperCount, T2A(PAPERS_EXT_NAME));
// 		sprintf_s(szPapersSavePath, "%sPaper\\%s_%d-%d_%s", T2A(g_strCurrentPath), T2A(strUser), dlg.m_nExamID, dlg.m_SubjectID, szTime);
// 		sprintf_s(szZipBaseName, "%s_%d-%d_%s", T2A(strUser), dlg.m_nExamID, dlg.m_SubjectID, szTime);
// 		sprintf_s(szZipName, "%s_%d-%d_%s%s", T2A(strUser), dlg.m_nExamID, dlg.m_SubjectID, szTime, T2A(PAPERS_EXT_NAME));
	}
	else
	{
//		sprintf_s(szPapersSrcPath, "%s\\%s", T2A(m_strPapersPath), pPapers->strPapersName.c_str());
		sprintf_s(szPapersSrcPath, "%sPaper\\%s", T2A(g_strCurrentPath), T2A(m_strPapersName));
		sprintf_s(szPapersSavePath, "%sPaper\\%s", T2A(g_strCurrentPath), pPapers->strPapersName.c_str());
		sprintf_s(szZipBaseName, "%s", pPapers->strPapersName.c_str());
		sprintf_s(szZipName, "%s%s", pPapers->strPapersName.c_str(), T2A(PAPERS_EXT_NAME));
	}
	CString strInfo;
	bool bWarn = false;
	strInfo.Format(_T("���ڱ���%s..."), A2T(szZipName));

	//��ʱĿ¼�������Ա�ѹ��ʱ����ɨ��
	std::string strSrcPicDirPath;
	std::string strPicPath = szPapersSrcPath;
	try
	{
		Poco::File tmpPath(CMyCodeConvert::Gb2312ToUtf8(strPicPath));

		if (!tmpPath.exists())
		{
			std::string strErr = Poco::format("�ļ���(%s)�����ڡ�������ӵ�ѹ������", strPicPath);
			g_pLogger->information(strErr);
			return;
		}

		char szCompressDirPath[MAX_PATH] = { 0 };
		sprintf_s(szCompressDirPath, "%sPaper\\%s_ToCompress", T2A(g_strCurrentPath), szZipBaseName);
		strSrcPicDirPath = szCompressDirPath;
		std::string strUtf8NewPath = CMyCodeConvert::Gb2312ToUtf8(strSrcPicDirPath);

		tmpPath.renameTo(strUtf8NewPath);
	}
	catch (Poco::Exception& exc)
	{
		std::string strLog = "��ʱ�ļ���������ʧ��(" + exc.message() + "): ";
		strLog.append(strPicPath);
		g_pLogger->information(strLog);
		strSrcPicDirPath = strPicPath;
	}

	pCOMPRESSTASK pTask = new COMPRESSTASK;
//	pTask->bDelSrcDir = false;
	pTask->strCompressFileName = szZipName;
	pTask->strExtName = T2A(PAPERS_EXT_NAME);
	pTask->strSavePath = szPapersSavePath;
	pTask->strSrcFilePath = strSrcPicDirPath;
	pTask->pPapersInfo = pPapers;
	pTask->bReleasePapers = false;			//ѹ����󣬲����Զ��ͷ��Ծ����Ϣ��ͨ���ⲿ�����ͷ�
	g_fmCompressLock.lock();
	g_lCompressTask.push_back(pTask);
	g_fmCompressLock.unlock();
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
	
	if (nFind < 0)
	{
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
	}
	return nFind;
}

LRESULT CPaperInputDlg::MsgZkzhRecog(WPARAM wParam, LPARAM lParam)
{
	pST_PaperInfo pPaper = (pST_PaperInfo)wParam;
	pPAPERSINFO   pPapers = (pPAPERSINFO)lParam;
	CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();
	if (g_nOperatingMode != 1 && !pDlg->m_bModifySN)
		return FALSE;

	if (pPapers != m_pCurrentPapers)
		return FALSE;

	USES_CONVERSION;
	int nCount = m_lPaperCtrl.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		pST_PaperInfo pItemPaper = (pST_PaperInfo)(DWORD_PTR)m_lPaperCtrl.GetItemData(i);
		if (pItemPaper == pPaper)
		{
			m_lPaperCtrl.SetItemText(i, 2, (LPCTSTR)A2T(pPaper->strSN.c_str()));
			break;
		}
	}
	return TRUE;
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


void sharpenImage11(const cv::Mat &image, cv::Mat &result, int nKernel)
{
	//��������ʼ���˲�ģ��
	cv::Mat kernel(nKernel, nKernel, CV_32F, cv::Scalar(0));
	kernel.at<float>(1, 1) = 5;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;
	kernel.at<float>(2, 1) = -1.0;

	result.create(image.size(), image.type());

	//��ͼ������˲�
	cv::filter2D(image, result, image.depth(), kernel);
}

float GetRtDensity1(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod)
{
	Mat matCompRoi;
	matCompRoi = matSrc(rt);
	cv::cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);
	cv::GaussianBlur(matCompRoi, matCompRoi, cv::Size(rcMod.nGaussKernel, rcMod.nGaussKernel), 0, 0);
	sharpenImage11(matCompRoi, matCompRoi, rcMod.nSharpKernel);

	const int channels[1] = { 0 };
	const float* ranges[1];
	const int histSize[1] = { 1 };
	float hranges[2];
	hranges[0] = g_nRecogGrayMin;
	hranges[1] = static_cast<float>(rcMod.nThresholdValue);
	ranges[0] = hranges;

	MatND src_hist;
	cv::calcHist(&matCompRoi, 1, channels, Mat(), src_hist, 1, histSize, ranges, false);

	float fRealVal = src_hist.at<float>(0);
	float fRealArea = rt.area();
	float fRealDensity = fRealVal / fRealArea;

	return fRealVal;
}

bool bGetMaxRect1(cv::Mat& matSrc, cv::Rect rt, RECTINFO rcMod, cv::Rect& rtMax)
{
	clock_t start, end;
	start = clock();

	bool bResult = false;

	int nResult = 0;
	std::vector<Rect>RectCompList;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matSrc.cols)
		{
			rt.width = matSrc.cols - rt.x;
		}
		if (rt.br().y > matSrc.rows)
		{
			rt.height = matSrc.rows - rt.y;
		}

		Mat matCompRoi;
		matCompRoi = matSrc(rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(rcMod.nGaussKernel, rcMod.nGaussKernel), 0, 0);
		sharpenImage11(matCompRoi, matCompRoi, rcMod.nSharpKernel);

#ifdef USES_GETTHRESHOLD_ZTFB
		const int channels[1] = { 0 };
		const int histSize[1] = { 150 };
		float hranges[2] = { 0, 150 };
		const float* ranges[1];
		ranges[0] = hranges;
		MatND hist;
		calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		int nThreshold = 150;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		if (nCount > 0)
		{
			float fMean = (float)nSum / nCount;		//��ֵ

			for (int h = 0; h < hist.rows; h++)	//histSize
			{
				float binVal = hist.at<float>(h);

				nDevSum += pow(h - fMean, 2)*binVal;
			}
			float fStdev = sqrt(nDevSum / nCount);	//��׼��
			nThreshold = fMean + 2 * fStdev;
			if (fStdev > fMean)
				nThreshold = fMean + fStdev;
		}
		if (nThreshold > 150) nThreshold = 150;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);

		// 		int blockSize = 25;		//25
		// 		int constValue = 10;
		// 		cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
#else
		threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
		//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
		Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
		dilate(matCompRoi, matCompRoi, element_Anticlutter);
		erode(matCompRoi, matCompRoi, element_Anticlutter);

		cv::Canny(matCompRoi, matCompRoi, 0, rcMod.nCannyKernel, 5);
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(6, 6)	��ͨ�հ׿��ʶ��
		dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//��ȡ����  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
#if 0
		//ģ��ͼ���ˮƽͬ��ͷƽ������

		RECTLIST::iterator itBegin;
		if (nHead == 1)	//���ˮƽͬ��ͷ
			itBegin = pModel->vecPaperModel[nPic]->lH_Head.begin();
		else if (nHead == 2)
			itBegin = pModel->vecPaperModel[nPic]->lV_Head.begin();
		RECTINFO rcFist = *itBegin;
		RECTINFO rcSecond = *(++itBegin);

		int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
		int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

		float fPer_W, fPer_H;	//ģ��ڶ��������һ����Ŀ��ߵı�����������Сֵ����
		cv::Rect rtFirst, rtSecond;
		if (nOrientation == 1 || nOrientation == 4)
		{
			rtSecond = rcSecond.rt;
			rtFirst = rcFist.rt;
			fPer_W = 0.5;
			fPer_H = 0.25;
		}
		else if (nOrientation == 2 || nOrientation == 3)
		{
			rtSecond.width = rcSecond.rt.height;
			rtSecond.height = rcSecond.rt.width;

			rtFirst.width = rcFist.rt.height;
			rtFirst.height = rcFist.rt.width;
			fPer_W = 0.25;
			fPer_H = 0.5;
		}

		if (pModel->nType == 1)
		{
			int nMid_modelW = rcSecond.rt.width;
			int nMid_modelH = rcSecond.rt.height;
			int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
			nMidInterW = 3;
			nMidInterH = 3;
			nHeadInterW = 4;
			nHeadInterH = 4;
			nMid_minW = nMid_modelW - nMidInterW;
			nMid_maxW = nMid_modelW + nMidInterW;
			nMid_minH = nMid_modelH - nMidInterH;
			nMid_maxH = nMid_modelH + nMidInterH;

			nHead_minW = rcFist.rt.width - nHeadInterW;
			nHead_maxW = rcFist.rt.width + nHeadInterW;
			nHead_minH = rcFist.rt.height - nHeadInterH;
			nHead_maxH = rcFist.rt.height + nHeadInterH;
		}
		else
		{
			float fOffset = 0.2;
			nMid_minW = rtSecond.width * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_maxW = rtSecond.width * (1 + fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_minH = rtSecond.height * (1 - fOffset);		//ͬ��
			nMid_maxH = rtSecond.height * (1 + fOffset);		//ͬ��

			nHead_minW = rtFirst.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nHead_maxW = rtFirst.width * (1 + fOffset);		//ͬ��
			nHead_minH = rtFirst.height * (1 - fOffset);		//ͬ��
			nHead_maxH = rtFirst.height * (1 + fOffset);		//ͬ��
		}

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;

			if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
			{
				if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//�ų���һ�������һ�����ͬ��ͷ
				{
					TRACE("����ͬ��ͷ(%d,%d,%d,%d), Ҫ��ΧW:[%d,%d], H[%d,%d], �ο���С(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
					continue;
				}
				else
				{
					TRACE("��βͬ��ͷ(����λ��ͬ��ͷ)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
				}
			}
			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
#else
		int nMaxArea = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;
			if (rm.area() > nMaxArea)
			{
				rtMax = rm;
				nMaxArea = rm.area();
			}
		}
#endif
		if (nMaxArea > 0)
			bResult = true;
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "ʶ��У�������쳣: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		nResult = -1;
	}
	end = clock();
	TRACE("�����������ʱ��: %d\n", end - start);

	return bResult;
}

int GetRects1(cv::Mat& matSrc, cv::Rect rt, pMODEL pModel, int nPic, int nOrientation, int nHead)
{
	clock_t start, end;
	start = clock();

	int nResult = 0;
	std::vector<Rect>RectCompList;
	try
	{
		if (rt.x < 0) rt.x = 0;
		if (rt.y < 0) rt.y = 0;
		if (rt.br().x > matSrc.cols)
		{
			rt.width = matSrc.cols - rt.x;
		}
		if (rt.br().y > matSrc.rows)
		{
			rt.height = matSrc.rows - rt.y;
		}

		Mat matCompRoi;
		matCompRoi = matSrc(rt);

		cvtColor(matCompRoi, matCompRoi, CV_BGR2GRAY);

		GaussianBlur(matCompRoi, matCompRoi, cv::Size(5, 5), 0, 0);
		sharpenImage11(matCompRoi, matCompRoi, 3);

#ifdef USES_GETTHRESHOLD_ZTFB
		const int channels[1] = { 0 };
		const int histSize[1] = { 150 };
		float hranges[2] = { 0, 150 };
		const float* ranges[1];
		ranges[0] = hranges;
		MatND hist;
		calcHist(&matCompRoi, 1, channels, Mat(), hist, 1, histSize, ranges);	//histSize, ranges

		int nSum = 0;
		int nDevSum = 0;
		int nCount = 0;
		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nCount += static_cast<int>(binVal);
			nSum += h*binVal;
		}
		float fMean = (float)nSum / nCount;		//��ֵ

		for (int h = 0; h < hist.rows; h++)	//histSize
		{
			float binVal = hist.at<float>(h);

			nDevSum += pow(h - fMean, 2)*binVal;
		}
		float fStdev = sqrt(nDevSum / nCount);	//��׼��
		int nThreshold = fMean + 2 * fStdev;
		if (fStdev > fMean)
			nThreshold = fMean + fStdev;

		if (nThreshold > 150) nThreshold = 150;
		threshold(matCompRoi, matCompRoi, nThreshold, 255, THRESH_BINARY);

// 		int blockSize = 25;		//25
// 		int constValue = 10;
// 		cv::adaptiveThreshold(matCompRoi, matCompRoi, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, blockSize, constValue);
#else
		threshold(matCompRoi, matCompRoi, 60, 255, THRESH_BINARY);
#endif
		//ȥ��������Ϣ�������ͺ�ʴ��ԭ, ��ȥ��һЩ��������
		Mat element_Anticlutter = getStructuringElement(MORPH_RECT, Size(_nAnticlutterKernel_, _nAnticlutterKernel_));	//Size(6, 6)	��ͨ�հ׿��ʶ��		Size(3, 3)
		dilate(matCompRoi, matCompRoi, element_Anticlutter);
		erode(matCompRoi, matCompRoi, element_Anticlutter);

		cv::Canny(matCompRoi, matCompRoi, 0, 90, 5);
		Mat element = getStructuringElement(MORPH_RECT, Size(6, 6));	//Size(6, 6)	��ͨ�հ׿��ʶ��
		dilate(matCompRoi, matCompRoi, element);
		IplImage ipl_img(matCompRoi);

		//the parm. for cvFindContours  
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;

		//��ȡ����  
		cvFindContours(&ipl_img, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
#if 1
		//ģ��ͼ���ˮƽͬ��ͷƽ������

		RECTLIST::iterator itBegin;
		if (nHead == 1)	//���ˮƽͬ��ͷ
			itBegin = pModel->vecPaperModel[nPic]->lH_Head.begin();
		else if (nHead == 2)
			itBegin = pModel->vecPaperModel[nPic]->lV_Head.begin();
		RECTINFO rcFist = *itBegin;
		RECTINFO rcSecond = *(++itBegin);

		int nMid_minW, nMid_maxW, nMid_minH, nMid_maxH;
		int nHead_minW, nHead_maxW, nHead_minH, nHead_maxH;

		float fPer_W, fPer_H;	//ģ��ڶ��������һ����Ŀ��ߵı�����������Сֵ����
		cv::Rect rtFirst, rtSecond;
		if (nOrientation == 1 || nOrientation == 4)
		{
			rtSecond = rcSecond.rt;
			rtFirst = rcFist.rt;
			fPer_W = 0.5;
			fPer_H = 0.25;
		}
		else if (nOrientation == 2 || nOrientation == 3)
		{
			rtSecond.width = rcSecond.rt.height;
			rtSecond.height = rcSecond.rt.width;

			rtFirst.width = rcFist.rt.height;
			rtFirst.height = rcFist.rt.width;
			fPer_W = 0.25;
			fPer_H = 0.5;
		}

		if (pModel->nType == 1)
		{
			int nMid_modelW = rcSecond.rt.width;
			int nMid_modelH = rcSecond.rt.height;
			int nMidInterW, nMidInterH, nHeadInterW, nHeadInterH;
			nMidInterW = 3;
			nMidInterH = 3;
			nHeadInterW = 4;
			nHeadInterH = 4;
			nMid_minW = nMid_modelW - nMidInterW;
			nMid_maxW = nMid_modelW + nMidInterW;
			nMid_minH = nMid_modelH - nMidInterH;
			nMid_maxH = nMid_modelH + nMidInterH;

			nHead_minW = rcFist.rt.width - nHeadInterW;
			nHead_maxW = rcFist.rt.width + nHeadInterW;
			nHead_minH = rcFist.rt.height - nHeadInterH;
			nHead_maxH = rcFist.rt.height + nHeadInterH;
		}
		else
		{
			float fOffset = 0.2;
			nMid_minW = rtSecond.width * (1 - fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_maxW = rtSecond.width * (1 + fOffset);		//�м�ͬ��ͷ�����ģ���м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nMid_minH = rtSecond.height * (1 - fOffset);		//ͬ��
			nMid_maxH = rtSecond.height * (1 + fOffset);		//ͬ��

			nHead_minW = rtFirst.width * (1 - fOffset);		//����ͬ��ͷ(��һ�������һ��)����������м�ͬ��ͷ��ȵ�ƫ�����ģ��ͬ��ͷ��ȵ�0.2
			nHead_maxW = rtFirst.width * (1 + fOffset);		//ͬ��
			nHead_minH = rtFirst.height * (1 - fOffset);		//ͬ��
			nHead_maxH = rtFirst.height * (1 + fOffset);		//ͬ��
		}

		int nYSum = 0;
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;

			if (rm.width < nMid_minW || rm.height < nMid_minH || rm.width > nMid_maxW || rm.height > nMid_maxH)
			{
				if (!(rm.width > nHead_minH && rm.width < nHead_maxW && rm.height > nHead_minH && rm.height < nHead_maxH))	//�ų���һ�������һ�����ͬ��ͷ
				{
					TRACE("����ͬ��ͷ(%d,%d,%d,%d), Ҫ��ΧW:[%d,%d], H[%d,%d], �ο���С(%d,%d)\n", rm.x, rm.y, rm.width, rm.height, nMid_minW, nMid_maxW, nMid_minH, nMid_maxH, rcSecond.rt.width, rcSecond.rt.height);
					continue;
				}
				else
				{
					TRACE("��βͬ��ͷ(����λ��ͬ��ͷ)(%d,%d,%d,%d)\n", rm.x, rm.y, rm.width, rm.height);
				}
			}
			RectCompList.push_back(rm);
			nYSum += rm.y;
		}
		cvReleaseMemStorage(&storage);
#else
		for (int iteratorIdx = 0; contour != 0; contour = contour->h_next, iteratorIdx++/*���µ�������*/)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			Rect rm = aRect;
			rm.x = rm.x + rt.x;
			rm.y = rm.y + rt.y;
			if (rm.width < 10 || rm.height < 7 || rm.width > 70 || rm.height > 50 || rm.area() < 70)	//********** ��ҪѰ��һ���µķ��������˾���	********
			{
				//				TRACE("���˾���:(%d,%d,%d,%d), ���: %d\n", rm.x, rm.y, rm.width, rm.height, rm.area());
				continue;
			}
			RectCompList.push_back(rm);
		}
#endif
		nResult = RectCompList.size();
	}
	catch (cv::Exception& exc)
	{
		std::string strLog = "ʶ��ͬ��ͷ�쳣: " + exc.msg;
		g_pLogger->information(strLog);
		TRACE(strLog.c_str());
		nResult = -1;
	}
	end = clock();
	TRACE("�����������ʱ��: %d\n", end - start);
	return nResult;
}

cv::Rect GetRectByOrientation1(cv::Rect& rtPic, cv::Rect rt, int nOrientation)
{
	int nW = rtPic.width;
	int nH = rtPic.height;
	cv::Rect rtResult;
	if (nOrientation == 1)	//matSrc����
	{
		rtResult = rt;
	}
	else if (nOrientation == 2)	//matSrc��ת90��
	{
		cv::Point pt1, pt2;
		pt1.x = nH - rt.tl().y;
		pt1.y = rt.tl().x;
		pt2.x = nH - rt.br().y;
		pt2.y = rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 3)	//matSrc��ת90��
	{
		cv::Point pt1, pt2;
		pt1.x = rt.tl().y;
		pt1.y = nW - rt.tl().x;
		pt2.x = rt.br().y;
		pt2.y = nW - rt.br().x;
		rtResult = cv::Rect(pt1, pt2);
	}
	else if (nOrientation == 4)	//matSrc��ת180��
	{
		cv::Point pt1, pt2;
		pt1.x = nW - rt.tl().x;
		pt1.y = nH - rt.tl().y;
		pt2.x = nW - rt.br().x;
		pt2.y = nH - rt.br().y;
		rtResult = cv::Rect(pt1, pt2);
	}
	return rtResult;
}

int CPaperInputDlg::CheckOrientation4Fix(cv::Mat& matSrc, int n)
{
	bool bFind = false;
	int nResult = 1;	//1:���򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180

	if (m_pModel->nHasHead)
		return nResult;
	
	std::string strLog;

	cv::Rect rtModelPic;
	rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
	rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matSrc.cols;
	rtSrcPic.height = matSrc.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matSrc.cols / matSrc.rows;

	if (m_pModel->nZkzhType == 2)			//ʹ�������ʱ����ͨ���������жϷ���
	{
		if (nModelPicPersent == nSrcPicPercent)
		{
			TRACE("��ģ��ͼƬ����һ��\n");
			for (int i = 1; i <= 4; i = i + 3)
			{
				COmrRecog omrRecogObj;
				bool bResult = omrRecogObj.RecogZkzh(n, matSrc, m_pModel, i);
				if (!bResult)
					continue;

				bFind = true;
				nResult = i;
				break;
			}
		}
		else
		{
			TRACE("��ģ��ͼƬ����һ��\n");
			for (int i = 2; i <= 3; i++)
			{
				COmrRecog omrRecogObj;
				bool bResult = omrRecogObj.RecogZkzh(n, matSrc, m_pModel, i);
				if (!bResult)
					continue;

				bFind = true;
				nResult = i;
				break;
			}
		}
		if (bFind)
			return nResult;

		strLog.append("ͨ����������ά���ж��Ծ���ת����ʧ�ܣ�����ͨ����λ���ж�\n");
	}

	int nCount = m_pModel->vecPaperModel[n]->lGray.size() + m_pModel->vecPaperModel[n]->lCourse.size();
	if (nCount == 0)
		return nResult;

	if (nModelPicPersent == nSrcPicPercent)	//��ģ��ͼƬ����һ�£����ж������Ƿ���һ��
	{
		TRACE("��ģ��ͼƬ����һ��\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			//�Ȳ鶨��
			RECTLIST lFix;
			COmrRecog omrRecogObj;
			bool bResult = omrRecogObj.RecogFixCP(n, matSrc, lFix, m_pModel, i);
// 			if (!bResult)
// 				continue;

#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			PicTransfer(0, matSrc, lFix, m_pModel->vecPaperModel[n]->lFix, inverseMat);
#endif

			RECTLIST lModelTmp;
			if (lFix.size() < 3)
			{
				RECTLIST::iterator itFix = lFix.begin();
				for (auto itFix : lFix)
				{
					RECTLIST::iterator itModel = m_pModel->vecPaperModel[n]->lFix.begin();
					for (int j = 0; itModel != m_pModel->vecPaperModel[n]->lFix.end(); j++, itModel++)
					{
						if (j == itFix.nTH)
						{
							RECTINFO rcModel = *itModel;

							cv::Rect rtModelPic;
							rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
							rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
							rcModel.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcModel.rt, i);

							lModelTmp.push_back(rcModel);
							break;
						}
					}
				}
			}

			TRACE("��Ҷ�У���\n");
			bool bContinue = false;
			int nRtCount = 0;
			for (auto rcGray : m_pModel->vecPaperModel[n]->lGray)
			{
				RECTINFO rcItem = rcGray;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
					rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
					rcItem.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
				}
				else
					GetPosition(lFix, m_pModel->vecPaperModel[n]->lFix, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��

				if (omrRecogObj.RecogRtVal(rcItem, matSrc))
				{
					if (rcItem.fRealDensity / rcGray.fStandardDensity > rcGray.fStandardValuePercent && rcItem.fRealValue / rcGray.fStandardValue > rcGray.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("�жϻҶ�У�����ܶȰٷֱ�: %f, ����Ҫ���: %f\n", rcItem.fRealValuePercent, rcGray.fStandardValuePercent);
// 						bContinue = true;
// 						break;
					}
				}
				else
				{
// 					bContinue = true;
// 					break;
				}
			}
			if (bContinue)
				continue;

			TRACE("��ĿУ���\n");
			bContinue = false;
			for (auto rcSubject : m_pModel->vecPaperModel[n]->lCourse)
			{
				RECTINFO rcItem = rcSubject;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
					rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
					rcItem.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
				}
				else
					GetPosition(lFix, m_pModel->vecPaperModel[n]->lFix, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��

				if (omrRecogObj.RecogRtVal(rcItem, matSrc))
				{
					if (rcItem.fRealDensity / rcSubject.fStandardDensity > rcSubject.fStandardValuePercent && rcItem.fRealValue / rcSubject.fStandardValue > rcSubject.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("�жϿ�ĿУ�����ܶȰٷֱ�: %f, ����Ҫ���: %f\n", rcItem.fRealValuePercent, rcSubject.fStandardValuePercent);
// 						bContinue = true;
// 						break;
					}
				}
				else
				{
// 					bContinue = true;
// 					break;
				}
			}
			if (bContinue)
				continue;

			//�ж�����
			int nAllCount = m_pModel->vecPaperModel[n]->lGray.size() + m_pModel->vecPaperModel[n]->lCourse.size();
			if (nAllCount <= 2)
			{
				if (nRtCount >= nAllCount)
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("��У�����=%d, ʵ��ʶ��У�����=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
			else
			{
				if (nRtCount >= (int)(nAllCount * 0.9))
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("��У�����=%d, ʵ��ʶ��У�����=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
		}

		if (!bFind)
		{
			TRACE("�޷��ж�ͼƬ����\n");
			strLog.append("�޷��ж�ͼƬ����\n");
			g_pLogger->information(strLog);
			nResult = 1;
		}
	}
	else	//��ģ��ͼƬ����һ�£����ж�������ת90����������ת90
	{
		TRACE("��ģ��ͼƬ����һ��\n");
		for (int i = 2; i <= 3; i++)
		{
			//�Ȳ鶨��
			RECTLIST lFix;
			COmrRecog omrRecogObj;
			bool bResult = omrRecogObj.RecogFixCP(n, matSrc, lFix, m_pModel, i);
// 			if (!bResult)
// 				continue;
#ifdef WarpAffine_TEST
			cv::Mat	inverseMat(2, 3, CV_32FC1);
			cv::Mat matDst;
			PicTransfer2(0, matSrc, matDst, lFix, m_pModel->vecPaperModel[n]->lFix, inverseMat);
#endif

			RECTLIST lModelTmp;
			if (lFix.size() < 3)
			{
				matDst = matSrc;

				RECTLIST::iterator itFix = lFix.begin();
				for (auto itFix : lFix)
				{
					RECTLIST::iterator itModel = m_pModel->vecPaperModel[n]->lFix.begin();
					for (int j = 0; itModel != m_pModel->vecPaperModel[n]->lFix.end(); j++, itModel++)
					{
						if (j == itFix.nTH)
						{
							RECTINFO rcModel = *itModel;

							cv::Rect rtModelPic;
							rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
							rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
							rcModel.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcModel.rt, i);

							lModelTmp.push_back(rcModel);
							break;
						}
					}
				}
			}

			TRACE("��Ҷ�У���\n");
			bool bContinue = false;
			int nRtCount = 0;
			for (auto rcGray : m_pModel->vecPaperModel[n]->lGray)
			{
				RECTINFO rcItem = rcGray;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
					rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
					rcItem.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcItem.rt, i);
					
					GetPosition(lFix, lModelTmp, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
				}
				else
					GetPosition(lFix, m_pModel->vecPaperModel[n]->lFix, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��

				if (omrRecogObj.RecogRtVal(rcItem, matDst))
				{
					if (rcItem.fRealDensity / rcGray.fStandardDensity > rcGray.fStandardValuePercent && rcItem.fRealValue / rcGray.fStandardValue > rcGray.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("�жϻҶ�У�����ܶȰٷֱ�: %f, ����Ҫ���: %f\n", rcItem.fRealValuePercent, rcGray.fStandardValuePercent);
// 						bContinue = true;
// 						break;
					}
				}
				else
				{
// 					bContinue = true;
// 					break;
				}
			}
			if (bContinue)
				continue;

			TRACE("��ĿУ���\n");
			bContinue = false;
			for (auto rcSubject : m_pModel->vecPaperModel[n]->lCourse)
			{
				RECTINFO rcItem = rcSubject;

				if (lFix.size() < 3)
				{
					cv::Rect rtModelPic;
					rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
					rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
					rcItem.rt = omrRecogObj.GetRectByOrientation(rtModelPic, rcItem.rt, i);

					GetPosition(lFix, lModelTmp, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
				}
				else
					GetPosition(lFix, m_pModel->vecPaperModel[n]->lFix, rcItem.rt);		//����ʵ�ʶ��������ȡ���ε����λ�ã�������Ϊ3��4ʱ��ȡ��ʵ���ϻ���ģ��λ��
			
				if (omrRecogObj.RecogRtVal(rcItem, matDst))
				{
					if (rcItem.fRealDensity / rcSubject.fStandardDensity > rcSubject.fStandardValuePercent && rcItem.fRealValue / rcSubject.fStandardValue > rcSubject.fStandardValuePercent)
					{
						++nRtCount;
					}
					else
					{
						TRACE("�жϿ�ĿУ�����ܶȰٷֱ�: %f, ����Ҫ���: %f\n", rcItem.fRealValuePercent, rcSubject.fStandardValuePercent);
// 						bContinue = true;
// 						break;
					}
				}
				else
				{
// 					bContinue = true;
// 					break;
				}
			}
			if (bContinue)
				continue;

			//�ж�����
			int nAllCount = m_pModel->vecPaperModel[n]->lGray.size() + m_pModel->vecPaperModel[n]->lCourse.size();
			if (nAllCount <= 2)
			{
				if (nRtCount >= nAllCount)
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("��У�����=%d, ʵ��ʶ��У�����=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
			else
			{
				if (nRtCount >= (int)(nAllCount * 0.9))
				{
					bFind = true;
					nResult = i;
					break;
				}
				std::string strTmpLog = Poco::format("��У�����=%d, ʵ��ʶ��У�����=%d\n", nAllCount, nRtCount);
				strLog.append(strTmpLog);
			}
		}

		if (!bFind)
		{
			TRACE("�޷��ж�ͼƬ���򣬲���Ĭ������90�ȵķ���\n");
			strLog.append("�޷��ж�ͼƬ���򣬲���Ĭ������90�ȵķ���\n");
			g_pLogger->information(strLog);
			nResult = 2;	//��������޷��ж�ͼ����ʱ��Ĭ��ģ����Ҫ����90�ȱ�ɴ�ͼ���򣬼�Ĭ�Ϸ��ط���Ϊ����90�ȣ���Ϊ����ֻ������90��������90������ѡ�񣬴˴�������Ĭ�ϵ�1������2
		}
	}
	return nResult;
}

int CPaperInputDlg::CheckOrientation4Head(cv::Mat& matSrc, int n)
{
	bool bFind = false;
	int nResult = 1;	//1:���򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180

	if (!m_pModel->nHasHead)
		return nResult;

	const float fMinPer = 0.5;		//ʶ�������/ģ������� ������Сֵ����Ϊ���ϸ�
	const float fMaxPer = 1.5;		//ʶ�������/ģ������� �������ֵ����Ϊ���ϸ�
	const float fMidPer = 0.8;

	cv::Rect rtModelPic;
	rtModelPic.width = m_pModel->vecPaperModel[n]->nPicW;
	rtModelPic.height = m_pModel->vecPaperModel[n]->nPicH;
	cv::Rect rtSrcPic;
	rtSrcPic.width = matSrc.cols;
	rtSrcPic.height = matSrc.rows;

	int nModelPicPersent = rtModelPic.width / rtModelPic.height;	//0||1
	int nSrcPicPercent = matSrc.cols / matSrc.rows;

	cv::Rect rt1 = m_pModel->vecPaperModel[n]->rtHTracker;
	cv::Rect rt2 = m_pModel->vecPaperModel[n]->rtVTracker;
	TRACE("ˮƽ��Ƥ��:(%d,%d,%d,%d), ��ֱ��Ƥ��(%d,%d,%d,%d)\n", rt1.x, rt1.y, rt1.width, rt1.height, rt2.x, rt2.y, rt2.width, rt2.height);

	float fFirst_H, fFirst_V, fSecond_H, fSecond_V;
	fFirst_H = fFirst_V = fSecond_H = fSecond_V = 0.0;
	if (nModelPicPersent == nSrcPicPercent)	//��ģ��ͼƬ����һ�£����ж������Ƿ���һ��
	{
		TRACE("��ģ��ͼƬ����һ��\n");
		for (int i = 1; i <= 4; i = i + 3)
		{
			TRACE("��ˮƽͬ��ͷ\n");
			cv::Rect rtH = GetRectByOrientation1(rtModelPic, m_pModel->vecPaperModel[n]->rtHTracker, i);
			int nHead_H = GetRects1(matSrc, rtH, m_pModel, n, i, 1);		//��ˮƽͬ��ͷ����
			int nSum_H = m_pModel->vecPaperModel[n]->lH_Head.size();

			float fSimilarity_H = (float)nHead_H / nSum_H;
			if (fSimilarity_H < fMinPer || fSimilarity_H > fMaxPer)
				continue;

			if (i == 1)
				fFirst_H = fSimilarity_H;
			else
				fSecond_H = fSimilarity_H;

			TRACE("�鴹ֱͬ��ͷ\n");
			cv::Rect rtH2 = GetRectByOrientation1(rtModelPic, m_pModel->vecPaperModel[n]->rtVTracker, i);
			int nHead_V = GetRects1(matSrc, rtH2, m_pModel, n, i, 2);		//�鴹ֱͬ��ͷ����
			int nSum_V = m_pModel->vecPaperModel[n]->lV_Head.size();

			float fSimilarity_V = (float)nHead_V / nSum_V;

			char szLog[300] = { 0 };
			sprintf_s(szLog, "rtH = (%d,%d,%d,%d), rtH2 = (%d,%d,%d,%d),\nnHead_H = %d, nHead_V = %d, nSum_H = %d, nSum_V = %d, H=%.2f, V=%.2f\n", rtH.tl().x, rtH.tl().y, rtH.width, rtH.height, rtH2.tl().x, rtH2.tl().y, rtH2.width, rtH2.height, \
					  nHead_H, nHead_V, nSum_H, nSum_V, fSimilarity_H, fSimilarity_V);
			g_pLogger->information(szLog);
			TRACE(szLog);

			if (fSimilarity_H > fMidPer)
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (fSimilarity_V > fMidPer)
				{
					bFind = true;
					nResult = i;
					break;
				}
				else    //fSimilarity_V in [0.5,0.8]	�п��ܣ��ٽ��н�һ���ж�
				{
					if (i == 1)
						fFirst_V = fSimilarity_V;
					else
						fSecond_V = fSimilarity_V;
				}
			}
			else	//fSimilarity_H in [0.5,0.8]	�п��ܣ��ٽ��н�һ���ж�
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (i == 1)
					fFirst_V = fSimilarity_V;
				else
					fSecond_V = fSimilarity_V;
			}
		}

		if (!bFind)
		{
			if (fFirst_H > fSecond_H && fFirst_V > fSecond_V)
			{
				nResult = 1;
			}
			else if (fFirst_H < fSecond_H && fFirst_V < fSecond_V)
			{
				nResult = 4;
			}
			else
			{
				TRACE("�޷��ж�ͼƬ����\n");
				g_pLogger->information("�޷��ж�ͼƬ����");
				nResult = 1;
			}
		}
	}
	else	//��ģ��ͼƬ����һ�£����ж�������ת90����������ת90
	{
		TRACE("��ģ��ͼƬ����һ��\n");
		for (int i = 2; i <= 3; i++)
		{
			TRACE("��ˮƽͬ��ͷ\n");
			cv::Rect rtH = GetRectByOrientation1(rtModelPic, m_pModel->vecPaperModel[n]->rtHTracker, i);
			int nHead_H = GetRects1(matSrc, rtH, m_pModel, n, i, 1);		//��ˮƽͬ��ͷ����
			int nSum_H = m_pModel->vecPaperModel[n]->lH_Head.size();

			float fSimilarity_H = (float)nHead_H / nSum_H;
			if (fSimilarity_H < fMinPer || fSimilarity_H > fMaxPer)
				continue;

			if (i == 2)
				fFirst_H = fSimilarity_H;
			else
				fSecond_H = fSimilarity_H;

			TRACE("�鴹ֱͬ��ͷ\n");
			cv::Rect rtH2 = GetRectByOrientation1(rtModelPic, m_pModel->vecPaperModel[n]->rtVTracker, i);
			int nHead_V = GetRects1(matSrc, rtH2, m_pModel, n, i, 2);		//�鴹ֱͬ��ͷ����
			int nSum_V = m_pModel->vecPaperModel[n]->lV_Head.size();

			float fSimilarity_V = (float)nHead_V / nSum_V;

			char szLog[300] = { 0 };
			sprintf_s(szLog, "rtH = (%d,%d,%d,%d), rtH2 = (%d,%d,%d,%d),\nnHead_H = %d, nHead_V = %d, nSum_H = %d, nSum_V = %d, H=%.2f, V=%.2f\n", rtH.tl().x, rtH.tl().y, rtH.width, rtH.height, rtH2.tl().x, rtH2.tl().y, rtH2.width, rtH2.height, \
					  nHead_H, nHead_V, nSum_H, nSum_V, fSimilarity_H, fSimilarity_V);
			g_pLogger->information(szLog);
			TRACE(szLog);

			if (fSimilarity_H > fMidPer)
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (fSimilarity_V > fMidPer)
				{
					bFind = true;
					nResult = i;
					break;
				}
				else    //fSimilarity_V in [0.5,0.8]	�п��ܣ��ٽ��н�һ���ж�
				{
					if (i == 2)
						fFirst_V = fSimilarity_V;
					else
						fSecond_V = fSimilarity_V;
				}
			}
			else	//fSimilarity_H in [0.5,0.8]	�п��ܣ��ٽ��н�һ���ж�
			{
				if (fSimilarity_V < fMinPer || fSimilarity_V > fMaxPer)
					continue;

				if (i == 2)
					fFirst_V = fSimilarity_V;
				else
					fSecond_V = fSimilarity_V;
			}
		}

		if (!bFind)
		{
			if (fFirst_H > fSecond_H && fFirst_V > fSecond_V)
			{
				nResult = 2;
			}
			else if (fFirst_H < fSecond_H && fFirst_V < fSecond_V)
			{
				nResult = 3;
			}
			else
			{
				TRACE("�޷��ж�ͼƬ����\n");
				g_pLogger->information("�޷��ж�ͼƬ����");
				nResult = 1;
			}
		}
	}
	return nResult;
}

int CPaperInputDlg::CheckOrientation(cv::Mat& matSrc, int n, bool bDoubleScan)
{
	clock_t start, end;
	start = clock();

	//*********************************
	//*********	���Խ��� **************
	//ǰ�᣺˫��ɨ��
	//1�����治��Ҫ��ת ==> ����Ҳ����Ҫ��ת
	//2��������Ҫ��ת90�� ==> ������Ҫ��ת90��
	//3��������Ҫ��ת90�� ==> ������Ҫ��ת90��
	//4��������Ҫ��ת180�� ==> ����Ҳ��Ҫ��ת180��
	//*********************************
	int nResult = 1;	//1:���򣬲���Ҫ��ת��2����ת90, 3����ת90, 4����ת180
	static int nFristOrientation = 1;
	if (bDoubleScan && n % 2 != 0)	//˫��ɨ��, ������˫��ɨ��ĵڶ�������
	{
		if (nFristOrientation == 1) nResult = 1;
		else if (nFristOrientation == 2) nResult = 3;
		else if (nFristOrientation == 3) nResult = 2;
		else if (nFristOrientation == 4) nResult = 4;
		end = clock();
		TRACE("�ж���ת����ʱ��: %dms\n", end - start);

		std::string strDirection;
		switch (nResult)
		{
			case 1: strDirection = "���򣬲���Ҫ��ת"; break;
			case 2: strDirection = "����90"; break;
			case 3: strDirection = "����90"; break;
			case 4: strDirection = "����180"; break;
		}
		std::string strLog = "˫��ɨ��ڶ��棬���ݵ�һ�淽���жϽ����" + strDirection;
		g_pLogger->information(strLog);
		TRACE("%s\n", strLog.c_str());
		return nResult;
	}

	cv::Mat matCom = matSrc.clone();
	if (m_pModel->nHasHead)
		nResult = CheckOrientation4Head(matCom, n);
	else
		nResult = CheckOrientation4Fix(matCom, n);

	if (bDoubleScan && n % 2 == 0)		//˫��ɨ�裬������ɨ��ĵ�һ��
		nFristOrientation = nResult;

	end = clock();
	TRACE("�ж���ת����ʱ��: %dms\n", end - start);

	std::string strDirection;
	switch (nResult)
	{
		case 1: strDirection = "���򣬲���Ҫ��ת"; break;
		case 2: strDirection = "����90"; break;
		case 3: strDirection = "����90"; break;
		case 4: strDirection = "����180"; break;
	}
	std::string strLog = "�����жϽ����" + strDirection;
	g_pLogger->information(strLog);
	TRACE("%s\n", strLog.c_str());

	return nResult;
}


void CPaperInputDlg::OnBnClickedBtnTest()
{
	std::string strStatisticsLog;
	int nModelOmrCount = 0;
	for (int k = 0; k < m_pModel->vecPaperModel.size(); k++)
	{
		nModelOmrCount += m_pModel->vecPaperModel[k]->lOMR2.size();
	}

	PAPERS_LIST::iterator itPapers = g_lPapers.begin();
	for (; itPapers != g_lPapers.end(); itPapers++)
	{
		int nPapersCount = (*itPapers)->lPaper.size() + (*itPapers)->lIssue.size();
		int nOmrCount = nModelOmrCount * nPapersCount;
		if (nPapersCount == 0)
			continue;

		char szStatisticsInfo[300] = { 0 };
		sprintf_s(szStatisticsInfo, "\n%sͳ����Ϣ: omrDoubt = %.2f(%d/%d), omrNull = %.2f(%d/%d), zkzhNull = %.2f(%d/%d)\n", (*itPapers)->strPapersName.c_str(), (float)(*itPapers)->nOmrDoubt / nOmrCount, (*itPapers)->nOmrDoubt, nOmrCount, \
				  (float)(*itPapers)->nOmrNull / nOmrCount, (*itPapers)->nOmrNull, nOmrCount, \
				  (float)(*itPapers)->nSnNull / nPapersCount, (*itPapers)->nSnNull, nPapersCount);
		strStatisticsLog.append(szStatisticsInfo);
	}
	g_pLogger->information(strStatisticsLog);
}

void CPaperInputDlg::ShowPaperByItem(int nItem)
{
	if (nItem < 0)
		return;

	pST_PaperInfo pPaper = (pST_PaperInfo)m_lPaperCtrl.GetItemData(nItem);

	m_pCurrentShowPaper = pPaper;
	if (pPaper->bIssuePaper)
		PaintIssueRect(pPaper);
	else
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


void CPaperInputDlg::ShowPapers(pPAPERSINFO pPapers)
{
	//��ʾ����ʶ����ɵ�׼��֤��
	USES_CONVERSION;
	//������ʾ�ϸ��Ծ�
	m_lPaperCtrl.DeleteAllItems();
	for (auto pPaper : pPapers->lPaper)
	{
		int nCount = m_lPaperCtrl.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lPaperCtrl.InsertItem(nCount, NULL);
		m_lPaperCtrl.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lPaperCtrl.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
		m_lPaperCtrl.SetItemText(nCount, 2, (LPCTSTR)A2T(pPaper->strSN.c_str()));
		if (pPaper->bModifyZKZH)
			m_lPaperCtrl.SetItemText(nCount, 3, _T("*"));
		m_lPaperCtrl.SetItemData(nCount, (DWORD_PTR)pPaper);
	}
	//��ʾ��Ҫ��ɨ���쳣�Ծ�
	m_lIssuePaperCtrl.DeleteAllItems();
	for (auto pPaper : pPapers->lIssue)
	{
		int nCount = m_lIssuePaperCtrl.GetItemCount();
		char szCount[10] = { 0 };
		sprintf_s(szCount, "%d", nCount + 1);
		m_lIssuePaperCtrl.InsertItem(nCount, NULL);
		m_lIssuePaperCtrl.SetItemText(nCount, 0, (LPCTSTR)A2T(szCount));
		m_lIssuePaperCtrl.SetItemText(nCount, 1, (LPCTSTR)A2T(pPaper->strStudentInfo.c_str()));
		CString strErrInfo = _T("");
		if (pPaper->strSN.empty())	strErrInfo = _T("����Ϊ��");
		if (pPaper->bReScan)	strErrInfo = _T("��ɨ");
		m_lIssuePaperCtrl.SetItemText(nCount, 2, (LPCTSTR)strErrInfo);
		m_lIssuePaperCtrl.SetItemData(nCount, (DWORD_PTR)pPaper);

		CString strTips = _T("�쳣�Ծ��������ϴ���Ҳ����������� ��Ҫ�����ҳ������浥��ɨ��");
		m_lIssuePaperCtrl.SetItemToolTipText(nCount, 0, strTips);
		m_lIssuePaperCtrl.SetItemToolTipText(nCount, 1, strTips);
		m_lIssuePaperCtrl.SetItemToolTipText(nCount, 2, strTips);
	}
}

void CPaperInputDlg::OnLvnKeydownListPaper(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	*pResult = 0;

	if (pLVKeyDow->wVKey == VK_UP)
	{
		m_nCurrItemPaperList--;
		if (m_nCurrItemPaperList <= 0)
			m_nCurrItemPaperList = 0;

		ShowPaperByItem(m_nCurrItemPaperList);
	}
	else if (pLVKeyDow->wVKey == VK_DOWN)
	{

		m_nCurrItemPaperList++;
		if (m_nCurrItemPaperList >= m_lPaperCtrl.GetItemCount() - 1)
			m_nCurrItemPaperList = m_lPaperCtrl.GetItemCount() - 1;

		ShowPaperByItem(m_nCurrItemPaperList);
	}
}


void CPaperInputDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_CheckRecogComplete)
	{
		bool bRecogComplete = true;
		bool bNeedShowZkzhDlg = false;
		for (auto p : m_pCurrentPapers->lPaper)
		{
			if (!p->bRecogComplete)
			{
				bRecogComplete = false;
				break;
			}
			if (p->strSN.empty())
				bNeedShowZkzhDlg = true;
		}
		if (bRecogComplete)
		{
			USES_CONVERSION;
			CScanToolDlg* pDlg = (CScanToolDlg*)GetParent();
			if ((g_nOperatingMode == 1 || pDlg->m_bModifySN) && bNeedShowZkzhDlg)
			{
				KillTimer(TIMER_CheckRecogComplete);
				if (!m_pStudentMgr)
				{
					USES_CONVERSION;
					m_pStudentMgr = new CStudentMgr();
					std::string strDbPath = T2A(g_strCurrentPath + _T("bmk.db"));
					bool bResult = m_pStudentMgr->InitDB(CMyCodeConvert::Gb2312ToUtf8(strDbPath));
				}
				CModifyZkzhDlg zkzhDlg(m_pModel, m_pCurrentPapers, m_pStudentMgr);
				zkzhDlg.DoModal();

				ShowPapers(m_pCurrentPapers);
			}
			else
				KillTimer(TIMER_CheckRecogComplete);
		}
	}
	CDialog::OnTimer(nIDEvent);
}

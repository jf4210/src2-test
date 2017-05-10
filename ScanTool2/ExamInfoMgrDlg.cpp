// ExamInfoMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool2.h"
#include "ExamInfoMgrDlg.h"
#include "afxdialogex.h"


// CExamInfoMgrDlg 对话框

IMPLEMENT_DYNAMIC(CExamInfoMgrDlg, CDialog)

CExamInfoMgrDlg::CExamInfoMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExamInfoMgrDlg::IDD, pParent)
	, m_nMaxShowExamListItem(0), m_nAllExamListItems(0), m_nCurrStartShowExamListItem(0)
{

}

CExamInfoMgrDlg::~CExamInfoMgrDlg()
{
}

void CExamInfoMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_Subject, m_comboSubject);
	DDX_Control(pDX, IDC_COMBO_Grade, m_comboGrade);
}


BOOL CExamInfoMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitCtrlPosition();
	InitShowData();

	return TRUE;
}

BOOL CExamInfoMgrDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CExamInfoMgrDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_Subject, &CExamInfoMgrDlg::OnCbnSelchangeComboSubject)
	ON_CBN_SELCHANGE(IDC_COMBO_Grade, &CExamInfoMgrDlg::OnCbnSelchangeComboGrade)
	ON_BN_CLICKED(IDC_BTN_First, &CExamInfoMgrDlg::OnBnClickedBtnFirst)
	ON_BN_CLICKED(IDC_BTN_Last, &CExamInfoMgrDlg::OnBnClickedBtnLast)
	ON_BN_CLICKED(IDC_BTN_Up, &CExamInfoMgrDlg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BTN_Down, &CExamInfoMgrDlg::OnBnClickedBtnDown)
END_MESSAGE_MAP()


// CExamInfoMgrDlg 消息处理程序
void CExamInfoMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 50;	//上边的间隔
	const int nBottomGap = 30;	//下边的间隔
	const int nLeftGap = 20;		//左边的空白间隔
	const int nRightGap = 20;	//右边的空白间隔
	int nGap = 5;

	m_rtExamList.left = nLeftGap;
	m_rtExamList.top = nTopGap;
	m_rtExamList.right = cx - nRightGap;
	m_rtExamList.bottom = cy - nBottomGap;

	int nStaticW = 40;
	int nCtrlH = 30;

	int nCurrLeft = nLeftGap;
	int nCurrTop = 5;
	if (GetDlgItem(IDC_STATIC_DlgMgr_Subject)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_DlgMgr_Subject)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nCtrlH);
		nCurrLeft += (nStaticW + nGap);
	}
	if (GetDlgItem(IDC_COMBO_Subject)->GetSafeHwnd())
	{
		GetDlgItem(IDC_COMBO_Subject)->MoveWindow(nCurrLeft, nCurrTop, nStaticW * 2, nCtrlH);
		nCurrLeft += (nStaticW * 2 + nGap * 3);
	}
	if (GetDlgItem(IDC_STATIC_DlgMgr_Grade)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_DlgMgr_Grade)->MoveWindow(nCurrLeft, nCurrTop, nStaticW, nCtrlH);
		nCurrLeft += (nStaticW + nGap);
	}
	if (GetDlgItem(IDC_COMBO_Grade)->GetSafeHwnd())
	{
		GetDlgItem(IDC_COMBO_Grade)->MoveWindow(nCurrLeft, nCurrTop, nStaticW * 2, nCtrlH);
		nCurrLeft += (nStaticW * 2 + nGap * 3);
	}

	//bottom

	nGap = 2;
	int nBtnW = 30;
	int nBtnH = nBottomGap - nGap - nGap;
	nCurrLeft = m_rtExamList.left + m_rtExamList.Width() / 2 - (nBtnW + nGap) * 2;
	nCurrTop = cy - nBottomGap - nGap;
	if (GetDlgItem(IDC_BTN_First)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_First)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (GetDlgItem(IDC_BTN_Up)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Up)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}

	if (GetDlgItem(IDC_BTN_Down)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Down)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
	if (GetDlgItem(IDC_BTN_Last)->GetSafeHwnd())
	{
		GetDlgItem(IDC_BTN_Last)->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		nCurrLeft += (nBtnW + nGap);
	}
}

void CExamInfoMgrDlg::InitSearchData()
{
	std::vector<std::string> vecSub;
	vecSub.push_back("全部");
	std::vector<std::string> vecGrade;
	vecGrade.push_back("全部");

	//获取列表中所有科目名称信息、年级信息
	int nAllExamItems = 0;
	for (auto examObj : g_lExamList)
	{
		pEXAMINFO pExam = examObj;
		for (auto subObj : examObj->lSubjects)
		{
			pEXAM_SUBJECT pSub = subObj;
			bool bFind = false;
			for (auto strSubName : vecSub)
			{
				if (pSub->strSubjName == strSubName)
				{
					bFind = true;
					break;
				}
			}
			if (!bFind && pSub->strSubjName != "")
				vecSub.push_back(pSub->strSubjName);
		}

		bool bFindGrade = false;
		for (auto strGrade : vecGrade)
		{
			if (pExam->strGradeName == strGrade)
			{
				bFindGrade = true;
				break;
			}
		}
		if (!bFindGrade && pExam->strGradeName != "")
			vecGrade.push_back(pExam->strGradeName);

		nAllExamItems = pExam->lSubjects.size();
	}
	m_nAllExamListItems = nAllExamItems;

	m_comboSubject.ResetContent();
	m_comboGrade.ResetContent();

	USES_CONVERSION;
	for (auto strSubName : vecSub)
		m_comboSubject.AddString(A2T(strSubName.c_str()));
	for (auto strGrade : vecGrade)
		m_comboGrade.AddString(A2T(strGrade.c_str()));

	m_comboSubject.AdjustDroppedWidth();
	m_comboGrade.AdjustDroppedWidth();

	m_comboSubject.SetCurSel(0);
	m_comboGrade.SetCurSel(0);
}

void CExamInfoMgrDlg::InitShowData()
{
	InitSearchData();
	GetSearchResultExamList();
	ShowExamList(m_lExamList, 1);
}

void CExamInfoMgrDlg::ReleaseData()
{
	EXAM_LIST::iterator itExam = m_lExamList.begin();
	for (; itExam != m_lExamList.end();)
	{
		pEXAMINFO pExam = *itExam;
		itExam = m_lExamList.erase(itExam);
		SAFE_RELEASE(pExam);
	}
}

void CExamInfoMgrDlg::GetSearchResultExamList()
{
	//++科目过滤
	USES_CONVERSION;
	CString strCurSub = _T("");
	CString strCurGrade = _T("");
	m_comboSubject.GetLBText(m_comboSubject.GetCurSel(), strCurSub);
	m_comboGrade.GetLBText(m_comboGrade.GetCurSel(), strCurGrade);
	std::string strCurrSubject = T2A(strCurSub);
	std::string strCurrGrade = T2A(strCurGrade);
	//--
	
	EXAM_LIST::iterator itExam = m_lExamList.begin();
	for (; itExam != m_lExamList.end();)
	{
		pEXAMINFO pExam = *itExam;
		itExam = m_lExamList.erase(itExam);
		SAFE_RELEASE(pExam);
	}

	for (auto examObj : g_lExamList)
	{
		pEXAMINFO pExam = examObj;
		if (pExam->strGradeName == strCurrGrade || strCurrGrade == "全部")
		{
			pEXAMINFO pShowExam = NULL;
			bool bFind = false;
			for (auto subObj : examObj->lSubjects)
			{
				pEXAM_SUBJECT pSub = subObj;
				if (pSub->strSubjName == strCurrSubject || strCurrSubject == "全部")
				{
					if (!pShowExam)
					{
						pShowExam = new EXAMINFO();
						pShowExam->nExamID		= pExam->nExamID;
						pShowExam->nExamGrade	= pExam->nExamGrade;
						pShowExam->nExamState	= pExam->nExamState;
						pShowExam->strExamID	= pExam->strExamID;
						pShowExam->strExamName	= pExam->strExamName;
						pShowExam->strExamTypeName = pExam->strExamTypeName;
						pShowExam->strGradeName = pExam->strGradeName;
					}
					pEXAM_SUBJECT pShowSub = new EXAM_SUBJECT();
//					memcpy(pShowSub, pSub, sizeof(EXAM_SUBJECT));
					pShowSub->nSubjID = pSub->nSubjID;
					pShowSub->strModelName = pSub->strModelName;
					pShowSub->strSubjName = pSub->strSubjName;
					pShowExam->lSubjects.push_back(pShowSub);
				}
			}
			if (pShowExam)
				m_lExamList.push_back(pShowExam);
		}
	}
}

void CExamInfoMgrDlg::ShowExamList(EXAM_LIST lExam, int nStartShow)
{
	if (nStartShow > m_nAllExamListItems) return;
	if (nStartShow <= 0) return;

	ReleaseDlgData();
	if (lExam.size() <= 0) return;

// 	//++科目过滤
// 	USES_CONVERSION;
// 	CString strCurSub = _T("");
// 	CString strCurGrade = _T("");
// 	m_comboSubject.GetLBText(m_comboSubject.GetCurSel(), strCurSub);
// 	m_comboGrade.GetLBText(m_comboGrade.GetCurSel(), strCurGrade);
// 	std::string strCurrSubject = T2A(strCurSub);
// 	std::string strCurrGrade = T2A(strCurGrade);
// 	//--

	int nGap = 2;
	int nExamDlg_H = 45;				//考试信息列表的高度
	int nRealW = m_rtExamList.Width();	//考试信息列表的宽度
	int nRealH = m_rtExamList.Height();	//实际有效的显示考试列表窗口的高度
	int nMaxShow = nRealH / (nExamDlg_H + nGap);		//当前窗口最大可以显示的窗口列表的数量
	m_nMaxShowExamListItem = nMaxShow;
	m_nCurrStartShowExamListItem = nStartShow;

	int nCurrLeft = m_rtExamList.left;
	int nCurrTop = m_rtExamList.top;

	int nAllExamItems = 0;
	int nCount = 0;
	for (auto examObj : lExam)
	{
		pEXAMINFO pExam = examObj;

		for (auto subObj : examObj->lSubjects)
		{
			pEXAM_SUBJECT pSub = subObj;
			nCount++;
			if (nCount >= nStartShow && nCount < nStartShow + nMaxShow)
			{
				CExamInfoDlg* pExamDlg = new CExamInfoDlg(this);
				pExamDlg->Create(CExamInfoDlg::IDD, this);
				pExamDlg->ShowWindow(SW_HIDE);

				pExamDlg->MoveWindow(nCurrLeft, nCurrTop, nRealW, nExamDlg_H);
				nCurrTop += (nExamDlg_H + nGap);


				pExamDlg->SetExamInfo(pExam, pSub);
				//**********************	设置上传数量	*******************************
				pExamDlg->SetUploadPapers(-1);
				pExamDlg->ShowWindow(SW_SHOW);
				m_vecExamInfoDlg.push_back(pExamDlg);
			}
		}
		nAllExamItems += pExam->lSubjects.size();
	}
	m_nAllExamListItems = nAllExamItems;
}


void CExamInfoMgrDlg::ReleaseDlgData()
{
	int nCoutn = m_vecExamInfoDlg.size();
	for (int i = 0; i < m_vecExamInfoDlg.size(); i++)
	{
		CExamInfoDlg* pDlg = m_vecExamInfoDlg[i];
		pDlg->DestroyWindow();
		SAFE_RELEASE(pDlg);
	}
	m_vecExamInfoDlg.clear();
}

void CExamInfoMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
	ShowExamList(m_lExamList, 1);
}


BOOL CExamInfoMgrDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(225, 222, 250)));

	return CDialog::OnEraseBkgnd(pDC);
}


void CExamInfoMgrDlg::OnDestroy()
{
	CDialog::OnDestroy();

	ReleaseData();
	ReleaseDlgData();
}


void CExamInfoMgrDlg::OnCbnSelchangeComboSubject()
{
	if (m_comboSubject.GetCurSel() < 0)
		return;
	GetSearchResultExamList();
	ShowExamList(m_lExamList, 1);
}


void CExamInfoMgrDlg::OnCbnSelchangeComboGrade()
{
	if (m_comboGrade.GetCurSel() < 0)
		return;

	GetSearchResultExamList();
	ShowExamList(m_lExamList, 1);
}


void CExamInfoMgrDlg::OnBnClickedBtnFirst()
{
	if (m_lExamList.size() == 0)
		GetSearchResultExamList();

	ShowExamList(m_lExamList, 1);
}


void CExamInfoMgrDlg::OnBnClickedBtnLast()
{
	if (m_lExamList.size() == 0)
		GetSearchResultExamList();

	int nMaxDlgs = ceil((double)m_nAllExamListItems / (double)m_nMaxShowExamListItem);
	ShowExamList(m_lExamList, (nMaxDlgs - 1) * m_nMaxShowExamListItem + 1);
}


void CExamInfoMgrDlg::OnBnClickedBtnUp()
{
	int nCurrStartShowExamListItem = m_nCurrStartShowExamListItem - m_nMaxShowExamListItem;
	ShowExamList(m_lExamList, nCurrStartShowExamListItem);
}


void CExamInfoMgrDlg::OnBnClickedBtnDown()
{
	int nCurrStartShowExamListItem = m_nCurrStartShowExamListItem + m_nMaxShowExamListItem;
	ShowExamList(m_lExamList, nCurrStartShowExamListItem);
}


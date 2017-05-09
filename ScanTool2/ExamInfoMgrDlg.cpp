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
	InitData();
	ShowExamList(g_lExamList, 1);

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
END_MESSAGE_MAP()


// CExamInfoMgrDlg 消息处理程序
void CExamInfoMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 50;	//上边的间隔
	const int nBottomGap = 20;	//下边的间隔
	const int nLeftGap = 2;		//左边的空白间隔
	const int nRightGap = 2;	//右边的空白间隔
	const int nGap = 5;

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
}

void CExamInfoMgrDlg::InitData()
{
	std::vector<std::string> vecSub;
	vecSub.push_back("全部");
	std::vector<std::string> vecGrade;
	vecGrade.push_back("全部");

	//获取列表中所有科目名称信息、年级信息
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
	}

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

void CExamInfoMgrDlg::ShowExamList(EXAM_LIST lExam, int nStartShow)
{
	if (lExam.size() <= 0) return;

	ReleaseDlgData();

	//++科目过滤
	USES_CONVERSION;
	CString strCurSub = _T("");
	CString strCurGrade = _T("");
	m_comboSubject.GetLBText(m_comboSubject.GetCurSel(), strCurSub);
	m_comboGrade.GetLBText(m_comboGrade.GetCurSel(), strCurGrade);
	std::string strCurrSubject = T2A(strCurSub);
	std::string strCurrGrade = T2A(strCurGrade);
	//--

	int nGap = 2;
	int nExamDlg_H = 35;					//考试信息列表的高度
	int nRealW = m_rtExamList.Width();	//考试信息列表的宽度
	int nRealH = m_rtExamList.Height();	//实际有效的显示考试列表窗口的高度
	int nMaxShow = nRealH / (nExamDlg_H + nGap);		//当前窗口最大可以显示的窗口列表的数量

	int nCurrLeft = m_rtExamList.left;
	int nCurrTop = m_rtExamList.top;

	int nCount = 0;
	for (auto examObj : lExam)
	{
		pEXAMINFO pExam = examObj;
		if (pExam->strGradeName == strCurrGrade || strCurrGrade == "全部")
		{
			for (auto subObj : examObj->lSubjects)
			{
				pEXAM_SUBJECT pSub = subObj;
				if (pSub->strSubjName == strCurrSubject || strCurrSubject == "全部")
				{
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
			}
		}		
	}
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
	ShowExamList(g_lExamList, 1);
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

	ReleaseDlgData();
}


void CExamInfoMgrDlg::OnCbnSelchangeComboSubject()
{
	if (m_comboSubject.GetCurSel() < 0)
		return;
	ShowExamList(g_lExamList, 1);
}


void CExamInfoMgrDlg::OnCbnSelchangeComboGrade()
{
	if (m_comboGrade.GetCurSel() < 0)
		return;
	ShowExamList(g_lExamList, 1);
}

// ExamInfoMgrDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ExamInfoMgrDlg.h"
#include "afxdialogex.h"


// CExamInfoMgrDlg �Ի���

IMPLEMENT_DYNAMIC(CExamInfoMgrDlg, CDialog)

CExamInfoMgrDlg::CExamInfoMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExamInfoMgrDlg::IDD, pParent)
	, m_nMaxShowExamListItem(0), m_nAllExamListItems(0), m_nShowPapersCount(0), m_nCurrShowPaper(1), m_nMaxSubsRow(2), m_nSubjectBtnH(30), m_nDlgMinH(100), m_strShowCurrPaper(_T(""))
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
	DDX_Text(pDX, IDC_STATIC_PaperCount, m_strShowCurrPaper);
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


// CExamInfoMgrDlg ��Ϣ��������
void CExamInfoMgrDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 50;	//�ϱߵļ��
	const int nBottomGap = 30;	//�±ߵļ��
	const int nLeftGap = 20;		//��ߵĿհ׼��
	const int nRightGap = 20;	//�ұߵĿհ׼��
	int nGap = 5;

	m_rtExamList.left = nLeftGap;
	m_rtExamList.top = nTopGap;
	m_rtExamList.right = cx - nRightGap;
	m_rtExamList.bottom = nTopGap + cy - nBottomGap;

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
	//����ҳ������
	int nSysLinkCount = m_vecBtnIndex.size();
	int nSysLinkW = 10;


	nGap = 2;
	int nBtnW = 30;
	int nBtnH = nBottomGap - nGap - nGap;
//	nCurrLeft = m_rtExamList.left + m_rtExamList.Width() / 2 - (nBtnW + nGap) * 2;
	nCurrLeft = m_rtExamList.left + m_rtExamList.Width() / 2 - (nBtnW + nGap) * 2 - (nSysLinkW + nGap) * nSysLinkCount / 2;
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
	for (int i = 0; i < m_vecBtnIndex.size(); i++)
	{
		if (m_vecBtnIndex[i] && m_vecBtnIndex[i]->GetSafeHwnd())
		{
			m_vecBtnIndex[i]->MoveWindow(nCurrLeft, nCurrTop, nSysLinkW, nBtnH);
			nCurrLeft += (nSysLinkW + nGap);
		}
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
	if (GetDlgItem(IDC_STATIC_PaperCount)->GetSafeHwnd())
	{
		int nW = nBtnW * 3;
		GetDlgItem(IDC_STATIC_PaperCount)->MoveWindow(nCurrLeft, nCurrTop, nW, nBtnH);
		nCurrLeft += (nW + nGap);
	}
//	Invalidate();
}

void CExamInfoMgrDlg::InitSearchData()
{
	std::vector<std::string> vecSub;
	vecSub.push_back("ȫ��");
	std::vector<std::string> vecGrade;
	vecGrade.push_back("ȫ��");

	//��ȡ�б������п�Ŀ������Ϣ���꼶��Ϣ
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
	m_nCurrShowPaper = 1;
	ShowExamList(m_lExamList, m_nCurrShowPaper);
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
	//++��Ŀ����
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
		if (pExam->strGradeName == strCurrGrade || strCurrGrade == "ȫ��")
		{
			pEXAMINFO pShowExam = NULL;
			bool bFind = false;
			for (auto subObj : examObj->lSubjects)
			{
				pEXAM_SUBJECT pSub = subObj;
				if (pSub->strSubjName == strCurrSubject || strCurrSubject == "ȫ��")
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

	GetAllShowPaperCount();
}

void CExamInfoMgrDlg::GetAllShowPaperCount()
{
	int nGap = 2;
	int nExamDlg_H = 45;				//������Ϣ�б��ĸ߶�
	int nShowCount = 1;
	int nTmpTop = m_rtExamList.top;
	for (auto examObj : m_lExamList)
	{
		pEXAMINFO pExam = examObj;

		int nSubs = pExam->lSubjects.size();
		int nMaxCountInH = ceil((float)nSubs / (float)m_nMaxSubsRow);	//����м��а�ť
		nExamDlg_H = nMaxCountInH * m_nSubjectBtnH + (nMaxCountInH - 1) * nGap + 6;
		if (nExamDlg_H < 50) nExamDlg_H = 50;

		if (nTmpTop + nExamDlg_H > m_rtExamList.Height())
		{
			nShowCount++;
			nTmpTop = m_rtExamList.top;
		}

		nTmpTop += (nExamDlg_H + nGap);
	}
	m_nShowPapersCount = nShowCount;
}

int CExamInfoMgrDlg::GetStartExamIndex(int n)
{
	int nGap = 2;
	int nExamDlg_H = 45;				//������Ϣ�б��ĸ߶�

	int nResult = 0;
	int nShowCount = 1;
	int nTmpTop = m_rtExamList.top;
	EXAM_LIST::iterator it = m_lExamList.begin();
	for (int i = 1; it != m_lExamList.end(); i++, it++)
	{
		pEXAMINFO pExam = *it;

		int nSubs = pExam->lSubjects.size();
		int nMaxCountInH = ceil((float)nSubs / (float)m_nMaxSubsRow);	//����м��а�ť
		nExamDlg_H = nMaxCountInH * m_nSubjectBtnH + (nMaxCountInH - 1) * nGap + 6;
		if (nExamDlg_H < m_nDlgMinH) nExamDlg_H = m_nDlgMinH;

		if (nTmpTop + nExamDlg_H > m_rtExamList.Height())
		{
			nShowCount++;
			nTmpTop = m_rtExamList.top;
		}

		nTmpTop += (nExamDlg_H + nGap);

		if (nShowCount == n)
		{
			nResult = i;
			break;
		}
	}
	return nResult;
}

void CExamInfoMgrDlg::DrawBorder(CDC *pDC)
{
	CPen *pOldPen = NULL;
	CPen pPen;
	CRect rcClient(0, 0, 0, 0);
	GetClientRect(&rcClient);
	pPen.CreatePen(PS_SOLID, 2, RGB(0, 0, 255));

	pDC->SelectStockObject(NULL_BRUSH);
	pOldPen = pDC->SelectObject(&pPen);
	pDC->Rectangle(&rcClient);
	pDC->SelectObject(pOldPen);
	pPen.Detach();
	ReleaseDC(pDC);
}

void CExamInfoMgrDlg::ShowExamList(EXAM_LIST lExam, int nStartShow)
{
	if (nStartShow > m_nAllExamListItems) return;
	if (nStartShow <= 0) return;

	ReleaseDlgData();
	if (lExam.size() <= 0) return;

	int nGap = 2;
	int nExamDlg_H = 45;				//������Ϣ�б��ĸ߶�
	int nRealW = m_rtExamList.Width();	//������Ϣ�б��Ŀ���
	int nRealH = m_rtExamList.Height();	//ʵ����Ч����ʾ�����б����ڵĸ߶�
	
	int nCurrLeft = m_rtExamList.left;
	int nCurrTop = m_rtExamList.top;

	int nCount = 0;
	for (auto examObj : lExam)
	{
		pEXAMINFO pExam = examObj;

		nCount++;
		if (nCount >= nStartShow)
		{
			int nSubs = pExam->lSubjects.size();
			int nMaxCountInH = ceil((float)nSubs / (float)m_nMaxSubsRow);	//����м��а�ť
			nExamDlg_H = nMaxCountInH * m_nSubjectBtnH + (nMaxCountInH - 1) * nGap + 6;
			if (nExamDlg_H < m_nDlgMinH) nExamDlg_H = m_nDlgMinH;

			if (nCurrTop + nExamDlg_H > m_rtExamList.Height())
				break;

			CSingleExamDlg* pExamDlg = new CSingleExamDlg(this);
			pExamDlg->Create(CSingleExamDlg::IDD, this);
			pExamDlg->ShowWindow(SW_HIDE);
			pExamDlg->MoveWindow(nCurrLeft, nCurrTop, nRealW, nExamDlg_H);
			nCurrTop += (nExamDlg_H + nGap);

			pExamDlg->m_nMaxSubsRow		= m_nMaxSubsRow;
			pExamDlg->m_nSubjectBtnH	= m_nSubjectBtnH;

			pExamDlg->SetExamInfo(pExam);
			pExamDlg->ShowWindow(SW_SHOW);
			m_vecExamInfoDlg.push_back(pExamDlg);
		}
	}
	m_nAllExamListItems = lExam.size();

	m_strShowCurrPaper.Format(_T("��ǰҳ: %d/%d"), m_nCurrShowPaper, m_nShowPapersCount);
	UpdateData(FALSE);
}


void CExamInfoMgrDlg::ReleaseDlgData()
{
	int nCoutn = m_vecExamInfoDlg.size();
	for (int i = 0; i < m_vecExamInfoDlg.size(); i++)
	{
		CSingleExamDlg* pDlg = m_vecExamInfoDlg[i];
		pDlg->DestroyWindow();
		SAFE_RELEASE(pDlg);
	}
	m_vecExamInfoDlg.clear();
	for (int i = 0; i < m_vecBtnIndex.size(); i++)
	{
		CLinkCtrl * pLinkCtrl = m_vecBtnIndex[i];
		SAFE_RELEASE(pLinkCtrl);
		m_vecBtnIndex[i] = NULL;
	}
	m_vecBtnIndex.clear();
}

void CExamInfoMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
	GetAllShowPaperCount();
	m_nCurrShowPaper = 1;
	ShowExamList(m_lExamList, m_nCurrShowPaper);
	Invalidate();
}


BOOL CExamInfoMgrDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 222, 250
	DrawBorder(pDC);

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
	m_nCurrShowPaper = 1;
	ShowExamList(m_lExamList, m_nCurrShowPaper);
}


void CExamInfoMgrDlg::OnCbnSelchangeComboGrade()
{
	if (m_comboGrade.GetCurSel() < 0)
		return;

	GetSearchResultExamList();
	m_nCurrShowPaper = 1;
	ShowExamList(m_lExamList, m_nCurrShowPaper);
}


void CExamInfoMgrDlg::OnBnClickedBtnFirst()
{
	if (m_lExamList.size() == 0)
		GetSearchResultExamList();

	m_nCurrShowPaper = 1;
	int nCurrStartShowExamListItem = GetStartExamIndex(m_nCurrShowPaper);
	ShowExamList(m_lExamList, nCurrStartShowExamListItem);
}


void CExamInfoMgrDlg::OnBnClickedBtnLast()
{
	if (m_lExamList.size() == 0)
		GetSearchResultExamList();

	m_nCurrShowPaper = m_nShowPapersCount;
	int nCurrStartShowExamListItem = GetStartExamIndex(m_nCurrShowPaper);
	ShowExamList(m_lExamList, nCurrStartShowExamListItem);
}


void CExamInfoMgrDlg::OnBnClickedBtnUp()
{
	m_nCurrShowPaper--;
	if (m_nCurrShowPaper < 1) m_nCurrShowPaper = 1;
	int nCurrStartShowExamListItem = GetStartExamIndex(m_nCurrShowPaper);
	ShowExamList(m_lExamList, nCurrStartShowExamListItem);
}


void CExamInfoMgrDlg::OnBnClickedBtnDown()
{
	m_nCurrShowPaper++;
	if (m_nCurrShowPaper > m_nShowPapersCount) m_nCurrShowPaper = m_nShowPapersCount;
	int nCurrStartShowExamListItem = GetStartExamIndex(m_nCurrShowPaper);
	ShowExamList(m_lExamList, nCurrStartShowExamListItem);
}

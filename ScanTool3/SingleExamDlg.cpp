// SingleExamDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "SingleExamDlg.h"
#include "afxdialogex.h"


// CSingleExamDlg �Ի���

IMPLEMENT_DYNAMIC(CSingleExamDlg, CDialog)

CSingleExamDlg::CSingleExamDlg(CWnd* pParent /*=NULL*/)
: CDialog(CSingleExamDlg::IDD, pParent)
, _pExamInfo(NULL), _strExamName(_T(""))
, m_nStatusSize(25), m_nSubjectBtnH(40), m_nMaxSubsRow(2)
{

}

CSingleExamDlg::~CSingleExamDlg()
{
	ReleaseData();
}

void CSingleExamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_ExamName, _strExamName);
}


BEGIN_MESSAGE_MAP(CSingleExamDlg, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CSingleExamDlg ��Ϣ�������

BOOL CSingleExamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitData();
	SetFontSize(m_nStatusSize);
	InitCtrlPosition();

	return TRUE;
}

BOOL CSingleExamDlg::PreTranslateMessage(MSG* pMsg)
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

void CSingleExamDlg::InitData()
{
	_strExamName = _T("");

	if (!_pExamInfo) return;

	USES_CONVERSION;
	_strExamName = A2T(_pExamInfo->strExamName.c_str());
	UpdateData(FALSE);
}

void CSingleExamDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 2;	//�ϱߵļ��
	const int nBottomGap = 2;	//�±ߵļ��
	const int nLeftGap = 2;		//��ߵĿհ׼��
	const int nRightGap = 2;	//�ұߵĿհ׼��
	int nGap = 2;
	int nStaticH = (cy - nTopGap - nBottomGap) / 3;	//��̬�ؼ��߶�

	int nBtnW = (cx - nLeftGap - nRightGap) * 0.1;	//һ������2����ť
	if (nBtnW < 60) nBtnW = 60;
	if (nBtnW > 100) nBtnW = 100;

	int nCurrTop = nTopGap;
	int nCurrLeft = nLeftGap;

	if (GetDlgItem(IDC_STATIC_ExamName)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		int nH = nStaticH * 2 - nGap;
		GetDlgItem(IDC_STATIC_ExamName)->MoveWindow(nCurrLeft, nCurrTop, nW, nH);
		nCurrTop += nH + nGap;
	}
	if (GetDlgItem(IDC_STATIC_ExamTime)->GetSafeHwnd())
	{
		int nW = cx - nLeftGap - nRightGap - nBtnW - nGap - nBtnW - nGap * 5;
		GetDlgItem(IDC_STATIC_ExamTime)->MoveWindow(nCurrLeft, nCurrTop, nW, nStaticH);
	}

	int nMaxBtnRow = m_nMaxSubsRow;			//һ�������ʾ2����Ŀ��ť
	int nBtnH = m_nSubjectBtnH;
	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		nCurrLeft = cx - nRightGap - (nMaxBtnRow - i % nMaxBtnRow) * nBtnW - (nMaxBtnRow - i % nMaxBtnRow - 1) * nGap;
		nCurrTop = nTopGap + (i / nMaxBtnRow) * (nBtnH + nGap);
		if (m_vecBtn[i]->GetSafeHwnd())
		{
			m_vecBtn[i]->MoveWindow(nCurrLeft, nCurrTop, nBtnW, nBtnH);
		}
	}
	TRACE("��ť����: %d\n", m_vecBtn.size());

	Invalidate();
}

void CSingleExamDlg::SetFontSize(int nSize)
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
	GetDlgItem(IDC_STATIC_ExamName)->SetFont(&m_fontStatus);
}

void CSingleExamDlg::ReleaseData()
{
	TRACE("CSingleExamDlg::ReleaseData, ԭʼ������С: %d\n", m_vecBtn.size());
	for (int i = 0; i < m_vecBtn.size(); i++)
	{
		CButton* pBtn = m_vecBtn[i];
		SAFE_RELEASE(pBtn);
		m_vecBtn[i] = NULL;
	}
	m_vecBtn.clear();
}

void CSingleExamDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CSingleExamDlg::SetExamInfo(pEXAMINFO pExamInfo)
{
	_pExamInfo = pExamInfo;

	ReleaseData();
	InitData();
	USES_CONVERSION;
	for (auto pSub : _pExamInfo->lSubjects)
	{
		pEXAM_SUBJECT pSubject = pSub;
		CButton* pNewButton = new CButton();// Ҳ���Զ���Ϊ��ĳ�Ա������
		CRect rcButton(10, 10, 50, 30); // ��ť�ڶԻ����е�λ�á�
		pNewButton->Create(A2T(pSub->strSubjName.c_str()), 0, rcButton, this, pSubject->nSubjID);
		pNewButton->ShowWindow(SW_SHOW);
		m_vecBtn.push_back(pNewButton);
	}
	InitCtrlPosition();
}


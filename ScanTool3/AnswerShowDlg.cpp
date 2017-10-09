// AnswerShowDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "AnswerShowDlg.h"
#include "afxdialogex.h"


// CAnswerShowDlg �Ի���

IMPLEMENT_DYNAMIC(CAnswerShowDlg, CDialogEx)

CAnswerShowDlg::CAnswerShowDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ANSWERSHOWDLG, pParent)
	, m_nCurrentItem(-1)
{

}

CAnswerShowDlg::~CAnswerShowDlg()
{
}

void CAnswerShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Answer, m_lAnswerListCtrl);
}


BEGIN_MESSAGE_MAP(CAnswerShowDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(NM_HOVER, IDC_LIST_Answer, &CAnswerShowDlg::OnNMHoverListAnswer)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Answer, &CAnswerShowDlg::OnNMDblclkListAnswer)
END_MESSAGE_MAP()

// CAnswerShowDlg ��Ϣ�������
BOOL CAnswerShowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitUI();
	
	return TRUE;
}

void CAnswerShowDlg::InitData(pST_PaperInfo pPaper)
{
	USES_CONVERSION;
	while (m_lAnswerListCtrl.DeleteColumn(1));
	OMRRESULTLIST::iterator it = pPaper->lOmrResult.begin();
	for(int i = 1; it != pPaper->lOmrResult.end(); it++, i++)
	{
		char szTh[10] = { 0 };
		sprintf_s(szTh, "%d", it->nTH);
		m_lAnswerListCtrl.InsertColumn(i, A2T(szTh), LVCFMT_CENTER, 40);

		int nRow = 0;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, _T("���"));
		++nRow;
		char szType[10] = { 0 };
		if (it->nSingle == 0)
			strcpy_s(szType, "��");
		else if (it->nSingle == 1)
			strcpy_s(szType, "��");
		else
			strcpy_s(szType, "��");
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(szType));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal1.c_str()));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal2.c_str()));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal3.c_str()));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		m_lAnswerListCtrl.SetItemText(nRow, i, A2T(it->strRecogVal.c_str()));
		++nRow;
		m_lAnswerListCtrl.InsertItem(nRow, NULL);
		if (it->nDoubt == 1)
			m_lAnswerListCtrl.SetItemText(nRow, i, _T("*"));
	}
}

void CAnswerShowDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	int nGap = 2;	//�ؼ��ļ��
	int nTopGap = 2;	//�����ϱ߱�Ե�ļ��
	int nBottomGap = 2;	//�����±߱�Ե�ļ��
	int nLeftGap = 2;	//������߱�Ե�ļ��
	int nRightGap = 2;	//�����ұ߱�Ե�ļ��
	int nCurrentTop = nTopGap;
	int nCurrentLeft = nLeftGap;

	if (m_lAnswerListCtrl.GetSafeHwnd())
	{
		int nW = cx - nCurrentLeft - nRightGap;
		int nH = cy - nCurrentTop - nBottomGap;
		m_lAnswerListCtrl.MoveWindow(nCurrentLeft, nCurrentTop, nW, nH);
	}
}

void CAnswerShowDlg::InitUI()
{
	m_lAnswerListCtrl.SetExtendedStyle(m_lAnswerListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES );
	m_lAnswerListCtrl.InsertColumn(0, _T("�Ծ����"), LVCFMT_CENTER, 80);

	int nRow = 0;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("���"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("��/��"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("ʶ��1"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("ʶ��2"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("ʶ��3"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("����"));
	++nRow;
	m_lAnswerListCtrl.InsertItem(nRow, NULL);
	m_lAnswerListCtrl.SetItemText(nRow, 0, _T("����"));
	++nRow;

	InitCtrlPosition();
}

void CAnswerShowDlg::SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem)
{
	if (nItem < 0) return;

	lCtrl.GetItemColors(1, nItem, crOldText, crOldBackground);
	for (int i = 0; i < lCtrl.GetItemCount(); i++)							//���ø�����ʾ(�ֶ����ñ�����ɫ)
		lCtrl.SetItemColors(i, nItem, RGB(0, 0, 0), RGB(112, 180, 254));	//70, 70, 255
}

void CAnswerShowDlg::UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem)
{
	if (nItem < 0) return;

	for (int i = 0; i < lCtrl.GetItemCount(); i++)
		if (!lCtrl.GetModified(i, nItem))
			lCtrl.SetItemColors(i, nItem, crOldText, crOldBackground);
		else
			lCtrl.SetItemColors(i, nItem, RGB(255, 0, 0), crOldBackground);
}

void CAnswerShowDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CAnswerShowDlg::OnNMHoverListAnswer(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;		//**********	�����������Ӧ��ͬʱ���ؽ��ֵ��Ϊ1�Ļ���	****************
						//**********	�ͻ��������TRACK SELECT��Ҳ���������ͣ	****************
						//**********	һ��ʱ����������Զ���ѡ��
}

void CAnswerShowDlg::OnNMDblclkListAnswer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	UnSetListCtrlHighLightShow(m_lAnswerListCtrl, m_nCurrentItem);
	m_nCurrentItem = pNMItemActivate->iSubItem;

	SetListCtrlHighLightShow(m_lAnswerListCtrl, m_nCurrentItem);
}

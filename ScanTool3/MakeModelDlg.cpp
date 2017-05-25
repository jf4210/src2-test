// MakeModelDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "MakeModelDlg.h"
#include "afxdialogex.h"


// CMakeModelDlg �Ի���

IMPLEMENT_DYNAMIC(CMakeModelDlg, CDialog)

CMakeModelDlg::CMakeModelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMakeModelDlg::IDD, pParent)
{

}

CMakeModelDlg::~CMakeModelDlg()
{
}

void CMakeModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMakeModelDlg, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CMakeModelDlg ��Ϣ�������


BOOL CMakeModelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

// 	InitExamData();
// 	InitChildDlg();
// 	InitCtrlPosition();
// 	m_scanThread.CreateThread();
// 
// 	SetFontSize(m_nStatusSize);
// 	m_comboSubject.AdjustDroppedWidth();

	return TRUE;
}

BOOL CMakeModelDlg::PreTranslateMessage(MSG* pMsg)
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

void CMakeModelDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPosition();
}

void CMakeModelDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.right;
	int cy = rcClient.bottom;

	const int nTopGap = 100;	//�ϱߵļ��
	const int nBottomGap = 10;	//�±ߵļ��
	const int nLeftGap = 20;		//��ߵĿհ׼��
	const int nRightGap = 20;	//�ұߵĿհ׼��
	int nGap = 5;

}
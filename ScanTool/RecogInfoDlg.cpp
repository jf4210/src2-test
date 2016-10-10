// RecogInfoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool.h"
#include "RecogInfoDlg.h"
#include "afxdialogex.h"
#include "MakeModelDlg.h"

// CRecogInfoDlg �Ի���

IMPLEMENT_DYNAMIC(CRecogInfoDlg, CDialog)

CRecogInfoDlg::CRecogInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecogInfoDlg::IDD, pParent)
	, m_nThresholdVal(0), m_fThresholdValPercent(0.0), m_strCPTypeName(_T("")), m_pCurRectInfo(NULL)
{

}

CRecogInfoDlg::~CRecogInfoDlg()
{
}

void CRecogInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Threshold, m_nThresholdVal);
	DDX_Text(pDX, IDC_EDIT_ThresholdPercent, m_fThresholdValPercent);
	DDX_Text(pDX, IDC_EDIT_CPType, m_strCPTypeName);
}

BEGIN_MESSAGE_MAP(CRecogInfoDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_SaveRecogInfo, &CRecogInfoDlg::OnBnClickedBtnSaverecoginfo)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CRecogInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);
	InitCtrlPosition();

	return TRUE;
}

void CRecogInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	InitCtrlPosition();
}


void CRecogInfoDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int nGap = 5;	//�ؼ��ļ��
	int nTopGap = 2;	//�����ϱ߱�Ե�ļ��
	int nBottomGap = 2;	//�����±߱�Ե�ļ��
	int nLeftGap = 2;	//������߱�Ե�ļ��
	int nRightGap = 2;	//�����ұ߱�Ե�ļ��

	int nCurrentTop = 0;
	int nGroupHeight = rcClient.Height();
	int nGroupWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nGroupStaticHeight = 15;	//group���ֵĸ߶�
	int nStaticHeight = 20;		//У�������Static�ؼ��߶�
	int nTopInGroup = nCurrentTop;
	if (GetDlgItem(IDC_STATIC_Group)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Group)->MoveWindow(nLeftGap, nTopInGroup, nGroupWidth, nGroupHeight);
		nTopInGroup = nTopInGroup + nGroupStaticHeight + nGap;		//����һ��group���ֵĸ߶�
	}
	int nStaticWidthInGrop = (nGroupWidth - 3 * nGap) / 3;
	if (GetDlgItem(IDC_STATIC_CPTypeInGroup)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_CPTypeInGroup)->MoveWindow(nLeftGap + nGap, nTopInGroup, nStaticWidthInGrop, nStaticHeight);
	}
	if (GetDlgItem(IDC_EDIT_CPType)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_CPType)->MoveWindow(nLeftGap + nStaticWidthInGrop + nGap * 2, nTopInGroup, nStaticWidthInGrop * 2, nStaticHeight);
		nTopInGroup = nTopInGroup + nStaticHeight + nGap;
	}
	if (GetDlgItem(IDC_STATIC_Threshold)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Threshold)->MoveWindow(nLeftGap + nGap, nTopInGroup, nStaticWidthInGrop, nStaticHeight);
	}
	if (GetDlgItem(IDC_EDIT_Threshold)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_Threshold)->MoveWindow(nLeftGap + nStaticWidthInGrop + nGap * 2, nTopInGroup, nStaticWidthInGrop, nStaticHeight);
		nTopInGroup = nTopInGroup + nStaticHeight + nGap;
	}
	if (GetDlgItem(IDC_STATIC_ThresholdPercent)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_ThresholdPercent)->MoveWindow(nLeftGap + nGap, nTopInGroup, nStaticWidthInGrop, nStaticHeight);
	}
	if (GetDlgItem(IDC_EDIT_ThresholdPercent)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_ThresholdPercent)->MoveWindow(nLeftGap + nStaticWidthInGrop + nGap * 2, nTopInGroup, nStaticWidthInGrop, nStaticHeight);
	}
	int nBtnInGroup_L = nLeftGap + nStaticWidthInGrop + nGap * 2 + nStaticWidthInGrop + nGap;
	int nBtnInGroup_W = nGroupWidth - nBtnInGroup_L - nGap;
	int nBtnInGroup_H = nStaticHeight * 2 + nGap;
	if (GetDlgItem(IDC_BTN_SaveRecogInfo)->GetSafeHwnd())
	{
		nTopInGroup = nCurrentTop + nGroupStaticHeight + nGap + nStaticHeight + nGap;
		GetDlgItem(IDC_BTN_SaveRecogInfo)->MoveWindow(nBtnInGroup_L, nTopInGroup, nBtnInGroup_W, nBtnInGroup_H);
	}
}

// CRecogInfoDlg ��Ϣ�������
void CRecogInfoDlg::OnBnClickedBtnSaverecoginfo()
{
	if (!m_pCurRectInfo)
		return;

	int nOldThresholdVal = m_nThresholdVal;
	UpdateData(TRUE);
	if (m_nThresholdVal > 255)	m_nThresholdVal = 255;
	if (m_nThresholdVal < 0)	m_nThresholdVal = 0;
	if (m_fThresholdValPercent > 100.0)	m_fThresholdValPercent = 100.0;
	if (m_fThresholdValPercent < 0.0)	m_fThresholdValPercent = 0.0;

	m_pCurRectInfo->nThresholdValue = m_nThresholdVal;
	m_pCurRectInfo->fStandardValuePercent = m_fThresholdValPercent / 100;

	if (nOldThresholdVal != m_nThresholdVal)
	{
		cv::Rect rm = m_pCurRectInfo->rt;
		cv::Mat imgResult = ((CMakeModelDlg*)GetParent())->m_vecPaperModelInfo[((CMakeModelDlg*)GetParent())->m_nCurrTabSel]->matDstImg(rm);
		((CMakeModelDlg*)GetParent())->RecogNewGrayValue(imgResult, *m_pCurRectInfo);
	}
	AfxMessageBox(_T("�޸ĳɹ�"));
}

void CRecogInfoDlg::ShowDetailRectInfo(pRECTINFO pCurRectInfo)
{
	if (!pCurRectInfo)
		return;

	m_pCurRectInfo = pCurRectInfo;
	//��ʾ��У������ϸ��Ϣ
	switch (pCurRectInfo->eCPType)
	{
	case Fix_CP:	m_strCPTypeName	= _T("����"); break;
	case H_HEAD:	m_strCPTypeName	= _T("ˮƽͬ��ͷ"); break;
	case V_HEAD:	m_strCPTypeName	= _T("��ֱͬ��ͷ"); break;
	case ABMODEL:	m_strCPTypeName	= _T("���͵�"); break;
	case COURSE:	m_strCPTypeName	= _T("��Ŀ��"); break;
	case QK_CP:		m_strCPTypeName	= _T("ȱ����"); break;
	case GRAY_CP:	m_strCPTypeName	= _T("�Ҷ�У���"); break;
	case WHITE_CP:	m_strCPTypeName	= _T("�հ�У���"); break;
	case SN:		m_strCPTypeName = _T("����"); break;
	case OMR:
	{
		if(m_pCurRectInfo->nSingle == 0) m_strCPTypeName = _T("��ѡ��");
		else m_strCPTypeName = _T("��ѡ��");
		break;
	}
	case ELECT_OMR:	m_strCPTypeName = _T("ѡ����"); break;
	default:
		m_strCPTypeName = _T(""); break;
	}
	m_nThresholdVal			= pCurRectInfo->nThresholdValue;
	m_fThresholdValPercent	= pCurRectInfo->fStandardValuePercent * 100;
	UpdateData(FALSE);
}

BOOL CRecogInfoDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}



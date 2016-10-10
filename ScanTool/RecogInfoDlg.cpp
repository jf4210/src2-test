// RecogInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool.h"
#include "RecogInfoDlg.h"
#include "afxdialogex.h"
#include "MakeModelDlg.h"

// CRecogInfoDlg 对话框

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

	int nGap = 5;	//控件的间隔
	int nTopGap = 2;	//距离上边边缘的间隔
	int nBottomGap = 2;	//距离下边边缘的间隔
	int nLeftGap = 2;	//距离左边边缘的间隔
	int nRightGap = 2;	//距离右边边缘的间隔

	int nCurrentTop = 0;
	int nGroupHeight = rcClient.Height();
	int nGroupWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nGroupStaticHeight = 15;	//group文字的高度
	int nStaticHeight = 20;		//校验点类型Static控件高度
	int nTopInGroup = nCurrentTop;
	if (GetDlgItem(IDC_STATIC_Group)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Group)->MoveWindow(nLeftGap, nTopInGroup, nGroupWidth, nGroupHeight);
		nTopInGroup = nTopInGroup + nGroupStaticHeight + nGap;		//加上一个group文字的高度
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

// CRecogInfoDlg 消息处理程序
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
	AfxMessageBox(_T("修改成功"));
}

void CRecogInfoDlg::ShowDetailRectInfo(pRECTINFO pCurRectInfo)
{
	if (!pCurRectInfo)
		return;

	m_pCurRectInfo = pCurRectInfo;
	//显示此校验点的详细信息
	switch (pCurRectInfo->eCPType)
	{
	case Fix_CP:	m_strCPTypeName	= _T("定点"); break;
	case H_HEAD:	m_strCPTypeName	= _T("水平同步头"); break;
	case V_HEAD:	m_strCPTypeName	= _T("垂直同步头"); break;
	case ABMODEL:	m_strCPTypeName	= _T("卷型点"); break;
	case COURSE:	m_strCPTypeName	= _T("科目点"); break;
	case QK_CP:		m_strCPTypeName	= _T("缺考点"); break;
	case GRAY_CP:	m_strCPTypeName	= _T("灰度校验点"); break;
	case WHITE_CP:	m_strCPTypeName	= _T("空白校验点"); break;
	case SN:		m_strCPTypeName = _T("考号"); break;
	case OMR:
	{
		if(m_pCurRectInfo->nSingle == 0) m_strCPTypeName = _T("单选题");
		else m_strCPTypeName = _T("多选题");
		break;
	}
	case ELECT_OMR:	m_strCPTypeName = _T("选做题"); break;
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



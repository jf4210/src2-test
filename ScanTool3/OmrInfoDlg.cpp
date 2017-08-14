// OmrInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "OmrInfoDlg.h"
#include "afxdialogex.h"


// COmrInfoDlg 对话框

IMPLEMENT_DYNAMIC(COmrInfoDlg, CDialog)

COmrInfoDlg::COmrInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COmrInfoDlg::IDD, pParent)
	, m_nTHSel(1), m_nXXSel(0), m_nDirectSel(0), m_bShowFist(FALSE), m_bSingle(TRUE)
{

}

COmrInfoDlg::~COmrInfoDlg()
{
}

void COmrInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PIC_OMR, m_picOmrShow);
	DDX_Control(pDX, IDC_RADIO_TX_Single, m_radioTX_S);
	DDX_Control(pDX, IDC_RADIO_TX_Multi, m_radioTX_M);
	DDX_Control(pDX, IDC_RADIO_TH_H, m_radioTH_H);
	DDX_Control(pDX, IDC_RADIO_TH_V, m_radioTH_V);
	DDX_Control(pDX, IDC_RADIO_XX_H, m_radioXX_H);
	DDX_Control(pDX, IDC_RADIO_XX_V, m_radioXX_V);
	DDX_Control(pDX, IDC_RADIO_Direct_ZX, m_radioDirectZX);
	DDX_Control(pDX, IDC_RADIO_Direct_FX, m_radioDirectFX);
}


BEGIN_MESSAGE_MAP(COmrInfoDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RADIO_TH_H, &COmrInfoDlg::OnBnClickedRadioThH)
	ON_BN_CLICKED(IDC_RADIO_TH_V, &COmrInfoDlg::OnBnClickedRadioThV)
	ON_BN_CLICKED(IDC_RADIO_XX_H, &COmrInfoDlg::OnBnClickedRadioXxH)
	ON_BN_CLICKED(IDC_RADIO_XX_V, &COmrInfoDlg::OnBnClickedRadioXxV)
	ON_BN_CLICKED(IDC_RADIO_Direct_ZX, &COmrInfoDlg::OnBnClickedRadioDirectZx)
	ON_BN_CLICKED(IDC_RADIO_Direct_FX, &COmrInfoDlg::OnBnClickedRadioDirectFx)
	ON_BN_CLICKED(IDC_RADIO_TX_Single, &COmrInfoDlg::OnBnClickedRadioTxSingle)
	ON_BN_CLICKED(IDC_RADIO_TX_Multi, &COmrInfoDlg::OnBnClickedRadioTxMulti)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_STN_CLICKED(IDC_STATIC_TH_H, &COmrInfoDlg::OnStnClickedStaticThH)
	ON_STN_CLICKED(IDC_STATIC_TH_V, &COmrInfoDlg::OnStnClickedStaticThV)
	ON_STN_CLICKED(IDC_STATIC_XX_H, &COmrInfoDlg::OnStnClickedStaticXxH)
	ON_STN_CLICKED(IDC_STATIC_XX_V, &COmrInfoDlg::OnStnClickedStaticXxV)
	ON_STN_CLICKED(IDC_STATIC_Direct_ZX, &COmrInfoDlg::OnStnClickedStaticDirectZx)
	ON_STN_CLICKED(IDC_STATIC_Direct_FX, &COmrInfoDlg::OnStnClickedStaticDirectFx)
	ON_STN_CLICKED(IDC_STATIC_TX_Single, &COmrInfoDlg::OnStnClickedStaticTxSingle)
	ON_STN_CLICKED(IDC_STATIC_TX_Multi, &COmrInfoDlg::OnStnClickedStaticTxMulti)
END_MESSAGE_MAP()

BOOL COmrInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);
	InitCtrlPosition();

	m_picOmrShow.OnInit(0, false);
	InitUI();
	ShowOmrPic();
	return TRUE;
}


// COmrInfoDlg 消息处理程序


void COmrInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	InitCtrlPosition();
	if (m_picOmrShow.GetSafeHwnd())
	{
		if (!m_bShowFist)
		{
			m_bShowFist = TRUE;
			ShowOmrPic();
		}
	}
}

void COmrInfoDlg::InitCtrlPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int nGap = 2;	//控件的间隔
	int nTopGap = 2;	//距离上边边缘的间隔
	int nBottomGap = 2;	//距离下边边缘的间隔
	int nLeftGap = 2;	//距离左边边缘的间隔
	int nRightGap = 2;	//距离右边边缘的间隔

	int nCurrentTop = 0;
	int nGroupWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nStaticHeight = 15;		//校验点类型Static控件高度
	int nStaticWidth = (rcClient.Width() - nLeftGap - nRightGap - 2 * nGap) / 3;
	int nRadioWidth = nStaticWidth;

	if (GetDlgItem(IDC_STATIC_TX))
	{
		GetDlgItem(IDC_STATIC_TX)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TX_Single)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TX_Single)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_TX_Single)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TX_Single)->MoveWindow(nLeftGap + nStaticWidth + nGap + 15 + nGap, nTopGap, nRadioWidth - 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TX_Multi)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TX_Multi)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_TX_Multi)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TX_Multi)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap + 15 + nGap, nTopGap, nRadioWidth - 15, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	if (GetDlgItem(IDC_STATIC_TH)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TH)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TH_H)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TH_H)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_TH_H)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TH_H)->MoveWindow(nLeftGap + nStaticWidth + nGap + 15 + nGap, nTopGap, nRadioWidth - 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TH_V)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TH_V)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_TH_V)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TH_V)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap + 15 + nGap, nTopGap, nRadioWidth - 15, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	if (GetDlgItem(IDC_STATIC_XX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_XX)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_XX_H)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_XX_H)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_XX_H)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_XX_H)->MoveWindow(nLeftGap + nStaticWidth + nGap + 15 + nGap, nTopGap, nRadioWidth - 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_XX_V)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_XX_V)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_XX_V)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_XX_V)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap + 15 + nGap, nTopGap, nRadioWidth - 15, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	if (GetDlgItem(IDC_STATIC_Direct)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Direct)->MoveWindow(nLeftGap, nTopGap, nStaticWidth, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_Direct_ZX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_Direct_ZX)->MoveWindow(nLeftGap + nStaticWidth + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_Direct_ZX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Direct_ZX)->MoveWindow(nLeftGap + nStaticWidth + nGap + 15 + nGap, nTopGap, nRadioWidth - 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_Direct_FX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_Direct_FX)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_Direct_FX)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_Direct_FX)->MoveWindow(nLeftGap + nStaticWidth + nGap + nRadioWidth + nGap + 15 + nGap, nTopGap, nRadioWidth - 15, nStaticHeight);
	}
	nTopGap = nTopGap + nStaticHeight + nGap;
	int nPicOmrWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nPicOmrHeight = rcClient.Height() - nTopGap;
	if (GetDlgItem(IDC_PIC_OMR)->GetSafeHwnd())
	{
		GetDlgItem(IDC_PIC_OMR)->MoveWindow(nLeftGap, nTopGap, nPicOmrWidth, nPicOmrHeight);
	}
}

void COmrInfoDlg::OnBnClickedRadioTxSingle()
{
	if (m_radioTX_S.GetCheck() == 1)
		m_bSingle = TRUE;
	else
		m_bSingle = FALSE;
}


void COmrInfoDlg::OnBnClickedRadioTxMulti()
{
	if (m_radioTX_M.GetCheck() == 1)
		m_bSingle = FALSE;
	else
		m_bSingle = TRUE;
}

void COmrInfoDlg::OnBnClickedRadioThH()
{
	if (m_nTHSel != 0)
	{
		m_nTHSel = 0;
		ShowOmrPic();
	}
}

void COmrInfoDlg::OnBnClickedRadioThV()
{
	if (m_nTHSel != 1)
	{
		m_nTHSel = 1;
		ShowOmrPic();
	}
}

void COmrInfoDlg::OnBnClickedRadioXxH()
{
	if (m_nXXSel != 0)
	{
		m_nXXSel = 0;
		ShowOmrPic();
	}
}

void COmrInfoDlg::OnBnClickedRadioXxV()
{
	if (m_nXXSel != 1)
	{
		m_nXXSel = 1;
		ShowOmrPic();
	}
}

void COmrInfoDlg::OnBnClickedRadioDirectZx()
{
	if (m_nDirectSel != 0)
	{
		m_nDirectSel = 0;
		ShowOmrPic();
	}
}

void COmrInfoDlg::OnBnClickedRadioDirectFx()
{
	if (m_nDirectSel != 1)
	{
		m_nDirectSel = 1;
		ShowOmrPic();
	}
}

void COmrInfoDlg::ShowOmrPic()
{
	CString strPicPath = g_strCurrentPath;	
	if (m_nTHSel == 0)
	{
		if (m_nXXSel == 0)
		{
			if (m_nDirectSel == 0)
			{
				strPicPath = _T(".\\res\\横向1.png");
				m_nCurrentOmrVal = 42;			//101010
			}
			else
			{
				strPicPath = _T(".\\res\\竖直2.png");
				m_nCurrentOmrVal = 41;			//101001
			}
		}
		else
		{
			if (m_nDirectSel == 0)
			{
				strPicPath = _T(".\\res\\竖直4.png");
				m_nCurrentOmrVal = 38;			//100110
			}
			else
			{
				strPicPath = _T(".\\res\\横向3.png");
				m_nCurrentOmrVal = 37;			//100101
			}
		}
	}
	else
	{
		if (m_nXXSel == 0)
		{
			if (m_nDirectSel == 0)
			{
				strPicPath = _T(".\\res\\竖直1.png");
				m_nCurrentOmrVal = 26;			//011010
			}
			else
			{
				strPicPath = _T(".\\res\\横向4.png");
				m_nCurrentOmrVal = 25;			//011001
			}
		}
		else
		{
			if (m_nDirectSel == 0)
			{
				strPicPath = _T(".\\res\\横向2.png");
				m_nCurrentOmrVal = 22;			//010110
			}
			else
			{
				strPicPath = _T(".\\res\\竖直3.png");
				m_nCurrentOmrVal = 21;			//010101
			}
		}
	}
	m_src_img = cv::imread((std::string)(CT2CA)strPicPath);
	m_picOmrShow.ShowImage(m_src_img, 0);
	m_picOmrShow.Invalidate();
}

BOOL COmrInfoDlg::PreTranslateMessage(MSG* pMsg)
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
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void COmrInfoDlg::ShowUI(int nOmrVal, int nSingleFlag)
{
	if (nSingleFlag == 0)	m_bSingle = TRUE;
	else	m_bSingle = FALSE;

	m_nCurrentOmrVal = nOmrVal;
	
	switch (nOmrVal)
	{
		case 42:
			m_nTHSel = 0;
			m_nXXSel = 0;
			m_nDirectSel = 0;
			break;
		case 41:
			m_nTHSel = 0;
			m_nXXSel = 0;
			m_nDirectSel = 1;
			break;
		case 38:
			m_nTHSel = 0;
			m_nXXSel = 1;
			m_nDirectSel = 0;
			break;
		case 37:
			m_nTHSel = 0;
			m_nXXSel = 1;
			m_nDirectSel = 1;
			break;
		case 26:
			m_nTHSel = 1;
			m_nXXSel = 0;
			m_nDirectSel = 0;
			break;
		case 25:
			m_nTHSel = 1;
			m_nXXSel = 0;
			m_nDirectSel = 1;
			break;
		case 22:
			m_nTHSel = 1;
			m_nXXSel = 1;
			m_nDirectSel = 0;
			break;
		case 21:
			m_nTHSel = 1;
			m_nXXSel = 1;
			m_nDirectSel = 1;
			break;
	}
	InitUI();
	ShowOmrPic();
}

void COmrInfoDlg::InitUI()
{
	if (m_bSingle)
	{
		m_radioTX_S.SetCheck(1);
		m_radioTX_M.SetCheck(0);
	}
	else
	{
		m_radioTX_S.SetCheck(0);
		m_radioTX_M.SetCheck(1);
	}
	if (m_nTHSel == 0)
	{
		m_radioTH_H.SetCheck(1);
		m_radioTH_V.SetCheck(0);
	}
	else
	{
		m_radioTH_H.SetCheck(0);
		m_radioTH_V.SetCheck(1);
	}
	if (m_nXXSel == 0)
	{
		m_radioXX_H.SetCheck(1);
		m_radioXX_V.SetCheck(0);
	}
	else
	{
		m_radioXX_H.SetCheck(0);
		m_radioXX_V.SetCheck(1);
	}
	if (m_nDirectSel == 0)
	{
		m_radioDirectZX.SetCheck(1);
		m_radioDirectFX.SetCheck(0);
	}
	else
	{
		m_radioDirectZX.SetCheck(0);
		m_radioDirectFX.SetCheck(1);
	}
}

HBRUSH COmrInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}


BOOL COmrInfoDlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);

	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(rcClient, &CBrush(RGB(255, 255, 255)));	//225, 242, 250

	return TRUE;
}


void COmrInfoDlg::OnStnClickedStaticThH()
{
	if (m_nTHSel != 0)
	{
		m_nTHSel = 0;
		ShowOmrPic();
	}
	m_radioTH_H.SetCheck(1);
	m_radioTH_V.SetCheck(0);
}

void COmrInfoDlg::OnStnClickedStaticThV()
{
	if (m_nTHSel != 1)
	{
		m_nTHSel = 1;
		ShowOmrPic();
	}
	m_radioTH_H.SetCheck(0);
	m_radioTH_V.SetCheck(1);
}


void COmrInfoDlg::OnStnClickedStaticXxH()
{
	if (m_nXXSel != 0)
	{
		m_nXXSel = 0;
		ShowOmrPic();
	}
	m_radioXX_H.SetCheck(1);
	m_radioXX_V.SetCheck(0);
}


void COmrInfoDlg::OnStnClickedStaticXxV()
{
	if (m_nXXSel != 1)
	{
		m_nXXSel = 1;
		ShowOmrPic();
	}
	m_radioXX_H.SetCheck(0);
	m_radioXX_V.SetCheck(1);
}


void COmrInfoDlg::OnStnClickedStaticDirectZx()
{
	if (m_nDirectSel != 0)
	{
		m_nDirectSel = 0;
		ShowOmrPic();
	}
	m_radioDirectZX.SetCheck(1);
	m_radioDirectFX.SetCheck(0);
}


void COmrInfoDlg::OnStnClickedStaticDirectFx()
{
	if (m_nDirectSel != 1)
	{
		m_nDirectSel = 1;
		ShowOmrPic();
	}
	m_radioDirectZX.SetCheck(0);
	m_radioDirectFX.SetCheck(1);
}


void COmrInfoDlg::OnStnClickedStaticTxSingle()
{
	m_bSingle = TRUE;
	m_radioTX_S.SetCheck(1);
	m_radioTX_M.SetCheck(0);
}


void COmrInfoDlg::OnStnClickedStaticTxMulti()
{
	m_bSingle = FALSE;
	m_radioTX_S.SetCheck(0);
	m_radioTX_M.SetCheck(1);
}

// OmrInfoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "OmrInfoDlg.h"
#include "afxdialogex.h"


// COmrInfoDlg �Ի���

IMPLEMENT_DYNAMIC(COmrInfoDlg, CDialog)

COmrInfoDlg::COmrInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COmrInfoDlg::IDD, pParent)
	, m_nTHSel(1), m_nXXSel(0), m_nDirectSel(0), m_bShowFist(FALSE), m_nSingle(0)
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
	DDX_Control(pDX, IDC_RADIO_TX_PD, m_radioTX_P);
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
	ON_STN_CLICKED(IDC_STATIC_TX_PD, &COmrInfoDlg::OnStnClickedStaticTxPd)
	ON_BN_CLICKED(IDC_RADIO_TX_PD, &COmrInfoDlg::OnBnClickedRadioTxPd)
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


// COmrInfoDlg ��Ϣ�������


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

	int nGap = 2;	//�ؼ��ļ��
	int nTopGap = 2;	//�����ϱ߱�Ե�ļ��
	int nBottomGap = 2;	//�����±߱�Ե�ļ��
	int nLeftGap = 2;	//������߱�Ե�ļ��
	int nRightGap = 2;	//�����ұ߱�Ե�ļ��

	int nCurrentTop = 0;
	int nGroupWidth = rcClient.Width() - nLeftGap - nRightGap;
	int nStaticHeight = 15;		//У�������Static�ؼ��߶�
	int nStaticWidth = (rcClient.Width() - nLeftGap - nRightGap - 2 * nGap) / 3;
	int nRadioWidth = nStaticWidth;
#if 1
	int nTmpStaticW = nStaticWidth - 20;
	int nTmpRadioW = nRadioWidth - 12;
	if (GetDlgItem(IDC_STATIC_TX))
	{
		GetDlgItem(IDC_STATIC_TX)->MoveWindow(nLeftGap, nTopGap, nTmpStaticW, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TX_Single)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TX_Single)->MoveWindow(nLeftGap + nTmpStaticW + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_TX_Single)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TX_Single)->MoveWindow(nLeftGap + nTmpStaticW + nGap + 15 + nGap, nTopGap, nTmpRadioW - 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TX_Multi)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TX_Multi)->MoveWindow(nLeftGap + nTmpStaticW + nGap + nTmpRadioW + nGap, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_TX_Multi)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TX_Multi)->MoveWindow(nLeftGap + nTmpStaticW + nGap + nTmpRadioW + nGap + 15 + nGap, nTopGap, nTmpRadioW - 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_RADIO_TX_PD)->GetSafeHwnd())
	{
		GetDlgItem(IDC_RADIO_TX_PD)->MoveWindow(nLeftGap + nTmpStaticW + nGap + (nTmpRadioW + nGap) * 2, nTopGap, 15, nStaticHeight);
	}
	if (GetDlgItem(IDC_STATIC_TX_PD)->GetSafeHwnd())
	{
		GetDlgItem(IDC_STATIC_TX_PD)->MoveWindow(nLeftGap + nTmpStaticW + nGap + (nTmpRadioW + nGap) * 2 + 15 + nGap, nTopGap, nTmpRadioW - 15, nStaticHeight);
	}
#else
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
#endif
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
		m_nSingle = 0;
}


void COmrInfoDlg::OnBnClickedRadioTxMulti()
{
	if (m_radioTX_M.GetCheck() == 1)
		m_nSingle = 1;
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
				strPicPath = _T(".\\res\\����1.png");
				m_nCurrentOmrVal = 42;			//101010
			}
			else
			{
				strPicPath = _T(".\\res\\��ֱ2.png");
				m_nCurrentOmrVal = 41;			//101001
			}
		}
		else
		{
			if (m_nDirectSel == 0)
			{
				strPicPath = _T(".\\res\\��ֱ4.png");
				m_nCurrentOmrVal = 38;			//100110
			}
			else
			{
				strPicPath = _T(".\\res\\����3.png");
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
				strPicPath = _T(".\\res\\��ֱ1.png");
				m_nCurrentOmrVal = 26;			//011010
			}
			else
			{
				strPicPath = _T(".\\res\\����4.png");
				m_nCurrentOmrVal = 25;			//011001
			}
		}
		else
		{
			if (m_nDirectSel == 0)
			{
				strPicPath = _T(".\\res\\����2.png");
				m_nCurrentOmrVal = 22;			//010110
			}
			else
			{
				strPicPath = _T(".\\res\\��ֱ3.png");
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
	m_nSingle = nSingleFlag;

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
	if (m_nSingle == 0)
	{
		m_radioTX_S.SetCheck(1);
		m_radioTX_M.SetCheck(0);
		m_radioTX_P.SetCheck(0);
	}
	else if(m_nSingle == 1)
	{
		m_radioTX_S.SetCheck(0);
		m_radioTX_M.SetCheck(1);
		m_radioTX_P.SetCheck(0);
	}
	else
	{
		m_radioTX_S.SetCheck(0);
		m_radioTX_M.SetCheck(0);
		m_radioTX_P.SetCheck(1);
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
		m_radioTH_H.SetCheck(1);
		m_radioTH_V.SetCheck(0);
		ShowOmrPic();
	}
}

void COmrInfoDlg::OnStnClickedStaticThV()
{
	if (m_nTHSel != 1)
	{
		m_nTHSel = 1;
		m_radioTH_H.SetCheck(0);
		m_radioTH_V.SetCheck(1);
		ShowOmrPic();
	}
}


void COmrInfoDlg::OnStnClickedStaticXxH()
{
	if (m_nXXSel != 0)
	{
		m_nXXSel = 0;
		m_radioXX_H.SetCheck(1);
		m_radioXX_V.SetCheck(0);
		ShowOmrPic();
	}
}


void COmrInfoDlg::OnStnClickedStaticXxV()
{
	if (m_nXXSel != 1)
	{
		m_nXXSel = 1;
		m_radioXX_H.SetCheck(0);
		m_radioXX_V.SetCheck(1);
		ShowOmrPic();
	}
}


void COmrInfoDlg::OnStnClickedStaticDirectZx()
{
	if (m_nDirectSel != 0)
	{
		m_nDirectSel = 0;
		m_radioDirectZX.SetCheck(1);
		m_radioDirectFX.SetCheck(0);
		ShowOmrPic();
	}
}


void COmrInfoDlg::OnStnClickedStaticDirectFx()
{
	if (m_nDirectSel != 1)
	{
		m_nDirectSel = 1;
		m_radioDirectZX.SetCheck(0);
		m_radioDirectFX.SetCheck(1);
		ShowOmrPic();
	}
}


void COmrInfoDlg::OnStnClickedStaticTxSingle()
{
	m_nSingle = 0;
	m_radioTX_S.SetCheck(1);
	m_radioTX_M.SetCheck(0);
	m_radioTX_P.SetCheck(0);
}


void COmrInfoDlg::OnStnClickedStaticTxMulti()
{
	m_nSingle = 1;
	m_radioTX_S.SetCheck(0);
	m_radioTX_M.SetCheck(1);
	m_radioTX_P.SetCheck(0);
}


void COmrInfoDlg::OnStnClickedStaticTxPd()
{
	m_nSingle = 2;
	m_radioTX_S.SetCheck(0);
	m_radioTX_M.SetCheck(0);
	m_radioTX_P.SetCheck(1);
}


void COmrInfoDlg::OnBnClickedRadioTxPd()
{
	if (m_radioTX_P.GetCheck())
		m_nSingle = 2;
}

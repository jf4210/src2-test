// ZgtSettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "ZgtSettingDlg.h"
#include "afxdialogex.h"


// CZgtSettingDlg 对话框

IMPLEMENT_DYNAMIC(CZgtSettingDlg, CDialog)

CZgtSettingDlg::CZgtSettingDlg(int nStartTH/* = 0*/, int nQuestionType, CWnd* pParent /*=NULL*/)
	: CTipBaseDlg(IDD_ZGTSETTINGDLG, pParent)
	, m_fStartTH(nStartTH + 1), m_nQuestionType(nQuestionType)
{

}

CZgtSettingDlg::~CZgtSettingDlg()
{
}

void CZgtSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_TK, m_btnRadio_TK);
	DDX_Control(pDX, IDC_RADIO_JDT, m_btnRadio_JDT);
	DDX_Control(pDX, IDC_RADIO_YYZW, m_btnRadio_YYZW);
	DDX_Control(pDX, IDC_RADIO_YWZW, m_btnRadio_YWZW);
	DDX_Control(pDX, IDC_RADIO_XZT, m_btnRadio_XZT);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_bmpBtnClose);
	DDX_Text(pDX, IDC_EDIT_TH, m_fStartTH);
}


BEGIN_MESSAGE_MAP(CZgtSettingDlg, CTipBaseDlg)
	ON_BN_CLICKED(IDC_RADIO_TK, &CZgtSettingDlg::OnBnClickedRadioTk)
	ON_BN_CLICKED(IDC_RADIO_JDT, &CZgtSettingDlg::OnBnClickedRadioJdt)
	ON_BN_CLICKED(IDC_RADIO_YYZW, &CZgtSettingDlg::OnBnClickedRadioYyzw)
	ON_BN_CLICKED(IDC_RADIO_YWZW, &CZgtSettingDlg::OnBnClickedRadioYwzw)
	ON_BN_CLICKED(IDC_RADIO_XZT, &CZgtSettingDlg::OnBnClickedRadioXzt)
	ON_STN_CLICKED(IDC_STATIC_TK, &CZgtSettingDlg::OnStnClickedStaticTk)
	ON_STN_CLICKED(IDC_STATIC_JDT, &CZgtSettingDlg::OnStnClickedStaticJdt)
	ON_STN_CLICKED(IDC_STATIC_YYZW, &CZgtSettingDlg::OnStnClickedStaticYyzw)
	ON_STN_CLICKED(IDC_STATIC_YWZW, &CZgtSettingDlg::OnStnClickedStaticYwzw)
	ON_STN_CLICKED(IDC_STATIC_XZT, &CZgtSettingDlg::OnStnClickedStaticXzt)
	ON_BN_CLICKED(IDOK, &CZgtSettingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CZgtSettingDlg::OnBnClickedBtnClose)
END_MESSAGE_MAP()


// CZgtSettingDlg 消息处理程序
BOOL CZgtSettingDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);
	UpdateData(FALSE);

	InitShowQuestionType(m_nQuestionType);

	return TRUE;
}

void CZgtSettingDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	if(m_btnRadio_TK.GetCheck()) m_nQuestionType = 1;
	if(m_btnRadio_JDT.GetCheck()) m_nQuestionType = 2;
	if(m_btnRadio_YYZW.GetCheck()) m_nQuestionType = 3;
	if(m_btnRadio_YWZW.GetCheck()) m_nQuestionType = 4;
	if(m_btnRadio_XZT.GetCheck()) m_nQuestionType = 5;

	CTipBaseDlg::OnOK();
}

void CZgtSettingDlg::OnBnClickedBtnClose()
{
	CTipBaseDlg::OnCancel();
}

void CZgtSettingDlg::InitShowQuestionType(int nType)
{
	if (nType == 1)
	{
		m_btnRadio_TK.SetCheck(1);
		m_btnRadio_JDT.SetCheck(0);
		m_btnRadio_YYZW.SetCheck(0);
		m_btnRadio_YWZW.SetCheck(0);
		m_btnRadio_XZT.SetCheck(0);
	}
	else if (nType == 2)
	{
		m_btnRadio_TK.SetCheck(0);
		m_btnRadio_JDT.SetCheck(1);
		m_btnRadio_YYZW.SetCheck(0);
		m_btnRadio_YWZW.SetCheck(0);
		m_btnRadio_XZT.SetCheck(0);
	}
	else if (nType == 3)
	{
		m_btnRadio_TK.SetCheck(0);
		m_btnRadio_JDT.SetCheck(0);
		m_btnRadio_YYZW.SetCheck(1);
		m_btnRadio_YWZW.SetCheck(0);
		m_btnRadio_XZT.SetCheck(0);
	}
	else if (nType == 4)
	{
		m_btnRadio_TK.SetCheck(0);
		m_btnRadio_JDT.SetCheck(0);
		m_btnRadio_YYZW.SetCheck(0);
		m_btnRadio_YWZW.SetCheck(1);
		m_btnRadio_XZT.SetCheck(0);
	}
	else if (nType == 5)
	{
		m_btnRadio_TK.SetCheck(0);
		m_btnRadio_JDT.SetCheck(0);
		m_btnRadio_YYZW.SetCheck(0);
		m_btnRadio_YWZW.SetCheck(0);
		m_btnRadio_XZT.SetCheck(1);
	}
}

void CZgtSettingDlg::OnBnClickedRadioTk()
{
	m_nQuestionType = 1;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnBnClickedRadioJdt()
{
	m_nQuestionType = 2;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnBnClickedRadioYyzw()
{
	m_nQuestionType = 3;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnBnClickedRadioYwzw()
{
	m_nQuestionType = 4;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnBnClickedRadioXzt()
{
	m_nQuestionType = 5;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnStnClickedStaticTk()
{
	m_nQuestionType = 1;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnStnClickedStaticJdt()
{
	m_nQuestionType = 2;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnStnClickedStaticYyzw()
{
	m_nQuestionType = 3;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnStnClickedStaticYwzw()
{
	m_nQuestionType = 4;
	InitShowQuestionType(m_nQuestionType);
}

void CZgtSettingDlg::OnStnClickedStaticXzt()
{
	m_nQuestionType = 5;
	InitShowQuestionType(m_nQuestionType);
}


// RecogCharacterDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ScanTool3.h"
#include "RecogCharacterDlg.h"
#include "afxdialogex.h"
#include "NewMessageBox.h"


// CRecogCharacterDlg �Ի���

IMPLEMENT_DYNAMIC(CRecogCharacterDlg, CDialog)

CRecogCharacterDlg::CRecogCharacterDlg(CString strRecoged, CWnd* pParent /*=NULL*/)
	: CTipBaseDlg(IDD_RECOGCHARACTERDLG, pParent)
	, m_strVal(strRecoged), m_strValBk(strRecoged)
{

}

CRecogCharacterDlg::~CRecogCharacterDlg()
{
}

void CRecogCharacterDlg::DoDataExchange(CDataExchange* pDX)
{
	CTipBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_bmpBtnClose);
	DDX_Text(pDX, IDC_EDIT_Character, m_strVal);
}


BEGIN_MESSAGE_MAP(CRecogCharacterDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CRecogCharacterDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_OK, &CRecogCharacterDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CRecogCharacterDlg ��Ϣ�������
BOOL CRecogCharacterDlg::OnInitDialog()
{
	CTipBaseDlg::OnInitDialog();

	m_bmpBtnClose.SetStateBitmap(IDB_Btn_MakeModel_CloseNormal, 0, IDB_Btn_MakeModel_CloseDown);

	UpdateData(FALSE);
	return TRUE;
}



void CRecogCharacterDlg::OnBnClickedBtnClose()
{
	OnCancel();
}


void CRecogCharacterDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	if(m_strVal.IsEmpty())
		if (!m_strValBk.IsEmpty())
		{
			CNewMessageBox	dlg;
			dlg.setShowInfo(2, 1, "ֵ����Ϊ��");
			dlg.DoModal();
			return;
		}

	OnOK();
}

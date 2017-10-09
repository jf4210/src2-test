#pragma once
#include "XListCtrl.h"
#include "global.h"

// CAnswerShowDlg �Ի���

class CAnswerShowDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAnswerShowDlg)

public:
	CAnswerShowDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAnswerShowDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANSWERSHOWDLG };
#endif


	CXListCtrl	m_lAnswerListCtrl;		//�Ծ���б�
	void	InitData(pST_PaperInfo pPaper);

private:
	void	InitCtrlPosition();
	void	InitUI();

	void	SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//�����б������ʾ
	void	UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//ȡ���б������ʾ
	COLORREF		crOldText, crOldBackground;

	int		m_nCurrentItem;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMHoverListAnswer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListAnswer(NMHDR *pNMHDR, LRESULT *pResult);
};

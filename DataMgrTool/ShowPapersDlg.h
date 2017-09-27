#pragma once
#include "ShowPicDlg.h"

// CShowPapersDlg �Ի���

class CShowPapersDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPapersDlg)

public:
	CShowPapersDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShowPapersDlg();

// �Ի�������
	enum { IDD = IDD_SHOWPAPERSDLG };

	CListCtrl	m_listPaper;
	void setShowPapers(pPAPERSINFO pPapers);
	void ShowPapers(pPAPERSINFO pPapers);
	void ShowPaper(pST_PaperInfo pPaper);
private:
	void InitUI();
	void InitCtrlPosition();
	void	LeftRotate();
	void	RightRotate();
	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);

	CShowPicDlg* m_pShowPicDlg;

	int			m_nCurrItemPaperList;
	pPAPERSINFO m_pPapers;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMDblclkListPapers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownListPapers(NMHDR *pNMHDR, LRESULT *pResult);
};

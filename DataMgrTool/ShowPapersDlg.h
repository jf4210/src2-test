#pragma once
#include "ShowPicDlg.h"

// CShowPapersDlg 对话框

class CShowPapersDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPapersDlg)

public:
	CShowPapersDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowPapersDlg();

// 对话框数据
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMDblclkListPapers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownListPapers(NMHDR *pNMHDR, LRESULT *pResult);
};

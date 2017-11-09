#pragma once
#include "ShowPicDlg.h"
#include "AnswerShowDlg.h"
#include "XListCtrl.h"

// CShowPapersDlg 对话框

class CShowPapersDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPapersDlg)

public:
	CShowPapersDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowPapersDlg();

// 对话框数据
	enum { IDD = IDD_SHOWPAPERSDLG };

	CXListCtrl	m_listPaper;
	void setShowPapers(pPAPERSINFO pPapers);
	void ShowPapers(pPAPERSINFO pPapers);
	void ShowPaper(pST_PaperInfo pPaper);
	void ShowOmrTh(pST_PaperInfo pPaper, int nTh);	//显示题号
private:
	void InitUI();
	void InitCtrlPosition();
	void	LeftRotate();
	void	RightRotate();
	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);

	void	SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//设置列表高亮显示
	void	UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//取消列表高亮显示
	COLORREF		crOldText, crOldBackground;

	int		m_nPaperShowType;	//显示类型：0-不显示，1-无怀疑, 2-显示怀疑，3-显示无怀疑和怀疑，4-显示空，5-显示无怀疑和空，6-显示怀疑和空, 7-显示所有
	void ShowPaperListByType(int nType);	//显示类型：0-不显示，1-无怀疑, 2-显示怀疑，3-显示无怀疑和怀疑，4-显示空，5-显示无怀疑和空，6-显示怀疑和空, 7-显示所有


	CButton		m_btnChkNormal;
	CButton		m_btnChkDoubt;
	CButton		m_btnChkNull;
	void	setCheckStatus(int nStatus);
	int		getCheckStatus();

	CShowPicDlg* m_pShowPicDlg;
	CAnswerShowDlg* m_pAnswerShowDlg;
	CEdit			m_edit_OmrRecogInfo;
	CString			m_strMsg;

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
	afx_msg void OnNMHoverListPapers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedChkNormal();
	afx_msg void OnBnClickedChkDoubt();
	afx_msg void OnBnClickedChkNull();
};

#pragma once
#include "StudentMgr.h"
#include "ctrl/XPGroupBox.h"
#include "ctrl/skinscrollwnd.h"
#include "BmpButton.h"

// CVagueSearchDlg 对话框

class CVagueSearchDlg : public CDialog
{
	DECLARE_DYNAMIC(CVagueSearchDlg)

public:
	CVagueSearchDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CVagueSearchDlg();

// 对话框数据
	enum { IDD = IDD_VAGUESEARCHDLG };

	void setExamInfo(CStudentMgr* pMgr, pMODEL pModel);
	bool vagueSearch(pST_PaperInfo pPaper);
private:
	CStudentMgr*	m_pStudentMgr;
	pMODEL			m_pModel;				//扫描试卷时的校验模板

	int				m_nSearchType;		//搜索类型，1-按姓名搜索，2-按准考证号
	CString			m_strSearchKey;		//搜索关键字
	CListCtrl		m_lcBmk;			//报名库列表控件
	CXPGroupBox		m_GroupStatic;
	CBmpButton		m_bmpBtnSearch;

	HBITMAP			m_bitmap_scrollbar;
private:
	void	InitUI();
	void	InitCtrlPosition();
	
	void	SetZkzhScaned(std::string strZkzh);		//设置一个考号已经扫描标识
	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedRadioSearchname();
	afx_msg void OnBnClickedRadioSearchzkzh();
	afx_msg void OnBnClickedBtnSearch();
	afx_msg void OnNMDblclkListZkzhsearchresult(NMHDR *pNMHDR, LRESULT *pResult);
};

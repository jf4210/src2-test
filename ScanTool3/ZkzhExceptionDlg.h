#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "StudentMgr.h"
#include "VagueSearchDlg.h"
#include "ShowPicDlg.h"


// CZkzhExceptionDlg 对话框

class CZkzhExceptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CZkzhExceptionDlg)

public:
	CZkzhExceptionDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CZkzhExceptionDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ZKZHEXCEPTIONDLG };
#endif

	CXListCtrl		m_lcZkzh;				//需要修改的准考证号列表
	COLORREF		crOldText, crOldBackground;

	pMODEL			m_pModel;				//扫描试卷时的校验模板
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentSelItem;		//当前选择的项

	void	SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg);
	void	ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, pST_PaperInfo pShowPaper = NULL);
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	InitData();
	void	ReleaseData();
	BOOL	PreTranslateMessage(MSG* pMsg);
	std::string GetDetailInfo(pST_PaperInfo pPaper);

	int		CheckZkzhInBmk(std::string strZkzh);
	void	CheckZkzhInBmk(pST_PaperInfo pPaper);
	void	SetZkzhStatus();	//设置准考证号的扫描状态
	void	ShowPaperByItem(int nItem);
	bool	VagueSearch(int nItem);		//对某项进行模糊查找

	STUDENT_LIST	m_lBmkStudent;

	CStudentMgr*	m_pStudentMgr;
	pST_PaperInfo	m_pDefShowPaper;		//默认显示的试卷
	CVagueSearchDlg* m_pVagueSearchDlg;	//模糊搜索窗口
	CShowPicDlg*	m_pShowPicDlg;		//图片显示窗口
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	afx_msg LRESULT OnEditEnd(WPARAM, LPARAM);
	afx_msg LRESULT OnLBtnDownEdit(WPARAM, LPARAM);
	afx_msg LRESULT MsgVagueSearchResult(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMDblclkListZkzhexception(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListZkzhexception(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
};

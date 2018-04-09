#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "VagueSearchDlg.h"
#include "ShowPicDlg.h"


// CLostCornerDlg 对话框

class CLostCornerDlg : public CDialog
{
	DECLARE_DYNAMIC(CLostCornerDlg)

public:
	CLostCornerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLostCornerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOSTCORNERDLG };
#endif

	CXListCtrl		m_lcLostCornerPaper;				//缺角的试卷列表

	pMODEL			m_pModel;				//扫描试卷时的校验模板
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentPaperID;
	int				m_nCurrentPicID;

	CVagueSearchDlg* m_pVagueSearchDlg;	//模糊搜索窗口
	CShowPicDlg*	m_pShowPicDlg;		//图片显示窗口

	COLORREF		crPaperOldText, crPaperOldBackground;
	COLORREF		crPicOldText, crPicOldBackground;
public:
	void	SetDlgInfo(CShowPicDlg* pShowDlg, CVagueSearchDlg* pSearchDlg);
	void	ReInitData(pMODEL pModel, pPAPERSINFO pPapersInfo);
private:
	void	InitData();
	void	InitUI();
	void	InitCtrlPosition();

	void	ShowPaperDetail(pST_PaperInfo pPaper);

	BOOL	PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNMDblclkListLostcorner(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListLostcorner(NMHDR *pNMHDR, LRESULT *pResult);
};

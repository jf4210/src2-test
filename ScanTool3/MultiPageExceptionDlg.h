#pragma once
#include "global.h"
#include "XListCtrl.h"


// CMultiPageExceptionDlg 对话框

class CMultiPageExceptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CMultiPageExceptionDlg)

public:
	CMultiPageExceptionDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMultiPageExceptionDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MULTIPAGEEXCEPTIONDLG };
#endif

	CXListCtrl		m_lcIssuePaper;				//异常的试卷列表
	CXListCtrl		m_lcIssuePics;				//试卷的图片列表

	pMODEL			m_pModel;				//扫描试卷时的校验模板
	pPAPERSINFO		m_pPapers;
	int				m_nCurrentPaperID;
	int				m_nCurrentPicID;
	COLORREF		crPaperOldText, crPaperOldBackground;
	COLORREF		crPicOldText, crPicOldBackground;

	CString			m_strPicZKZH;
	int				m_nPicPagination;		//图片的页码
public:
	void	InitData();
	void	InitUI();
	void	InitCtrlPosition();
	void	ShowPaperDetail(pST_PaperInfo pPaper);
	void	ShowPicDetail(pST_PicInfo pPic);
	void	SetPicInfo(pST_PicInfo pPic);
	BOOL	PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMDblclkListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListPics(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListPaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListPics(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

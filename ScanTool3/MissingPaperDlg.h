#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"
#include "XListCtrl.h"
#include "StudentMgr.h"

// CMissingPaperDlg 对话框

class CMissingPaperDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CMissingPaperDlg)

public:
	CMissingPaperDlg(pPAPERSINFO pPapers, pMODEL pModel, CStudentMgr* pMgr, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMissingPaperDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MISSINGPAPERDLG };
#endif


	CXListCtrl		m_lcMissingZkzh;				//缺失的准考证号列表
	CXListCtrl		m_lcKC;							//扫描试卷所属的考场列表

	CBmpButton		m_bmpBtnClose;

	CString		m_strKD;	//考点
	CString		m_strKC;	//考场
	CString		m_strZW;	//座位


private:
	CStudentMgr*	m_pStudentMgr;
	pPAPERSINFO		m_pPapers;
	pMODEL			m_pModel;
	int				m_nCurrentSelItem;
	COLORREF		crOldText, crOldBackground;

	STUDENT_LIST	m_lAllStudent;

	CFont	fontStatus;
	void	SetFontSize();

	void	InitUI();
	void	InitCtrlPosition();
	void	InitData();
	void	showStudentInfo(int nItem);
	BOOL	PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMHoverListMissingpaper(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListMissingpaper(NMHDR *pNMHDR, LRESULT *pResult);
};

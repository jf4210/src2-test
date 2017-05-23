#pragma once
#include "global.h"
#include "PicShow.h"

// CShowPicDlg 对话框

class CShowPicDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPicDlg)

public:
	CShowPicDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowPicDlg();

// 对话框数据
	enum { IDD = IDD_SHOWPICDLG };

	void	setShowPaper(pST_PaperInfo pPaper);
	void	UpdateUI();
private:
	CTabCtrl		m_tabPicShowCtrl;		//图片显示控件

	std::vector<CPicShow*>	m_vecPicShow;	//存储图片显示窗口指针，有多个模板图片时，对应到不同的tab控件页面
	int						m_nCurrTabSel;	//当前Tab控件选择的页面
	CPicShow*				m_pCurrentPicShow;		//当前图片显示控件
	int						m_nModelPicNums;		//模板图片数，即一份模板有多少图片，对应多少试卷

	pST_PaperInfo			m_pCurrPaper;

	void	InitUI();
	void	InitCtrlPosition();
	void	PaintRecognisedRect(pST_PaperInfo pPaper);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchangeTabPicshow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
};

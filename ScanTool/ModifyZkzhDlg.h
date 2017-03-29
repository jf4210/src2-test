#pragma once

#include "global.h"
#include "PicShow.h"
#include "XListCtrl.h"
#include "StudentMgr.h"
// CModifyZkzhDlg 对话框

class CModifyZkzhDlg : public CDialog
{
	DECLARE_DYNAMIC(CModifyZkzhDlg)

public:
	CModifyZkzhDlg(pMODEL pModel, pPAPERSINFO pPapersInfo, CStudentMgr* pStuMgr, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModifyZkzhDlg();

// 对话框数据
	enum { IDD = IDD_MODIFYZKZHDLG };

	CXListCtrl		m_lcZkzh;				//需要修改的准考证号列表
	CTabCtrl		m_tabPicShowCtrl;		//图片显示控件

	std::vector<CPicShow*>	m_vecPicShow;	//存储图片显示窗口指针，有多个模板图片时，对应到不同的tab控件页面
	int						m_nCurrTabSel;	//当前Tab控件选择的页面
	CPicShow*				m_pCurrentPicShow;		//当前图片显示控件
	int						m_nModelPicNums;		//模板图片数，即一份模板有多少图片，对应多少试卷

	pMODEL			m_pModel;				//扫描试卷时的校验模板
	pPAPERSINFO		m_pPapers;
	pST_PaperInfo	m_pCurrentShowPaper;
	int				m_nCurrentSelItem;		//当前选择的项

	CString			m_strCurZkzh;
	COLORREF		crOldText, crOldBackground;

	int				m_nSearchType;		//搜索类型，1-按姓名搜索，2-按准考证号
	CString			m_strSearchKey;		//搜索关键字
	CListCtrl		m_lcBmk;			//报名库列表控件
private:
	CStudentMgr*	m_pStudentMgr;
private:
	void	InitUI();
	void	InitTab();
	void	InitCtrlPosition();
	void	InitData();

	LRESULT RoiRBtnUp(WPARAM wParam, LPARAM lParam);
	void	LeftRotate();
	void	RightRotate();

	void	ShowPaperByItem(int nItem);
	void	ShowPaperZkzhPosition(pST_PaperInfo pPaper);
	BOOL	PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	afx_msg LRESULT OnEditEnd(WPARAM, LPARAM);
	afx_msg LRESULT OnLBtnDownEdit(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchangeTabZkzhpic(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverListZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedRadioSearchzkzh();
	afx_msg void OnBnClickedRadioSearchname();
	afx_msg void OnBnClickedBtnSearch();
	afx_msg void OnNMDblclkListZkzhsearchresult(NMHDR *pNMHDR, LRESULT *pResult);
};

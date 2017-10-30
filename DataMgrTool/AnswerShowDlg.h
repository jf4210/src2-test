#pragma once
#include "XListCtrl.h"
#include "DMTDef.h"

// CAnswerShowDlg 对话框

class CAnswerShowDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAnswerShowDlg)

public:
	CAnswerShowDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAnswerShowDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANSWERSHOWDLG };
#endif


	CXListCtrl	m_lAnswerListCtrl;		//试卷袋列表
	void	InitData(pST_PaperInfo pPaper);
	void	InitModel(pMODEL pModel);
	void	SetShowType(int nType);	//显示类型：0-不显示，1-无怀疑, 2-显示怀疑，3-显示无怀疑和怀疑，4-显示空，5-显示无怀疑和空，6-显示怀疑和空, 7-显示所有
private:
	void	InitCtrlPosition();
	void	InitUI();

	void	SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//设置列表高亮显示
	void	UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//取消列表高亮显示
	COLORREF		crOldText, crOldBackground;

	CButton		m_btnChkRight;
	CButton		m_btnChkDoubt;
	CButton		m_btnChkNull;
	void	setCheckStatus(int nStatus);
	int		getCheckStatus();

	int		m_nCurrentItem;
	pMODEL	m_pModel;
	int		m_nShowType;	//显示类型：0-不显示，1-无怀疑, 2-显示怀疑，3-显示无怀疑和怀疑，4-显示空，5-显示无怀疑和空，6-显示怀疑和空, 7-显示所有
	pST_PaperInfo m_pShowPaper;

//	CDialog*	m_pShowPicDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMHoverListAnswer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListAnswer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedChkRight();
	afx_msg void OnBnClickedChkDoubt();
	afx_msg void OnBnClickedChkNull();
};

#pragma once
#include "XListCtrl.h"
#include "global.h"

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

private:
	void	InitCtrlPosition();
	void	InitUI();

	void	SetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//设置列表高亮显示
	void	UnSetListCtrlHighLightShow(CXListCtrl& lCtrl, int nItem);		//取消列表高亮显示
	COLORREF		crOldText, crOldBackground;

	int		m_nCurrentItem;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMHoverListAnswer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListAnswer(NMHDR *pNMHDR, LRESULT *pResult);
};

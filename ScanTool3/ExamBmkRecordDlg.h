#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ctrl/skinscrollwnd.h"
#include "BmpButton.h"
#include "ComboBoxExt.h"

// CScanRecordDlg 对话框

class CExamBmkRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CExamBmkRecordDlg)

public:
	CExamBmkRecordDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CExamBmkRecordDlg();

// 对话框数据
	enum { IDD = IDD_EXAMBMKRECORDDLG };

	void	ResetBmkList();
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);

	void	GetBmkSearchResult();

	CBmpButton		m_bmpBtnExport;
	CXListCtrl		m_lcBmk;
	HBITMAP			m_bitmap_scrollbar;
	
	CComboBoxExt	m_comboSubject;		//科目下拉列表
	CComboBoxExt	m_comboScanStatus;	//扫描状态过滤

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangeComboBmkSubject();
	afx_msg void OnCbnSelchangeComboBmkScanstatus();
	afx_msg void OnBnClickedBtnExambmkExportscan();
};

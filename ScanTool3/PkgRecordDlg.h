#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ctrl/skinscrollwnd.h"

// CPkgRecordDlg 对话框

class CPkgRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CPkgRecordDlg)

public:
	CPkgRecordDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPkgRecordDlg();

// 对话框数据
	enum { IDD = IDD_PKGRECORDDLG };

	void	UpdateChildDlg();
private:
	void	InitUI();
	void	InitCtrlPosition();

	CXListCtrl	m_lcPkg;
	HBITMAP			m_bitmap_scrollbar;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ctrl/skinscrollwnd.h"

// CPkgRecordDlg �Ի���

class CPkgRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CPkgRecordDlg)

public:
	CPkgRecordDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CPkgRecordDlg();

// �Ի�������
	enum { IDD = IDD_PKGRECORDDLG };

	void	UpdateChildDlg();
private:
	void	InitUI();
	void	InitCtrlPosition();

	CXListCtrl	m_lcPkg;
	HBITMAP			m_bitmap_scrollbar;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

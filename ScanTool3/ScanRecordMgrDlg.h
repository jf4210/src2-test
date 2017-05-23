#pragma once
#include "PkgRecordDlg.h"
#include "ScanBmkRecordDlg.h"

// CScanRecordMgrDlg 对话框

class CScanRecordMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanRecordMgrDlg)

public:
	CScanRecordMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanRecordMgrDlg();

// 对话框数据
	enum { IDD = IDD_SCANRECORDMGRDLG };

private:
	void	InitUI();
	void	InitCtrlPosition();
	void	ReleaseData();

	CPkgRecordDlg*		m_pPkgRecordDlg;
	CScanBmkRecordDlg*	m_pBmkRecordDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnBmkrecord();
	afx_msg void OnBnClickedBtnPkgrecord();
	afx_msg void OnBnClickedBtnRebackscan();
};

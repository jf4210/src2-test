#pragma once
#include "ShowModelInfoDlg.h"

// CScanModleMgrDlg 对话框

class CScanModleMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanModleMgrDlg)

public:
	CScanModleMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanModleMgrDlg();

// 对话框数据
	enum { IDD = IDD_SCANMODLEMGRDLG };

private:
	CShowModelInfoDlg*	m_pShowModelInfoDlg;
	CListCtrl			m_ModelListCtrl;

private:
	void InitUI();
	void InitCtrlPosition();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnBnClickedBtnDlmodel();
	afx_msg void OnNMDblclkListModel(NMHDR *pNMHDR, LRESULT *pResult);
};

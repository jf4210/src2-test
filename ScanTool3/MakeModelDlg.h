#pragma once
#include "global.h"

// CMakeModelDlg 对话框

class CMakeModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CMakeModelDlg)

public:
	CMakeModelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMakeModelDlg();

// 对话框数据
	enum { IDD = IDD_MAKEMODELDLG };
private:
	void	InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


// sendFileTestDlg.h : 头文件
//

#pragma once
#include "global.h"
#include "SendFileThread.h"


// CsendFileTestDlg 对话框
class CsendFileTestDlg : public CDialogEx
{
// 构造
public:
	CsendFileTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SENDFILETEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	std::vector<CSendFileThread*> m_vecRecogThreadObj;
	Poco::Thread* m_pRecogThread;
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSendfile();
	afx_msg void OnDestroy();
};

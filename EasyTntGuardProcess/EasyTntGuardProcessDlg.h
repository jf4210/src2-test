
// EasyTntGuardProcessDlg.h : 头文件
//

#pragma once


// CEasyTntGuardProcessDlg 对话框
class CEasyTntGuardProcessDlg : public CDialogEx
{
// 构造
public:
	CEasyTntGuardProcessDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_EASYTNTGUARDPROCESS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	HANDLE m_hThread;
private:
	void InitConf();
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
	afx_msg void OnDestroy();
};

#pragma once


// CDataCheckDlg 对话框

class CDataCheckDlg : public CDialog
{
	DECLARE_DYNAMIC(CDataCheckDlg)

public:
	CDataCheckDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDataCheckDlg();

// 对话框数据
	enum { IDD = IDD_DATACHECKDLG };

private:
	void	InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
};

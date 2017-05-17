#pragma once


// CScanRecordDlg 对话框

class CScanRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanRecordDlg)

public:
	CScanRecordDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanRecordDlg();

// 对话框数据
	enum { IDD = IDD_SCANRECORDDLG };

private:
	void	InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
};

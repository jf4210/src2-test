#pragma once


// CDataCheckDlg �Ի���

class CDataCheckDlg : public CDialog
{
	DECLARE_DYNAMIC(CDataCheckDlg)

public:
	CDataCheckDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDataCheckDlg();

// �Ի�������
	enum { IDD = IDD_DATACHECKDLG };

private:
	void	InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
};

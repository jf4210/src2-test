#pragma once


// CScanRecordDlg �Ի���

class CScanRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanRecordDlg)

public:
	CScanRecordDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanRecordDlg();

// �Ի�������
	enum { IDD = IDD_SCANRECORDDLG };

private:
	void	InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
};

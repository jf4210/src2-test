
// EasyTntGuardProcessDlg.h : ͷ�ļ�
//

#pragma once


// CEasyTntGuardProcessDlg �Ի���
class CEasyTntGuardProcessDlg : public CDialogEx
{
// ����
public:
	CEasyTntGuardProcessDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_EASYTNTGUARDPROCESS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	HANDLE m_hThread;
private:
	void InitConf();
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
};

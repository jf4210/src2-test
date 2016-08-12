
// sendFileTestDlg.h : ͷ�ļ�
//

#pragma once
#include "global.h"
#include "SendFileThread.h"
#include "MulticastServer.h"

// CsendFileTestDlg �Ի���
class CsendFileTestDlg : public CDialogEx
{
// ����
public:
	CsendFileTestDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SENDFILETEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	std::vector<CSendFileThread*> m_vecRecogThreadObj;
	Poco::Thread* m_pRecogThread;

	MulticastServer*	m_pMulticastServer;

	CMFCEditBrowseCtrl	m_ctrlSendFile;

	int		m_nThreads;
	int		m_nFileTasks;
	std::string m_strServerIP;
	int			m_nServerPort;
	std::string m_strMulticastIP;
	int			m_nMulticastPort;
private:
	void InitConfig();
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
	afx_msg void OnBnClickedBtnSendfile();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnSendtest();
	afx_msg void OnBnClickedBtnMulticast();
};

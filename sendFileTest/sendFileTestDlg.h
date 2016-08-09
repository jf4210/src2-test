
// sendFileTestDlg.h : ͷ�ļ�
//

#pragma once
#include "global.h"
#include "SendFileThread.h"


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
};

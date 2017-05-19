#pragma once
#include "global.h"

// CWaitDownloadDlg �Ի���

class CWaitDownloadDlg : public CDialog
{
	DECLARE_DYNAMIC(CWaitDownloadDlg)

public:
	CWaitDownloadDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CWaitDownloadDlg();

// �Ի�������
	enum { IDD = IDD_WAITDOWNLOADDLG };

private:
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

private:
	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ

	CString		m_strWaitInfo;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

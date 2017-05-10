#pragma once
#include "global.h"
#include "ScanDlg.h"

// CScanMgrDlg �Ի���

class CScanMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanMgrDlg)

public:
	CScanMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanMgrDlg();

// �Ի�������
	enum { IDD = IDD_SCANMGRDLG };

	void	UpdateInfo();
private:
	void	InitCtrlPosition();
	void	InitChildDlg();
	void	ReleaseDlg();
private:
	CRect		m_rtChildDlg;		//��ʾ�Ӵ��ڵ�����
	CScanDlg*	m_pScanDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

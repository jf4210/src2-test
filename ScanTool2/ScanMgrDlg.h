#pragma once
#include "global.h"
#include "ScanDlg.h"
#include "DataCheckDlg.h"
#include "ScanRecordDlg.h"

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
	void	SearchModel();	//��������ģ��
private:
	void	InitCtrlPosition();
	void	InitChildDlg();
	void	ReleaseDlg();
private:
	CRect		m_rtChildDlg;		//��ʾ�Ӵ��ڵ�����
	CScanDlg*	m_pScanDlg;
	CDataCheckDlg*	m_pDataCheckDlg;
	CScanRecordDlg* m_pScanRecordDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnDatacheck();
	afx_msg void OnBnClickedBtnScanrecord();
};

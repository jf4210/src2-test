#pragma once
#include "global.h"
#include "ScanDlg.h"
#include "WaitDownloadDlg.h"
// #include "DataCheckDlg.h"
// #include "ScanRecordDlg.h"
#include "ScanThread.h"

// CScanMgrDlg �Ի���

class CScanMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanMgrDlg)

public:
	CScanMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanMgrDlg();

// �Ի�������
	enum { IDD = IDD_SCANMGRDLG };

	CScanThread		m_scanThread;

	void	ShowDlg();
	void	UpdateInfo();
	void	SearchModel();		//��������ģ��
	bool DownLoadModel();	//����ģ��
private:
	void	InitData();
	void	InitCtrlPosition();
	void	InitChildDlg();
	void	ReleaseDlg();
	void	SetFontSize(int nSize);
	void	DrawBorder(CDC *pDC);	//���Ʊ߿���
	void	ShowChildDlg(int n);
private:
	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ

	CRect		m_rtChildDlg;		//��ʾ�Ӵ��ڵ�����
	CScanDlg*	m_pScanDlg;
	CWaitDownloadDlg*	m_pWaitDownloadDlg;

	CString			m_strExamName;
	CComboBoxExt	m_comboSubject;
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
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

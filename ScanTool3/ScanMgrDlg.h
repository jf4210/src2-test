#pragma once
#include "global.h"
#include "ScanDlg.h"
#include "WaitDownloadDlg.h"
#include "ScanProcessDlg.h"
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
	bool	SearchModel();		//��������ģ��
	bool	DownLoadModel();	//����ģ��
	void	ShowChildDlg(int n);
	void	ResetChildDlg();	//�Ӵ�������
	void	UpdateChildDlgInfo();	//�����Ӵ�����Ϣ
private:
	void	InitData();
	void	InitCtrlPosition();
	void	InitChildDlg();
	void	ReleaseDlg();
	void	SetFontSize(int nSize);
	void	DrawBorder(CDC *pDC);	//���Ʊ߿���

	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);

private:
	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ

	CRect		m_rtChildDlg;		//��ʾ�Ӵ��ڵ�����
	CWaitDownloadDlg*	m_pWaitDownloadDlg;
	CScanDlg*			m_pScanDlg;
	CScanProcessDlg*	m_pScanProcessDlg;

	CString			m_strExamName;
	CComboBoxExt		m_comboSubject;
//	CComboBox		m_comboSubject;
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
	afx_msg void OnBnClickedBtnChangeexam();
};

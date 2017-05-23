#pragma once
#include "PkgRecordDlg.h"
#include "ScanBmkRecordDlg.h"

// CScanRecordMgrDlg �Ի���

class CScanRecordMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanRecordMgrDlg)

public:
	CScanRecordMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanRecordMgrDlg();

// �Ի�������
	enum { IDD = IDD_SCANRECORDMGRDLG };

private:
	void	InitUI();
	void	InitCtrlPosition();
	void	ReleaseData();

	CPkgRecordDlg*		m_pPkgRecordDlg;
	CScanBmkRecordDlg*	m_pBmkRecordDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnBmkrecord();
	afx_msg void OnBnClickedBtnPkgrecord();
	afx_msg void OnBnClickedBtnRebackscan();
};

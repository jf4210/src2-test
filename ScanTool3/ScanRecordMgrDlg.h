#pragma once
#include "PkgRecordDlg.h"
#include "ScanBmkRecordDlg.h"
#include "ExamBmkRecordDlg.h"
#include "BmpButton.h"

// CScanRecordMgrDlg �Ի���

class CScanRecordMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanRecordMgrDlg)

public:
	CScanRecordMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanRecordMgrDlg();

// �Ի�������
	enum { IDD = IDD_SCANRECORDMGRDLG };
	
	void	SetReBackDlg(int nFlag);		//�㷵�ذ�ťʱ���Ƿ����Ǹ����ڣ�Ŀǰ��������ڣ�һ���Ծ�����ڣ�����ɨ����̴��ڣ��������ط������Խ��뵽��ǰ���ڣ���Ҫ��¼���ص�λ��   1--�����Ծ�����ڣ�2--����ɨ����̴���
	void	UpdateChildDlg();
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	ReleaseData();
	void	DrawBorder(CDC *pDC);

	CPkgRecordDlg*		m_pPkgRecordDlg;
	CScanBmkRecordDlg*	m_pBmkRecordDlg;
	CExamBmkRecordDlg*	m_pExamBmkRecordDlg;
	int		m_nReturnFlag;		//1--�����Ծ�����ڣ�2--����ɨ����̴���

	CBmpButton		m_bmpBtnBmk;
	CBmpButton		m_bmpBtnPkg;
	CBmpButton		m_bmpBtnReturn;

	CRect			m_rtChildDlg;
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

#pragma once
#include "XListCtrl.h"

// CScanProcessDlg �Ի���

class CScanProcessDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanProcessDlg)

public:
	CScanProcessDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanProcessDlg();

// �Ի�������
	enum { IDD = IDD_SCANPROCESSDLG };

private:
	void	InitUI();
	void	InitCtrlPosition();

private:
	CXListCtrl	m_lcPicture;
	CRect		m_rtChildDlg;	//�ӿؼ�λ��
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

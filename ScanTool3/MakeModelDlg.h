#pragma once
#include "global.h"

// CMakeModelDlg �Ի���

class CMakeModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CMakeModelDlg)

public:
	CMakeModelDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMakeModelDlg();

// �Ի�������
	enum { IDD = IDD_MAKEMODELDLG };
private:
	void	InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

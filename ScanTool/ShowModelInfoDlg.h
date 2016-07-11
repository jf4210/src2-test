#pragma once
#include "global.h"

// CShowModelInfoDlg �Ի���

class CShowModelInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowModelInfoDlg)

public:
	CShowModelInfoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShowModelInfoDlg();

// �Ի�������
	enum { IDD = IDD_SHOWMODELINFODLG };

public:
	void ShowModelInfo(pMODEL pModel, int nFlag = 0);

private:
	void InitUI();
	void InitCtrlPosition();
	void SetFontSize(int nSize);

	CString			m_strModelName;
	CString			m_strModelDesc;

	int				m_nStatusSize;			//�����С
	CFont			m_fontStatus;			//����
	COLORREF		m_colorStatus;			//������ɫ
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

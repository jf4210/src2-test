#pragma once
#include "global.h"

// CRecogInfoDlg �Ի���

class CRecogInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecogInfoDlg)

public:
	CRecogInfoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRecogInfoDlg();

// �Ի�������
	enum { IDD = IDD_RECOGINFODLG };

public:
	pRECTINFO	m_pCurRectInfo;
	CString		m_strCPTypeName;		//У����������
	int			m_nThresholdVal;		//У����ʶ��ֵ
	float		m_fThresholdValPercent;	//У���ﵽ��ֵ�ı���

	void		ShowDetailRectInfo(pRECTINFO	pCurRectInfo);

private:
	void		InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSaverecoginfo();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

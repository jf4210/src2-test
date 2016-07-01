#pragma once
#include "ShowModelInfoDlg.h"

// CScanModleMgrDlg �Ի���

class CScanModleMgrDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanModleMgrDlg)

public:
	CScanModleMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanModleMgrDlg();

// �Ի�������
	enum { IDD = IDD_SCANMODLEMGRDLG };

private:
	CShowModelInfoDlg*	m_pShowModelInfoDlg;
	CListCtrl			m_ModelListCtrl;
	pMODEL				m_pModel;
	std::vector<pMODEL>	m_vecModel;
	int					m_nCurModelItem;
private:
	void InitUI();
	void InitCtrlPosition();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnBnClickedBtnDlmodel();
	afx_msg void OnNMDblclkListModel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnDelmodel();
	afx_msg void OnBnClickedBtnAddmodel();
	afx_msg void OnNMHoverListModel(NMHDR *pNMHDR, LRESULT *pResult);
};

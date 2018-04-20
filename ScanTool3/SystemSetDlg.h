#pragma once
#include "modelInfo.h"
#include "TipBaseDlg.h"
#include "BmpButton.h"
#include "BaseTabDlg.h"
#include "ctrl/XPGroupBox.h"


// CSystemSetDlg �Ի���

class CSystemSetDlg : public CBaseTabDlg
{
	DECLARE_DYNAMIC(CSystemSetDlg)

public:
	CSystemSetDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSystemSetDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYSTEMSETDLG };
#endif

	int		m_nChkLostCorner;
	int		m_nUseRectFit_SN;
	int		m_nUseRectFit_Omr;

	CButton		m_chkLostCorner;	//ȱ�Ǽ��
	CButton		m_chkUseRectFit_SN;	//ʹ�þ������
	CButton		m_chkUseRectFit_Omr;
	int		m_nRectFitInterval_SN;
	int		m_nRectFitInterval_Omr;


	void	InitData(AdvanceParam& stParam);
	BOOL	SaveParamData(AdvanceParam& stParam);
private:
	void	InitUI();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedChkLostcorner();
	afx_msg void OnBnClickedChkUserectfit();
	afx_msg void OnBnClickedChkUserectfitOmr();
	afx_msg void OnDeltaposSpinRectfitZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinRectfitOmr(NMHDR *pNMHDR, LRESULT *pResult);
};

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
	int		m_nUseRectFit;

	CButton		m_chkLostCorner;	//ȱ�Ǽ��
	CButton		m_chkUseRectFit;	//ʹ�þ������
	int		m_nRectFitInterval;


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
};

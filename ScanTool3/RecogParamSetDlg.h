#pragma once
#include "modelInfo.h"
#include "BaseTabDlg.h"
#include "ctrl/XPGroupBox.h"
// CRecogParamSetDlg 对话框

class CRecogParamSetDlg : public CBaseTabDlg
{
	DECLARE_DYNAMIC(CRecogParamSetDlg)

public:
	CRecogParamSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRecogParamSetDlg();

// 对话框数据
	enum { IDD = IDD_RECOGPARAMSETDLG };

	CXPGroupBox		m_groupSensitivity;
	CXPGroupBox		m_groupThreshold;
	CXPGroupBox		m_groupOther;

	int		m_nThresholdZkzh;
	int		m_nThresholdOmr;
	int		m_nDefThresholdZkzh;	//默认考号二值化阀值
	int		m_nDefThresholdOmr;		//默认Omr二值化阀值

	int m_nCurrentFixThreshold;
	int m_nCurrentGrayThreshold;
	int m_nDefFixThreshold;
	int m_nDefGrayThreshold;

	int m_nPersentFix;
	int m_nPersentGray;
	int m_nPersentZkzh;
	int m_nPersentOmr;
	int m_nDefPersentFix;
	int m_nDefPersentGray;
	int m_nDefPersentZkzh;
	int m_nDefPersentOmr;

	CSpinButtonCtrl m_Spin_CharacterAnchorPt;
	int		m_nCharacterAnchorPt;		//文字定点个数
	int		m_nDefCharacterAnchorPt;	//默认文字定点个数
	int		m_nCharacterConfidence;		//文字识别要求的信心值
	int		m_nDefCharacterConfidence;	//默认文字识别要求的信心值

	CSpinButtonCtrl m_Spin_Zkzh;
	CSpinButtonCtrl m_Spin_Omr;
	int			m_nSensitiveZkzh;
	int			m_nSensitiveOmr;
	int			m_nDefSensitiveZkzh;	//默认考号灵敏度
	int			m_nDefSensitiveOmr;		//默认Omr灵敏度

	void	InitData(AdvanceParam& stParam);
	BOOL	SaveParamData(AdvanceParam& stParam);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnDefParam();
	afx_msg void OnDeltaposSpinZkzh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinOmr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinCharacteranchorpt(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

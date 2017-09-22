#pragma once
#include "modelInfo.h"
#include "BaseTabDlg.h"
#include "ctrl/XPGroupBox.h"
// CRecogParamSetDlg �Ի���

class CRecogParamSetDlg : public CBaseTabDlg
{
	DECLARE_DYNAMIC(CRecogParamSetDlg)

public:
	CRecogParamSetDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRecogParamSetDlg();

// �Ի�������
	enum { IDD = IDD_RECOGPARAMSETDLG };

	CXPGroupBox		m_groupSensitivity;
	CXPGroupBox		m_groupThreshold;
	CXPGroupBox		m_groupOther;

	int		m_nThresholdZkzh;
	int		m_nThresholdOmr;
	int		m_nDefThresholdZkzh;	//Ĭ�Ͽ��Ŷ�ֵ����ֵ
	int		m_nDefThresholdOmr;		//Ĭ��Omr��ֵ����ֵ

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
	int		m_nCharacterAnchorPt;		//���ֶ������
	int		m_nDefCharacterAnchorPt;	//Ĭ�����ֶ������
	int		m_nCharacterConfidence;		//����ʶ��Ҫ�������ֵ
	int		m_nDefCharacterConfidence;	//Ĭ������ʶ��Ҫ�������ֵ

	CSpinButtonCtrl m_Spin_Zkzh;
	CSpinButtonCtrl m_Spin_Omr;
	int			m_nSensitiveZkzh;
	int			m_nSensitiveOmr;
	int			m_nDefSensitiveZkzh;	//Ĭ�Ͽ���������
	int			m_nDefSensitiveOmr;		//Ĭ��Omr������

	void	InitData(AdvanceParam& stParam);
	BOOL	SaveParamData(AdvanceParam& stParam);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
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

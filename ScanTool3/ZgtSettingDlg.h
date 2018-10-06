#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"


// CZgtSettingDlg 对话框

class CZgtSettingDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CZgtSettingDlg)

public:
	CZgtSettingDlg(int nStartTH = 0, int nQuestionType = 1, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CZgtSettingDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ZGTSETTINGDLG };
#endif

	CBmpButton		m_bmpBtnClose;
	CButton			m_btnRadio_TK;
	CButton			m_btnRadio_JDT;
	CButton			m_btnRadio_YYZW;
	CButton			m_btnRadio_YWZW;
	CButton			m_btnRadio_XZT;
public:
	float	m_fStartTH;	//起始题号
	int		m_nQuestionType;	//题目类型，1-填空题，2-解答题，3-英语作文，4-语文作文，5-选做题
private:
	void	InitShowQuestionType(int nType);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadioTk();
	afx_msg void OnBnClickedRadioJdt();
	afx_msg void OnBnClickedRadioYyzw();
	afx_msg void OnBnClickedRadioYwzw();
	afx_msg void OnBnClickedRadioXzt();
	afx_msg void OnStnClickedStaticTk();
	afx_msg void OnStnClickedStaticJdt();
	afx_msg void OnStnClickedStaticYyzw();
	afx_msg void OnStnClickedStaticYwzw();
	afx_msg void OnStnClickedStaticXzt();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnClose();
};

#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"


// CZgtSettingDlg �Ի���

class CZgtSettingDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CZgtSettingDlg)

public:
	CZgtSettingDlg(int nStartTH = 0, int nQuestionType = 1, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CZgtSettingDlg();

// �Ի�������
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
	float	m_fStartTH;	//��ʼ���
	int		m_nQuestionType;	//��Ŀ���ͣ�1-����⣬2-����⣬3-Ӣ�����ģ�4-�������ģ�5-ѡ����
private:
	void	InitShowQuestionType(int nType);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
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

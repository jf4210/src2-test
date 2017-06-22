#pragma once
#include "CV_picture.h"
#include "global.h"
// COmrInfoDlg �Ի���

class COmrInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(COmrInfoDlg)

public:
	COmrInfoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~COmrInfoDlg();

// �Ի�������
	enum { IDD = IDD_OMRINFODLG };
public:
	CV_picture  m_picOmrShow;	//Ч��ͼ����
	cv::Mat		m_src_img;

	CButton		m_radioTX_S;
	CButton		m_radioTX_M;

	CButton		m_radioTH_H;
	CButton		m_radioTH_V;

	CButton		m_radioXX_H;
	CButton		m_radioXX_V;

	CButton		m_radioDirectZX;
	CButton		m_radioDirectFX;
	
	BOOL		m_bSingle;		//���ͣ��Ƿ�ѡ��
	int			m_nCurrentOmrVal;	//������š�ѡ�������Ϲ��ɵ�6λ�����Ƶ�ֵ����ʶ8�����
	void		ShowUI(int nOmrVal, int nSingleFlag);
private:
	int			m_nTHSel;		//���ѡ��	0-����1-����
	int			m_nXXSel;		//ѡ��ѡ��	0-����1-����
	int			m_nDirectSel;	//����ѡ��	0-����1-����

	bool		m_bShowFist;

	void		InitCtrlPosition();
	void		ShowOmrPic();
	void		InitUI();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedRadioThH();
	afx_msg void OnBnClickedRadioThV();
	afx_msg void OnBnClickedRadioXxH();
	afx_msg void OnBnClickedRadioXxV();
	afx_msg void OnBnClickedRadioDirectZx();
	afx_msg void OnBnClickedRadioDirectFx();
	afx_msg void OnBnClickedRadioTxSingle();
	afx_msg void OnBnClickedRadioTxMulti();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnStnClickedStaticThH();
	afx_msg void OnStnClickedStaticThV();
	afx_msg void OnStnClickedStaticXxH();
	afx_msg void OnStnClickedStaticXxV();
	afx_msg void OnStnClickedStaticDirectZx();
	afx_msg void OnStnClickedStaticDirectFx();
};

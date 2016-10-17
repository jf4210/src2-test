#pragma once
#include "CV_picture.h"
#include "global.h"

// CSNInfoSetDlg �Ի���

class CSNInfoSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CSNInfoSetDlg)

public:
	CSNInfoSetDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSNInfoSetDlg();

// �Ի�������
	enum { IDD = IDD_SNINFOSETDLG };
public:
	CV_picture  m_picSNShow;	//Ч��ͼ����
	cv::Mat		m_src_img;
	
	CButton		m_chkUseBarCode;
	CButton		m_radioSN_H;
	CButton		m_radioSN_V;

	CButton		m_radioADD_Z;
	CButton		m_radioADD_F;
	int			m_nCurrentSNVal;	//����ѡ�������Ϲ��ɵ�4λ�����Ƶ�ֵ����ʶ4�����
	int			m_nZkzhType;		//׼��֤��ʶ������: 1 - OMR��2 - ����or��ά��
	void		InitType(int nType);
	void		ShowUI(int nSnVal);
private:
	int			m_nSNSel;		//SNѡ��	0-����1-����
	int			m_nOptionSel;	//��ŷ���ѡ��	0-�����ң�1-���ҵ���

	bool		m_bShowFist;

	void		InitCtrlPosition();
	void		ShowSNPic();
	void		InitUI();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedRadioSnH();
	afx_msg void OnBnClickedRadioSnV();
	afx_msg void OnBnClickedRadioOption1();
	afx_msg void OnBnClickedRadioOption2();
	afx_msg void OnBnClickedCheckUseBarcode();
};

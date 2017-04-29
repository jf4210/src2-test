#pragma once
#include "DMTDef.h"

// CRecogParamDlg �Ի���

class CRecogParamDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecogParamDlg)

public:
	CRecogParamDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CRecogParamDlg();

// �Ի�������
	enum { IDD = IDD_RECOGPARAMDLG };

public:
	int		m_nChkSN;
	int		m_nChkOmr;
	int		m_nChkElectOmr;
	int		m_nNoRecogVal;		//�Ծ����ʶ��ֵ��omr���� + omr�� + SN���������ڴ˷�ֵ�Ž�����ʶ��
	CString		m_strEzsAddr;

	int		m_nHandleResult;	//0-ʹ��ѹ������ʽ��1-ֱ�ӷ��ͽ����ezs��2-�����ͽ��Ҳ��ѹ���Ծ��������ģʽ����ʶ����
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChkSn();
	afx_msg void OnBnClickedChkOmr();
	afx_msg void OnBnClickedChkElecomr();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadioCompresspkg();
	afx_msg void OnBnClickedRadioSendezs();
	afx_msg void OnBnClickedRadioNocompress();
};

#pragma once


// CTHSetDlg �Ի���

class CTHSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CTHSetDlg)

public:
	CTHSetDlg(int nStartTH = 0, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CTHSetDlg();

// �Ի�������
	enum { IDD = IDD_THSETDLG };

public:
	int		m_nStartTH;	//��ʼ���
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

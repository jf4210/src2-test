#pragma once


// CScanerInfoDlg �Ի���

class CScanerInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanerInfoDlg)

public:
	CScanerInfoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanerInfoDlg();

// �Ի�������
	enum { IDD = IDD_SCANERINFODLG };

public:
	void			setShowInfo(CString& strUser, CString& strNickName);

private:
	CString		m_strUser;
	CString		m_strNickName;

	int				m_nStatusSize;			//�����С
	CFont			m_fontStatus;			//����
	CFont			m_fontUnLogin;
	COLORREF		m_colorStatus;			//������ɫ

	void			SetFontSize(int nSize);
	void			InitCtrlPosition();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

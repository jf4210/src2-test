
// ScanToolStartDlg.h : ͷ�ļ�
//

#pragma once


// CScanToolStartDlg �Ի���
class CScanToolStartDlg : public CDialogEx
{
// ����
public:
	CScanToolStartDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SCANTOOLSTART_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


	void InitCtrlPosition();

public:
	int				m_nStatusSize;			//�����С
	CFont			m_fontStatus;			//����
	COLORREF		m_colorStatus;			//������ɫ

	CString			m_strAppPath;

	void			SetFontSize(int nSize);
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSimple();
	afx_msg void OnBnClickedBtnSpecialty();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

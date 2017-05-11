#pragma once
#include "CommonTWAIN.h"
#include "TwainApp.h"
#include "ComboBoxExt.h"

// CScanDlg �Ի���

class CScanDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanDlg)

public:
	CScanDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanDlg();

// �Ի�������
	enum { IDD = IDD_SCANDLG };

	void	InitScanner();
	void	UpdateInfo();
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	
private:
	CString		m_strExamName;
	CString		m_strSubjectName;


	TwainApp*		_pTWAINApp;
	int				m_nCurrentScanCount;	//��ǰɨ����Ҫɨ���Ծ�����
	int				m_nModelPicNums;		//ģ��ͼƬ������һ��ģ���ж���ͼƬ����Ӧ�����Ծ�
	std::string		m_strCurrPicSavePath;		//gb2312

	CComboBoxExt	m_comboScanner;
	CComboBoxExt	m_comboDuplex;

	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	COLORREF		m_colorStatus;			//״̬��������ɫ
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnChangeexam();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnScan();
};

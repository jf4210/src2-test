#pragma once
#include "global.h"
#include "CommonTWAIN.h"
#include "TwainApp.h"
#include "ComboBoxExt.h"
#include "BmpButton.h"

// CScanDlg �Ի���

class CScanDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanDlg)

public:
	CScanDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanDlg();

// �Ի�������
	enum { IDD = IDD_SCANDLG };

	void	SetScanSrcInfo(std::vector<CString>& vec);
	void	UpdateInfo();
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	
private:
	int				m_nCurrentScanCount;	//��ǰɨ����Ҫɨ���Ծ�����
	int				m_nModelPicNums;		//ģ��ͼƬ������һ��ģ���ж���ͼƬ����Ӧ�����Ծ�
	std::string		m_strCurrPicSavePath;		//gb2312
	bool			m_bAdvancedScan;		//�Ƿ���ʾ�߼�ɨ�����

	CComboBoxExt	m_comboScanner;
	CComboBoxExt	m_comboDuplex;

	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	CFont			m_fontScanBtn;			//ɨ�谴ť����
	COLORREF		m_colorStatus;			//״̬��������ɫ

	CBitmap			m_bmpBk;
	CBmpButton		m_bmpBtnScan;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnScan();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedChkAdvancescan();
};

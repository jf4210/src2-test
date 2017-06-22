#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"
#include "ComboBoxExt.h"
#include "global.h"
#include "CommonTWAIN.h"
#include "TwainApp.h"
#include "ScanThread.h"

// CScanModelPaperDlg �Ի���

class CScanModelPaperDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CScanModelPaperDlg)

public:
	CScanModelPaperDlg(void* pDlg, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CScanModelPaperDlg();

// �Ի�������
	enum { IDD = IDD_SCANMODELPAPERDLG };

	CString			m_strSavePath;
public:
	void	SetScanSrc(std::vector<CString>& vec);
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);

	//----------------------
//	TwainApp*		_pTWAINApp;
	std::vector<CString> m_vecScanSrc;
//	void	InitScanner();
	void	SetScanSrcInfo(std::vector<CString>& vec);
	//----------------------

	void*			m_pNotifyDlg;
	bool			m_bAdvancedScan;

	int				m_nStatusSize;			//״̬�������С
	CFont			m_fontStatus;			//״̬������
	CFont			m_fontBtn;
	COLORREF		m_colorStatus;			//״̬��������ɫ

	CBmpButton		m_bmpBtnClose;
	CBmpButton		m_bmpBtnScan;
	CBmpButton		m_bmpBtnBrowser;
	CComboBoxExt	m_comboScanSrc;
	CComboBoxExt	m_comboDuplex;
	CComboBoxExt	m_comboPaperSize;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnScanmodelClose();
	afx_msg void OnBnClickedChkMakemodelAdvance();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnMakemodelScan();
};

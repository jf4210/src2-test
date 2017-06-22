#pragma once
#include "TipBaseDlg.h"
#include "BmpButton.h"
#include "ComboBoxExt.h"
#include "global.h"
#include "CommonTWAIN.h"
#include "TwainApp.h"
#include "ScanThread.h"

// CScanModelPaperDlg 对话框

class CScanModelPaperDlg : public CTipBaseDlg
{
	DECLARE_DYNAMIC(CScanModelPaperDlg)

public:
	CScanModelPaperDlg(void* pDlg, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanModelPaperDlg();

// 对话框数据
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

	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	CFont			m_fontBtn;
	COLORREF		m_colorStatus;			//状态栏字体颜色

	CBmpButton		m_bmpBtnClose;
	CBmpButton		m_bmpBtnScan;
	CBmpButton		m_bmpBtnBrowser;
	CComboBoxExt	m_comboScanSrc;
	CComboBoxExt	m_comboDuplex;
	CComboBoxExt	m_comboPaperSize;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnScanmodelClose();
	afx_msg void OnBnClickedChkMakemodelAdvance();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnMakemodelScan();
};

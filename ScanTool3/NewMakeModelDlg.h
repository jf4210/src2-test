#pragma once
#include "global.h"
#include "BmpButton.h"
#include "ComboBoxExt.h"
#include "ScanModelPaperDlg.h"
#include "NewModelDlg.h"
// CMakeModelDlg 对话框

class CNewMakeModelDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewMakeModelDlg)

public:
	CNewMakeModelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNewMakeModelDlg();

// 对话框数据
	enum { IDD = IDD_NEWMAKEMODELDLG };

public:
	CScanThread		m_scanThread;
	pTW_IDENTITY GetScanSrc(int nIndex);
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	void	InitExamData();


	//----------------------
	TwainApp*		_pTWAINApp;
	std::vector<CString> m_vecScanSrc;
	void	InitScanner();

	LRESULT	ScanDone(WPARAM wParam, LPARAM lParam);
	LRESULT	ScanErr(WPARAM wParam, LPARAM lParam);
	//----------------------

	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

	CBmpButton		m_bmpBtnScan;
	CBmpButton		m_bmpBtnUpload;
	CBmpButton		m_bmpBtnSave;
	CBmpButton		m_bmpBtnDown;
	CComboBoxExt	m_comboSubject;

	CRect			m_rtChild;
	CNewModelDlg*	m_pNewModelDlg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnScanpaper();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
};

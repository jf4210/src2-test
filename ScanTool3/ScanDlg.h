#pragma once
#include "global.h"
#include "CommonTWAIN.h"
#include "TwainApp.h"
#include "ComboBoxExt.h"
#include "BmpButton.h"

// CScanDlg 对话框

class CScanDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanDlg)

public:
	CScanDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanDlg();

// 对话框数据
	enum { IDD = IDD_SCANDLG };

	void	SetScanSrcInfo(std::vector<CString>& vec);
	void	UpdateInfo();
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);
	
private:
	int				m_nCurrentScanCount;	//当前扫描需要扫描试卷数量
	int				m_nModelPicNums;		//模板图片数，即一份模板有多少图片，对应多少试卷
	std::string		m_strCurrPicSavePath;		//gb2312
	bool			m_bAdvancedScan;		//是否显示高级扫描界面

	CComboBoxExt	m_comboScanner;
	CComboBoxExt	m_comboDuplex;

	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	CFont			m_fontScanBtn;			//扫描按钮字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

	CBitmap			m_bmpBk;
	CBmpButton		m_bmpBtnScan;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
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

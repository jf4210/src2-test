#pragma once
#include "global.h"
#include "CommonTWAIN.h"
#include "TwainApp.h"
#include "ComboBoxExt.h"

// CScanDlg 对话框

class CScanDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanDlg)

public:
	CScanDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanDlg();

// 对话框数据
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
	int				m_nCurrentScanCount;	//当前扫描需要扫描试卷数量
	int				m_nModelPicNums;		//模板图片数，即一份模板有多少图片，对应多少试卷
	std::string		m_strCurrPicSavePath;		//gb2312

	CComboBoxExt	m_comboScanner;
	CComboBoxExt	m_comboDuplex;

	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
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

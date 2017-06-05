#pragma once
#include "global.h"
#include "XListCtrl.h"
#include "ctrl/skinscrollwnd.h"
#include "BmpButton.h"

// CScanRecordDlg 对话框

class CScanBmkRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanBmkRecordDlg)

public:
	CScanBmkRecordDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanBmkRecordDlg();

// 对话框数据
	enum { IDD = IDD_SCANBMKRECORDDLG };

	void	UpDateInfo();
private:
	void	InitUI();
	void	InitCtrlPosition();
	void	SetFontSize(int nSize);

	int				m_nStatusSize;			//状态栏字体大小
	CFont			m_fontStatus;			//状态栏字体
	COLORREF		m_colorStatus;			//状态栏字体颜色

	CBmpButton		m_bmpBtnExport;
	CXListCtrl		m_lcBmk;
	HBITMAP			m_bitmap_scrollbar;
	
	CString			m_strGmkCount;		//报名库总人数
	CString			m_strScanCount;		//已扫人数
	CString			m_strUnScanCount;	//未扫人数
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMClickListBmk(NMHDR *pNMHDR, LRESULT *pResult);
};

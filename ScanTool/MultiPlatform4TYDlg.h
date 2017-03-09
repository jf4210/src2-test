#pragma once
#include "global.h"

// CMultiPlatform4TYDlg 对话框

class CMultiPlatform4TYDlg : public CDialog
{
	DECLARE_DYNAMIC(CMultiPlatform4TYDlg)

public:
	CMultiPlatform4TYDlg(VEC_PLATFORM_TY& vecPlatform, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMultiPlatform4TYDlg();

// 对话框数据
	enum { IDD = IDD_MULTIPLATFORM4TYDLG };

public:
	CListCtrl		m_lcPlatform;
	int				m_nStatusSize;			//字体大小
	CFont			m_fontStatus;			//字体
	COLORREF		m_colorStatus;			//字体颜色

	void			SetFontSize(int nSize);

	void	InitData();

	pST_PLATFORMINFO	_pPlatform;
	VEC_PLATFORM_TY&	_vecPlatform;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMDblclkListPlatform(NMHDR *pNMHDR, LRESULT *pResult);
};
